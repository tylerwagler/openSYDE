//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for showing all node applications of a specific node (implementation)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_UsHandler.hpp"
#include "C_GtGetText.hpp"
#include "C_PuiProject.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdNdeDbViewWidget.hpp"
#include "C_SdNdeDbProperties.hpp"
#include "C_OgeWiCustomMessage.hpp"
#include "ui_C_SdNdeDbViewWidget.h"
#include "C_PopSaveAsDialogWidget.hpp"
#include "C_SdNdeProgrammingOptions.hpp"
#include "C_OscHalcDefFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDbViewWidget::C_SdNdeDbViewWidget(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mpc_Ui(new Ui::C_SdNdeDbViewWidget),
   mu32_NodeIndex(0)
{
   this->mpc_Ui->setupUi(this);

   InitStaticNames();

   // init icons
   this->mpc_Ui->pc_PushButtonAdd->SetCustomIcons("://images/IconAddEnabled.svg", "://images/IconAddHovered.svg",
                                                  "://images/IconAddClicked.svg", "://images/IconAddDisabled.svg");
   this->mpc_Ui->pc_PushButtonCodeGenerationOptions->SetSvg("://images/system_definition/IconGenerateCodeSettings.svg",
                                                            "://images/system_definition/IconGenerateCodeSettings.svg");

   //Connects
   connect(this->mpc_Ui->pc_ListWidget, &C_SdNdeDbListWidget::SigDelete, this,
           &C_SdNdeDbViewWidget::m_OnDelete);
   connect(this->mpc_Ui->pc_ListWidget, &C_SdNdeDbListWidget::SigErrorChange, this,
           &C_SdNdeDbViewWidget::SigErrorChange);
   connect(this->mpc_Ui->pc_ListWidget, &C_SdNdeDbListWidget::SigAppDisplay, this,
           &C_SdNdeDbViewWidget::m_OnAppDisplay);
   connect(this->mpc_Ui->pc_PushButtonAdd, &stw::opensyde_gui_elements::C_OgePubIconOnly::clicked, this,
           &C_SdNdeDbViewWidget::AddApp);
   connect(this->mpc_Ui->pc_ListWidget, &C_SdNdeDbListWidget::SigOwnedDataPoolsChanged, this,
           &C_SdNdeDbViewWidget::SigOwnedDataPoolsChanged);
   connect(this->mpc_Ui->pc_PushButtonCodeGenerationOptions, &QPushButton::clicked, this,
           &C_SdNdeDbViewWidget::m_ProgrammingOptions);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDbViewWidget::~C_SdNdeDbViewWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::InitStaticNames(void) const
{
   //Tool tips
   this->mpc_Ui->pc_LabelApplicationName->SetToolTipInformation(
      C_GtGetText::h_GetText("Data Blocks"),
      C_GtGetText::h_GetText(
         "Data Blocks specify files that must be updated to the node.\n\n"
         "Those files can represent any kind of application or data. They can be either created by another tool chain "
         "or developed by using the openSYDE file generation feature. The second is available for devices with "
         "programming support or with NVM-based hardware configuration. "));

   this->mpc_Ui->pc_PushButtonAdd->SetToolTipInformation(C_GtGetText::h_GetText("Add"),
                                                         C_GtGetText::h_GetText("Add new Data Block."));

   this->mpc_Ui->pc_PushButtonCodeGenerationOptions->SetToolTipInformation(C_GtGetText::h_GetText(
                                                                              "Source Code Generation Settings"),
                                                                           C_GtGetText::h_GetText(
                                                                              "Edit source code generation settings."));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node index

   \param[in]  ou32_NodeIndex    Node index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::SetNodeIndex(const uint32_t ou32_NodeIndex)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);

   this->mu32_NodeIndex = ou32_NodeIndex;
   tgl_assert(pc_Node != NULL);
   if (pc_Node != NULL)
   {
      this->mpc_Ui->pc_ListWidget->SetIndex(ou32_NodeIndex);

      this->mpc_Ui->pc_ListWidget->clear();
      for (uint32_t u32_ItApp = 0; u32_ItApp < pc_Node->c_Applications.size(); ++u32_ItApp)
      {
         this->mpc_Ui->pc_ListWidget->AddApplication(ou32_NodeIndex, u32_ItApp);
      }
   }

   this->m_UpdateCount();
   this->m_HandleNoDatablocksLabel();
   this->m_HandleAddButtonAvailability();
   this->m_HandleCodeGenerationSettingsButtonAvailability();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Scrolls to the application with the index ou32_ApplicationIndex

   \param[in]  ou32_ApplicationIndex   Index of application
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::ShowApplication(const uint32_t ou32_ApplicationIndex) const
{
   this->mpc_Ui->pc_ListWidget->scrollToItem(
      this->mpc_Ui->pc_ListWidget->item(static_cast<int32_t>(ou32_ApplicationIndex)));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add a new application
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::AddApp(void)
{
   bool q_Continue = true;

   // check if empty path (new unsaved project)
   if (C_PuiProject::h_GetInstance()->IsEmptyProject() == true)
   {
      // warn user
      C_OgeWiCustomMessage c_MessageBox(this);
      c_MessageBox.SetType(C_OgeWiCustomMessage::eWARNING);
      c_MessageBox.SetHeading(C_GtGetText::h_GetText("Add Data Blocks"));
      c_MessageBox.SetDescription(C_GtGetText::h_GetText(
                                     "This project is not saved yet. Adding Data Blocks might cause "
                                     "problems with file or directory paths."));
      c_MessageBox.SetDetails(C_GtGetText::h_GetText(
                                 "Paths that are handled as relative to *.syde file can not be resolved correctly!"));
      c_MessageBox.SetOkButtonText(C_GtGetText::h_GetText("Continue"));
      c_MessageBox.SetCustomMinHeight(230, 270);
      c_MessageBox.SetCancelButtonText(C_GtGetText::h_GetText("Cancel"));
      if (c_MessageBox.Execute() != C_OgeWiCustomMessage::eOK)
      {
         q_Continue = false;
      }
   }

   if (q_Continue)
   {
      const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

      tgl_assert(pc_Node != NULL);
      if (pc_Node != NULL)
      {
         if (pc_Node->IsAnyUpdateAvailable() == true)
         {
            this->m_AddManualApplication();
         }
         else
         {
            //no fbl support
            C_OgeWiCustomMessage c_MessageBox(this);
            c_MessageBox.SetType(C_OgeWiCustomMessage::E_Type::eWARNING);
            c_MessageBox.SetHeading(C_GtGetText::h_GetText("Add Datablocks"));
            c_MessageBox.SetDescription(C_GtGetText::h_GetText("There is no Flashloader support for this device type. "
                                                               "Data Blocks cannot be added."));
            c_MessageBox.SetCustomMinHeight(180, 180);
            c_MessageBox.Execute();
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update applications
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::UpdateApplications(void) const
{
   this->mpc_Ui->pc_ListWidget->UpdateApplications();
   this->m_OnAppDisplay();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add new project action

   \param[in]  oq_IsNewNode   Is Node new or not
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::AddFromTsp(const bool oq_IsNewNode)
{
   const QString c_LAST_KNOWN_TSP_PATH = C_UsHandler::h_GetInstance()->GetProjSdTopologyLastKnownTspPath();

   const QPointer<C_OgePopUpDialog> c_New = new C_OgePopUpDialog(this, this);

   C_SdNdeDbAddNewProject * const pc_Dialog = new C_SdNdeDbAddNewProject(this->mu32_NodeIndex, *c_New,
                                                                         oq_IsNewNode);

   pc_Dialog->SetTspPath(c_LAST_KNOWN_TSP_PATH);

   //Help
   //connect(pc_New, &C_OgePopUpDialog::SigHelp, pc_SettingsWidget, &C_GiSyLineWidget::HandleHelp);

   //Resize
   const QSize c_SIZE(970, 745);

   c_New->SetSize(c_SIZE);

   //init

   if (c_New->exec() == static_cast<int32_t>(QDialog::Accepted))
   {
      C_OgeWiCustomMessage c_Message(this);
      const QString c_Details = "";
      pc_Dialog->ApplyV3Content();
      this->m_UpdateTrigger(false);
      c_Message.SetHeading(C_GtGetText::h_GetText("Import TSP"));

      c_Message.SetCustomMinHeight(180, 180);
      c_Message.SetCustomMinWidth(650);
      c_Message.SetType(C_OgeWiCustomMessage::eINFORMATION);
      c_Message.SetCustomMinHeight(180, 300);
      c_Message.SetDetails(c_Details);
      c_Message.SetDescription(C_GtGetText::h_GetText("Node definition successfully imported."));
      c_Message.Execute();
   }

   if (c_New != NULL)
   {
      C_UsHandler::h_GetInstance()->SetProjSdTopologyLastKnownTspPath(pc_Dialog->GetTspPath());
      c_New->HideOverlay();
      c_New->deleteLater();
   }
} //lint !e593  //no memory leak because of the parent of pc_Dialog and the Qt memory management

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show/hide no data blocks declared label
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_HandleNoDatablocksLabel(void) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   if ((pc_Node != NULL) && (pc_Node->IsAnyUpdateAvailable() == true))
   {
      //are there any data blocks?
      if (pc_Node->c_Applications.empty() == true)
      {
         //no data blocks display label
         this->mpc_Ui->pc_LabelNoDatablocks->setVisible(true);
         this->mpc_Ui->pc_LabelNoDatablocks->setText(C_GtGetText::h_GetText(
                                                        "No Data Block declared. \nAdd any via the '+' button"));
      }
      else
      {
         //data blocks present, no label required
         this->mpc_Ui->pc_LabelNoDatablocks->setVisible(false);
      }
   }
   else
   {
      this->mpc_Ui->pc_LabelNoDatablocks->setVisible(true);
      this->mpc_Ui->pc_LabelNoDatablocks->setText(C_GtGetText::h_GetText(
                                                     "No openSYDE protocol support. \nData Blocks cannot be added."));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle add button availability
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_HandleAddButtonAvailability(void) const
{
   bool q_Enabled = true;
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   if (pc_Node != NULL)
   {
      if (pc_Node->IsAnyUpdateAvailable() == true)
      {
         q_Enabled = true;
      }
      else
      {
         q_Enabled = false;
      }
   }
   else
   {
      q_Enabled = false;
   }
   this->mpc_Ui->pc_PushButtonAdd->setEnabled(q_Enabled);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add new application

   \param[in]  oe_Type  Application type

   \return
   Application position (in node)
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SdNdeDbViewWidget::m_AddApplication(const C_OscNodeApplication::E_Type oe_Type) const
{
   C_OscNodeApplication c_Appl;
   uint32_t u32_Retval;

   // add datablock
   c_Appl.c_Name = "DataBlock";
   c_Appl.c_Comment = "";
   c_Appl.q_Active = true;
   c_Appl.e_Type = oe_Type;
   u32_Retval = m_AddApplication(c_Appl);

   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add new application

   \param[in,out]  orc_Application  Application content

   \return
   Application position (in node)
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SdNdeDbViewWidget::m_AddApplication(C_OscNodeApplication & orc_Application) const
{
   uint32_t u32_Retval = 0UL;
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   tgl_assert(pc_Node != NULL);
   if (pc_Node != NULL)
   {
      u32_Retval = static_cast<uint32_t>(pc_Node->c_Applications.size());
      tgl_assert(C_PuiSdHandler::h_GetInstance()->InsertApplication(this->mu32_NodeIndex, u32_Retval,
                                                                    orc_Application) == C_NO_ERR);
      //No reload required
      this->mpc_Ui->pc_ListWidget->AddApplication(this->mu32_NodeIndex, u32_Retval);
      this->m_UpdateCount();
      this->m_HandleAddButtonAvailability();
      this->m_HandleCodeGenerationSettingsButtonAvailability();
   }
   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Update Trigger for UI pages

   \param[in]  oq_OnlyUpdateProperties    Only update properties
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_UpdateTrigger(const bool oq_OnlyUpdateProperties)
{
   Q_EMIT (this->SigUpdateTrigger(this->mu32_NodeIndex, oq_OnlyUpdateProperties));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   On App Display
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_OnAppDisplay() const
{
   this->m_UpdateCount();
   this->m_HandleNoDatablocksLabel();
   this->m_HandleAddButtonAvailability();
   this->m_HandleCodeGenerationSettingsButtonAvailability();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle delete

   \param[in]  ou32_NodeIndex          Node index
   \param[in]  ou32_ApplicationIndex   Application index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_OnDelete(const uint32_t ou32_NodeIndex, const uint32_t ou32_ApplicationIndex)
{
   tgl_assert(this->mu32_NodeIndex == ou32_NodeIndex);
   tgl_assert(C_PuiSdHandler::h_GetInstance()->RemoveApplication(ou32_NodeIndex, ou32_ApplicationIndex) == C_NO_ERR);

   //Trigger reload (also important for index update)
   this->SetNodeIndex(ou32_NodeIndex);

   // inform about changes of owned Datapools because they are now not assigned anymore
   Q_EMIT (this->SigOwnedDataPoolsChanged());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Deletes all the node's applications

   \param[in]  ou32_NodeIndex    Index of selected node
   \param[in]  orc_Applications  Array of the node's applications
*/
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update data block count
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_UpdateCount(void) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   tgl_assert(pc_Node != NULL);
   if (pc_Node != NULL)
   {
      this->mpc_Ui->pc_LabelApplicationName->setText(static_cast<QString>("Data Blocks (%1)").arg(pc_Node->
                                                                                                  c_Applications.size()));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of programming options button
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_ProgrammingOptions(void) const
{
   const std::vector<const C_OscNodeApplication *> c_ProgrammableApplications =
      C_PuiSdHandler::h_GetInstance()->GetProgrammableApplications(this->mu32_NodeIndex);

   if (c_ProgrammableApplications.size() > 0)
   {
      //Set parent for better hierarchy handling via window manager
      const QPointer<C_OgePopUpDialog> c_New = new C_OgePopUpDialog(this->parentWidget(), this->parentWidget());
      const QPointer<const C_SdNdeProgrammingOptions> c_Dialog = new C_SdNdeProgrammingOptions(*c_New,
                                                                                               this->mu32_NodeIndex);

      //Help
      //connect(pc_New, &C_OgePopUpDialog::SigHelp, pc_SettingsWidget, &C_GiSyLineWidget::HandleHelp);

      //Resize
      c_New->SetSize(QSize(810, 655));

      if (c_New->exec() == static_cast<int32_t>(QDialog::Accepted))
      {
         if (c_Dialog.isNull() == false)
         {
            c_Dialog->Save();
         }
      }
      //Hide overlay after dialog is not relevant anymore
      if (c_New.isNull() == false)
      {
         c_New->HideOverlay();
         c_New->deleteLater();
      }
   } //lint !e429  //no memory leak because of the parent of pc_New and pc_Dialog and the Qt memory management
   else
   {
      C_OgeWiCustomMessage c_Message(this->parentWidget(), C_OgeWiCustomMessage::E_Type::eWARNING);
      c_Message.SetHeading(C_GtGetText::h_GetText("File generation settings"));
      c_Message.SetDescription(C_GtGetText::h_GetText(
                                  "File generation settings are not available. "
                                  "\nThere are no Data Blocks with active file generation declared."));
      c_Message.SetCustomMinHeight(180, 180);
      c_Message.Execute();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add new manual application
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_AddManualApplication(void)
{
   const QPointer<C_OgePopUpDialog> c_New = new C_OgePopUpDialog(this, this);
   const C_SdNdeDbProperties * const pc_Dialog = new C_SdNdeDbProperties(this->mu32_NodeIndex, -1, *c_New);

   //Resize
   c_New->SetSize(C_SdNdeDbProperties::h_GetBinaryWindowSize());

   if (c_New->exec() == static_cast<int32_t>(QDialog::Accepted))
   {
      C_OscNodeApplication c_Tmp;

      uint32_t u32_Index;
      pc_Dialog->ApplyNewData(c_Tmp);
      u32_Index = m_AddApplication(c_Tmp);
      pc_Dialog->HandleDataPools(u32_Index);
      // Inform about change (this also triggers this->UpdateApplications())
      Q_EMIT (this->SigOwnedDataPoolsChanged());
   }

   if (c_New != NULL)
   {
      c_New->HideOverlay();
      c_New->deleteLater();
   }
} //lint !e429  //no memory leak because of the parent of pc_Dialog and the Qt memory management

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle visibility of button "File generation settings"
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDbViewWidget::m_HandleCodeGenerationSettingsButtonAvailability(void) const
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   tgl_assert(pc_Node != NULL);
   if (pc_Node != NULL)
   {
      tgl_assert(pc_Node->pc_DeviceDefinition != NULL);
      if (pc_Node->pc_DeviceDefinition != NULL)
      {
         tgl_assert(pc_Node->u32_SubDeviceIndex < pc_Node->pc_DeviceDefinition->c_SubDevices.size());
         if (pc_Node->u32_SubDeviceIndex < pc_Node->pc_DeviceDefinition->c_SubDevices.size())
         {
            if ((pc_Node->pc_DeviceDefinition->c_SubDevices[pc_Node->u32_SubDeviceIndex].q_ProgrammingSupport ==
                 true) &&
                (C_PuiSdHandler::h_GetInstance()->GetProgrammableApplications(this->mu32_NodeIndex).size() > 0UL))
            {
               this->mpc_Ui->pc_PushButtonCodeGenerationOptions->setVisible(true);
            }
            else
            {
               this->mpc_Ui->pc_PushButtonCodeGenerationOptions->setVisible(false);
            }
         }
      }
   }
}
