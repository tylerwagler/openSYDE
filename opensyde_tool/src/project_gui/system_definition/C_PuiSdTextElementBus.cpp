//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI text element bus data: stores UI information (implementation)

   UI text element bus data: stores UI information

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_PuiSdTextElementBus.hpp"

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
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdTextElementBus::C_PuiSdTextElementBus() :
   C_PuiBsTextElement(),
   u32_BusIndex(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdTextElementBus::~C_PuiSdTextElementBus()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdTextElementBus::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                   this->u32_BusIndex);

   C_PuiBsTextElement::CalcHash(oru32_HashValue);
}
