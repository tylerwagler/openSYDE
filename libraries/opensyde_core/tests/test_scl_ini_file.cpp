//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_SclIniFile

   C_SclIniFile had no test coverage, and for some time parsed nothing at all: the
   section and comment checks kept their 1-based C_SclString indices when the class
   moved to std::string, so they inspected the second character of every line. No
   section header was ever recognised, every section lookup failed, and every read
   returned its default -- silently, because a missing key is not an error.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <cstdint>
#include "C_SclIniFile.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write an ini file into the platform temp directory and return its path
*/
//----------------------------------------------------------------------------------------------------------------------
std::string h_WriteIni(const std::string & orc_Name, const std::string & orc_Content)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / orc_Name;
   std::ofstream c_Out(c_Path, std::ios::binary);

   c_Out << orc_Content;
   c_Out.close();
   return c_Path.string();
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Section headers are recognised

   Regression pin. This is the assertion that failed for every section in the file.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, ParsesSections)
{
   const std::string c_Path = h_WriteIni("osy_test_sections.ini",
                                         "[Alpha]\n"
                                         "Number=42\n"
                                         "Text=hello\n"
                                         "\n"
                                         "[Beta]\n"
                                         "Flag=1\n");
   C_SclIniFile c_Ini(c_Path);

   EXPECT_TRUE(c_Ini.SectionExists("Alpha"));
   EXPECT_TRUE(c_Ini.SectionExists("Beta"));
   EXPECT_FALSE(c_Ini.SectionExists("Gamma"));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Values are read back, and a missing key yields the supplied default
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, ReadsValuesAndFallsBackToDefaults)
{
   const std::string c_Path = h_WriteIni("osy_test_values.ini",
                                         "[Alpha]\n"
                                         "Number=42\n"
                                         "Text=hello\n"
                                         "Flag=1\n");
   C_SclIniFile c_Ini(c_Path);

   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "Number", 0));
   EXPECT_EQ("hello", c_Ini.ReadString("Alpha", "Text", ""));
   EXPECT_TRUE(c_Ini.ReadBool("Alpha", "Flag", false));

   //absent key and absent section both fall back rather than failing
   EXPECT_EQ(7, c_Ini.ReadInteger("Alpha", "Missing", 7));
   EXPECT_EQ("fallback", c_Ini.ReadString("Nowhere", "Text", "fallback"));

   EXPECT_TRUE(c_Ini.ValueExists("Alpha", "Number"));
   EXPECT_FALSE(c_Ini.ValueExists("Alpha", "Missing"));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Comment lines are treated as comments, not as key/value pairs

   The comment check carried the same 1-based index as the section check.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, TreatsCommentLinesAsComments)
{
   const std::string c_Path = h_WriteIni("osy_test_comments.ini",
                                         "; a leading comment\n"
                                         "[Alpha]\n"
                                         "; explains Number\n"
                                         "Number=42\n");
   C_SclIniFile c_Ini(c_Path);

   EXPECT_TRUE(c_Ini.SectionExists("Alpha"));
   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "Number", 0));
   //the comment text must not have become a key
   EXPECT_FALSE(c_Ini.ValueExists("Alpha", "; explains Number"));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Indented section headers still parse

   Leading whitespace is trimmed before the first character is inspected.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, ParsesIndentedSections)
{
   const std::string c_Path = h_WriteIni("osy_test_indent.ini",
                                         "   [Alpha]\n"
                                         "\tNumber=42\n");
   C_SclIniFile c_Ini(c_Path);

   EXPECT_TRUE(c_Ini.SectionExists("Alpha"));
   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "Number", 0));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Values written through the API are read back after a save/reload round trip
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, RoundTripsThroughDisk)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_test_roundtrip.ini";

   (void)std::filesystem::remove(c_Path);
   {
      C_SclIniFile c_Ini(c_Path.string());
      c_Ini.WriteInteger("Alpha", "Number", 99);
      c_Ini.WriteString("Alpha", "Text", "written");
      c_Ini.UpdateFile();
   }

   C_SclIniFile c_Reloaded(c_Path.string());
   EXPECT_TRUE(c_Reloaded.SectionExists("Alpha"));
   EXPECT_EQ(99, c_Reloaded.ReadInteger("Alpha", "Number", 0));
   EXPECT_EQ("written", c_Reloaded.ReadString("Alpha", "Text", ""));

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Integers spelled in hex read as the number, not as 0

   ReadInteger replaced C_SclString::ToInt(), which accepted a "0x" prefix; the INI dialects read through this class
   (CiA 306 EDS files among them) use it. A base-10-only std::stoi read "0x2A" as 0 -- silently, since 0 is a
   perfectly good integer.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SclIniFile, ReadsHexIntegers)
{
   const std::string c_Path = h_WriteIni("osy_test_hex.ini",
                                         "[Alpha]\n"
                                         "Dec=42\n"
                                         "Hex=0x2A\n"
                                         "HexUpper=0X2a\n"
                                         "NegHex=-0x10\n"
                                         "Count=0x0004\n");
   C_SclIniFile c_Ini(c_Path);
   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "Dec", 0));
   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "Hex", 0));
   EXPECT_EQ(42, c_Ini.ReadInteger("Alpha", "HexUpper", 0));
   EXPECT_EQ(-16, c_Ini.ReadInteger("Alpha", "NegHex", 0));
   EXPECT_EQ(4U, c_Ini.ReadUint16("Alpha", "Count", 0U));
   (void)std::filesystem::remove(c_Path);
}
