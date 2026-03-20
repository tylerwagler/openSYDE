//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated radio button with objectName/variant-based styling (implementation)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeRabBase.hpp"
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
C_OgeRabBase::C_OgeRabBase(QWidget * const opc_Parent) :
   C_OgeRabToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeRabBase::~C_OgeRabBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set radio variant and trigger QSS re-polish
   \param[in] orc_Variant Variant name for QSS targeting
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeRabBase::SetRadioVariant(const QString & orc_Variant)
{
   this->mc_RadioVariant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current radio variant
   \return Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeRabBase::GetRadioVariant(void) const
{
   return this->mc_RadioVariant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Configures the stylesheet for the receivers variant of this type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeRabBase::SetStyleVariantReceivers(void)
{
   C_OgeWiUtil::h_ApplyStylesheetProperty(this, "Margin", 3);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this, "IndicatorMargin", 4);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this, "CheckedColor", 4);
}
