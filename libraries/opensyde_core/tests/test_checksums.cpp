#include <cstring>
#include <cstddef>
#include <cstdint>
#include <vector>
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

// CRC-32C (Castagnoli): init 0xFFFFFFFF, NO final XOR -- the CalcCRC32C convention
// that matches the SSE4.2 _mm_crc32 primitive.
TEST(Checksums, CalcCRC32C_CheckValue)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32C(kac_TestString, ku32_TestStringLen, u32_Crc);
   EXPECT_EQ(0x1CF96D7CU, u32_Crc);
}

TEST(Checksums, CalcCRC32C_Empty)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32C("", 0, u32_Crc);
   EXPECT_EQ(0xFFFFFFFFU, u32_Crc);
}

TEST(Checksums, CalcCRC32C_Incremental)
{
   uint32_t u32_Crc = 0xFFFFFFFFU;
   stw::scl::C_SclChecksums::CalcCRC32C(kac_TestString, 4, u32_Crc);
   stw::scl::C_SclChecksums::CalcCRC32C(kac_TestString + 4, 5, u32_Crc);
   EXPECT_EQ(0x1CF96D7CU, u32_Crc);
}

namespace
{
//Independent reference CRC-32C so HW==SW==expected is checked regardless of path.
uint32_t mh_ReferenceCRC32C(const void * const opv_Data, const uint32_t ou32_Len, uint32_t u32_Crc)
{
   const uint8_t * const pu8_Data = static_cast<const uint8_t *>(opv_Data);
   uint32_t au32_Table[256];
   for (uint32_t u32_I = 0U; u32_I < 256U; u32_I++)
   {
      uint32_t u32_C = u32_I;
      for (uint32_t u32_Bit = 0U; u32_Bit < 8U; u32_Bit++)
      {
         u32_C = (u32_C & 1U) ? ((u32_C >> 1U) ^ 0x82F63B78U) : (u32_C >> 1U);
      }
      au32_Table[u32_I] = u32_C;
   }
   for (uint32_t u32_I = 0U; u32_I < ou32_Len; u32_I++)
   {
      u32_Crc = (u32_Crc >> 8U) ^ au32_Table[(u32_Crc ^ pu8_Data[u32_I]) & 0xFFU];
   }
   return u32_Crc;
}
}

TEST(Checksums, CalcCRC32C_MatchesReference)
{
   std::vector<uint8_t> c_Buffer(1024);
   for (std::size_t u32_I = 0U; u32_I < c_Buffer.size(); u32_I++)
   {
      c_Buffer[u32_I] = static_cast<uint8_t>((u32_I * 37U) & 0xFFU);
   }
   for (const uint32_t u32_Len : {0U, 1U, 4U, 5U, 16U, 63U, 64U, 1024U})
   {
      uint32_t u32_Crc = 0xFFFFFFFFU;
      stw::scl::C_SclChecksums::CalcCRC32C(c_Buffer.data(), u32_Len, u32_Crc);
      EXPECT_EQ(mh_ReferenceCRC32C(c_Buffer.data(), u32_Len, 0xFFFFFFFFU), u32_Crc) << "len=" << u32_Len;
   }
}

