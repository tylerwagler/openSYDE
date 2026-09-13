//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Button with tool tip functionality (implementation)

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QEvent>
#include <QHelpEvent>

#include "C_OgePubToolTipBase.hpp"

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
C_OgePubToolTipBase::C_OgePubToolTipBase(QWidget * const opc_Parent) :
   QPushButton(opc_Parent),
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
bool C_OgePubToolTipBase::event(QEvent * const opc_Event)
{
   bool q_Handled;
   const bool q_Return = this->m_HandleToolTipEvent(opc_Event, *this, q_Handled);

   return (q_Handled == true) ? q_Return : QPushButton::event(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit the signal asking other widgets to drop their tool tips
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubToolTipBase::m_EmitHideOtherToolTips(void)
{
   Q_EMIT this->SigHideOtherToolTips();
}
