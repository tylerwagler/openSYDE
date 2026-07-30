//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Popup dialog for global tool settings

   Gives the user the opportunity to customize tool behaviour

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "stwtypes.hpp"
#include "C_ImpUtil.hpp"
#include "C_PuiUtil.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_GtGetText.hpp"
#include "C_NagToolSettingsPopupDialog.hpp"
#include "ui_C_NagToolSettingsPopupDialog.h"
#include "C_OscUtils.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_PuiProject.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_PopUtil.hpp"
#include "C_NagUnUsedProjectFilesPopUpDialog.hpp"
#include "C_UsHandler.hpp"
#include "C_OscCryptoAgentAccessUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] orc_Parent Reference to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_NagToolSettingsPopupDialog::C_NagToolSettingsPopupDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent) :
   QWidget(&orc_Parent),
   mpc_Ui(new Ui::C_NagToolSettingsPopupDialog),
   mrc_ParentDialog(orc_Parent)
{
   this->mpc_Ui->setupUi(this);
   InitStaticNames();

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   // Remove "..." string
   this->mpc_Ui->pc_PushButtonPathExecutable->setText("");
   this->mpc_Ui->pc_PushButtonPathConfigFile->setText("");

   this->mpc_Ui->pc_SpinBoxPort->SetMinimumCustom(0);
   this->mpc_Ui->pc_SpinBoxPort->SetMaximumCustom(65535);

   this->m_LoadEnvironmentSection();

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_NagToolSettingsPopupDialog::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_CancelClicked);
   connect(this->mpc_Ui->pc_PushButtonPathExecutable, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_OnClickPathExecutable);
   connect(this->mpc_Ui->pc_PushButtonPathConfigFile, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_OnClickPathConfigFile);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_NagToolSettingsPopupDialog::~C_NagToolSettingsPopupDialog(void)
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::InitStaticNames(void) const
{
   //Global settings section
   this->mrc_ParentDialog.SetTitle(C_GtGetText::h_GetText("Tool"));
   this->mrc_ParentDialog.SetSubTitle(C_GtGetText::h_GetText("Settings"));
   this->mpc_Ui->pc_LabelGlobalSettings->setText(C_GtGetText::h_GetText("General"));

   this->mpc_Ui->pc_LabelPathHandling->setText(C_GtGetText::h_GetText("Relative/absolute path handling"));
   this->mpc_Ui->pc_LabelSkipTsp->setText(C_GtGetText::h_GetText("Skip TSP Import assistance"));

   this->mpc_Ui->pc_LabelPathHandling->SetToolTipInformation(C_GtGetText::h_GetText("Path handling"),
                                                             C_GtGetText::h_GetText(
                                                                "Choose if file paths shall be handled as relative or absolute paths."
                                                                "\nIf the option \"Ask User\" is active, openSYDE will ask everytime a file action is performed."));

   this->mpc_Ui->pc_LabelSkipTsp->SetToolTipInformation(C_GtGetText::h_GetText("TSP Import assistance"),
                                                        C_GtGetText::h_GetText(
                                                           "Choose if openSYDE automatically redirects you to TSP Import when a new node is added to the Topology."
                                                           "\nIf the option \"Ask User\" is active, openSYDE will ask everytime a node is added."));

   this->mpc_Ui->pc_CbxPathHandling->addItem(C_GtGetText::h_GetText("Ask User"));
   this->mpc_Ui->pc_CbxPathHandling->addItem(C_GtGetText::h_GetText("Relative"));
   this->mpc_Ui->pc_CbxPathHandling->addItem(C_GtGetText::h_GetText("Absolute"));

   this->mpc_Ui->pc_CbxSkipTsp->addItem(C_GtGetText::h_GetText("Ask User"));
   this->mpc_Ui->pc_CbxSkipTsp->addItem(C_GtGetText::h_GetText("Skip"));

   //Crypto Agent
   this->mpc_Ui->pc_LabelClientAuthenticationSettings->setText(C_GtGetText::h_GetText("Client Authentication Settings"));
   this->mpc_Ui->pc_LabelInfo->setText(
      C_GtGetText::h_GetText(
         "Note: The \"Crypto Agent\" is an external tool used to solve crypto-related challenges. "
         "In openSYDE context it is used to validates authentication PEM files"
         " when a device requests client authentication."));
   this->mpc_Ui->pc_LabelPathExecutable->setText(C_GtGetText::h_GetText("Path to \"Crypto Agent\" executable:"));
   this->mpc_Ui->pc_LabelPathExecutable->SetToolTipInformation(C_GtGetText::h_GetText(
                                                                  "Path to \"Crypto Agent\" executable"),
                                                               C_GtGetText::h_GetText(
                                                                  "Path to \"Crypto Agent\" executable."
                                                                  " Absolute or relative to openSYDE.exe."));
   this->mpc_Ui->pc_LabelPathConfigFile->setText(C_GtGetText::h_GetText("Path to \"Crypto Agent\" config file:"));
   this->mpc_Ui->pc_LabelPathConfigFile->SetToolTipInformation(C_GtGetText::h_GetText(
                                                                  "Path to \"Crypto Agent\" config file"),
                                                               C_GtGetText::h_GetText(
                                                                  "Path to \"Crypto Agent\" config file."
                                                                  " Absolute or relative to path to \"Crypto Agent\" executable."));
   this->mpc_Ui->pc_LabelIpAddress->setText(C_GtGetText::h_GetText("Server IP Address:"));
   this->mpc_Ui->pc_LabelIpAddress->SetToolTipInformation(C_GtGetText::h_GetText("Server IP Address"),
                                                          C_GtGetText::h_GetText(
                                                             "Bind address for the \"Crypto Agent\" TCP Server. \n\nDefault: 127.0.0.1 = localhost"));
   this->mpc_Ui->pc_LabelPort->setText(C_GtGetText::h_GetText("Port:"));
   this->mpc_Ui->pc_LabelPort->SetToolTipInformation(C_GtGetText::h_GetText("Port"),
                                                     C_GtGetText::h_GetText(
                                                        "Port to be used by the \"Crypto Agent\" TCP Server. \n\nDefault: 50963"));
   this->mpc_Ui->pc_LabelAutostart->setText(C_GtGetText::h_GetText("Autostart:"));
   this->mpc_Ui->pc_LabelAutostart->SetToolTipInformation(C_GtGetText::h_GetText("Autostart"),
                                                          C_GtGetText::h_GetText(
                                                             "If Enabled: Automatically start \"Crypto Agent\" tool "
                                                             "on openSYDE tool start.\n\nDefault: Enabled"));
   this->mpc_Ui->pc_LabelAutostop->setText(C_GtGetText::h_GetText("Autostop:"));
   this->mpc_Ui->pc_LabelAutostop->SetToolTipInformation(C_GtGetText::h_GetText("Autostop"),
                                                         C_GtGetText::h_GetText(
                                                            "If Enabled: Automatically stop \"Crypto Agent\" tool on "
                                                            "openSYDE tool shutdown. \n\nDefault: Enabled"));
   this->mpc_Ui->pc_CheckBoxAutostart->setText(C_GtGetText::h_GetText("Enabled"));
   this->mpc_Ui->pc_CheckBoxAutostop->setText(C_GtGetText::h_GetText("Enabled"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out] opc_KeyEvent Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::keyPressEvent(QKeyEvent * const opc_KeyEvent)
{
   bool q_CallOrg = true;

   //Handle all enter key cases manually
   if ((opc_KeyEvent->key() == static_cast<int32_t>(Qt::Key_Enter)) ||
       (opc_KeyEvent->key() == static_cast<int32_t>(Qt::Key_Return)))
   {
      if (((opc_KeyEvent->modifiers().testFlag(Qt::ControlModifier) == true) &&
           (opc_KeyEvent->modifiers().testFlag(Qt::AltModifier) == false)) &&
          (opc_KeyEvent->modifiers().testFlag(Qt::ShiftModifier) == false))
      {
         this->mrc_ParentDialog.accept();
      }
      else
      {
         q_CallOrg = false;
      }
   }
   if (q_CallOrg == true)
   {
      QWidget::keyPressEvent(opc_KeyEvent);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OkClicked(void)
{
   //write settings to user settings
   C_UsToolSettings c_ToolSettings;
   C_OscCryptoAgentSettings c_NewCryptoAgentSettings;
   const std::vector<int32_t> c_Ip = this->mpc_Ui->pc_WiIp->GetIpAddress();

   c_ToolSettings.SetPathHandlingSelection(this->mpc_Ui->pc_CbxPathHandling->currentText());
   c_ToolSettings.SetSkipTspSelection(this->mpc_Ui->pc_CbxSkipTsp->currentText());
   c_NewCryptoAgentSettings.c_CryptoAgentExecutablePath =
      this->mpc_Ui->pc_LineEditPathExecutable->GetPath().toStdString().c_str();
   c_NewCryptoAgentSettings.c_CryptoAgentConfigFilePath =
      this->mpc_Ui->pc_LineEditPathConfigFile->GetPath().toStdString().c_str();
   tgl_assert(c_Ip.size() == 4UL);
   if (c_Ip.size() == 4UL)
   {
      for (uint32_t u32_It = 0UL; u32_It < 4UL; ++u32_It)
      {
         c_NewCryptoAgentSettings.au8_CryptoAgentIp[u32_It] = static_cast<uint8_t>(c_Ip[u32_It]);
      }
   }
   c_NewCryptoAgentSettings.u16_CryptoAgentPort = static_cast<uint16_t>(this->mpc_Ui->pc_SpinBoxPort->value());
   c_NewCryptoAgentSettings.q_CryptoAgentAutoStart = this->mpc_Ui->pc_CheckBoxAutostart->isChecked();
   c_NewCryptoAgentSettings.q_CryptoAgentAutoStop = this->mpc_Ui->pc_CheckBoxAutostop->isChecked();
   c_ToolSettings.SetCryptoAgentSettings(c_NewCryptoAgentSettings);

   C_OscCryptoAgentAccessUtil::h_SetCryptoAgentSettings(c_NewCryptoAgentSettings);
   C_UsHandler::h_GetInstance()->SetToolSettings(c_ToolSettings);

   this->mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of cancel button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_CancelClicked()
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Reads the corresponding user settings and initializes UI elements accordingly
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_LoadEnvironmentSection()
{
   const C_UsToolSettings c_ToolSettings = C_UsHandler::h_GetInstance()->GetToolSettings();
   const C_OscCryptoAgentSettings & rc_CryptoAgentSettings =
      c_ToolSettings.GetCryptoAgentSettings();

   this->mpc_Ui->pc_CbxPathHandling->setCurrentText(c_ToolSettings.GetPathHandlingSelection());
   this->mpc_Ui->pc_CbxSkipTsp->setCurrentText(c_ToolSettings.GetSkipTspSelection());

   this->mpc_Ui->pc_LineEditPathExecutable->SetPath(rc_CryptoAgentSettings.c_CryptoAgentExecutablePath.c_str());
   this->mpc_Ui->pc_LineEditPathConfigFile->SetPath(rc_CryptoAgentSettings.c_CryptoAgentConfigFilePath.c_str());
   this->mpc_Ui->pc_WiIp->SetIpAddress(rc_CryptoAgentSettings.au8_CryptoAgentIp);
   this->mpc_Ui->pc_SpinBoxPort->setValue(rc_CryptoAgentSettings.u16_CryptoAgentPort);
   this->mpc_Ui->pc_CheckBoxAutostart->setChecked(rc_CryptoAgentSettings.q_CryptoAgentAutoStart);
   this->mpc_Ui->pc_CheckBoxAutostop->setChecked(rc_CryptoAgentSettings.q_CryptoAgentAutoStop);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  On click path executable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OnClickPathExecutable()
{
   const QString c_Heading = C_GtGetText::h_GetText("Select path to \"Crypto Agent\" executable file");
   const QString c_FilterName = C_GtGetText::h_GetText("Executable file (*.exe)");
   const QString c_Extension = "*.exe";

   m_HandleFileSelect(*this->mpc_Ui->pc_LineEditPathExecutable, c_Heading, c_FilterName, c_Extension,
                      C_Uti::h_GetExePath());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  On click path config file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OnClickPathConfigFile()
{
   const QString c_Heading = C_GtGetText::h_GetText("Select path to \"Crypto Agent\" config file");
   const QString c_FilterName = C_GtGetText::h_GetText("Config file (*.conf)");
   const QString c_Extension = "*.conf";
   QString c_Folder =
      TglExtractFilePath(this->mpc_Ui->pc_LineEditPathExecutable->GetPath().toStdString().c_str()).c_str();

   c_Folder.removeLast();
   m_HandleFileSelect(*this->mpc_Ui->pc_LineEditPathConfigFile, c_Heading, c_FilterName, c_Extension, c_Folder);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle file select

   \param[in,out]  orc_Widget             Widget
   \param[in]      orc_Heading            QFileDialog heading
   \param[in]      orc_Filter             QFileDialog filter
   \param[in]      orc_Extension          QFileDialog default suffix
   \param[in]      orc_ReferenceFolder    Reference folder
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_HandleFileSelect(opensyde_gui_elements::C_OgeLeFilePath & orc_Widget,
                                                      const QString & orc_Heading, const QString & orc_Filter,
                                                      const QString & orc_Extension,
                                                      const QString & orc_ReferenceFolder)
{
   QString c_Folder; // for default folder
   const stw::scl::C_SclString c_CurrentFile =
      C_Uti::h_ConcatPathIfNecessary(orc_ReferenceFolder, orc_Widget.GetPath()).toStdString().c_str();

   if (TglDirectoryExists(TglExtractFilePath(c_CurrentFile)) == true)
   {
      c_Folder = TglExtractFilePath(c_CurrentFile).c_str();
   }
   else
   {
      c_Folder = orc_ReferenceFolder;
   }
   {
      const QString c_NewFilePath = C_OgeWiUtil::h_GetOpenFileName(
         this, orc_Heading, c_Folder, orc_Filter, orc_Extension,
         QFileDialog::DontConfirmOverwrite); // overwrite is handled later

      if (c_NewFilePath != "")
      {
         // check if relative path is possible and appreciated
         const QString c_AdaptedPath =
            C_ImpUtil::h_AskUserToSaveRelativePath(this, c_NewFilePath, orc_ReferenceFolder);

         if (c_AdaptedPath != "")
         {
            orc_Widget.SetPath(c_AdaptedPath.toStdString().c_str());
         }
      }
   }
}
