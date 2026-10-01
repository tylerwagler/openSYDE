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

#include <QFileDialog>
#include <QListWidget>

#include <cstdint>
#include "C_NagToolSettingsPopupDialog.hpp"
#include "ui_C_NagToolSettingsPopupDialog.h"
#include "C_OscUtils.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_PuiProject.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_PopUtil.hpp"
#include "C_NagUnUsedProjectFilesPopUpDialog.hpp"
#include "C_UsHandler.hpp"
#include "C_Uti.hpp"
#include "C_ImpUtil.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_OscCryptoAgentAccessUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
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
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_NagToolSettingsPopupDialog)
{
   this->mpc_Ui->setupUi(this);
   InitStaticNames();

   this->m_InitEnvironmentSection();
   this->m_InitDeviceRootsSection();
   this->m_InitCryptoAgentSection();

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_NagToolSettingsPopupDialog::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_CancelClicked);

   connect(this->mpc_Ui->pc_PushButtonAddDeviceRoot, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_AddDeviceRoot);
   connect(this->mpc_Ui->pc_PushButtonRemoveDeviceRoot, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_RemoveDeviceRoot);
   connect(this->mpc_Ui->pc_PushButtonMoveUpDeviceRoot, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_MoveDeviceRootUp);
   connect(this->mpc_Ui->pc_PushButtonMoveDownDeviceRoot, &QPushButton::clicked, this,
           &C_NagToolSettingsPopupDialog::m_MoveDeviceRootDown);
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
   this->mrc_ParentDialog.SetTitle("Tool");
   this->mrc_ParentDialog.SetSubTitle("Settings");
   this->mpc_Ui->pc_LabelGlobalSettings->setText("General");

   this->mpc_Ui->pc_LabelPathHandling->setText("Relative/absolute path handling");
   this->mpc_Ui->pc_LabelSkipTsp->setText("Skip TSP Import assistance");

   this->mpc_Ui->pc_LabelPathHandling->SetToolTipInformation("Path handling",
                                                             "Choose if file paths shall be handled as relative or absolute paths."
                                                                "\nIf the option \"Ask User\" is active, openSYDE will ask everytime a file action is performed.");

   this->mpc_Ui->pc_LabelSkipTsp->SetToolTipInformation("TSP Import assistance",
                                                        "Choose if openSYDE automatically redirects you to TSP Import when a new node is added to the Topology."
                                                           "\nIf the option \"Ask User\" is active, openSYDE will ask everytime a node is added.");

   this->mpc_Ui->pc_CbxPathHandling->addItem("Ask User");
   this->mpc_Ui->pc_CbxPathHandling->addItem("Relative");
   this->mpc_Ui->pc_CbxPathHandling->addItem("Absolute");

   this->mpc_Ui->pc_CbxSkipTsp->addItem("Ask User");
   this->mpc_Ui->pc_CbxSkipTsp->addItem("Skip");

   //Device Roots section
   this->mpc_Ui->pc_LabelDeviceRoots->setText("Device Roots");
   this->mpc_Ui->pc_LabelDeviceRootsNote->setText("Folders scanned for device-bundle manifests "
                                                     "(`device.syd`). On duplicate device names, the "
                                                     "first matching root in the list wins. Changes "
                                                     "take effect on the next launch.");
   this->mpc_Ui->pc_LabelDeviceRootsNote->setWordWrap(true);
   this->mpc_Ui->pc_PushButtonAddDeviceRoot->setText("Add...");
   this->mpc_Ui->pc_PushButtonRemoveDeviceRoot->setText("Remove");
   this->mpc_Ui->pc_PushButtonMoveUpDeviceRoot->setText("Move Up");
   this->mpc_Ui->pc_PushButtonMoveDownDeviceRoot->setText("Move Down");

   //Crypto Agent section
   this->mpc_Ui->pc_LabelClientAuthenticationSettings->setText("Client Authentication");
   this->mpc_Ui->pc_LabelCryptoAgentInfo->setText(
      "The \"Crypto Agent\" is an external tool that holds the authentication keys (PEM files) and answers "
      "the challenge when a device requests client authentication.");
   this->mpc_Ui->pc_LabelPathExecutable->setText("Crypto Agent executable");
   this->mpc_Ui->pc_LabelPathExecutable->SetToolTipInformation(
      "Crypto Agent executable", "Absolute path, or relative to the openSYDE executable.");
   this->mpc_Ui->pc_LabelPathConfigFile->setText("Crypto Agent config file");
   this->mpc_Ui->pc_LabelPathConfigFile->SetToolTipInformation(
      "Crypto Agent config file", "Absolute path, or relative to the Crypto Agent executable.");
   this->mpc_Ui->pc_LabelIpAddress->setText("Server IP address");
   this->mpc_Ui->pc_LabelIpAddress->SetToolTipInformation(
      "Server IP address", "Address the Crypto Agent TCP server listens on.\n\nDefault: 127.0.0.1 (localhost)");
   this->mpc_Ui->pc_LabelPort->setText("Port");
   this->mpc_Ui->pc_LabelPort->SetToolTipInformation(
      "Port", "Port the Crypto Agent TCP server listens on.\n\nDefault: 50963");
   this->mpc_Ui->pc_LabelAutostart->setText("Autostart");
   this->mpc_Ui->pc_LabelAutostart->SetToolTipInformation(
      "Autostart", "Start the Crypto Agent when openSYDE starts, unless it is already running.\n\nDefault: enabled");
   this->mpc_Ui->pc_LabelAutostop->setText("Autostop");
   this->mpc_Ui->pc_LabelAutostop->SetToolTipInformation(
      "Autostop", "Stop the Crypto Agent when openSYDE closes.\n\nDefault: enabled");
   this->mpc_Ui->pc_CheckBoxAutostart->setText("Enabled");
   this->mpc_Ui->pc_CheckBoxAutostop->setText("Enabled");
   this->mpc_Ui->pc_PushButtonPathExecutable->setText("");
   this->mpc_Ui->pc_PushButtonPathConfigFile->setText("");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OkClicked(void)
{
   //write settings to user settings

   C_UsHandler::h_GetInstance()->SetPathHandlingSelection(this->mpc_Ui->pc_CbxPathHandling->currentText());
   C_UsHandler::h_GetInstance()->SetSkipTspSelection(this->mpc_Ui->pc_CbxSkipTsp->currentText());

   QStringList c_DeviceRoots;
   QListWidget * const pc_List = this->mpc_Ui->pc_ListDeviceRoots;
   for (int32_t s32_It = 0; s32_It < pc_List->count(); ++s32_It)
   {
      c_DeviceRoots.append(pc_List->item(s32_It)->text());
   }
   C_UsHandler::h_GetInstance()->SetDeviceRootPaths(c_DeviceRoots);

   {
      C_OscCryptoAgentSettings c_CryptoAgent;
      const std::vector<int32_t> c_Ip = this->mpc_Ui->pc_WiIp->GetIpAddress();
      c_CryptoAgent.c_CryptoAgentExecutablePath = this->mpc_Ui->pc_LineEditPathExecutable->GetPath().toStdString();
      c_CryptoAgent.c_CryptoAgentConfigFilePath = this->mpc_Ui->pc_LineEditPathConfigFile->GetPath().toStdString();
      if (c_Ip.size() == 4UL)
      {
         for (uint32_t u32_It = 0UL; u32_It < 4UL; ++u32_It)
         {
            c_CryptoAgent.au8_CryptoAgentIp[u32_It] = static_cast<uint8_t>(c_Ip[u32_It]);
         }
      }
      c_CryptoAgent.u16_CryptoAgentPort = static_cast<uint16_t>(this->mpc_Ui->pc_SpinBoxPort->value());
      c_CryptoAgent.q_CryptoAgentAutoStart = this->mpc_Ui->pc_CheckBoxAutostart->isChecked();
      c_CryptoAgent.q_CryptoAgentAutoStop = this->mpc_Ui->pc_CheckBoxAutostop->isChecked();
      C_UsHandler::h_GetInstance()->SetCryptoAgentSettings(c_CryptoAgent);
      //takes effect for the next authentication; autostart only runs when openSYDE starts
      C_OscCryptoAgentAccessUtil::h_SetCryptoAgentSettings(
         C_UsHandler::h_GetInstance()->GetCryptoAgentSettingsForAccess());
   }

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
void C_NagToolSettingsPopupDialog::m_InitEnvironmentSection()
{
   this->mpc_Ui->pc_CbxPathHandling->setCurrentText(C_UsHandler::h_GetInstance()->GetPathHandlingSelection());
   this->mpc_Ui->pc_CbxSkipTsp->setCurrentText(C_UsHandler::h_GetInstance()->GetSkipTspSelection());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Populate the device-roots list widget from current user settings.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_InitDeviceRootsSection(void)
{
   this->mpc_Ui->pc_ListDeviceRoots->clear();
   this->mpc_Ui->pc_ListDeviceRoots->addItems(C_UsHandler::h_GetInstance()->GetDeviceRootPaths());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Prompt the user for a directory and append it to the device-roots list.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_AddDeviceRoot(void)
{
   const QString c_Path = QFileDialog::getExistingDirectory(
      this,
      "Select Device Root Folder",
      QString());

   if (c_Path.isEmpty() == false)
   {
      this->mpc_Ui->pc_ListDeviceRoots->addItem(c_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Remove the currently selected entry from the device-roots list.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_RemoveDeviceRoot(void)
{
   const int32_t s32_Row = this->mpc_Ui->pc_ListDeviceRoots->currentRow();
   if (s32_Row >= 0)
   {
      QListWidgetItem * const pc_Item = this->mpc_Ui->pc_ListDeviceRoots->takeItem(s32_Row);
      delete pc_Item;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Move the currently selected device-roots entry one position up (higher priority).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_MoveDeviceRootUp(void)
{
   QListWidget * const pc_List = this->mpc_Ui->pc_ListDeviceRoots;
   const int32_t s32_Row = pc_List->currentRow();

   if (s32_Row > 0)
   {
      QListWidgetItem * const pc_Item = pc_List->takeItem(s32_Row);
      pc_List->insertItem(s32_Row - 1, pc_Item);
      pc_List->setCurrentRow(s32_Row - 1);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Move the currently selected device-roots entry one position down (lower priority).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_MoveDeviceRootDown(void)
{
   QListWidget * const pc_List = this->mpc_Ui->pc_ListDeviceRoots;
   const int32_t s32_Row = pc_List->currentRow();

   if ((s32_Row >= 0) && (s32_Row < (pc_List->count() - 1)))
   {
      QListWidgetItem * const pc_Item = pc_List->takeItem(s32_Row);
      pc_List->insertItem(s32_Row + 1, pc_Item);
      pc_List->setCurrentRow(s32_Row + 1);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Populate the crypto agent section from current user settings.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_InitCryptoAgentSection(void)
{
   const C_OscCryptoAgentSettings c_CryptoAgent = C_UsHandler::h_GetInstance()->GetCryptoAgentSettings();

   this->mpc_Ui->pc_SpinBoxPort->SetMinimumCustom(1);
   this->mpc_Ui->pc_SpinBoxPort->SetMaximumCustom(65535);

   this->mpc_Ui->pc_LineEditPathExecutable->SetPath(QString::fromStdString(c_CryptoAgent.c_CryptoAgentExecutablePath),
                                                    C_Uti::h_GetExePath());
   this->mpc_Ui->pc_LineEditPathConfigFile->SetPath(QString::fromStdString(c_CryptoAgent.c_CryptoAgentConfigFilePath),
                                                    this->m_GetCryptoAgentFolder());
   this->mpc_Ui->pc_WiIp->SetIpAddress(&c_CryptoAgent.au8_CryptoAgentIp[0]);
   this->mpc_Ui->pc_SpinBoxPort->setValue(c_CryptoAgent.u16_CryptoAgentPort);
   this->mpc_Ui->pc_CheckBoxAutostart->setChecked(c_CryptoAgent.q_CryptoAgentAutoStart);
   this->mpc_Ui->pc_CheckBoxAutostop->setChecked(c_CryptoAgent.q_CryptoAgentAutoStop);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Let the user pick the crypto agent executable (stored relative to the openSYDE executable if wanted).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OnClickPathExecutable(void)
{
   const QString c_Suffix = C_Uti::h_GetExeSuffix();
   const QString c_Filter = c_Suffix.isEmpty() ? QString("Executable (*)") : ("Executable (*" + c_Suffix + ")");

   this->m_SelectFile(*this->mpc_Ui->pc_LineEditPathExecutable, "Select Crypto Agent executable", c_Filter,
                      c_Suffix, C_Uti::h_GetExePath());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Let the user pick the crypto agent config file (stored relative to the agent executable if wanted).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_OnClickPathConfigFile(void)
{
   this->m_SelectFile(*this->mpc_Ui->pc_LineEditPathConfigFile, "Select Crypto Agent config file",
                      "Config file (*.conf)", ".conf", this->m_GetCryptoAgentFolder());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Folder of the crypto agent executable as currently entered, resolved against the openSYDE executable

   \return
   Absolute folder of the crypto agent executable
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_NagToolSettingsPopupDialog::m_GetCryptoAgentFolder(void) const
{
   const QString c_Executable =
      C_Uti::h_ConcatPathIfNecessary(C_Uti::h_GetExePath(), this->mpc_Ui->pc_LineEditPathExecutable->GetPath());

   return QFileInfo(c_Executable).absolutePath();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Show a file dialog and store the chosen file, relative to the reference folder if the user wants

   \param[in,out]  orc_LineEdit          Line edit to show the chosen path in
   \param[in]      orc_Heading           File dialog heading
   \param[in]      orc_Filter            File dialog filter
   \param[in]      orc_DefaultSuffix     File dialog default suffix
   \param[in]      orc_ReferenceFolder   Absolute folder a relative path is relative to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_NagToolSettingsPopupDialog::m_SelectFile(C_OgeLeFilePath & orc_LineEdit, const QString & orc_Heading,
                                                const QString & orc_Filter, const QString & orc_DefaultSuffix,
                                                const QString & orc_ReferenceFolder)
{
   const QFileInfo c_Current(C_Uti::h_ConcatPathIfNecessary(orc_ReferenceFolder, orc_LineEdit.GetPath()));
   const QString c_StartFolder = c_Current.absoluteDir().exists() ? c_Current.absolutePath() : orc_ReferenceFolder;
   const QString c_NewPath = C_OgeWiUtil::h_GetOpenFileName(this, orc_Heading, c_StartFolder, orc_Filter,
                                                            orc_DefaultSuffix);

   if (c_NewPath.isEmpty() == false)
   {
      const QString c_Adapted = C_ImpUtil::h_AskUserToSaveRelativePath(this, c_NewPath, orc_ReferenceFolder);
      if (c_Adapted.isEmpty() == false)
      {
         orc_LineEdit.SetPath(c_Adapted, orc_ReferenceFolder);
      }
   }
}
