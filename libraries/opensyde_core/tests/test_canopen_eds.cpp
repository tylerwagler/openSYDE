//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscCanOpenObjectDictionary (EDS/DCF parsing)

   EDS parsing had no test coverage. These cover the load contract: a well-formed
   file parses, and a malformed one is rejected with a message rather than being
   accepted silently.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <cstdint>
#include "C_OscErrorCategory.hpp"
#include "C_OscCanOpenObjectDictionary.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write an EDS file into the platform temp directory and return its path

   The suite runs on three platforms, so the path comes from
   std::filesystem::temp_directory_path() rather than a hardcoded /tmp.
*/
//----------------------------------------------------------------------------------------------------------------------
std::string h_WriteEds(const std::string & orc_Name, const std::string & orc_Content)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / orc_Name;
   std::ofstream c_Out(c_Path, std::ios::binary);

   c_Out << orc_Content;
   c_Out.close();
   return c_Path.string();
}

//A minimal but well-formed EDS. The three *Objects blocks declare no SupportedObjects,
//so the cross-reference checks pass trivially and only the object sections matter.
const std::string hc_VALID_EDS =
   "[FileInfo]\n"
   "FileName=test.eds\n"
   "FileVersion=1\n"
   "FileRevision=0\n"
   "Description=unit test fixture\n"
   "CreatedBy=tests\n"
   "\n"
   "[DeviceInfo]\n"
   "VendorName=Test Vendor\n"
   "ProductName=Test Product\n"
   "BaudRate_125=1\n"
   "Granularity=8\n"
   "NrOfRXPDO=0\n"
   "NrOfTXPDO=0\n"
   "\n"
   "[1000]\n"
   "ParameterName=Device Type\n"
   "ObjectType=0x7\n"
   "DataType=0x0007\n"
   "AccessType=ro\n"
   "DefaultValue=0\n";
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A well-formed EDS loads and its object dictionary is populated
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanOpenEds, LoadsWellFormedFile)
{
   const std::string c_Path = h_WriteEds("osy_test_valid.eds", hc_VALID_EDS);
   C_OscCanOpenObjectDictionary c_Dictionary;

   const std::error_code c_Result = c_Dictionary.LoadFromFile(c_Path);

   EXPECT_FALSE(static_cast<bool>(c_Result)) << "unexpected error: " << c_Dictionary.GetLastErrorText();
   EXPECT_NE(c_Dictionary.GetCanOpenObject(0x1000U), nullptr);

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A missing file is reported as Errc::range, not as a parse failure
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanOpenEds, MissingFileIsRange)
{
   C_OscCanOpenObjectDictionary c_Dictionary;
   const std::filesystem::path c_Absent =
      std::filesystem::temp_directory_path() / "osy_no_such_file_4f2a9c.eds";

   EXPECT_EQ(stw::errors::Errc::range, c_Dictionary.LoadFromFile(c_Absent.string()));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A malformed object description fails the load instead of being swallowed

   Regression pin. The parse loop aborts on a bad object and sets mc_LastError, but the
   "check whether all referenced objects exist" step then reassigned the return value
   unconditionally. With no SupportedObjects declared those checks succeed, so the parse
   error was overwritten with success and the file imported as if valid -- while
   GetLastErrorText() still held the real message.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanOpenEds, MalformedObjectIsNotSwallowed)
{
   std::string c_Content = hc_VALID_EDS;
   const std::string c_Good = "DataType=0x0007";

   ASSERT_NE(std::string::npos, c_Content.find(c_Good));
   c_Content.replace(c_Content.find(c_Good), c_Good.size(), "DataType=NotANumber");

   const std::string c_Path = h_WriteEds("osy_test_malformed.eds", c_Content);
   C_OscCanOpenObjectDictionary c_Dictionary;

   const std::error_code c_Result = c_Dictionary.LoadFromFile(c_Path);

   EXPECT_TRUE(static_cast<bool>(c_Result)) << "malformed DataType accepted as a valid EDS";
   EXPECT_EQ(stw::errors::Errc::config, c_Result);
   //the load must not claim success while leaving an error message behind
   EXPECT_NE("", c_Dictionary.GetLastErrorText());

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A SupportedObjects count that references an undescribed object fails the load
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanOpenEds, DanglingMandatoryObjectReferenceIsRejected)
{
   const std::string c_Content = hc_VALID_EDS +
                                 "\n[MandatoryObjects]\n"
                                 "SupportedObjects=1\n"
                                 "1=0x1001\n";
   const std::string c_Path = h_WriteEds("osy_test_dangling.eds", c_Content);
   C_OscCanOpenObjectDictionary c_Dictionary;

   const std::error_code c_Result = c_Dictionary.LoadFromFile(c_Path);

   EXPECT_EQ(stw::errors::Errc::config, c_Result);
   EXPECT_NE("", c_Dictionary.GetLastErrorText());

   (void)std::filesystem::remove(c_Path);
}
