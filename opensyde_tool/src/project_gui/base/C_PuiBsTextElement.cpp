//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI text element data: stores UI information (implementation)

   UI text element data: stores UI information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "constants.hpp"
#include "C_PuiBsTextElement.hpp"

#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiBsTextElement::C_PuiBsTextElement(void) :
   C_PuiBsBox(),
   c_UiFontStyle(QFont("Segoe UI", 18)),
   c_UiFontColorBright(mc_STYLE_GUIDE_COLOR_34),
   c_UiFontColorDark(mc_STYLE_GUIDE_COLOR_0),
   c_UiText("Text Element")
{
   c_UiFontStyle.setPixelSize(18);
   c_UiFontStyle.setItalic(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiBsTextElement::~C_PuiBsTextElement(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiBsTextElement::CalcHash(uint32_t & oru32_HashValue) const
{
   const QString c_Font = this->c_UiFontStyle.toString();

   hash_util::CalcHashMembers(oru32_HashValue,
                              c_Font,
                              this->c_UiFontColorBright.red(), this->c_UiFontColorBright.green(),
                              this->c_UiFontColorBright.blue(), this->c_UiFontColorBright.alpha(),
                              this->c_UiFontColorDark.red(), this->c_UiFontColorDark.green(),
                              this->c_UiFontColorDark.blue(), this->c_UiFontColorDark.alpha(),
                              this->c_UiText);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}
