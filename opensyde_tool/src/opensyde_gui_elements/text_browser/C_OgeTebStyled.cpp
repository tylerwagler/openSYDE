//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only text browser with Q_PROPERTY variant

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeTebStyled.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor

   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeTebStyled::C_OgeTebStyled(QWidget * const opc_Parent) :
   C_OgeTebContextMenuBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeTebStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTebStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   // Apply Label-specific behavior
   if (orc_Variant == "Label")
   {
      this->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
   }
   this->style()->unpolish(this);
   this->style()->polish(this);
}
