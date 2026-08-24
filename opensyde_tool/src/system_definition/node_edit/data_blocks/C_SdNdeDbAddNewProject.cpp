//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for new project import/creation (implementation)

   Widget for new project import/creation

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QDir>
#include <QFileDialog>

#include "C_Uti.hpp"
#include "stwtypes.hpp"
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_ImpUtil.hpp"
#include "C_PuiUtil.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "C_PuiProject.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_SdNdeDbAddNewProject.hpp"
#include "ui_C_SdNdeDbAddNewProject.h"
#include "C_PuiTargetSupportPackageFiler.hpp"
#include "C_SdNdeNodePropertiesTabContentWidget.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const QString C_SdNdeDbAddNewProject::mhc_START_TD = "<td style=\"padding: 0 9px 0 0;\">";
const QString C_SdNdeDbAddNewProject::mhc_CONTINUE_TD = "<td style=\"padding: 0 9px 0 9px;\">";
const QString C_SdNdeDbAddNewProject::mhc_SUFFIX = "syde_tsp";

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in]     ou32_NodeIndex             Node index
   \param[in]     oq_IsCurrentNodeNew        Is Node new or not
   \param[in,out] orc_Parent                 Reference to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDbAddNewProject::C_SdNdeDbAddNewProject(const uint32_t ou32_NodeIndex,
                                               stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                               const bool oq_IsCurrentNodeNew) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_SdNdeDbAddNewProject),
   ms32_TspReadResult(-1),
   mu32_NodeIndex(ou32_NodeIndex),
   mq_IsCurrentNodeNew(oq_IsCurrentNodeNew)
{
   this->mpc_Ui->setupUi(this);

   //lint -e{1938}  static const is guaranteed preinitialized before main
   this->mpc_Ui->pc_LineEditTSP->SetDragAndDropActiveForFile(mhc_SUFFIX);
   this->mpc_Ui->pc_LineEditCreateIn->SetDragAndDropActiveForFolder(true);

   InitStaticNames();
   m_Init(ou32_NodeIndex);
   this->mpc_Ui->pc_TextEditTSPDescription->setReadOnly(true);

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   // Remove "..." string
   this->mpc_Ui->pc_PushButtonCreateIn->setText("");
   this->mpc_Ui->pc_PushButtonTSP->setText("");

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_SdNdeDbAddNewProject::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this, &C_SdNdeDbAddNewProject::m_CancelClicked);
   connect(this->mpc_Ui->pc_PushButtonTSP, &QPushButton::clicked, this,
           &C_SdNdeDbAddNewProject::m_TspButtonClicked);
   connect(this->mpc_Ui->pc_PushButtonCreateIn, &QPushButton::clicked, this,
           &C_SdNdeDbAddNewProject::m_CreateInButtonClicked);
   connect(this->mpc_Ui->pc_LineEditTSP, &C_OgeLeFilePath::editingFinished,
           this, &C_SdNdeDbAddNewProject::m_OnLoadTsp);
   connect(this->mpc_Ui->pc_LineEditCreateIn, &C_OgeLeFilePath::SigPathDropped,
           this, &C_SdNdeDbAddNewProject::m_OnDroppedCreatinPath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDbAddNewProject::~C_SdNdeDbAddNewProject(void) noexcept
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::InitStaticNames(void) const
{
   this->mrc_ParentDialog.SetSubTitle("Import openSYDE Target Support Package");
   this->mpc_Ui->pc_LabelHeadingPreview->setText("Properties");
   this->mpc_Ui->pc_LabelTSP->setText("openSYDE Target Support Package");
   this->mpc_Ui->pc_LabelCreateIn->setText("Create In Directory");
   this->mpc_Ui->pc_PushButtonOk->setText("Import");
   this->mpc_Ui->pc_PushButtonCancel->setText("Cancel");

   //tooltips
   this->mpc_Ui->pc_LabelTSP->SetToolTipInformation(
      "openSYDE Target Support Package",
      "openSYDE Target Support Package provided by target deployment");

   this->mpc_Ui->pc_LabelCreateIn->SetToolTipInformation(
      "Create In Directory",
      "Location where the openSYDE Target Support Package content (e.g.: Target project, Flashware, ...) should be extracted at.");

   this->mpc_Ui->pc_PushButtonTSP->SetToolTipInformation(
      "Browse",
      "Browse for openSYDE Target Support Package.");

   this->mpc_Ui->pc_PushButtonCreateIn->SetToolTipInformation(
      "Browse",
      "Browse for location where openSYDE Target Support Package should be extracted at.");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current TSP path

   \return
   Current TSP path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SdNdeDbAddNewProject::GetTspPath(void) const
{
   return C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditTSP->GetPath());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Setter for full TSP path.

   For appropriate QLineEdit the minimized path is shown.
   Also loads the TSP if possible

   \param[in] orc_New New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::SetTspPath(const QString & orc_New)
{
   this->mpc_Ui->pc_LineEditTSP->SetPath(orc_New, C_PuiProject::h_GetInstance()->GetFolderPath());
   this->m_OnLoadTsp();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Apply v3 content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::ApplyV3Content()
{
   const C_OscNode * const pc_Core = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
   const C_PuiSdNode * const pc_Ui = C_PuiSdHandler::h_GetInstance()->GetUiNode(this->mu32_NodeIndex);

   tgl_assert((pc_Core != NULL) && (pc_Ui != NULL));
   if ((pc_Core != NULL) && (pc_Ui != NULL))
   {
      C_SdNdeDbAddNewProject::mh_KeepTspProperties(*pc_Core, this->mc_OscNode, *pc_Ui, this->mc_UiNode);
      m_ApplyV2PathAdaptationToV3();
      C_PuiSdHandler::h_GetInstance()->ReplaceNode(this->mu32_NodeIndex, this->mc_OscNode,
                                                   this->mc_UiNode);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_OkClicked(void)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   QDir c_CreateInFolder(
      C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditCreateIn->GetPath()));

   if (pc_Node != NULL)
   {
      bool q_Continue = false;
      bool q_ValidTsp = false;
      if (this->ms32_TspReadResult != C_NO_ERR)
      {
         QString c_Details;
         C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eERROR);
         c_Message.SetHeading("Import TSP");
         switch (this->ms32_TspReadResult)
         {
         case C_RANGE:
            c_Details = "Specified openSYDE Target Support Package does not exist.";
            break;
         case C_NOACT:
            c_Details =
               "Specified file is present but its structure is invalid (e.g. invalid XML file).";
            break;
         case C_CONFIG:
            c_Details = "In specified file is a XML node or attribute missing.";
            break;
         default:
            c_Details = "Unknown error occurred.";
            break;
         }
         c_Message.SetDescription(static_cast<QString>("Could not load specified openSYDE Target Support Package."));
         c_Message.SetDetails(c_Details);
         c_Message.SetCustomMinHeight(180, 250);
         c_Message.Execute();
      }
      else
      {
         // no error on parsing TSP (see m_OnLoadTSP())
         q_ValidTsp = true;
         const std::string c_DeviceName = this->mc_Tsp.c_DeviceName;
         if (c_DeviceName != pc_Node->c_DeviceType)
         {
            C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eERROR);
            c_Message.SetHeading("Import TSP");
            c_Message.SetDescription(
               static_cast<QString>("The openSYDE Target Support Package device \"%1\" does not match the "
                                       "device type \"%2\" of this node").
               arg(c_DeviceName.c_str()).
               arg(pc_Node->c_DeviceType.c_str()));
            c_Message.SetCustomMinHeight(230, 180);
            c_Message.Execute();
         }
         // No output dir
         else if (this->mpc_Ui->pc_LineEditCreateIn->GetPath().isEmpty() == true)
         {
            C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eERROR);
            c_Message.SetHeading("Import TSP");
            c_Message.SetDescription(static_cast<QString>("Define a directory for the project here."));
            c_Message.SetCustomMinHeight(180, 180);
            c_Message.Execute();
         }
         // TSP device matches current node and dialog is executed properly -> user may continue
         else
         {
            q_Continue = true;
         }
      }

      if ((q_Continue == true) && (q_ValidTsp == true))
      {
         bool q_IsWarningSet = true;

         C_OgeWiCustomMessage c_Warning(this, C_OgeWiCustomMessage::eWARNING);
         c_Warning.SetHeading("Import TSP");
         c_Warning.SetOkButtonText("Continue");
         c_Warning.SetCancelButtonText("Cancel");

         if (this->mq_IsCurrentNodeNew == false)
         {
            c_Warning.SetDescription("All existing node configuration will be deleted. Do you really want to continue?");
            q_IsWarningSet = true;
         }
         else
         {
            q_IsWarningSet = false;
         }

         if (q_IsWarningSet == true)
         {
            // only if warning is set the message box is executed. User cancels message box -> shall not continue
            if (c_Warning.Execute() != C_OgeWiCustomMessage::eOK)
            {
               q_Continue = false;
            }
         }
         // when no warning is set user may continue
      }

      if (q_Continue == true)
      {
         if (!this->mc_Tsp.c_TemplatePath.empty())
         {
            if ((c_CreateInFolder.exists() == true) &&
                (c_CreateInFolder.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0))
            {
               q_Continue = false;

               C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eQUESTION);
               c_Message.SetHeading("Import TSP");
               c_Message.SetCancelButtonText("Cancel");
               c_Message.SetNoButtonText("Continue without Clearing");
               c_Message.SetOkButtonText("Clear and Continue");
               c_Message.SetDescription(
                  static_cast<QString>("Directory \"%1\" is not empty. \n\nShould this directory be cleared? "
                                          "Attention: Your data will be lost!").arg(c_CreateInFolder.
                                                                                     absolutePath()));
               c_Message.SetCustomMinHeight(230, 230);
               c_Message.SetCustomMinWidth(800);

               switch (c_Message.Execute())
               {
               case C_OgeWiCustomMessage::eYES:
                  QApplication::setOverrideCursor(Qt::WaitCursor);
                  if (c_CreateInFolder.removeRecursively() == true)
                  {
                     q_Continue = true;
                     QApplication::restoreOverrideCursor();
                  }
                  else
                  {
                     C_OgeWiCustomMessage c_Message2(this, C_OgeWiCustomMessage::eERROR);
                     c_Message2.SetHeading("Import TSP");
                     c_Message2.SetDescription(
                        static_cast<QString>("Could not clear directory \"%1\".").
                        arg(C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditCreateIn->GetPath())));
                     QApplication::restoreOverrideCursor();
                     c_Message2.SetCustomMinHeight(180, 180);
                     c_Message2.Execute();
                  }
                  break;
               case C_OgeWiCustomMessage::eNO:
                  q_Continue = true;
                  break;
               case C_OgeWiCustomMessage::eCANCEL:
               default:
                  //Abort
                  break;
               }
            }
            // if folder does not exist yet, q_Continue remains true

            if (q_Continue == true)
            {
               const QFileInfo c_TspFileInfo(this->GetTspPath()); // file path -> use absoluteDir() to get directory of
                                                                  // file
               std::string c_ErrorText;
               QString c_Path;

               c_Path = QDir::cleanPath(c_TspFileInfo.absoluteDir().absoluteFilePath(
                                           this->mc_Tsp.c_TemplatePath.c_str()));
               QApplication::setOverrideCursor(Qt::WaitCursor);
               if (C_OscZipFile::h_UnpackZipFile(c_Path.toStdString().c_str(),
                                                 C_PuiUtil::h_GetAbsolutePathFromProject(
                                                    this->mpc_Ui->pc_LineEditCreateIn->GetPath()).toStdString().c_str(),
                                                 &c_ErrorText) == C_NO_ERR)
               {
                  this->mrc_ParentDialog.accept();
                  QApplication::restoreOverrideCursor();
               }
               else
               {
                  C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eERROR);
                  c_Message.SetHeading("Import TSP");
                  c_Message.SetDescription(static_cast<QString>("Could not extract openSYDE Target Support Package "
                                                                   "from file \"%1\" to directory \"%2\".").arg(
                                              c_Path).
                                           arg(C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditCreateIn
                                                                                       ->
                                                                                       GetPath())));
                  c_Message.SetCustomMinWidth(800);
                  c_Message.SetDetails(c_ErrorText.c_str());
                  c_Message.SetCustomMinHeight(230, 300);
                  QApplication::restoreOverrideCursor();
                  c_Message.Execute();
               }
            }
         }
         else
         {
            this->mrc_ParentDialog.accept();
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Cancel button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_CancelClicked(void)
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle TSP path button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_TspButtonClicked(void)
{
   QString c_FolderName; // for default folder
   QString c_FilePath = "";
   const QFileInfo c_File(C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditTSP->GetPath()));
   const QString c_FilterName = static_cast<QString>("openSYDE Target Support Package file") +
                                " (*." + mhc_SUFFIX + ")";

   if (c_File.exists() == true)
   {
      c_FolderName = c_File.path();
   }
   else
   {
      c_FolderName = C_PuiProject::h_GetInstance()->GetFolderPath();
   }

   c_FilePath =
      C_OgeWiUtil::h_GetOpenFileName(this, "Select openSYDE Target Support Package File",
                                     c_FolderName, c_FilterName, mhc_SUFFIX);
   if (c_FilePath != "")
   {
      this->SetTspPath(c_FilePath);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle create in path button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_CreateInButtonClicked(void)
{
   QString c_FolderName; // for default folder

   const QDir c_Folder(C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditCreateIn->GetPath()));

   if (c_Folder.exists() == true)
   {
      c_FolderName = c_Folder.path();
   }
   else
   {
      c_FolderName = C_PuiProject::h_GetInstance()->GetFolderPath();
   }

   const QString c_Path = QFileDialog::getExistingDirectory(
      this,
      "Select Directory for Target Project",
      c_FolderName, QFileDialog::ShowDirsOnly);

   if (c_Path != "")
   {
      this->m_SetCreateInPath(c_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle a dropped folder path in create in line edit
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_OnDroppedCreatinPath(void)
{
   this->m_SetCreateInPath(this->mpc_Ui->pc_LineEditCreateIn->GetPath());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Setter for full create in path.

   \param[in] orc_New New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_SetCreateInPath(const QString & orc_New)
{
   // check if relative path is possible and appreciated
   const QString c_Path = C_ImpUtil::h_AskUserToSaveRelativePath(this, orc_New,
                                                                 C_PuiProject::h_GetInstance()->GetFolderPath());

   if (c_Path != "")
   {
      this->mpc_Ui->pc_LineEditCreateIn->SetPath(c_Path, C_PuiProject::h_GetInstance()->GetFolderPath());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle loading TSP
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_OnLoadTsp(void)
{
   this->ms32_TspReadResult =
      C_PuiTargetSupportPackageFiler::h_LoadTspV3(C_PuiUtil::h_GetAbsolutePathFromProject(this->mpc_Ui->pc_LineEditTSP
                                                                                          ->GetPath()),
                                                  mc_Tsp, this->mc_OscNode, this->mc_UiNode);
   this->mpc_Ui->pc_LabelCreateIn->setText("Template Project Extract Directory");
   this->mpc_Ui->pc_LabelCreateIn->SetToolTipInformation(
      "Template Project Extract Directory",
      "Location where the openSYDE Target Support Package content (e.g.: Target project, Flashware, ...) should be extracted at.");
   if (this->ms32_TspReadResult == C_NO_ERR)
   {
      const bool q_HaveTemplate = !this->mc_Tsp.c_TemplatePath.empty();
      this->mpc_Ui->pc_LabelCreateIn->setDisabled(!q_HaveTemplate);
      this->mpc_Ui->pc_LineEditCreateIn->setDisabled(!q_HaveTemplate);
      this->mpc_Ui->pc_PushButtonCreateIn->setDisabled(!q_HaveTemplate);

      QString c_Text = "<html><body>";
      m_AddV3TopSection(c_Text);
      m_AddV3TemplateSection(c_Text);
      c_Text += "</body></html>";
      this->mpc_Ui->pc_TextEditTSPDescription->setHtml(c_Text);
   }
   else
   {
      // C_BUSY here used to dispatch to the V2 reader; V2 TSPs are no longer supported by the GUI
      // (use osy_tsp_convert to upgrade legacy TSPs to V3).
      this->mpc_Ui->pc_TextEditTSPDescription->setPlainText("<openSYDE Target Support Package description>");
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle top section of TSP V3 description

   \param[in,out] orc_Content Text to append to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_AddV3TopSection(QString & orc_Content) const
{
   orc_Content += "<h3>" + static_cast<QString>("openSYDE Target Support Package") + "</h3>";
   orc_Content += "<table>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "Format Version:";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += "3";
   orc_Content += "</td>";
   orc_Content += "</tr>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "Device Type: ";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += this->mc_Tsp.c_DeviceName.c_str();
   orc_Content += "</td>";
   orc_Content += "</tr>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "Description: ";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += mc_Tsp.c_Comment.c_str();
   orc_Content += "</td>";
   orc_Content += "</tr>";
   orc_Content += "</table>";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle template section of TSP V3 description

   \param[in,out] orc_Content Text to append to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_AddV3TemplateSection(QString & orc_Content) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   orc_Content += "<h3>" + static_cast<QString>("Node Definition Content") + "</h3>";
   orc_Content += "<table>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "Data Blocks:";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += QString::number(this->mc_OscNode.c_Applications.size());
   orc_Content += "</td>";
   orc_Content += "</tr>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "Datapools: ";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += QString::number(this->mc_OscNode.c_DataPools.size());
   orc_Content += "</td>";
   orc_Content += "</tr>";
   orc_Content += "<tr>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
   orc_Content += "COMM Messages: ";
   orc_Content += "</td>";
   orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
   orc_Content += QString::number(this->mc_OscNode.CountAllLocalMessages());
   orc_Content += "</td>";
   orc_Content += "</tr>";
   if (pc_Node->c_HalcConfig.IsClear() == false)
   {
      orc_Content += "<tr>";
      orc_Content += C_SdNdeDbAddNewProject::mhc_START_TD;
      orc_Content += "Hardware Configuration: ";
      orc_Content += "</td>";
      orc_Content += C_SdNdeDbAddNewProject::mhc_CONTINUE_TD;
      orc_Content += "./" + static_cast<QString>(this->mc_OscNode.c_HalcConfig.c_OriginalFileName.c_str());
      orc_Content += "</td>";
      orc_Content += "</tr>";
   }
   orc_Content += "</table>";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init path and title based on specified application

   \param[in] ou32_NodeIndex        Node index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_Init(const uint32_t ou32_NodeIndex) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);

   if (pc_Node != NULL)
   {
      //Use default
      const QString c_NodePath = C_OscUtils::h_NiceifyStringForFileName(pc_Node->c_Properties.c_Name).c_str();
      this->mpc_Ui->pc_LineEditCreateIn->SetPath(c_NodePath, C_PuiProject::h_GetInstance()->GetFolderPath());

      //Set Title
      const std::string c_NodeName = pc_Node->c_Properties.c_Name;
      const QString c_QnodeName = static_cast<QString>(c_NodeName.c_str());
      const QString c_Title = c_QnodeName;
      this->mrc_ParentDialog.SetTitle(c_Title);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Tsp keep properties

   \param[in]      orc_PreviousCoreNode   Previous core node
   \param[in,out]  orc_NewCoreNode        New core node
   \param[in]      orc_PreviousUiNode     Previous ui node
   \param[in,out]  orc_NewUiNode          New ui node
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::mh_KeepTspProperties(const C_OscNode & orc_PreviousCoreNode, C_OscNode & orc_NewCoreNode,
                                                  const C_PuiSdNode & orc_PreviousUiNode, C_PuiSdNode & orc_NewUiNode)
{
   orc_NewCoreNode.c_Properties.c_Name = orc_PreviousCoreNode.c_Properties.c_Name;
   orc_NewCoreNode.c_Properties.c_Comment = orc_PreviousCoreNode.c_Properties.c_Comment;
   orc_NewUiNode.c_UiPosition = orc_PreviousUiNode.c_UiPosition;
   orc_NewUiNode.c_UiBusConnections = orc_PreviousUiNode.c_UiBusConnections;
   tgl_assert(orc_NewCoreNode.c_Properties.c_ComInterfaces.size() ==
              orc_PreviousCoreNode.c_Properties.c_ComInterfaces.size());
   if (orc_NewCoreNode.c_Properties.c_ComInterfaces.size() == orc_PreviousCoreNode.c_Properties.c_ComInterfaces.size())
   {
      for (uint32_t u32_ItIntf = 0UL; u32_ItIntf < orc_NewCoreNode.c_Properties.c_ComInterfaces.size(); ++u32_ItIntf)
      {
         const C_OscNodeComInterfaceSettings & rc_PrevIntf =
            orc_PreviousCoreNode.c_Properties.c_ComInterfaces[u32_ItIntf];
         C_OscNodeComInterfaceSettings & rc_NewIntf = orc_NewCoreNode.c_Properties.c_ComInterfaces[u32_ItIntf];
         if (rc_PrevIntf.GetBusConnectedRawValue())
         {
            rc_NewIntf.AddConnection(rc_PrevIntf.u32_BusIndex);
         }
         else
         {
            rc_NewIntf.RemoveConnection();
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Apply v2 path adaptation to v3
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbAddNewProject::m_ApplyV2PathAdaptationToV3()
{
   for (uint32_t u32_ItDb = 0UL; u32_ItDb < this->mc_OscNode.c_Applications.size(); ++u32_ItDb)
   {
      C_OscNodeApplication & rc_App = this->mc_OscNode.c_Applications[u32_ItDb];
      const QString c_ProjectPath =
         C_Uti::h_ConcatPathIfNecessary(this->mpc_Ui->pc_LineEditCreateIn->GetPath(),
                                        rc_App.c_ProjectPath.c_str());
      rc_App.c_ProjectPath = c_ProjectPath.toStdString().c_str();
   }
}

