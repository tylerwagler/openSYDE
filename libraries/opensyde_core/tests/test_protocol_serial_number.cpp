//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Characterisation tests for C_OscProtocolSerialNumber

   A device serial number identifies which ECU is being talked to and which one
   is about to be flashed. The class had no test coverage, and it carried a live
   defect: the C_SclString -> std::string migration replaced the hex-aware
   ToInt() with std::stoi(), which defaults to base 10 and stops at the 'x', so
   every byte parsed from a serial number string resolved to 0. Two different
   devices produced the same all-zero serial number.

   These tests pin the parsing and formatting behaviour so that regression cannot
   return, and so the phase 5 migration of protocol_drivers has a safety net to
   work against. protocol_drivers is ECU-facing and otherwise untested.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>
#include <system_error>
#include <vector>

#include <cstdint>
#include "C_OscErrorCategory.hpp"
#include "C_OscProtocolSerialNumber.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(ProtocolSerialNumber, DefaultConstructedIsInvalid)
{
   const C_OscProtocolSerialNumber c_Serial;

   EXPECT_FALSE(c_Serial.q_IsValid);

   // Recording actual behaviour, not endorsing it: an unset serial number still
   // renders as a plausible-looking "00.000000.0000" rather than an empty or
   // obviously-invalid string. Anything displaying a serial must therefore check
   // q_IsValid rather than trusting the formatted output to look wrong.
   EXPECT_EQ("00.000000.0000", c_Serial.GetSerialNumberAsPlainString());
}

TEST(ProtocolSerialNumber, PosSerialNumberRoundTrips)
{
   C_OscProtocolSerialNumber c_Serial;
   const uint8_t au8_Sn[6] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x12U};

   c_Serial.SetPosSerialNumber(au8_Sn);

   EXPECT_TRUE(c_Serial.q_IsValid);
   EXPECT_FALSE(c_Serial.q_FsnSerialNumber);
   for (uint8_t u8_Index = 0U; u8_Index < 6U; u8_Index++)
   {
      EXPECT_EQ(au8_Sn[u8_Index], c_Serial.au8_SerialNumber[u8_Index]) << "byte " << static_cast<int>(u8_Index);
   }
   EXPECT_NE("", c_Serial.GetSerialNumberAsFormattedString());
}

/// The regression that motivated these tests. A 12-character hex serial number
/// must parse to those bytes; under the defect every byte came out 0.
TEST(ProtocolSerialNumber, ExtSerialNumberParsesHexDigitsNotZeros)
{
   C_OscProtocolSerialNumber c_Serial;

   // manufacturer format 0 selects the old STW POS layout, parsed from 12 hex chars
   const std::error_code c_Result = c_Serial.SetExtSerialNumber(std::string("012345678912"), 0U);

   EXPECT_EQ(Errc::success, c_Result);
   EXPECT_TRUE(c_Serial.q_IsValid);

   const uint8_t au8_Expected[6] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x12U};
   for (uint8_t u8_Index = 0U; u8_Index < 6U; u8_Index++)
   {
      EXPECT_EQ(au8_Expected[u8_Index], c_Serial.au8_SerialNumber[u8_Index])
         << "byte " << static_cast<int>(u8_Index);
   }

   // the regression signature: every byte zero
   bool q_AllZero = true;
   for (uint8_t u8_Index = 0U; u8_Index < 6U; u8_Index++)
   {
      if (c_Serial.au8_SerialNumber[u8_Index] != 0U)
      {
         q_AllZero = false;
      }
   }
   EXPECT_FALSE(q_AllZero) << "serial number parsed as all zeros - hex parsing is broken again";
}

/// Under the defect, every serial number collapsed to the same all-zero value,
/// so two distinct devices compared equal. That is the dangerous consequence.
TEST(ProtocolSerialNumber, DistinctSerialNumbersDoNotCompareEqual)
{
   C_OscProtocolSerialNumber c_First;
   C_OscProtocolSerialNumber c_Second;

   EXPECT_EQ(Errc::success, c_First.SetExtSerialNumber(std::string("001122334455"), 0U));
   EXPECT_EQ(Errc::success, c_Second.SetExtSerialNumber(std::string("AABBCCDDEEFF"), 0U));

   EXPECT_FALSE(c_First == c_Second) << "two different devices reported the same serial number";
}

TEST(ProtocolSerialNumber, ParsesUppercaseAndLowercaseHex)
{
   C_OscProtocolSerialNumber c_Upper;
   C_OscProtocolSerialNumber c_Lower;

   EXPECT_EQ(Errc::success, c_Upper.SetExtSerialNumber(std::string("AABBCCDDEEFF"), 0U));
   EXPECT_EQ(Errc::success, c_Lower.SetExtSerialNumber(std::string("aabbccddeeff"), 0U));

   EXPECT_TRUE(c_Upper == c_Lower) << "hex case affected the parsed value";
   EXPECT_EQ(0xAAU, c_Upper.au8_SerialNumber[0]);
   EXPECT_EQ(0xFFU, c_Upper.au8_SerialNumber[5]);
}

TEST(ProtocolSerialNumber, ByteVectorOverloadRoundTrips)
{
   C_OscProtocolSerialNumber c_Serial;
   const std::vector<uint8_t> c_Bytes = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x12U};

   EXPECT_EQ(Errc::success, c_Serial.SetExtSerialNumber(c_Bytes, 0U));
   EXPECT_TRUE(c_Serial.q_IsValid);

   for (uint8_t u8_Index = 0U; u8_Index < 6U; u8_Index++)
   {
      EXPECT_EQ(c_Bytes[u8_Index], c_Serial.au8_SerialNumber[u8_Index]);
   }
}

/// Ordering is used to sort discovered devices; it must be a strict weak
/// ordering, not merely "not equal".
TEST(ProtocolSerialNumber, OrderingIsConsistent)
{
   C_OscProtocolSerialNumber c_Low;
   C_OscProtocolSerialNumber c_High;

   EXPECT_EQ(Errc::success, c_Low.SetExtSerialNumber(std::string("000000000001"), 0U));
   EXPECT_EQ(Errc::success, c_High.SetExtSerialNumber(std::string("000000000002"), 0U));

   EXPECT_TRUE(c_Low < c_High);
   EXPECT_FALSE(c_High < c_Low);
   EXPECT_FALSE(c_Low < c_Low) << "irreflexivity violated";
}

TEST(ProtocolSerialNumber, PlainStringIsNonEmptyForValidSerial)
{
   C_OscProtocolSerialNumber c_Serial;

   EXPECT_EQ(Errc::success, c_Serial.SetExtSerialNumber(std::string("0123456789AB"), 0U));

   const std::string c_Plain = c_Serial.GetSerialNumberAsPlainString();
   EXPECT_FALSE(c_Plain.empty());
   EXPECT_NE("000000000000", c_Plain) << "plain string shows an all-zero serial";
}

/// A manufacturer format other than 0 keeps the string form rather than parsing
/// it into the six POS bytes.
TEST(ProtocolSerialNumber, NonZeroManufacturerFormatKeepsStringForm)
{
   C_OscProtocolSerialNumber c_Serial;

   EXPECT_EQ(Errc::success, c_Serial.SetExtSerialNumber(std::string("SN-12345"), 1U));

   EXPECT_TRUE(c_Serial.q_IsValid);
   EXPECT_TRUE(c_Serial.q_FsnSerialNumber);
   EXPECT_EQ(1U, c_Serial.u8_SerialNumberManufacturerFormat);
   EXPECT_EQ("SN-12345", c_Serial.c_SerialNumberExt);
}
