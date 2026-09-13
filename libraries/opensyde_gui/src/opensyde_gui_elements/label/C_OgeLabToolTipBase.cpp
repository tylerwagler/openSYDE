//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label with tool tip (implementation)

   Label with tool tip

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QEvent>
#include <QHelpEvent>
#include "C_OgeLabToolTipBase.hpp"

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
C_OgeLabToolTipBase::C_OgeLabToolTipBase(QWidget * const opc_Parent) :
   QLabel(opc_Parent),
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
bool C_OgeLabToolTipBase::event(QEvent * const opc_Event)
{
   bool q_Handled;
   const bool q_Return = this->m_HandleToolTipEvent(opc_Event, *this, q_Handled);

   return (q_Handled == true) ? q_Return : QLabel::event(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit the signal letting the owner refresh the text before it is shown
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabToolTipBase::m_EmitLastChanceToUpdateToolTip(void)
{
   Q_EMIT this->SigLastChanceToUpdateToolTip();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit the signal asking other widgets to drop their tool tips
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabToolTipBase::m_EmitHideOtherToolTips(void)
{
   Q_EMIT this->SigHideOtherToolTips();
}
