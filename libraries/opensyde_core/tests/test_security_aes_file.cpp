//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for AES file encryption

   Covers the AES-256-GCM file format that replaced AES-128-ECB.

   The previous implementation had three defects these tests pin against: the key
   was derived by a single unsalted MD5 (so the same password always produced the
   same key, and the keyspace was precomputable), ECB leaked plaintext structure
   because identical blocks encrypt identically, and nothing authenticated the
   ciphertext so tampering went undetected.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <cstdint>
#include "C_OscSecurityAesFile.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace
{
/// Write a file with the given bytes.
void mh_WriteFile(const std::string & orc_Path, const std::vector<uint8_t> & orc_Data)
{
   std::ofstream c_Stream(orc_Path.c_str(), std::ofstream::binary | std::ofstream::trunc);

   if (orc_Data.size() > 0U)
   {
      c_Stream.write(reinterpret_cast<const char *>(&orc_Data[0]), static_cast<std::streamsize>(orc_Data.size()));
   }
   c_Stream.close();
}

/// Read a whole file.
std::vector<uint8_t> mh_ReadFile(const std::string & orc_Path)
{
   std::ifstream c_Stream(orc_Path.c_str(), std::ifstream::binary);
   std::vector<uint8_t> c_Data((std::istreambuf_iterator<char>(c_Stream)), std::istreambuf_iterator<char>());

   return c_Data;
}

/// Plaintext with many identical 16-byte blocks. Under ECB the ciphertext of
/// these repeats identically, which is the leak GCM removes.
std::vector<uint8_t> mh_RepetitivePlaintext(void)
{
   std::vector<uint8_t> c_Data;

   for (uint32_t u32_Block = 0U; u32_Block < 64U; u32_Block++)
   {
      for (uint32_t u32_Byte = 0U; u32_Byte < 16U; u32_Byte++)
      {
         c_Data.push_back(0xA5U);
      }
   }
   return c_Data;
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(SecurityAesFile, RoundTripRecoversPlaintext)
{
   const std::string c_In = "aes_rt_in.bin";
   const std::string c_Enc = "aes_rt_enc.bin";
   const std::string c_Out = "aes_rt_out.bin";
   const std::vector<uint8_t> c_Plain = mh_RepetitivePlaintext();

   mh_WriteFile(c_In, c_Plain);

   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("correct horse", c_In, c_Enc)));
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_DecryptFile("correct horse", c_Enc, c_Out)));

   EXPECT_EQ(c_Plain, mh_ReadFile(c_Out));

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
   (void)std::remove(c_Out.c_str());
}

TEST(SecurityAesFile, WrongPasswordIsRejected)
{
   const std::string c_In = "aes_wp_in.bin";
   const std::string c_Enc = "aes_wp_enc.bin";
   const std::string c_Out = "aes_wp_out.bin";

   mh_WriteFile(c_In, mh_RepetitivePlaintext());
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("right password", c_In, c_Enc)));

   // Must fail, not silently emit garbage. Under the all-zero-key defect any
   // password decrypted any file.
   EXPECT_TRUE(static_cast<bool>(C_OscSecurityAesFile::h_DecryptFile("wrong password", c_Enc, c_Out)));

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
   (void)std::remove(c_Out.c_str());
}

TEST(SecurityAesFile, TamperedCiphertextIsDetected)
{
   const std::string c_In = "aes_tp_in.bin";
   const std::string c_Enc = "aes_tp_enc.bin";
   const std::string c_Out = "aes_tp_out.bin";

   mh_WriteFile(c_In, mh_RepetitivePlaintext());
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("pw", c_In, c_Enc)));

   // Flip one bit in the ciphertext body.
   std::vector<uint8_t> c_Cipher = mh_ReadFile(c_Enc);
   ASSERT_GT(c_Cipher.size(), C_OscSecurityAesFile::hu32_HEADER_LENGTH);
   c_Cipher[C_OscSecurityAesFile::hu32_HEADER_LENGTH] ^= 0x01U;
   mh_WriteFile(c_Enc, c_Cipher);

   // GCM authenticates, so this must fail. ECB had no integrity at all.
   EXPECT_TRUE(static_cast<bool>(C_OscSecurityAesFile::h_DecryptFile("pw", c_Enc, c_Out)));

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
   (void)std::remove(c_Out.c_str());
}

TEST(SecurityAesFile, HeaderIsWellFormed)
{
   const std::string c_In = "aes_hd_in.bin";
   const std::string c_Enc = "aes_hd_enc.bin";

   mh_WriteFile(c_In, mh_RepetitivePlaintext());
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("pw", c_In, c_Enc)));

   const std::vector<uint8_t> c_Cipher = mh_ReadFile(c_Enc);
   ASSERT_GE(c_Cipher.size(), C_OscSecurityAesFile::hu32_HEADER_LENGTH);

   EXPECT_EQ('O', c_Cipher[0]);
   EXPECT_EQ('S', c_Cipher[1]);
   EXPECT_EQ('Y', c_Cipher[2]);
   EXPECT_EQ('E', c_Cipher[3]);
   EXPECT_EQ('N', c_Cipher[4]);
   EXPECT_EQ('C', c_Cipher[5]);
   EXPECT_EQ(C_OscSecurityAesFile::hu8_FORMAT_VERSION, c_Cipher[6]);
   EXPECT_EQ(C_OscSecurityAesFile::hu8_ALGO_AES256_GCM, c_Cipher[7]);

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
}

/// A file without the magic — including anything written by the previous
/// headerless ECB format — must be refused rather than mis-decrypted.
TEST(SecurityAesFile, LegacyOrGarbageFileIsRejected)
{
   const std::string c_Enc = "aes_lg_enc.bin";
   const std::string c_Out = "aes_lg_out.bin";
   std::vector<uint8_t> c_Garbage(128U, 0x00U);

   mh_WriteFile(c_Enc, c_Garbage);
   EXPECT_TRUE(static_cast<bool>(C_OscSecurityAesFile::h_DecryptFile("pw", c_Enc, c_Out)));

   (void)std::remove(c_Enc.c_str());
   (void)std::remove(c_Out.c_str());
}

/// The core property ECB lacked: repeated plaintext blocks must not produce
/// repeated ciphertext blocks.
TEST(SecurityAesFile, RepeatedPlaintextDoesNotRepeatInCiphertext)
{
   const std::string c_In = "aes_ecb_in.bin";
   const std::string c_Enc = "aes_ecb_enc.bin";

   mh_WriteFile(c_In, mh_RepetitivePlaintext());
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("pw", c_In, c_Enc)));

   const std::vector<uint8_t> c_Cipher = mh_ReadFile(c_Enc);
   ASSERT_GT(c_Cipher.size(), C_OscSecurityAesFile::hu32_HEADER_LENGTH + 32U);

   // Compare the first two 16-byte ciphertext blocks; identical plaintext blocks
   // produced identical ciphertext under ECB.
   bool q_BlocksIdentical = true;
   for (uint32_t u32_Byte = 0U; u32_Byte < 16U; u32_Byte++)
   {
      const size_t un_First = C_OscSecurityAesFile::hu32_HEADER_LENGTH + u32_Byte;
      const size_t un_Second = C_OscSecurityAesFile::hu32_HEADER_LENGTH + 16U + u32_Byte;
      if (c_Cipher[un_First] != c_Cipher[un_Second])
      {
         q_BlocksIdentical = false;
      }
   }
   EXPECT_FALSE(q_BlocksIdentical) << "identical plaintext blocks produced identical ciphertext (ECB behaviour)";

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
}

/// Same password, same plaintext, two files: the random salt and nonce must make
/// the ciphertext differ. Under the unsalted MD5 scheme it was byte-identical.
TEST(SecurityAesFile, SamePasswordProducesDifferentCiphertext)
{
   const std::string c_In = "aes_sp_in.bin";
   const std::string c_EncA = "aes_sp_a.bin";
   const std::string c_EncB = "aes_sp_b.bin";

   mh_WriteFile(c_In, mh_RepetitivePlaintext());
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("same pw", c_In, c_EncA)));
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("same pw", c_In, c_EncB)));

   EXPECT_NE(mh_ReadFile(c_EncA), mh_ReadFile(c_EncB));

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_EncA.c_str());
   (void)std::remove(c_EncB.c_str());
}

TEST(SecurityAesFile, EmptyInputRoundTrips)
{
   const std::string c_In = "aes_mt_in.bin";
   const std::string c_Enc = "aes_mt_enc.bin";
   const std::string c_Out = "aes_mt_out.bin";

   mh_WriteFile(c_In, std::vector<uint8_t>());

   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_EncryptFile("pw", c_In, c_Enc)));
   EXPECT_FALSE(static_cast<bool>(C_OscSecurityAesFile::h_DecryptFile("pw", c_Enc, c_Out)));

   EXPECT_EQ(0U, mh_ReadFile(c_Out).size());

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Enc.c_str());
   (void)std::remove(c_Out.c_str());
}
