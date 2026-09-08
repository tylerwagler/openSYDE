//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Characterisation tests for C_HexFile

   C_HexFile parses the Intel HEX images that get flashed onto ECUs. It is ~2500
   lines of pointer-heavy C-style code built around a hand-rolled doubly-linked
   ring buffer, and it had no test coverage at all.

   These tests pin the observable behaviour of the public API — what it accepts,
   what it rejects, and what it reports — so that the two changes the remediation
   plan wants here can be made safely: migrating the uint32_t error returns to
   std::error_code (phase 5), and replacing the T_HexLine ring buffer with a
   standard container (phase 6.3). Neither should be attempted against untested
   code.

   They are deliberately characterisation tests: they record what the parser does
   today, not what it ideally ought to do.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "stwtypes.hpp"
#include "C_HexFile.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::hex_file;

namespace
{
/* -- Module Global Functions --------------------------------------------------------------------------------------- */

/// Write text content to a scratch file and return its path.
std::string mh_WriteHex(const std::string & orc_Name, const std::string & orc_Content)
{
   std::ofstream c_Stream(orc_Name.c_str(), std::ofstream::binary | std::ofstream::trunc);

   c_Stream << orc_Content;
   c_Stream.close();
   return orc_Name;
}

/// Minimal valid Intel HEX: 4 bytes at 0x0000, then EOF.
/// :04 0000 00 01020304 F2
const char * const mpcn_SIMPLE =
   ":0400000001020304F2\n"
   ":00000001FF\n";

/// Two data records at 0x0000 and 0x0010, then EOF.
const char * const mpcn_TWO_RECORDS =
   ":0400000001020304F2\n"
   ":0400100005060708D2\n"
   ":00000001FF\n";
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(HexFile, LoadsMinimalValidFile)
{
   const std::string c_Path = mh_WriteHex("hf_simple.hex", mpcn_SIMPLE);
   C_HexFile c_File;

   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_EQ(0x0000U, c_File.MinAdr());
   EXPECT_EQ(0x0003U, c_File.MaxAdr());
   EXPECT_EQ(4U, c_File.ByteCount());

   (void)std::remove(c_Path.c_str());
}

TEST(HexFile, ReportsAddressRangeAcrossRecords)
{
   const std::string c_Path = mh_WriteHex("hf_two.hex", mpcn_TWO_RECORDS);
   C_HexFile c_File;

   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_EQ(0x0000U, c_File.MinAdr());
   EXPECT_EQ(0x0013U, c_File.MaxAdr());
   EXPECT_EQ(8U, c_File.ByteCount());

   (void)std::remove(c_Path.c_str());
}

TEST(HexFile, MissingFileIsAnError)
{
   C_HexFile c_File;

   EXPECT_TRUE(static_cast<bool>(c_File.LoadFromFile("hf_does_not_exist.hex")));
}

/// A record whose checksum byte is wrong must be rejected rather than silently
/// accepted — this data ends up on an ECU.
TEST(HexFile, BadChecksumIsRejected)
{
   const std::string c_Path = mh_WriteHex("hf_badcrc.hex",
                                          ":0400000001020304FF\n"
                                          ":00000001FF\n");
   C_HexFile c_File;

   EXPECT_TRUE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   (void)std::remove(c_Path.c_str());
}

TEST(HexFile, GarbageIsRejected)
{
   const std::string c_Path = mh_WriteHex("hf_garbage.hex", "this is not a hex file at all\n");
   C_HexFile c_File;

   EXPECT_TRUE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   (void)std::remove(c_Path.c_str());
}

TEST(HexFile, EmptyFileIsRejected)
{
   const std::string c_Path = mh_WriteHex("hf_empty.hex", "");
   C_HexFile c_File;

   EXPECT_TRUE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   (void)std::remove(c_Path.c_str());
}

/// Clear() must return the object to a reusable state; the ring buffer is torn
/// down here, so this is the path most likely to break under a container swap.
TEST(HexFile, ClearResetsAndAllowsReload)
{
   const std::string c_Path = mh_WriteHex("hf_clear.hex", mpcn_SIMPLE);
   C_HexFile c_File;

   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_EQ(4U, c_File.ByteCount());

   c_File.Clear();
   EXPECT_EQ(0U, c_File.ByteCount());

   // reload into the same object
   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_EQ(4U, c_File.ByteCount());

   (void)std::remove(c_Path.c_str());
}

/// Loading twice without an intervening Clear() must not corrupt state.
TEST(HexFile, ReloadWithoutClearIsSafe)
{
   const std::string c_Path = mh_WriteHex("hf_reload.hex", mpcn_SIMPLE);
   C_HexFile c_File;

   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   const uint32_t u32_First = c_File.ByteCount();

   EXPECT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_EQ(u32_First, c_File.ByteCount());

   (void)std::remove(c_Path.c_str());
}

/// LineInit()/NextLine() walk the ring buffer.
///
/// Note the asymmetry, which is easy to get wrong: LineInit() returns the first
/// line's data AND leaves the cursor on that same line, while NextLine() returns
/// the CURRENT line and then advances. So treating LineInit()'s return value as
/// the first visited element and then calling NextLine() yields the first line
/// twice. The correct walk uses LineInit() for positioning only.
TEST(HexFile, LineWalkVisitsEveryRecordAndTerminates)
{
   const std::string c_Path = mh_WriteHex("hf_walk.hex", mpcn_TWO_RECORDS);
   C_HexFile c_File;

   ASSERT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   //position only; do not count this as a visit
   ASSERT_NE(nullptr, c_File.LineInit());

   uint32_t u32_Visited = 0U;
   const uint8_t * pu8_Line = c_File.NextLine();
   while (pu8_Line != nullptr)
   {
      u32_Visited++;
      ASSERT_LT(u32_Visited, 1000U) << "line walk did not terminate";
      pu8_Line = c_File.NextLine();
   }

   //every record including the EOF record, each exactly once
   EXPECT_EQ(c_File.LineCount(), u32_Visited);

   (void)std::remove(c_Path.c_str());
}

/// The cursor is a ring: the last element's pt_Next points back at the entry.
/// NextLine() detects that and returns nullptr rather than looping forever.
/// Pinning this matters because the ring is what phase 6.3 would replace.
TEST(HexFile, LineWalkIsRestartableAfterExhaustion)
{
   const std::string c_Path = mh_WriteHex("hf_restart.hex", mpcn_TWO_RECORDS);
   C_HexFile c_File;

   ASSERT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   uint32_t u32_FirstPass = 0U;
   (void)c_File.LineInit();
   while (c_File.NextLine() != nullptr)
   {
      u32_FirstPass++;
      ASSERT_LT(u32_FirstPass, 1000U);
   }

   //re-initialising must give an identical second traversal
   uint32_t u32_SecondPass = 0U;
   (void)c_File.LineInit();
   while (c_File.NextLine() != nullptr)
   {
      u32_SecondPass++;
      ASSERT_LT(u32_SecondPass, 1000U);
   }

   EXPECT_EQ(u32_FirstPass, u32_SecondPass);

   (void)std::remove(c_Path.c_str());
}

TEST(HexFile, NextBinDataYieldsLoadedBytes)
{
   const std::string c_Path = mh_WriteHex("hf_bin.hex", mpcn_SIMPLE);
   C_HexFile c_File;

   ASSERT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   ASSERT_NE(nullptr, c_File.LineInit());

   uint32_t u32_Address = 0U;
   uint8_t u8_Size = 0U;
   const uint8_t * const pu8_Data = c_File.NextBinData(u32_Address, u8_Size);

   ASSERT_NE(nullptr, pu8_Data);
   EXPECT_EQ(0x0000U, u32_Address);
   EXPECT_EQ(4U, u8_Size);
   EXPECT_EQ(0x01U, pu8_Data[0]);
   EXPECT_EQ(0x02U, pu8_Data[1]);
   EXPECT_EQ(0x03U, pu8_Data[2]);
   EXPECT_EQ(0x04U, pu8_Data[3]);

   (void)std::remove(c_Path.c_str());
}

/// Round-trip through the writer. Save must produce something Load accepts, with
/// the same address range and byte count.
TEST(HexFile, SaveReloadPreservesRangeAndCount)
{
   const std::string c_In = mh_WriteHex("hf_rt_in.hex", mpcn_TWO_RECORDS);
   const std::string c_Out = "hf_rt_out.hex";
   C_HexFile c_First;

   ASSERT_FALSE(static_cast<bool>(c_First.LoadFromFile(c_In.c_str())));
   ASSERT_FALSE(static_cast<bool>(c_First.SaveToFile(c_Out.c_str())));

   C_HexFile c_Second;
   ASSERT_FALSE(static_cast<bool>(c_Second.LoadFromFile(c_Out.c_str())));

   EXPECT_EQ(c_First.MinAdr(), c_Second.MinAdr());
   EXPECT_EQ(c_First.MaxAdr(), c_Second.MaxAdr());
   EXPECT_EQ(c_First.ByteCount(), c_Second.ByteCount());

   (void)std::remove(c_In.c_str());
   (void)std::remove(c_Out.c_str());
}

TEST(HexFile, GetDataDumpMatchesLoadedContent)
{
   const std::string c_Path = mh_WriteHex("hf_dump.hex", mpcn_SIMPLE);
   C_HexFile c_File;

   ASSERT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));

   std::error_code c_Error;
   const C_HexDataDump * const pc_Dump = c_File.GetDataDump(c_Error);

   ASSERT_NE(nullptr, pc_Dump);
   EXPECT_FALSE(static_cast<bool>(c_Error));
   ASSERT_EQ(1U, pc_Dump->at_Blocks.size());
   EXPECT_EQ(0x0000U, pc_Dump->at_Blocks[0].u32_AddressOffset);
   EXPECT_EQ(4U, pc_Dump->at_Blocks[0].au8_Data.size());

   (void)std::remove(c_Path.c_str());
}

/// Validate() on a freshly loaded, well-formed file must report success.
TEST(HexFile, ValidateAcceptsWellFormedFile)
{
   const std::string c_Path = mh_WriteHex("hf_val.hex", mpcn_TWO_RECORDS);
   C_HexFile c_File;

   ASSERT_FALSE(static_cast<bool>(c_File.LoadFromFile(c_Path.c_str())));
   EXPECT_FALSE(static_cast<bool>(c_File.Validate()));

   (void)std::remove(c_Path.c_str());
}

/* -- error_code migration ------------------------------------------------------------------------------------------ */

/// Errors now compare against a named enumerator instead of being masked with
/// ERR_MASK. That is the whole point of the migration.
TEST(HexFileError, MissingFileMapsToCantOpenFile)
{
   C_HexFile c_File;
   const std::error_code c_Error = c_File.LoadFromFile("hf_no_such_file.hex");

   EXPECT_TRUE(static_cast<bool>(c_Error));
   EXPECT_EQ(HexFileErrc::cant_open_file, c_Error);
   EXPECT_STREQ("stw.hex_file", c_Error.category().name());
}

TEST(HexFileError, BadChecksumMapsToChecksumErrorAndReportsLine)
{
   const std::string c_Path = mh_WriteHex("hfe_badcrc.hex",
                                          ":0400000001020304F2\n"
                                          ":0400100005060708FF\n"
                                          ":00000001FF\n");
   C_HexFile c_File;
   const std::error_code c_Error = c_File.LoadFromFile(c_Path.c_str());

   EXPECT_TRUE(static_cast<bool>(c_Error));
   EXPECT_EQ(HexFileErrc::hexline_checksum, c_Error);

   //the line number that used to be packed into the low 28 bits
   EXPECT_EQ(2U, c_File.GetLastErrorLineNumber());

   (void)std::remove(c_Path.c_str());
}

TEST(HexFileError, SuccessIsFalsyAndCarriesNoLine)
{
   const std::string c_Path = mh_WriteHex("hfe_ok.hex", mpcn_SIMPLE);
   C_HexFile c_File;
   const std::error_code c_Error = c_File.LoadFromFile(c_Path.c_str());

   EXPECT_FALSE(static_cast<bool>(c_Error));
   EXPECT_EQ(0U, c_File.GetLastErrorLineNumber());

   (void)std::remove(c_Path.c_str());
}

/// The legacy packed representation is still used internally, so the split into
/// identity and context is pinned directly.
TEST(HexFileError, LegacySplitSeparatesIdentityFromContext)
{
   //ERR_HEXLINE_SYNTAX with line 42 packed into the low nibbles
   const uint32_t u32_Packed = 0x80000000UL | 42UL;

   EXPECT_EQ(HexFileErrc::hexline_syntax, h_HexFileErrorFromLegacy(u32_Packed));
   EXPECT_EQ(42U, h_HexFileLineFromLegacy(u32_Packed));

   //codes that carry no context must report none
   EXPECT_EQ(HexFileErrc::not_enough_memory, h_HexFileErrorFromLegacy(0xE0000000UL));
   EXPECT_EQ(0U, h_HexFileLineFromLegacy(0xE0000000UL));

   EXPECT_EQ(HexFileErrc::success, h_HexFileErrorFromLegacy(0UL));
}

TEST(HexFileError, MessagesAreDescriptive)
{
   EXPECT_EQ("File not found", make_error_code(HexFileErrc::cant_open_file).message());
   EXPECT_EQ("Wrong checksum in hex line", make_error_code(HexFileErrc::hexline_checksum).message());
   EXPECT_EQ("Out of memory", make_error_code(HexFileErrc::not_enough_memory).message());
}
