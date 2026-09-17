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
#include "C_OscImportEdsDcf.hpp"
#include "C_OscEdsDcfImportMessageGroup.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscCanMessage.hpp"

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

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Importing a TPDO reads every numeric field at its real position

   Regression pin for the EDS/DCF importer's integer parsers, which read each character one position ahead of the
   loop counter (a leftover of the 1-based string class): "254" came back as 54, "1" as an empty string, and
   "$NODEID+0x180" only survived because the dropped character happened to be the one the parser strips anyway.
   One transmit PDO of the device with a node-id-relative COB-ID, transmission type 254, an event timer and a
   single mapped UNSIGNED8 object with limits covers both parsers through the public import.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanOpenEds, ImportReadsNumericFieldsAtTheirRealPosition)
{
   const std::string c_Path = h_WriteEds(
      "osy_eds_import_tpdo.eds",
      "[FileInfo]\nFileName=import.eds\nFileVersion=1\nFileRevision=1\n"
      "[DeviceInfo]\nVendorName=Test\nProductName=Importable\nNrOfRxPDO=0\nNrOfTxPDO=1\n"
      "[MandatoryObjects]\nSupportedObjects=1\n1=0x1000\n"
      "[1000]\nParameterName=Device Type\nObjectType=0x7\nDataType=0x0007\nAccessType=ro\nDefaultValue=0\nPDOMapping=0\n"
      "[OptionalObjects]\nSupportedObjects=2\n1=0x1800\n2=0x1A00\n"
      "[1800]\nParameterName=TPDO1 communication\nObjectType=0x9\nSubNumber=4\n"
      "[1800sub0]\nParameterName=Highest sub-index\nObjectType=0x7\nDataType=0x0005\nAccessType=ro\nDefaultValue=5\nPDOMapping=0\n"
      "[1800sub1]\nParameterName=COB-ID\nObjectType=0x7\nDataType=0x0007\nAccessType=rw\nDefaultValue=$NODEID+0x180\nPDOMapping=0\n"
      "[1800sub2]\nParameterName=Transmission type\nObjectType=0x7\nDataType=0x0005\nAccessType=rw\nDefaultValue=254\nPDOMapping=0\n"
      "[1800sub5]\nParameterName=Event timer\nObjectType=0x7\nDataType=0x0006\nAccessType=rw\nDefaultValue=100\nPDOMapping=0\n"
      "[1A00]\nParameterName=TPDO1 mapping\nObjectType=0x9\nSubNumber=2\n"
      "[1A00sub0]\nParameterName=Number of entries\nObjectType=0x7\nDataType=0x0005\nAccessType=rw\nDefaultValue=1\nPDOMapping=0\n"
      "[1A00sub1]\nParameterName=Mapping entry 1\nObjectType=0x7\nDataType=0x0007\nAccessType=rw\nDefaultValue=0x60000108\nPDOMapping=0\n"
      "[ManufacturerObjects]\nSupportedObjects=1\n1=0x6000\n"
      "[6000]\nParameterName=Inputs\nObjectType=0x9\nSubNumber=2\n"
      "[6000sub0]\nParameterName=Number of entries\nObjectType=0x7\nDataType=0x0005\nAccessType=ro\nDefaultValue=1\nPDOMapping=0\n"
      "[6000sub1]\nParameterName=Speed\nObjectType=0x7\nDataType=0x0005\nAccessType=ro\nDefaultValue=0x10\n"
      "LowLimit=0x02\nHighLimit=0xF0\nPDOMapping=1\n");

   C_OscEdsDcfImportMessageGroup c_Rx;
   C_OscEdsDcfImportMessageGroup c_Tx;
   C_OscEdsDcfImportMessageGroup c_InvalidRx;
   C_OscEdsDcfImportMessageGroup c_InvalidTx;
   std::vector<std::vector<std::string> > c_Notes;
   std::vector<std::vector<std::string> > c_InvalidNotes;
   std::string c_Error;
   const std::error_code c_Result = C_OscImportEdsDcf::h_Import(c_Path, 10U, c_Rx, c_Tx, c_Notes, c_Error,
                                                                C_OscCanProtocol::eCAN_OPEN, c_InvalidRx, c_InvalidTx,
                                                                c_InvalidNotes);
   ASSERT_FALSE(static_cast<bool>(c_Result)) << c_Result.message() << " " << c_Error;

   //the device's transmit PDO is what the manager receives
   EXPECT_EQ(0U, c_Tx.c_OscMessageData.size());
   EXPECT_EQ(0U, c_InvalidRx.c_OscMessageData.size()) << "the PDO was sorted out as invalid";
   ASSERT_EQ(1U, c_Rx.c_OscMessageData.size());
   const C_OscCanMessage & rc_Msg = c_Rx.c_OscMessageData[0];
   EXPECT_EQ(0x18AU, rc_Msg.u32_CanId) << "$NODEID+0x180 with node id 10";
   EXPECT_EQ(C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_254, rc_Msg.e_TxMethod) << "transmission type read as 254";
   ASSERT_EQ(1U, rc_Msg.c_Signals.size()) << "mapping count read as 1";
   EXPECT_EQ(8U, rc_Msg.c_Signals[0].u16_ComBitLength);
   EXPECT_EQ(0U, rc_Msg.c_Signals[0].u16_ComBitStart);
   ASSERT_EQ(1U, c_Rx.c_OscSignalData.size());
   EXPECT_EQ("Speed", c_Rx.c_OscSignalData[0].c_Name);
   ASSERT_EQ(1U, c_Rx.c_SignalDefaultMinMaxValuesUsed.size());
   EXPECT_EQ(0U, c_Rx.c_SignalDefaultMinMaxValuesUsed[0]) << "limits from the file, not the type defaults";
   EXPECT_EQ(0x02U, c_Rx.c_OscSignalData[0].c_MinValue.GetValueU8());
   EXPECT_EQ(0xF0U, c_Rx.c_OscSignalData[0].c_MaxValue.GetValueU8());
   ASSERT_EQ(1U, c_Rx.c_OscSignalData[0].c_DataSetValues.size());
   EXPECT_EQ(0x10U, c_Rx.c_OscSignalData[0].c_DataSetValues[0].GetValueU8()) << "hex default value";

   (void)std::filesystem::remove(c_Path);
}
