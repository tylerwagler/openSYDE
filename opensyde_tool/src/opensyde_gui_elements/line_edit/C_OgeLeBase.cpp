//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated line edit with objectName/variant-based styling (implementation)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeLeBase.hpp"
#include "C_OgeWiUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeBase::C_OgeLeBase(QWidget * const opc_Parent) :
   C_OgeLeToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeBase::~C_OgeLeBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set line edit variant and trigger QSS re-polish
   \param[in] orc_Variant Variant name for QSS targeting
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeBase::SetLineEditVariant(const QString & orc_Variant)
{
   this->mc_LineEditVariant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current line edit variant
   \return Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLeBase::GetLineEditVariant(void) const
{
   return this->mc_LineEditVariant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set background color (stylesheet color index)
   \param[in] os32_Value New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeBase::SetBackgroundColor(const int32_t os32_Value)
{
   C_OgeWiUtil::h_ApplyStylesheetProperty(this, "Background", os32_Value);
}
