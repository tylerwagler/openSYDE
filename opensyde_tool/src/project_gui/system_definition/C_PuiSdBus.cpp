//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI bus: stores additional UI information (implementation)

   UI bus: stores additional UI information.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_PuiSdBus.hpp"

#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Initialize default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdBus::C_PuiSdBus() :
   C_PuiBsLineBase()
{
   s32_UiWidthPixels = 6;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdBus::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                   this->c_UiColorMiddleLine.red(),
                                                   this->c_UiColorMiddleLine.green(),
                                                   this->c_UiColorMiddleLine.blue(),
                                                   this->c_UiColorMiddleLine.alpha());

   C_PuiBsLineBase::CalcHash(oru32_HashValue);
}
