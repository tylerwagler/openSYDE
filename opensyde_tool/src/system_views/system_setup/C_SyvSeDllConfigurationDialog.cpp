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

#include <QDir>
#include <QFile>

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

   this->mpc_Ui->pc_LabelCustomDllPath->setVisible(true);
   this->mpc_Ui->pc_ComboBoxAdapterValue->setVisible(true);

#ifndef _WIN32
   // Linux: populate the combo with discovered SocketCAN interfaces. The combo stays editable so
   // users can also type a name the kernel will create shortly (e.g. before
   // `ip link add dev vcan0 type vcan && ip link set up vcan0`).
   this->m_PopulateSocketCanInterfaces();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::m_PopulateSocketCanInterfaces(void) const
{
   const QString c_Current = this->mpc_Ui->pc_ComboBoxAdapterValue->currentText();
   this->mpc_Ui->pc_ComboBoxAdapterValue->clear();
   const QStringList c_Interfaces = mh_DiscoverSocketCanInterfaces();
   for (int32_t s32_Idx = 0; s32_Idx < c_Interfaces.size(); ++s32_Idx)
   {
      this->mpc_Ui->pc_ComboBoxAdapterValue->addItem(c_Interfaces.at(s32_Idx));
   }

   if (c_Current.isEmpty() == false)
   {
      const int32_t s32_Existing = this->mpc_Ui->pc_ComboBoxAdapterValue->findText(c_Current);
      if (s32_Existing >= 0)
      {
         this->mpc_Ui->pc_ComboBoxAdapterValue->setCurrentIndex(s32_Existing);
      }
      else
      {
         this->mpc_Ui->pc_ComboBoxAdapterValue->setEditText(c_Current);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
QStringList C_SyvSeDllConfigurationDialog::mh_DiscoverSocketCanInterfaces(void)
{
   QStringList c_Result;

#ifndef _WIN32
   // ARPHRD_CAN == 280 in linux/if_arp.h. Every CAN-family interface (can*, vcan*, slcan*, custom-
   // named) exposes that value in /sys/class/net/<name>/type.
   const QDir c_NetDir("/sys/class/net");
   const QStringList c_Entries = c_NetDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
   for (int32_t s32_Idx = 0; s32_Idx < c_Entries.size(); ++s32_Idx)
   {
      const QString & rc_Name = c_Entries.at(s32_Idx);
      QFile c_TypeFile(QString("/sys/class/net/%1/type").arg(rc_Name));
      if (c_TypeFile.open(QIODevice::ReadOnly | QIODevice::Text) == true)
      {
         const QString c_Contents = QString::fromUtf8(c_TypeFile.readAll()).trimmed();
         if (c_Contents == QStringLiteral("280"))
         {
            c_Result.append(rc_Name);
         }
         c_TypeFile.close();
      }
   }
   c_Result.sort();
#endif

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
void C_SyvSeDllConfigurationDialog::SetAdapterConfig(const C_OscCanAdapterConfig & orc_Config)
{
   this->mc_AdapterConfig = orc_Config;

   const QString c_Display = orc_Config.c_ChannelId.empty() ?
                             QString::fromStdString(C_OscCanAdapterConfig::h_GetPlatformDefault().c_ChannelId) :
                             QString::fromStdString(orc_Config.c_ChannelId);
   const int32_t s32_Existing = this->mpc_Ui->pc_ComboBoxAdapterValue->findText(c_Display);
   if (s32_Existing >= 0)
   {
      this->mpc_Ui->pc_ComboBoxAdapterValue->setCurrentIndex(s32_Existing);
   }
   else
   {
      this->mpc_Ui->pc_ComboBoxAdapterValue->setEditText(c_Display);
   }
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
   const QString c_Field = this->mpc_Ui->pc_ComboBoxAdapterValue->currentText().trimmed();

   if (c_Field.isEmpty() == false)
   {
      c_Result.c_ChannelId = c_Field.toStdString();
   }
   if (this->mu64_Bitrate > 0U)
   {
      c_Result.u32_BitrateBps = static_cast<uint32_t>(this->mu64_Bitrate);
   }

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
      c_Config.u32_BitrateBps = static_cast<uint32_t>(this->mu64_Bitrate);
   }

   stw::scl::C_SclString c_Error;
   C_CanDispatcher * const pc_Dispatcher = C_OscCanAdapterFactory::h_CreateAdapter(c_Config, c_Error);

   C_OgeWiCustomMessage c_MessageBox(this->parentWidget());
   c_MessageBox.SetHeading(C_GtGetText::h_GetText("PC CAN Interface configuration"));
   c_MessageBox.SetCustomMinHeight(180, 180);

   if (pc_Dispatcher == nullptr)
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
