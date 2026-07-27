//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Combo box styled for smaller widget usage (implementation)

   Combo box styled for smaller widget usage.
   This class does not contain any functionality,
   but needs to exist, to have a unique group,
   to apply a specific stylesheet for.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QApplication>
#include "C_OgeCbxIconDelegate.hpp"
#include "C_CamOgeCbxTableSmall.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamOgeCbxTableSmall::C_CamOgeCbxTableSmall(QWidget * const opc_Parent) :
   C_OgeCbxTableBase(opc_Parent)
{
   //this code allows to handle the QAbstractItemView::item in stylesheets
   C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();

   this->setItemDelegate(pc_ItemDelegate);
   this->setMaxVisibleItems(10);
   this->view()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
} //lint !e429  no memory leak because of the parent all elements and the Qt memory management

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamOgeCbxTableSmall::~C_CamOgeCbxTableSmall(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Override showPopup to ensure the dropdown shows all items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamOgeCbxTableSmall::showPopup(void)
{
   this->setMaxVisibleItems(10);
   QComboBox::showPopup();
   // The combo-box editor is constrained to the table row height (~20 px), which
   // confuses Qt's popup geometry calculation. Manually size the popup to fit.
   if (this->count() > 0)
   {
      const int32_t s32_RowH = this->view()->sizeHintForRow(0);
      const int32_t s32_Desired = (s32_RowH * this->count()) + 10;
      const QWidgetList tlw = QApplication::topLevelWidgets();
      for (QWidget * const pc_Widget : tlw)
      {
         if ((pc_Widget->windowFlags().testFlag(Qt::Popup) == true) && (pc_Widget->isVisible() == true))
         {
            pc_Widget->setMinimumHeight(s32_Desired);
            pc_Widget->resize(pc_Widget->width(), s32_Desired);
            break;
         }
      }
   }
}
