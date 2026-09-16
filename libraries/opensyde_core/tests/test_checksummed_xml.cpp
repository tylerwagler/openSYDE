//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscChecksummedXml and the parameter set file version

   Both write a value as "0x"-prefixed hex and read it back through a parser that was
   hard-wired to base 10, so both read 0. The class could not load a file it had just
   written, and parameter set files failed twice over: once on the CRC and once on the
   version check.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include <cstdint>
#include "C_OscErrorCategory.hpp"
#include "C_OscChecksummedXml.hpp"
#include "C_OscParamSetFilerBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A checksummed file loads back the CRC it just wrote

   Regression pin, and the sharpest statement of the bug: SaveToFile wrote the CRC as
   "0x1A2B", LoadFromFile read it with GetAttributeUint32, which returned 0, and the
   comparison against the real CRC failed. The class could not round trip its own output,
   so every parameter set file was rejected with Errc::checksum.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(ChecksummedXml, RoundTripsItsOwnOutput)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_test_crc.xml";

   (void)std::filesystem::remove(c_Path);
   {
      C_OscChecksummedXml c_Xml;
      c_Xml.CreateAndSelectNodeChild("root");
      c_Xml.SetAttributeString("payload", "hello");
      c_Xml.SetAttributeUint32("number", 42U);
      ASSERT_FALSE(static_cast<bool>(c_Xml.SaveToFile(c_Path.string())));
   }

   C_OscChecksummedXml c_Read;
   const std::error_code c_Result = c_Read.LoadFromFile(c_Path.string());

   EXPECT_FALSE(static_cast<bool>(c_Result)) << "round trip failed: " << c_Result.message();
   EXPECT_EQ("root", c_Read.SelectRoot());
   EXPECT_EQ("hello", c_Read.GetAttributeString("payload"));
   EXPECT_EQ(42U, c_Read.GetAttributeUint32("number"));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Tampering with the content is still detected

   The fix must not turn the checksum into a formality.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(ChecksummedXml, DetectsTamperedContent)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_test_crc_bad.xml";

   (void)std::filesystem::remove(c_Path);
   {
      C_OscChecksummedXml c_Xml;
      c_Xml.CreateAndSelectNodeChild("root");
      c_Xml.SetAttributeString("payload", "hello");
      ASSERT_FALSE(static_cast<bool>(c_Xml.SaveToFile(c_Path.string())));
   }

   //rewrite one attribute value without touching the stored CRC
   {
      C_OscXmlParser c_Plain;
      ASSERT_FALSE(static_cast<bool>(c_Plain.LoadFromFile(c_Path.string())));
      c_Plain.SelectRoot();
      c_Plain.SetAttributeString("payload", "tampered");
      ASSERT_FALSE(static_cast<bool>(c_Plain.SaveToFile(c_Path.string())));
   }

   C_OscChecksummedXml c_Read;
   EXPECT_EQ(stw::errors::Errc::checksum, c_Read.LoadFromFile(c_Path.string()));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The parameter set file version written as hex is accepted on read

   Regression pin. h_SaveFileVersion writes "0x0001"; h_CheckFileVersion read it with a
   base-10 std::stoi, got 0, and rejected the file as an unsupported version.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(ParamSetFileVersion, HexVersionRoundTrips)
{
   C_OscXmlParser c_Parser;

   c_Parser.CreateAndSelectNodeChild("opensyde-parameter-sets");
   C_OscParamSetFilerBase::h_SaveFileVersion(c_Parser);

   //h_SaveFileVersion leaves the parser on "opensyde-parameter-sets", which is the
   //precondition h_CheckFileVersion documents
   EXPECT_FALSE(static_cast<bool>(C_OscParamSetFilerBase::h_CheckFileVersion(c_Parser)));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A genuinely unsupported version is still rejected
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(ParamSetFileVersion, UnsupportedVersionIsRejected)
{
   C_OscXmlParser c_Parser;

   c_Parser.CreateAndSelectNodeChild("opensyde-parameter-sets");
   c_Parser.CreateAndSelectNodeChild("file-version");
   c_Parser.SetNodeContent("0x0099");
   c_Parser.SelectNodeParent();

   EXPECT_EQ(stw::errors::Errc::config, C_OscParamSetFilerBase::h_CheckFileVersion(c_Parser));
}
