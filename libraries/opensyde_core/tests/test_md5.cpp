//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_Md5Checksum

   The MD5 implementation is homegrown and had no test coverage at all. These pin it
   against the published RFC 1321 test-suite vectors, and cover the three entry points
   (buffer, FILE *, path) plus the documented "empty string on problems" contract.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <vector>

#include <cstdint>
#include "C_Md5Checksum.hpp"
#include "TglFile.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::md5;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace
{
std::string h_Md5OfString(const std::string & orc_Text)
{
   return C_Md5Checksum::GetMD5(reinterpret_cast<const uint8_t *>(orc_Text.data()),
                                static_cast<uint32_t>(orc_Text.size()));
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   RFC 1321 appendix A.5 test suite -- the canonical MD5 vectors
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(Md5Checksum, MatchesRfc1321TestSuite)
{
   EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", h_Md5OfString(""));
   EXPECT_EQ("0cc175b9c0f1b6a831c399e269772661", h_Md5OfString("a"));
   EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", h_Md5OfString("abc"));
   EXPECT_EQ("f96b697d7cb7938d525a2f31aaf161d0", h_Md5OfString("message digest"));
   EXPECT_EQ("c3fcd3d76192e4007dfb496cca67e13b", h_Md5OfString("abcdefghijklmnopqrstuvwxyz"));
   EXPECT_EQ("d174ab98d277d9f5a5611c2c9f419d9f",
             h_Md5OfString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
   EXPECT_EQ("57edf4a22be3c955ac49da2e2107b67a",
             h_Md5OfString("1234567890123456789012345678901234567890"
                           "1234567890123456789012345678901234567890"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Inputs that straddle the 64-byte block boundary the implementation buffers on
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(Md5Checksum, HandlesBlockBoundaries)
{
   //55/56/64 bytes bracket the point where the length padding no longer fits in the final block
   EXPECT_EQ("ef1772b6dff9a122358552954ad0df65", h_Md5OfString(std::string(55U, 'a')));
   EXPECT_EQ("3b0c8ac703f828b04c6c197006d17218", h_Md5OfString(std::string(56U, 'a')));
   EXPECT_EQ("014842d480b571495a4a0363793f7367", h_Md5OfString(std::string(64U, 'a')));
   //larger than the 4096-byte read buffer used by the FILE * overload
   EXPECT_EQ("0d0c9c4db6953fee9e03f528cafd7d3e", h_Md5OfString(std::string(10000U, 'a')));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The FILE * and path overloads agree with the buffer overload
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(Md5Checksum, FileOverloadsMatchBufferOverload)
{
   //deliberately longer than the 4096-byte block loop so more than one fread iteration runs
   const std::string c_Content(9000U, 'x');
   const std::string c_Path = "test_md5_sample.bin";

   {
      std::FILE * const pc_Out = std::fopen(c_Path.c_str(), "wb");
      ASSERT_NE(pc_Out, nullptr);
      ASSERT_EQ(c_Content.size(), std::fwrite(c_Content.data(), 1U, c_Content.size(), pc_Out));
      (void)std::fclose(pc_Out);
   }

   const std::string c_Expected = h_Md5OfString(c_Content);
   EXPECT_EQ(32U, c_Expected.size());

   EXPECT_EQ(c_Expected, C_Md5Checksum::GetMD5(c_Path));

   std::FILE * const pc_In = std::fopen(c_Path.c_str(), "rb");
   ASSERT_NE(pc_In, nullptr);
   EXPECT_EQ(c_Expected, C_Md5Checksum::GetMD5(pc_In));
   (void)std::fclose(pc_In);

   (void)std::remove(c_Path.c_str());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Failure paths return the documented empty string rather than a plausible-looking digest

   The header documents "empty string if there are problems". A digest is indistinguishable
   from a good one by inspection, so returning one on failure is worse than returning nothing.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(Md5Checksum, FailuresReturnEmptyString)
{
   EXPECT_EQ("", C_Md5Checksum::GetMD5(static_cast<std::FILE *>(nullptr)));
   EXPECT_EQ("", C_Md5Checksum::GetMD5("no_such_file_hopefully_9d8c7b6a.bin"));
}
