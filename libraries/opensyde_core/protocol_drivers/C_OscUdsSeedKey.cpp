//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Seed-to-key calculation for UDS SecurityAccess (0x27)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUdsSeedKey.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscUdsSeedKey::~C_OscUdsSeedKey(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   \param[in]  orc_Key   key to answer every seed with; the default is openSYDE's non-secure constant (23)
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscUdsSeedKeyConstant::C_OscUdsSeedKeyConstant(const std::vector<uint8_t> & orc_Key) :
   mc_Key(orc_Key)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The constant key, whatever the seed

   \param[in]   ou8_Level   ignored
   \param[in]   orc_Seed    ignored
   \param[out]  orc_Key     the constant

   \return
   Errc::success
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscUdsSeedKeyConstant::CalculateKey(const uint8_t ou8_Level, const std::vector<uint8_t> & orc_Seed,
                                                      std::vector<uint8_t> & orc_Key) const
{
   (void)ou8_Level;
   (void)orc_Seed;
   orc_Key = mc_Key;
   return Errc::success;
}
