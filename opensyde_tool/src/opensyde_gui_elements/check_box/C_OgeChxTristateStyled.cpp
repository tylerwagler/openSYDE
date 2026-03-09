//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only tristate check box with Q_PROPERTY variant

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeChxTristateStyled.hpp"

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
C_OgeChxTristateStyled::C_OgeChxTristateStyled(QWidget * const opc_Parent) :
   C_OgeChxTristateBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor with variant

   \param[in]      orc_Variant   Style variant name
   \param[in,out]  opc_Parent    Parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeChxTristateStyled::C_OgeChxTristateStyled(const QString & orc_Variant, QWidget * const opc_Parent) :
   C_OgeChxTristateBase(opc_Parent),
   mc_Variant(orc_Variant)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get style variant name

   \return  Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeChxTristateStyled::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set style variant name and re-polish for QSS

   \param[in]  orc_Variant    Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxTristateStyled::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}
