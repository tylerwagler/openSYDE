//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only spin box with tooltip support

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeSpxStyled.hpp"

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
C_OgeSpxStyled::C_OgeSpxStyled(QWidget * const opc_Parent) :
   C_OgeSpxToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor with variant

   \param[in]      orc_Variant   Style variant name
   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeSpxStyled::C_OgeSpxStyled(const QString & orc_Variant, QWidget * const opc_Parent) :
   C_OgeSpxToolTipBase(opc_Parent),
   mc_Variant(orc_Variant)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeSpxStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeSpxStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}
