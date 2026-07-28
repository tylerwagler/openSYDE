#include <cstring>
#include <cstdint>
#include "gtest/gtest.h"
#include "C_SclChecksums.hpp"

// KAT vectors verified against CRC-32/ISO-HDLC and CRC-16/CCITT:
//   "123456789" → CRC-32: 0xCBF43926
//   "123456789" → CRC-16/CCITT (init=0xFFFF): 0x29B1
//   "123456789" → CRC-16/CCITT (init=0x0000): 0x31C3

static const char * const kac_TestString = "123456789";
static const uint32_t ku32_TestStringLen = 9;

TEST(Checksums, CalcCRC32_CheckValue)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32(kac_TestString, ku32_TestStringLen, u32_Crc);
   u32_Crc ^= 0xFFFFFFFFU;
   EXPECT_EQ(0xCBF43926U, u32_Crc);
}

TEST(Checksums, CalcCRC32_Empty)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32("", 0, u32_Crc);
   u32_Crc ^= 0xFFFFFFFFU;
   EXPECT_EQ(0x00000000U, u32_Crc);
}

TEST(Checksums, CalcCRC32_Incremental)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32(kac_TestString, 4, u32_Crc);
   stw::scl::C_SclChecksums::CalcCRC32(kac_TestString + 4, 5, u32_Crc);
   u32_Crc ^= 0xFFFFFFFFU;
   EXPECT_EQ(0xCBF43926U, u32_Crc);
}

TEST(Checksums, CalcCRC16_CheckValue_InitZero)
{
   uint16_t u16_Crc = 0x0000U;
   stw::scl::C_SclChecksums::CalcCRC16(kac_TestString, ku32_TestStringLen, u16_Crc);
   EXPECT_EQ(0x31C3U, u16_Crc);
}

TEST(Checksums, CalcCRC16_Empty)
{
   uint16_t u16_Crc = 0x0000U;
   stw::scl::C_SclChecksums::CalcCRC16("", 0, u16_Crc);
   EXPECT_EQ(0x0000U, u16_Crc);
}

TEST(Checksums, CalcCRC16_CCITTInit)
{
   uint16_t u16_Crc = 0xFFFFU;
   stw::scl::C_SclChecksums::CalcCRC16(kac_TestString, ku32_TestStringLen, u16_Crc);
   EXPECT_EQ(0x29B1U, u16_Crc);
}

TEST(Checksums, CalcCRC16_Incremental)
{
   uint16_t u16_Crc = 0x0000U;
   stw::scl::C_SclChecksums::CalcCRC16(kac_TestString, 4, u16_Crc);
   stw::scl::C_SclChecksums::CalcCRC16(kac_TestString + 4, 5, u16_Crc);
   EXPECT_EQ(0x31C3U, u16_Crc);
}

TEST(Checksums, CalcCRC16STW_Empty)
{
   uint16_t u16_Crc = 0x0000U;
   stw::scl::C_SclChecksums::CalcCRC16STW("", 0, u16_Crc);
   EXPECT_EQ(0x0000U, u16_Crc);
}

TEST(Checksums, CalcCRC16STW_NonZeroInit)
{
   uint16_t u16_Crc = 0xFFFFU;
   stw::scl::C_SclChecksums::CalcCRC16STW(kac_TestString, ku32_TestStringLen, u16_Crc);
   EXPECT_NE(0x0000U, u16_Crc);
}

TEST(Checksums, CalcCRC32TriCore_Unaligned_ReturnsError)
{
   const uint8_t pu8_Data[] = { 1, 2, 3, 4, 5 };
   uint32_t u32_Crc = 0x00000000U;
   EXPECT_EQ(-1, stw::scl::C_SclChecksums::CalcCRC32TriCore(pu8_Data, 5, u32_Crc));
}

TEST(Checksums, CalcCRC32TriCore_ZeroBytes)
{
   uint32_t u32_Crc = 0x00000000U;
   EXPECT_EQ(0, stw::scl::C_SclChecksums::CalcCRC32TriCore("", 0, u32_Crc));
   EXPECT_EQ(0x00000000U, u32_Crc);
}

TEST(Checksums, CalcCRC32TriCore_Incremental)
{
   const uint32_t au32_Data[] = { 0x12345678U, 0x9ABCDEF0U };
   uint32_t u32_CrcFull = 0x00000000U;
   stw::scl::C_SclChecksums::CalcCRC32TriCore(au32_Data, 8, u32_CrcFull);

   uint32_t u32_CrcInc = 0x00000000U;
   stw::scl::C_SclChecksums::CalcCRC32TriCore(au32_Data, 4, u32_CrcInc);
   stw::scl::C_SclChecksums::CalcCRC32TriCore(au32_Data + 1, 4, u32_CrcInc);
   EXPECT_EQ(u32_CrcFull, u32_CrcInc);
}

TEST(Checksums, NullPointer_DoesNotCrash)
{
   uint32_t u32_Crc32 = 0xFFFFFFFFU;
   uint16_t u16_Crc16 = 0x0000U;
   EXPECT_NO_FATAL_FAILURE(stw::scl::C_SclChecksums::CalcCRC32(nullptr, 0, u32_Crc32));
   EXPECT_NO_FATAL_FAILURE(stw::scl::C_SclChecksums::CalcCRC16(nullptr, 0, u16_Crc16));
   EXPECT_NO_FATAL_FAILURE(stw::scl::C_SclChecksums::CalcCRC16STW(nullptr, 0, u16_Crc16));
}
