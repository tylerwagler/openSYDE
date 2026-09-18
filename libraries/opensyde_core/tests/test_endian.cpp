//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for C_OscEndian, the one place the wire byte order lives
///
/// The protocol drivers used to reassemble every multi-byte value by hand, and a transposed
/// index in one of those sites is invisible to the compiler. These tests pin each helper to
/// a known byte pattern, in both directions, and check that the two orders really are
/// mirror images of each other.
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
///            Copyright 2026 Elytron Defense. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include "gtest/gtest.h"
#include "C_OscEndian.hpp"

using stw::opensyde_core::C_OscEndian;

namespace
{
//distinct bytes so a transposition shows up as a different value, not a coincidence
constexpr uint8_t mhau8_BYTES[8] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU};
}

TEST(Endian, BigEndianReadsMostSignificantByteFirst)
{
   EXPECT_EQ(0x0123U, C_OscEndian::h_GetU16Big(mhau8_BYTES));
   EXPECT_EQ(0x01234567U, C_OscEndian::h_GetU32Big(mhau8_BYTES));
   EXPECT_EQ(0x0123456789ABCDEFULL, C_OscEndian::h_GetU64Big(mhau8_BYTES));
}

TEST(Endian, LittleEndianReadsLeastSignificantByteFirst)
{
   EXPECT_EQ(0x2301U, C_OscEndian::h_GetU16Little(mhau8_BYTES));
   EXPECT_EQ(0x67452301U, C_OscEndian::h_GetU32Little(mhau8_BYTES));
   EXPECT_EQ(0xEFCDAB8967452301ULL, C_OscEndian::h_GetU64Little(mhau8_BYTES));
}

TEST(Endian, BigEndianWritesMostSignificantByteFirst)
{
   uint8_t au8_Out[8] = {0U};

   C_OscEndian::h_SetU16Big(0x0123U, au8_Out);
   EXPECT_EQ(0x01U, au8_Out[0]);
   EXPECT_EQ(0x23U, au8_Out[1]);

   C_OscEndian::h_SetU32Big(0x01234567U, au8_Out);
   EXPECT_EQ(0x01U, au8_Out[0]);
   EXPECT_EQ(0x67U, au8_Out[3]);

   C_OscEndian::h_SetU64Big(0x0123456789ABCDEFULL, au8_Out);
   for (uint32_t u32_Index = 0U; u32_Index < 8U; ++u32_Index)
   {
      EXPECT_EQ(mhau8_BYTES[u32_Index], au8_Out[u32_Index]) << u32_Index;
   }
}

TEST(Endian, LittleEndianWritesLeastSignificantByteFirst)
{
   uint8_t au8_Out[8] = {0U};

   C_OscEndian::h_SetU16Little(0x0123U, au8_Out);
   EXPECT_EQ(0x23U, au8_Out[0]);
   EXPECT_EQ(0x01U, au8_Out[1]);

   C_OscEndian::h_SetU32Little(0x01234567U, au8_Out);
   EXPECT_EQ(0x67U, au8_Out[0]);
   EXPECT_EQ(0x01U, au8_Out[3]);

   C_OscEndian::h_SetU64Little(0xEFCDAB8967452301ULL, au8_Out);
   for (uint32_t u32_Index = 0U; u32_Index < 8U; ++u32_Index)
   {
      EXPECT_EQ(mhau8_BYTES[u32_Index], au8_Out[u32_Index]) << u32_Index;
   }
}

TEST(Endian, SetThenGetRoundTripsEveryWidthInBothOrders)
{
   const uint64_t u64_Value = 0xFEDCBA9876543210ULL;
   uint8_t au8_Out[8] = {0U};

   C_OscEndian::h_SetU16Big(static_cast<uint16_t>(u64_Value), au8_Out);
   EXPECT_EQ(static_cast<uint16_t>(u64_Value), C_OscEndian::h_GetU16Big(au8_Out));
   C_OscEndian::h_SetU16Little(static_cast<uint16_t>(u64_Value), au8_Out);
   EXPECT_EQ(static_cast<uint16_t>(u64_Value), C_OscEndian::h_GetU16Little(au8_Out));

   C_OscEndian::h_SetU32Big(static_cast<uint32_t>(u64_Value), au8_Out);
   EXPECT_EQ(static_cast<uint32_t>(u64_Value), C_OscEndian::h_GetU32Big(au8_Out));
   C_OscEndian::h_SetU32Little(static_cast<uint32_t>(u64_Value), au8_Out);
   EXPECT_EQ(static_cast<uint32_t>(u64_Value), C_OscEndian::h_GetU32Little(au8_Out));

   C_OscEndian::h_SetU64Big(u64_Value, au8_Out);
   EXPECT_EQ(u64_Value, C_OscEndian::h_GetU64Big(au8_Out));
   C_OscEndian::h_SetU64Little(u64_Value, au8_Out);
   EXPECT_EQ(u64_Value, C_OscEndian::h_GetU64Little(au8_Out));
}

TEST(Endian, TheTwoOrdersAreMirrorImages)
{
   //Reading the same bytes in the other order must give the byte-swapped value. This is the
   //check that catches a helper that is "almost" right, such as a 64 bit read built from the
   //two 32 bit halves in the wrong order.
   const uint64_t u64_Big = C_OscEndian::h_GetU64Big(mhau8_BYTES);
   const uint64_t u64_Little = C_OscEndian::h_GetU64Little(mhau8_BYTES);

   uint8_t au8_Swapped[8] = {0U};
   C_OscEndian::h_SetU64Little(u64_Big, au8_Swapped);
   EXPECT_EQ(u64_Little, C_OscEndian::h_GetU64Big(au8_Swapped));
}

TEST(Endian, HelpersAreUsableAtCompileTime)
{
   constexpr uint8_t au8_Magic[4] = {0xDEU, 0xADU, 0xBEU, 0xEFU};
   static_assert(C_OscEndian::h_GetU32Big(au8_Magic) == 0xDEADBEEFU);
   static_assert(C_OscEndian::h_GetU32Little(au8_Magic) == 0xEFBEADDEU);
   static_assert(C_OscEndian::h_GetU16Big(au8_Magic) == 0xDEADU);
   SUCCEED();
}
