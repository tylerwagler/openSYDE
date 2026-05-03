//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for pop-up dialog content widgets (implementation)

   Provides the shared Ctrl+Enter accept boilerplate so each pop-up content widget can override
   m_OnEnterAccept instead of duplicating the keyPressEvent body.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_OgePopUpContentBase.hpp"

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

   \param[in,out] orc_Parent  Reference to parent pop-up dialog
   \param[in,out] opc_Parent  Optional Qt parent widget
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePopUpContentBase::C_OgePopUpContentBase(C_OgePopUpDialog & orc_Parent, QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mrc_ParentDialog(orc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out] opc_KeyEvent Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePopUpContentBase::keyPressEvent(QKeyEvent * const opc_KeyEvent)
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
         this->m_OnEnterAccept();
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
/*! \brief   Called when the user presses Ctrl+Enter (no Alt, no Shift)

   Default implementation: accept the parent dialog. Override to add pre-flight logic
   (validation, save, etc.) before accepting.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePopUpContentBase::m_OnEnterAccept(void)
{
   this->mrc_ParentDialog.accept();
}
