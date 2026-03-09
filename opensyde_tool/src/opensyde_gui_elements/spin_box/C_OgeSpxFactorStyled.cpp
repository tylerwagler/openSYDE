//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only factor spin box

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeSpxFactorStyled.hpp"

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
C_OgeSpxFactorStyled::C_OgeSpxFactorStyled(QWidget * const opc_Parent) :
   C_OgeSpxFactor(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor with variant

   \param[in]      orc_Variant   Style variant name
   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeSpxFactorStyled::C_OgeSpxFactorStyled(const QString & orc_Variant, QWidget * const opc_Parent) :
   C_OgeSpxFactor(opc_Parent),
   mc_Variant(orc_Variant)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeSpxFactorStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeSpxFactorStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}
