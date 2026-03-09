//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only tab widget with Q_PROPERTY variant

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeTawStyled.hpp"

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

   Sets expanding tab bar (common to all style variants).

   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeTawStyled::C_OgeTawStyled(QWidget * const opc_Parent) :
   C_OgeTawToolTipBase(opc_Parent)
{
   this->tabBar()->setExpanding(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeTawStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTawStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}
