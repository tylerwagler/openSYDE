//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscApplicationInfoBlock::ParseFromBLOB

   The application information block is the record a firmware image writes into
   its own hex file to say what it is: device ID, project name, project version,
   build date. openSYDE reads it to decide what is about to be flashed onto an
   ECU, so a block that parses into fabricated content is not a cosmetic problem.

   The parser reads a fixed-layout blob whose length is whatever the hex file
   happened to supply, which makes its bounds arithmetic the interesting part.
   These tests cover the accepted shapes and, in particular, the truncated ones.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <system_error>
#include <vector>

#include <cstdint>
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscApplicationInfoBlock.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

namespace
{
/// Build a V2 block header: 8-byte magic "Lx_?zg." + NUL, version, content map.
std::vector<uint8_t> h_MakeV2Header(const uint8_t ou8_ContentMap)
{
   std::vector<uint8_t> c_Data;

   for (uint8_t u8_Index = 0U; u8_Index < APPLICATION_INFO_MAGIC_LENGTH_V2; u8_Index++)
   {
      c_Data.push_back(static_cast<uint8_t>(APPLICATION_INFO_MAGIC_V2[u8_Index]));
   }
   c_Data.push_back(2U);              //struct version
   c_Data.push_back(ou8_ContentMap);
   return c_Data;
}

/// Build a V1 block header: 7-byte magic "Lx_?z2.", version, content map.
std::vector<uint8_t> h_MakeV1Header(const uint8_t ou8_ContentMap)
{
   std::vector<uint8_t> c_Data;

   for (uint8_t u8_Index = 0U; u8_Index < APPLICATION_INFO_MAGIC_LENGTH_V1; u8_Index++)
   {
      c_Data.push_back(static_cast<uint8_t>(APPLICATION_INFO_MAGIC_V1[u8_Index]));
   }
   c_Data.push_back(1U);              //struct version
   c_Data.push_back(ou8_ContentMap);
   return c_Data;
}
}

TEST(ApplicationInfoBlock, RejectsBlobShorterThanHeader)
{
   C_OscApplicationInfoBlock c_Block;
   const std::vector<uint8_t> c_Data = h_MakeV2Header(0x00U);

   for (uint16_t u16_Length = 0U; u16_Length < 9U; u16_Length++)
   {
      EXPECT_EQ(Errc::overflow, c_Block.ParseFromBLOB(c_Data.data(), u16_Length))
         << "accepted a " << u16_Length << "-byte blob";
   }
}

TEST(ApplicationInfoBlock, RejectsUnknownMagic)
{
   C_OscApplicationInfoBlock c_Block;
   std::vector<uint8_t> c_Data = h_MakeV2Header(0x00U);

   c_Data[0] = 'X';
   EXPECT_EQ(Errc::config, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
}

TEST(ApplicationInfoBlock, RejectsUnknownStructVersion)
{
   C_OscApplicationInfoBlock c_Block;
   std::vector<uint8_t> c_Data = h_MakeV2Header(0x00U);

   c_Data[APPLICATION_INFO_MAGIC_LENGTH_V2] = 9U; //not 1, 2 or 3
   EXPECT_EQ(Errc::config, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
}

/// An empty content map means no optional field is present, so the header alone
/// is a complete block.
TEST(ApplicationInfoBlock, ParsesHeaderOnlyBlockWithEmptyContentMap)
{
   C_OscApplicationInfoBlock c_Block;
   const std::vector<uint8_t> c_Data = h_MakeV2Header(0x00U);

   ASSERT_EQ(Errc::success, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
   EXPECT_EQ(2U, c_Block.u8_StructVersion);
   EXPECT_FALSE(c_Block.ContainsDeviceID());
   EXPECT_FALSE(c_Block.ContainsProjectName());
   EXPECT_EQ("", c_Block.GetDeviceID());
}

TEST(ApplicationInfoBlock, ParsesDeviceIdWhenPresent)
{
   C_OscApplicationInfoBlock c_Block;
   std::vector<uint8_t> c_Data = h_MakeV2Header(0x01U); //bit0 = device ID
   const std::string c_DeviceId = "ESX-4CS";

   //V2 device ID field is 17 bytes, space padded
   for (uint8_t u8_Index = 0U; u8_Index < 17U; u8_Index++)
   {
      c_Data.push_back((u8_Index < c_DeviceId.size()) ? static_cast<uint8_t>(c_DeviceId[u8_Index]) : 0x20U);
   }

   //Every length check in m_ParsePayload is "remaining <= field length", so a block that ends exactly on the
   //last byte of its final field is rejected - one spare byte is required. That is inherited from upstream
   //(CXFLECUInformation has the identical comparisons), so it is recorded here rather than treated as a defect.
   EXPECT_EQ(Errc::overflow, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));

   c_Data.push_back(0x00U);
   ASSERT_EQ(Errc::success, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
   EXPECT_TRUE(c_Block.ContainsDeviceID());
   EXPECT_EQ(c_DeviceId, c_Block.GetDeviceID());
}

/// A field the content map promises but the blob does not carry must be
/// rejected, not filled from whatever follows in the buffer.
TEST(ApplicationInfoBlock, RejectsTruncatedDeviceIdField)
{
   C_OscApplicationInfoBlock c_Block;
   std::vector<uint8_t> c_Data = h_MakeV2Header(0x01U); //promises a device ID

   //only 5 of the 17 device ID bytes are actually present
   for (uint8_t u8_Index = 0U; u8_Index < 5U; u8_Index++)
   {
      c_Data.push_back(static_cast<uint8_t>('A'));
   }

   EXPECT_EQ(Errc::overflow, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
}

TEST(ApplicationInfoBlock, ParsesV1Header)
{
   C_OscApplicationInfoBlock c_Block;
   const std::vector<uint8_t> c_Data = h_MakeV1Header(0x00U);

   ASSERT_EQ(Errc::success, c_Block.ParseFromBLOB(c_Data.data(), static_cast<uint16_t>(c_Data.size())));
   EXPECT_EQ(1U, c_Block.u8_StructVersion);
}

/// The V2 header is 10 bytes: 8 magic + 1 version + 1 content map. The length
/// guard only requires 9 (correct for V1, whose magic is a byte shorter), so a
/// 9-byte V2 blob reaches the content map read with nothing left to read.
///
/// The index is then 10 against 9 bytes available, and the remaining length is
/// computed as an unsigned subtraction: 9 - 10 wraps to 65535. Every bounds
/// check in m_ParsePayload compares against that, so all of them pass, and the
/// payload fields are filled from whatever lies past the end of the data.
///
/// In C_OscHexFile that buffer is 365 bytes reused across the whole address
/// scan, memset only over the bytes actually read - so the bytes past the end
/// are the previous iteration's contents, and the block parses "successfully"
/// into a device ID and project name that were never in this firmware image.
TEST(ApplicationInfoBlock, RejectsV2BlobTruncatedBeforeContentMap)
{
   C_OscApplicationInfoBlock c_Block;

   //10 header bytes plus a payload of recognisable filler, mimicking the reused
   //buffer in C_OscHexFile that still holds the previous block's bytes
   std::vector<uint8_t> c_Data = h_MakeV2Header(0x01U);
   c_Data.resize(200U, static_cast<uint8_t>('Z'));

   //but the hex file only actually supplied 9 bytes
   const std::error_code c_Result = c_Block.ParseFromBLOB(c_Data.data(), 9U);

   EXPECT_EQ(Errc::overflow, c_Result)
      << "a 9-byte V2 blob was accepted; the content map and payload came from past the end of the data";
   EXPECT_NE("ZZZZZZZZZZZZZZZZZ", c_Block.GetDeviceID()) << "device ID was filled from beyond the supplied data";
}

/// V3 carries no content map byte, so 9 bytes is a complete V3 header. It must
/// stay accepted - the fix for the V2 case must not raise the bar for V3.
TEST(ApplicationInfoBlock, V3HeaderOfNineBytesStillRejectsMissingPayload)
{
   C_OscApplicationInfoBlock c_Block;
   std::vector<uint8_t> c_Data;

   for (uint8_t u8_Index = 0U; u8_Index < APPLICATION_INFO_MAGIC_LENGTH_V2; u8_Index++)
   {
      c_Data.push_back(static_cast<uint8_t>(APPLICATION_INFO_MAGIC_V2[u8_Index]));
   }
   c_Data.push_back(3U); //struct version 3: all fields mandatory, no content map byte
   c_Data.resize(200U, static_cast<uint8_t>('Z'));

   //V3 declares every field mandatory, and none of them are present in 9 bytes
   EXPECT_EQ(Errc::overflow, c_Block.ParseFromBLOB(c_Data.data(), 9U));
   EXPECT_NE("ZZZZZZZZZZZZZZZZZ", c_Block.GetDeviceID());
}
