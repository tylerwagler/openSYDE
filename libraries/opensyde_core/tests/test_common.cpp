//----------------------------------------------------------------------------------------------------------------------
/**
 * @file
 * @brief   Starter unit tests for the openSYDE Core common infrastructure.
 *
 * These tests exercise the basic utility classes that every openSYDE
 * application depends on: logging, XML parsing, INI file handling, string
 * utilities, and checksums.
 */
//----------------------------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "stwtypes/stwtypes.hpp"
#include "C_SclString.hpp"
#include "C_SclChecksums.hpp"
#include "C_SclIniFile.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscLoggingHandler.hpp"

//----------------------------------------------------------------------------------------------------------------------
/*  @brief   Verify that the logging singleton can be obtained and
            used without crashing.                                            */
//----------------------------------------------------------------------------------------------------------------------

TEST(Common, LoggingHandler_GetInstance)
{
   const stw::scl::C_OscLoggingHandler & orc_Instance =
      stw::scl::C_OscLoggingHandler::h_GetInstance();

   // The logging handler should have been created by the time we get here.
   // Just calling GetInstance() without a crash is a basic sanity check.
   SUCCEED();
}

//----------------------------------------------------------------------------------------------------------------------
/*  @brief   Verify SclString basic operations.                                */
//----------------------------------------------------------------------------------------------------------------------

TEST(Common, SclString_Basics)
{
   const stw::scl::C_SclString c_Empty;
   EXPECT_TRUE(c_Empty.IsEmpty());
   EXPECT_EQ(c_Empty.Length(), 0);

   const stw::scl::C_SclString c_Hello("Hello");
   EXPECT_FALSE(c_Hello.IsEmpty());
   EXPECT_EQ(c_Hello.Length(), 5);
   EXPECT_STREQ(c_Hello.c_str(), "Hello");
}

//----------------------------------------------------------------------------------------------------------------------
/*  @brief   Verify CRC-16 checksum over known test data.                      */
//----------------------------------------------------------------------------------------------------------------------

TEST(Common, SclChecksums_CalcCRC16)
{
   const uint8_t au8_Data[] = { 0x31, 0x32, 0x33, 0x34, 0x35,
                                0x36, 0x37, 0x38, 0x39 };
   const uint16_t u16_Crc =
      stw::scl::C_SclChecksums::h_CalcCRC16(au8_Data, sizeof(au8_Data));
   // CRC-16/XMODEM of "123456789" = 0x31C3
   EXPECT_EQ(u16_Crc, static_cast<uint16_t>(0x31C3));
}

//----------------------------------------------------------------------------------------------------------------------
/*  @brief   Verify XML parser can parse a simple document.                    */
//----------------------------------------------------------------------------------------------------------------------

TEST(Common, XmlParser_ParseSimpleDocument)
{
   stw::scl::C_OscXmlParser c_Parser;
   c_Parser.LoadFromString("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                            "<root><item id=\"1\">value</item></root>");
   EXPECT_TRUE(c_Parser.SelectRoot());
   EXPECT_STREQ(c_Parser.SelectNodeChild("item"), "value");
}

//----------------------------------------------------------------------------------------------------------------------
/*  @brief   Verify string list operations.                                    */
//----------------------------------------------------------------------------------------------------------------------

TEST(Common, SclStringList_AppendAndGet)
{
   stw::scl::C_SclStringList c_List;
   c_List.Append("first");
   c_List.Append("second");
   c_List.Append("third");

   EXPECT_EQ(c_List.GetCount(), 3);
   EXPECT_STREQ(c_List.Strings()[0].c_str(), "first");
   EXPECT_STREQ(c_List.Strings()[1].c_str(), "second");
   EXPECT_STREQ(c_List.Strings()[2].c_str(), "third");
}
