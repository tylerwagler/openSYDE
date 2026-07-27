//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data set array edit widget

   Data set array edit widget

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_SdNdeDpListArrayEditWidget.hpp"
#include "ui_C_SdNdeDpListArrayEditWidget.h"
#include "C_PuiSdHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] orc_Parent          Reference to parent
   \param[in]     oru32_NodeIndex     Node index
   \param[in]     oru32_DataPoolIndex Node data pool index
   \param[in]     oru32_ListIndex     Node data pool list index
   \param[in]     oru32_ElementIndex  Node data pool list element index
   \param[in]     ore_ArrayEditType   Enum for node data pool list element variable
   \param[in]     oru32_DataSetIndex  If min or max use 0
                                  Else use data set index
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListArrayEditWidget::C_SdNdeDpListArrayEditWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                                           const uint32_t & oru32_NodeIndex,
                                                           const uint32_t & oru32_DataPoolIndex,
                                                           const uint32_t & oru32_ListIndex,
                                                           const uint32_t & oru32_ElementIndex,
                                                           const C_SdNdeDpUtil::E_ArrayEditType & ore_ArrayEditType,
                                                           const uint32_t & oru32_DataSetIndex) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_SdNdeDpListArrayEditWidget),
   mu32_NodeIndex(oru32_NodeIndex),
   mu32_DataPoolIndex(oru32_DataPoolIndex),
   mu32_ListIndex(oru32_ListIndex),
   mu32_ElementIndex(oru32_ElementIndex),
   me_ArrayEditType(ore_ArrayEditType),
   mu32_DataSetIndex(oru32_DataSetIndex)
{
   this->mpc_Ui->setupUi(this);
   this->mrc_ParentDialog.SetWidget(this);
   InitStaticNames();
   this->mpc_Ui->pc_TableView->SetElement(oru32_NodeIndex, oru32_DataPoolIndex, oru32_ListIndex, oru32_ElementIndex,
                                          ore_ArrayEditType, oru32_DataSetIndex);

   //Connects
   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this,
           &C_SdNdeDpListArrayEditWidget::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this,
           &C_SdNdeDpListArrayEditWidget::m_CancelClicked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDpListArrayEditWidget::~C_SdNdeDpListArrayEditWidget(void)
{
   delete mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListArrayEditWidget::InitStaticNames(void) const
{
   const C_OscNodeDataPool * const pc_DataPool = C_PuiSdHandler::h_GetInstance()->GetOscDataPool(this->mu32_NodeIndex,
                                                                                                 this->mu32_DataPoolIndex);
   const C_OscNodeDataPoolListElement * const pc_Element =
      C_PuiSdHandler::h_GetInstance()->GetOscDataPoolListElement(
         this->mu32_NodeIndex, this->mu32_DataPoolIndex, this->mu32_ListIndex, this->mu32_ElementIndex);
   const C_OscNodeDataPoolDataSet * const pc_DataSet = C_PuiSdHandler::h_GetInstance()->GetOscDataPoolListDataSet(
      this->mu32_NodeIndex, this->mu32_DataPoolIndex,
      this->mu32_ListIndex,
      this->mu32_DataSetIndex);

   //Build title
   if ((pc_DataPool != NULL) && (pc_Element != NULL))
   {
      const QString c_Type = C_PuiSdHandler::h_GetElementTypeName(pc_DataPool->e_Type);
      QString c_EditType;
      switch (this->me_ArrayEditType)
      {
      case C_SdNdeDpUtil::eARRAY_EDIT_MIN:
         c_EditType = "Minimum";
         break;
      case C_SdNdeDpUtil::eARRAY_EDIT_MAX:
         c_EditType = "Maximum";
         break;
      case C_SdNdeDpUtil::eARRAY_EDIT_DATA_SET:
         if (pc_DataSet != NULL)
         {
            //Translation: 1: data set name
            c_EditType = static_cast<QString>("Dataset Value");
         }
         break;
      default:
         c_EditType = "Unknown";
         break;
      }

      //Translation: 1: Data element type, 2: Data element name, 3: Value type
      this->mrc_ParentDialog.SetTitle(static_cast<QString>("%1 %2 (%3)").arg(c_Type).arg(
                                   pc_Element->c_Name.c_str()).arg(c_EditType));
   }

   this->mrc_ParentDialog.SetSubTitle(static_cast<QString>("Array Editor"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   GetUndocommand and take ownership

   Internal undo command is reseted

   \return
   NULL No changes
   Else Undocommand accumulating all changes
*/
//----------------------------------------------------------------------------------------------------------------------
QUndoCommand * C_SdNdeDpListArrayEditWidget::TakeUndoCommand(void) const
{
   return this->mpc_Ui->pc_TableView->TakeUndoCommand();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set model view manager

   \param[in,out] opc_Value Model view manager
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListArrayEditWidget::SetModelViewManager(C_SdNdeDpListModelViewManager * const opc_Value)
const
{
   this->mpc_Ui->pc_TableView->SetModelViewManager(opc_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   On ok clicked
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListArrayEditWidget::m_OkClicked(void)
{
   mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   On cancel clicked
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeDpListArrayEditWidget::m_CancelClicked(void)
{
   mrc_ParentDialog.reject();
}
