//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI boundary data: stores UI information (implementation)

   UI boundary data: stores UI information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "constants.hpp"
#include "C_PuiBsBoundary.hpp"

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
C_PuiBsBoundary::C_PuiBsBoundary() :
   C_PuiBsBox(),
   c_UiBorderColorBright(mc_STYLE_GUIDE_COLOR_7),
   c_UiBorderColorDark(mc_STYLE_GUIDE_COLOR_2),
   c_UiBackgroundColorBright(mc_STYLE_GUIDE_COLOR_12),
   c_UiBackgroundColorDark(mc_STYLE_GUIDE_COLOR_36),
   s32_UiBorderWidth(1)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiBsBoundary::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_UiBorderColorBright.red(), this->c_UiBorderColorBright.green(),
                              this->c_UiBorderColorBright.blue(), this->c_UiBorderColorBright.alpha(),
                              this->c_UiBorderColorDark.red(), this->c_UiBorderColorDark.green(),
                              this->c_UiBorderColorDark.blue(), this->c_UiBorderColorDark.alpha(),
                              this->c_UiBackgroundColorBright.red(), this->c_UiBackgroundColorBright.green(),
                              this->c_UiBackgroundColorBright.blue(), this->c_UiBackgroundColorBright.alpha(),
                              this->c_UiBackgroundColorDark.red(), this->c_UiBackgroundColorDark.green(),
                              this->c_UiBackgroundColorDark.blue(), this->c_UiBackgroundColorDark.alpha(),
                              this->s32_UiBorderWidth);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}
