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

#include "C_SclChecksums.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
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
   c_UiBorderColor(mc_STYLE_GUIDE_COLOR_7),
   c_UiBackgroundColor(mc_STYLE_GUIDE_COLOR_12),
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
   int32_t s32_Value;

   s32_Value = this->c_UiBorderColor.red();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBorderColor.green();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBorderColor.blue();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBorderColor.alpha();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);

   s32_Value = this->c_UiBackgroundColor.red();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBackgroundColor.green();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBackgroundColor.blue();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);
   s32_Value = this->c_UiBackgroundColor.alpha();
   stw::scl::C_SclChecksums::CalcCRC32(&s32_Value, sizeof(s32_Value), oru32_HashValue);

   stw::scl::C_SclChecksums::CalcCRC32(&this->s32_UiBorderWidth, sizeof(this->s32_UiBorderWidth), oru32_HashValue);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}
