//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label with context menu support

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeLabStyledContextMenu.hpp"

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
C_OgeLabStyledContextMenu::C_OgeLabStyledContextMenu(QWidget * const opc_Parent) :
   C_OgeLabContextMenuBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLabStyledContextMenu::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name (e.g. "DescriptionMessage", "HeadingGroupBold")
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabStyledContextMenu::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   // Force stylesheet re-evaluation after property change
   this->style()->unpolish(this);
   this->style()->polish(this);
}
