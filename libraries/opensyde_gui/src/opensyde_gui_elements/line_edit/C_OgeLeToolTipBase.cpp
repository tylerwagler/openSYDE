//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Line edit field with tool tip (implementation)

   Line edit field with tool tip

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QEvent>
#include <QHelpEvent>
#include "C_OgeLeToolTipBase.hpp"

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
C_OgeLeToolTipBase::C_OgeLeToolTipBase(QWidget * const opc_Parent) :
   C_OgeLeContextMenuBase(opc_Parent),
   C_OgeToolTipBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten default event slot

   Here: Handle tool tip

   \param[in,out] opc_Event Event identification and information

   \return
   True  Event was recognized and processed
   False Event ignored
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OgeLeToolTipBase::event(QEvent * const opc_Event)
{
   bool q_Handled;
   const bool q_Return = this->m_HandleToolTipEvent(opc_Event, *this, q_Handled);

   return (q_Handled == true) ? q_Return : QLineEdit::event(opc_Event);
}
