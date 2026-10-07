//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the UDS negative response code table

   Three copies of this table used to exist (the openSYDE driver, the CAN Monitor's openSYDE interpreter and its
   UDS interpreter) and their wording had drifted. These tests pin the one that replaced them to the ISO 14229-1
   Table A.1 spelling so a future edit cannot quietly reintroduce a variant.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "C_OscUdsNrc.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(UdsNrc, UsesIsoSpellingIncludingHyphens)
{
   //These are the ones the three old tables disagreed on
   EXPECT_EQ("positiveResponse", C_OscUdsNrc::h_ToText(C_OscUdsNrc::hu8_POSITIVE_RESPONSE));
   EXPECT_EQ("sub-functionNotSupported", C_OscUdsNrc::h_ToText(C_OscUdsNrc::hu8_SUB_FUNCTION_NOT_SUPPORTED));
   EXPECT_EQ("requestCorrectlyReceived-ResponsePending",
             C_OscUdsNrc::h_ToText(C_OscUdsNrc::hu8_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING));
   EXPECT_EQ("exceedNumberOfAttempts", C_OscUdsNrc::h_ToText(C_OscUdsNrc::hu8_EXCEED_NUMBER_OF_ATTEMPTS));
   EXPECT_EQ("sub-functionNotSupportedInActiveSession",
             C_OscUdsNrc::h_ToText(C_OscUdsNrc::hu8_SUB_FUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION));
}

TEST(UdsNrc, CodesTheDriverReactsToKeepTheirValues)
{
   //The update sequences and the flash driver compare against these; the values are wire protocol
   EXPECT_EQ(0x11U, C_OscUdsNrc::hu8_SERVICE_NOT_SUPPORTED);
   EXPECT_EQ(0x13U, C_OscUdsNrc::hu8_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT);
   EXPECT_EQ(0x22U, C_OscUdsNrc::hu8_CONDITIONS_NOT_CORRECT);
   EXPECT_EQ(0x24U, C_OscUdsNrc::hu8_REQUEST_SEQUENCE_ERROR);
   EXPECT_EQ(0x31U, C_OscUdsNrc::hu8_REQUEST_OUT_OF_RANGE);
   EXPECT_EQ(0x35U, C_OscUdsNrc::hu8_INVALID_KEY);
   EXPECT_EQ(0x37U, C_OscUdsNrc::hu8_REQUIRED_TIME_DELAY_NOT_EXPIRED);
   EXPECT_EQ(0x39U, C_OscUdsNrc::hu8_SECURE_DATA_TRANSMISSION_NOT_ALLOWED);
   EXPECT_EQ(0x70U, C_OscUdsNrc::hu8_UPLOAD_DOWNLOAD_NOT_ACCEPTED);
   EXPECT_EQ(0x72U, C_OscUdsNrc::hu8_GENERAL_PROGRAMMING_FAILURE);
   EXPECT_EQ(0x78U, C_OscUdsNrc::hu8_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING);
}

TEST(UdsNrc, UnknownCodeShowsTheHexValue)
{
   EXPECT_EQ("unknownNegativeResponseCode(0x4F)", C_OscUdsNrc::h_ToText(0x4FU));
   EXPECT_EQ("unknownNegativeResponseCode(0xFF)", C_OscUdsNrc::h_ToText(0xFFU));
   EXPECT_EQ("unknownNegativeResponseCode(0x01)", C_OscUdsNrc::h_ToText(0x01U));
}

TEST(UdsNrc, EveryDefinedCodeHasAName)
{
   //Walk the whole byte range: anything named must not fall through to the unknown form, and there must be no
   //accidental duplicate value in the constants (which a search-by-value would silently resolve to the first)
   uint32_t u32_Named = 0U;
   for (uint32_t u32_Code = 0U; u32_Code <= 0xFFU; ++u32_Code)
   {
      const std::string c_Text = C_OscUdsNrc::h_ToText(static_cast<uint8_t>(u32_Code));
      if (c_Text.rfind("unknownNegativeResponseCode", 0U) != 0U)
      {
         ++u32_Named;
      }
   }
   //ISO 14229-1 Table A.1 as carried here: 46 codes
   EXPECT_EQ(46U, u32_Named);
}
