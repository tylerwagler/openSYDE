//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label (inherits QLabel)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeLabStyled.hpp"

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
C_OgeLabStyled::C_OgeLabStyled(QWidget * const opc_Parent) :
   QLabel(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor with variant

   \param[in]      orc_Variant   Style variant name
   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLabStyled::C_OgeLabStyled(const QString & orc_Variant, QWidget * const opc_Parent) :
   QLabel(opc_Parent),
   mc_Variant(orc_Variant)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLabStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name (e.g. "HeadingWidget", "GroupItem")
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   // Force stylesheet re-evaluation after property change
   this->style()->unpolish(this);
   this->style()->polish(this);
}
