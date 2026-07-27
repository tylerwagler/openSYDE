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

#include "stwtypes.hpp"
#include "C_NagToolSettingsPopupDialog.hpp"
#include "ui_C_NagToolSettingsPopupDialog.h"
#include "C_OscUtils.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_PuiProject.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "C_PopUtil.hpp"
#include "C_NagUnUsedProjectFilesPopUpDialog.hpp"
#include "C_UsHandler.hpp"

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
