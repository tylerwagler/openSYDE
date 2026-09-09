//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for save-as dialog (implementation)

   Widget for save-as dialog. Most functionality copied/moved from
   C_PopSaveAsWidget.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <limits>
#include <QFileDialog>

#include <cstdint>
#include "C_OscUtils.hpp"
#include "C_PuiProject.hpp"
#include "C_PopErrorHandling.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_Uti.hpp"
#include "C_HeHandler.hpp"
#include "stwerrors.hpp"

#include "C_PopSaveAsDialogWidget.hpp"
#include "ui_C_PopSaveAsDialogWidget.h"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::errors;

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
C_PopSaveAsDialogWidget::C_PopSaveAsDialogWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_PopSaveAsDialogWidget)
{
   this->mpc_Ui->setupUi(this);

   // init names
   InitStaticNames();

   this->mpc_Ui->pc_LineEditPath->SetDragAndDropActiveForFolder(true);

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   // make Enter activate Save (the accept button for this popup)
   this->mpc_Ui->pc_PushButtonSave->setDefault(true);

   //Default input
   m_InitDefaultProjectName();

   // no strange cursor
   QApplication::restoreOverrideCursor();

   // Remove "..." string
   this->mpc_Ui->pc_PushButtonBrowse->setText("");

   connect(this->mpc_Ui->pc_PushButtonBrowse, &QPushButton::clicked, this, &C_PopSaveAsDialogWidget::m_OnBrowse);
   connect(this->mpc_Ui->pc_PushButtonSave, &QPushButton::clicked, this, &C_PopSaveAsDialogWidget::m_OnSave);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this, &C_PopSaveAsDialogWidget::m_OnCancel);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PopSaveAsDialogWidget::~C_PopSaveAsDialogWidget(void)
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::InitStaticNames(void) const
{
   this->mrc_ParentDialog.SetTitle("Project");
   this->mrc_ParentDialog.SetSubTitle("Save As");
   this->mpc_Ui->pc_LabelHeadingPreview->setText("Location");
   this->mpc_Ui->pc_LabelName->setText("Project Name");
   this->mpc_Ui->pc_LabelPath->setText("Path");
   this->mpc_Ui->pc_LabelVersion->setText("File Format");
   // V2 was dropped together with the V2 system-definition filer; V3 is now the only output format.
   this->mpc_Ui->pc_ComboBoxVersion->addItem("V3");
   this->mpc_Ui->pc_LabelVersion->setVisible(false);
   this->mpc_Ui->pc_ComboBoxVersion->setVisible(false);
   this->mpc_Ui->pc_LineEditName->setPlaceholderText("");
   this->mpc_Ui->pc_LineEditPath->setPlaceholderText("");
   this->mpc_Ui->pc_PushButtonSave->setText("Save");
   this->mpc_Ui->pc_PushButtonCancel->setText("Cancel");
   //Tool tips
   this->mpc_Ui->pc_LabelName->SetToolTipInformation("Project Name",
                                                     "Project name to choose. This will also be used as the default folder name your project is saved in.");
   this->mpc_Ui->pc_LabelPath->SetToolTipInformation("Path",
                                                     "Path to create the new folder for the new project in");
   this->mpc_Ui->pc_LabelVersion->SetToolTipInformation("File Format",
                                                        "Projects are saved in the V3 file format.");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle save user settings operation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::SaveUserSettings() const
{
   C_UsHandler::h_GetInstance()->SetCurrentSaveAsPath(this->mpc_Ui->pc_LineEditPath->GetPath());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle default project name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::m_InitDefaultProjectName(void) const
{
   const QString c_ProjectPath = C_PuiProject::h_GetInstance()->GetPath();
   QString c_Proposal;
   QString c_AbsolutePath = static_cast<QFileInfo>(m_GetValidPath(c_ProjectPath)).absoluteDir().absolutePath();

   c_AbsolutePath.remove(c_AbsolutePath.lastIndexOf("/"), c_AbsolutePath.length());

   this->mpc_Ui->pc_LineEditPath->SetPath(c_AbsolutePath);

   if (c_ProjectPath.compare("") == 0)
   {
      c_Proposal = "NewProject";
   }
   else
   {
      c_Proposal = static_cast<QString>("Copy_of_%1").arg(C_PuiProject::h_GetInstance()->GetName());
   }
   this->mpc_Ui->pc_LineEditName->setText(c_Proposal);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project to file

   \param[in] orc_File   File

   \return
   see C_PuiProject::h_GetInstance()->Save(...) for return values
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_PopSaveAsDialogWidget::m_SaveToFile(const QString & orc_File) const
{
   int32_t s32_Return;

   // remember old project path
   const QString c_RememberPath = C_PuiProject::h_GetInstance()->GetPath();

   // try to save and return errors else
   C_PuiProject::h_GetInstance()->SetPath(orc_File);
   s32_Return = C_PuiProject::h_GetInstance()->Save(true);
   C_PopErrorHandling::h_ProjectSaveErr(s32_Return, this->parentWidget());

   if (s32_Return == C_NO_ERR)
   {
      // use real path for active project and recent projects
      const QString c_Path = C_PuiProject::h_GetInstance()->GetPath();
      C_UsHandler::h_GetInstance()->AddToRecentProjects(c_Path);
      C_UsHandler::h_GetInstance()->ChangeActiveProjectName(c_Path);
   }
   else
   {
      // reset to previous project path
      C_PuiProject::h_GetInstance()->SetPath(c_RememberPath);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Replace empty path if necessary

   \param[in] orc_Path Some path

   \return
   Non empty path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PopSaveAsDialogWidget::m_GetValidPath(const QString & orc_Path) const
{
   QString c_Retval = orc_Path;

   if (c_Retval.compare("") == 0)
   {
      c_Retval = C_Uti::h_GetExePath();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle browse event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::m_OnBrowse(void)
{
   const QString c_Folder = m_GetValidPath(this->mpc_Ui->pc_LineEditPath->GetPath());

   const QString c_Path =
      QFileDialog::getExistingDirectory(this, "Select Project Location", c_Folder,
                                        QFileDialog::ShowDirsOnly);

   if (c_Path.compare("") != 0)
   {
      this->mpc_Ui->pc_LineEditPath->SetPath(c_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::m_OnSave(void)
{
   QApplication::setOverrideCursor(Qt::WaitCursor);

   const QString c_BasePath = this->mpc_Ui->pc_LineEditPath->GetPath();
   const QString c_Name = this->mpc_Ui->pc_LineEditName->text();

   const bool q_ValidName = C_OscUtils::h_CheckValidFileName(c_Name.toStdString().c_str());
   const bool q_ValidPath = C_OscUtils::h_CheckValidFilePath(c_BasePath.toStdString().c_str());

   if ((q_ValidName == true) && (q_ValidPath == true))
   {
      const QString c_Path = c_BasePath + '/' + c_Name;
      const QString c_FilePathAndName = c_Path + '/' + c_Name + ".syde";
      const QDir c_Dir(c_Path);

      if (c_Dir.exists() == false)
      {
         c_Dir.mkdir(c_Path);
         if (m_SaveToFile(c_FilePathAndName) == C_NO_ERR)
         {
            // accept dialog if successfully saved
            this->mrc_ParentDialog.accept();
         }
         QApplication::restoreOverrideCursor();
      }
      else
      {
         C_OgeWiCustomMessage c_Box(this, C_OgeWiCustomMessage::E_Type::eERROR);
         QApplication::restoreOverrideCursor();
         c_Box.SetHeading("Project save");
         c_Box.SetDescription("A project with this name already exists. Choose another name.");
         c_Box.SetCustomMinHeight(180, 180);
         c_Box.Execute();
      }
   }
   else
   {
      C_OgeWiCustomMessage c_Box(this, C_OgeWiCustomMessage::E_Type::eERROR);
      QApplication::restoreOverrideCursor();
      QString c_Details = "Invalid content: \n";
      if (q_ValidName == false)
      {
         c_Details += (c_Name.isEmpty()) ?  "Empty project name" : c_Name;
         c_Details += "\n";
      }

      if (q_ValidPath == false)
      {
         c_Details += (c_BasePath.isEmpty()) ?  "Empty project path" : c_BasePath;
      }

      c_Box.SetHeading("Save project as");
      c_Box.SetDescription("Name or/and path is empty or contains invalid characters. "
                                                  "Please choose valid name and path.");
      c_Box.SetDetails(c_Details);
      c_Box.SetCustomMinHeight(230, 270);
      c_Box.Execute();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Cancel button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PopSaveAsDialogWidget::m_OnCancel(void)
{
   this->mrc_ParentDialog.reject();
}

