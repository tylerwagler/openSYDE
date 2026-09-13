//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standard color button in color picker (implementation)

   This class does not contain any functionality,
   but needs to exist, to have a unique group,
   to apply a specific stylesheet for..

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgePubColor.hpp"
#include "C_OgeWiUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor

   Set up GUI with all elements.

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePubColor::C_OgePubColor(QWidget * const opc_Parent) :
   QPushButton(opc_Parent),
   C_OgeToolTipBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set background color

   \param[in]  orc_BackgroundColor   New background color
   \param[in]  orc_TopBorderColor    New top border color
   \param[in]  orc_RestBorderColor   New rest border color
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubColor::SetColors(const QColor & orc_BackgroundColor, const QColor & orc_TopBorderColor,
                              const QColor & orc_RestBorderColor)
{
   this->setStyleSheet(
      "stw--opensyde_gui_elements--C_OgePubColor"
      "{"
      "   background-color: " + orc_BackgroundColor.name() + ";"
      "   border-top-color: " + orc_TopBorderColor.name() + ";"
      "   border-right-color: " + orc_RestBorderColor.name() + ";"
      "   border-bottom-color: " + orc_RestBorderColor.name() + ";"
      "   border-left-color: " + orc_RestBorderColor.name() + ";"
      "}");
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
bool C_OgePubColor::event(QEvent * const opc_Event)
{
   bool q_Handled;
   const bool q_Return = this->m_HandleToolTipEvent(opc_Event, *this, q_Handled);

   return (q_Handled == true) ? q_Return : QPushButton::event(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit the signal asking other widgets to drop their tool tips
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubColor::m_EmitHideOtherToolTips(void)
{
   Q_EMIT this->SigHideOtherToolTips();
}
