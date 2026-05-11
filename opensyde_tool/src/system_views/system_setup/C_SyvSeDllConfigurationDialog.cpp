//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for choosing and configuring the PC CAN adapter (implementation)

   Replaces the legacy STW-CAN-DLL configuration dialog. On Linux the dialog exposes a single
   SocketCAN interface name field (placeholder "can0"); on Windows it exposes a PEAK channel
   number. The legacy PEAK/Vector/Other radio buttons are hidden via setVisible() — the
   underlying .ui file is left unchanged for minimal-disruption migration; a future commit may
   replace it with a purpose-built layout.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_SyvSeDllConfigurationDialog.hpp"
#include "ui_C_SyvSeDllConfigurationDialog.h"

#include "C_GtGetText.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_OscCanAdapterFactory.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_CanDispatcher.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;
using namespace stw::can;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_SyvSeDllConfigurationDialog::C_SyvSeDllConfigurationDialog(C_OgePopUpDialog & orc_Parent) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_SyvSeDllConfigurationDialog),
   mc_AdapterConfig(C_OscCanAdapterConfig::h_GetPlatformDefault()),
   mu64_Bitrate(0U)
{
   mpc_Ui->setupUi(this);

   this->mrc_ParentDialog.SetWidget(this);

   this->InitText();
   this->m_ApplyPlatformLayout();

   this->mpc_Ui->pc_PushButtonBrowse->setText("");
   this->mpc_Ui->pc_PushButtonVariables->setText("");

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked,
           this, &C_SyvSeDllConfigurationDialog::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked,
           this, &C_SyvSeDllConfigurationDialog::m_CancelClicked);
   connect(this->mpc_Ui->pc_PushButtonTestConnection, &QPushButton::clicked,
           this, &C_SyvSeDllConfigurationDialog::m_TestConnectionClicked);
}

//----------------------------------------------------------------------------------------------------------------------
C_SyvSeDllConfigurationDialog::~C_SyvSeDllConfigurationDialog()
{
   delete mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::InitText(void) const
{
   this->mrc_ParentDialog.SetTitle(C_GtGetText::h_GetText("PC CAN Interface"));
   this->mrc_ParentDialog.SetSubTitle(C_GtGetText::h_GetText("Configuration"));

   this->mpc_Ui->pc_PushButtonOk->setText(C_GtGetText::h_GetText("OK"));
   this->mpc_Ui->pc_PushButtonCancel->setText(C_GtGetText::h_GetText("Cancel"));
   this->mpc_Ui->pc_PushButtonTestConnection->setText(C_GtGetText::h_GetText("Test Connection"));
   this->mpc_Ui->pc_LabelBitrateInfo->setText(C_GtGetText::h_GetText(
                                                 "CAN bitrate will be applied automatically."));

#ifdef _WIN32
   this->mpc_Ui->pc_LabelBusHeading->setText(C_GtGetText::h_GetText("PEAK USB Channel"));
   this->mpc_Ui->pc_LabelCustomDllPath->setText(C_GtGetText::h_GetText("Channel (1-16)"));
#else
   this->mpc_Ui->pc_LabelBusHeading->setText(C_GtGetText::h_GetText("SocketCAN Interface"));
   this->mpc_Ui->pc_LabelCustomDllPath->setText(C_GtGetText::h_GetText("Interface name"));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::m_ApplyPlatformLayout(void) const
{
   // Hide the legacy PEAK/Vector/Other radio buttons. Only adapter type per platform is supported now;
   // the .ui file still has the widgets but they're inert.
   this->mpc_Ui->pc_RadioButtonPeak->setVisible(false);
   this->mpc_Ui->pc_RadioButtonVector->setVisible(false);
   this->mpc_Ui->pc_RadioButtonOther->setVisible(false);

   // Hide DLL-specific buttons that no longer apply.
   this->mpc_Ui->pc_PushButtonConfigureDll->setVisible(false);
   this->mpc_Ui->pc_PushButtonBrowse->setVisible(false);
   this->mpc_Ui->pc_PushButtonVariables->setVisible(false);

   // The single path/interface field stays visible — it's used as the value input on both platforms.
   this->mpc_Ui->pc_LabelCustomDllPath->setVisible(true);
   this->mpc_Ui->pc_LineEditCustomDllPath->setVisible(true);
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::SetAdapterConfig(const C_OscCanAdapterConfig & orc_Config)
{
   this->mc_AdapterConfig = orc_Config;

#ifdef _WIN32
   this->mpc_Ui->pc_LineEditCustomDllPath->setText(QString::number(orc_Config.u16_PeakChannel));
#else
   const QString c_Display = orc_Config.c_SocketCanInterface.IsEmpty() ?
                             QString("can0") :
                             QString(orc_Config.c_SocketCanInterface.c_str());
   this->mpc_Ui->pc_LineEditCustomDllPath->setText(c_Display);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::SetBitrate(const uint64_t ou64_Bitrate)
{
   this->mu64_Bitrate = ou64_Bitrate;
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig C_SyvSeDllConfigurationDialog::GetAdapterConfig(void) const
{
   C_OscCanAdapterConfig c_Result = this->mc_AdapterConfig;
   const QString c_Field = this->mpc_Ui->pc_LineEditCustomDllPath->text().trimmed();

#ifdef _WIN32
   c_Result.e_Type = eCAN_ADAPTER_PEAK;
   bool q_Ok = false;
   const uint32_t u32_Channel = c_Field.toUInt(&q_Ok);
   c_Result.u16_PeakChannel = (q_Ok && (u32_Channel >= 1U) && (u32_Channel <= 16U)) ?
                              static_cast<uint16_t>(u32_Channel) : static_cast<uint16_t>(1U);
   c_Result.u32_PeakBitrateKbits = (this->mu64_Bitrate > 0U) ?
                                   static_cast<uint32_t>(this->mu64_Bitrate / 1000U) :
                                   c_Result.u32_PeakBitrateKbits;
#else
   c_Result.e_Type = eCAN_ADAPTER_SOCKET_CAN;
   c_Result.c_SocketCanInterface = c_Field.isEmpty() ?
                                   stw::scl::C_SclString("can0") :
                                   stw::scl::C_SclString(c_Field.toStdString().c_str());
#endif

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::m_OkClicked(void) const
{
   this->mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::m_CancelClicked(void) const
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::m_TestConnectionClicked(void) const
{
   C_OscCanAdapterConfig c_Config = this->GetAdapterConfig();

   if (this->mu64_Bitrate > 0U)
   {
      c_Config.u32_PeakBitrateKbits = static_cast<uint32_t>(this->mu64_Bitrate / 1000U);
   }

   stw::scl::C_SclString c_Error;
   C_CanDispatcher * const pc_Dispatcher = C_OscCanAdapterFactory::h_CreateAdapter(c_Config, c_Error);

   C_OgeWiCustomMessage c_MessageBox(this->parentWidget());
   c_MessageBox.SetHeading(C_GtGetText::h_GetText("PC CAN Interface configuration"));
   c_MessageBox.SetCustomMinHeight(180, 180);

   if (pc_Dispatcher == NULL)
   {
      c_MessageBox.SetType(C_OgeWiCustomMessage::E_Type::eERROR);
      c_MessageBox.SetDescription(QString(c_Error.c_str()));
   }
   else
   {
      const int32_t s32_Init =
         pc_Dispatcher->CAN_Init(static_cast<int32_t>(this->mu64_Bitrate > 0U ? this->mu64_Bitrate / 1000U : 0U));
      if (s32_Init == C_NO_ERR)
      {
         c_MessageBox.SetType(C_OgeWiCustomMessage::E_Type::eINFORMATION);
         c_MessageBox.SetDescription(C_GtGetText::h_GetText("Connection test successful. CAN Interface is ready for use."));
         (void)pc_Dispatcher->CAN_Exit();
      }
      else
      {
         c_MessageBox.SetType(C_OgeWiCustomMessage::E_Type::eWARNING);
         c_MessageBox.SetDescription(C_GtGetText::h_GetText(
                                        "CAN initialization failed. Verify adapter is connected and the interface is up."));
      }
      delete pc_Dispatcher;
   }

   c_MessageBox.Execute();
}
