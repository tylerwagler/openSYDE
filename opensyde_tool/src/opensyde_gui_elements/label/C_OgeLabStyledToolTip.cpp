//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label with tooltip support

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeLabStyledToolTip.hpp"

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
C_OgeLabStyledToolTip::C_OgeLabStyledToolTip(QWidget * const opc_Parent) :
   C_OgeLabToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor with variant

   \param[in]      orc_Variant   Style variant name
   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLabStyledToolTip::C_OgeLabStyledToolTip(const QString & orc_Variant, QWidget * const opc_Parent) :
   C_OgeLabToolTipBase(opc_Parent),
   mc_Variant(orc_Variant)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLabStyledToolTip::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name (e.g. "GroupItem", "DashboardTab")
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabStyledToolTip::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   // Force stylesheet re-evaluation after property change
   this->style()->unpolish(this);
   this->style()->polish(this);
}
