//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN message logger: a raw frame interpreted against a system definition

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscComMessageLogger.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSystemDefinitionFiler.hpp"
#include "osy_test_models.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
//the decoded message is a protected accessor; a test subclass makes it visible
class C_TestLogger :
   public C_OscComMessageLogger
{
public:
   const C_OscComMessageLoggerData & Last(void) const
   {
      return this->m_GetHandledCanMessage();
   }
};
}

//----------------------------------------------------------------------------------------------------------------------
TEST(ComMessageLogger, InterpretsAFrameAgainstTheSystemDefinition)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_logger_rt";
   const std::filesystem::path c_Path = c_Dir / "system_definition.syde_sysdef";

   (void)std::filesystem::remove_all(c_Dir);
   C_OscSystemDefinition c_SysDef;
   {
      C_OscSystemBus c_Bus;
      c_Bus.c_Name = "Drive CAN";
      c_Bus.e_Type = C_OscSystemBus::eCAN;
      c_Bus.u64_BitRate = 500000ULL;
      c_SysDef.c_Buses.push_back(c_Bus);
   }
   c_SysDef.c_Nodes.push_back(h_MakeNode("Engine", 7U, 0U));
   ASSERT_FALSE(static_cast<bool>(C_OscSystemDefinitionFiler::h_SaveSystemDefinitionFile(c_SysDef, c_Path.string(),
                                                                                         nullptr)));

   C_TestLogger c_Logger;
   std::vector<C_OscSystemBus> c_Buses;
   ASSERT_FALSE(static_cast<bool>(c_Logger.AddOsySysDef(c_Path.string(), 0U, c_Buses)));
   ASSERT_EQ(1U, c_Buses.size());
   c_Logger.Start();

   //the J1939 "EngineStatus" message from the shared model: extended id 0x18FEF100, multiplexer in bits 0..3
   //(Intel), value 7 selects a 16-bit Motorola signal at start bit 12 mapped to "EngineSpeed"
   stw::can::T_STWCAN_Msg_RX c_Msg;
   (void)std::memset(&c_Msg, 0, sizeof(c_Msg));
   c_Msg.u32_ID = 0x18FEF100U;
   c_Msg.u8_XTD = 1U;
   c_Msg.u8_DLC = 8U;
   c_Msg.au8_Data[0] = 0x07U;
   c_Msg.au8_Data[1] = 0x12U;
   c_Msg.au8_Data[2] = 0x34U;
   c_Msg.au8_Data[3] = 0x56U;
   c_Msg.u64_TimeStamp = 1500000ULL;
   ASSERT_FALSE(static_cast<bool>(c_Logger.HandleCanMessage(c_Msg, false)));

   const C_OscComMessageLoggerData & rc_Data = c_Logger.Last();
   EXPECT_EQ("EngineStatus", rc_Data.c_Name);
   EXPECT_EQ("18FEF100x", rc_Data.c_CanIdHex) << "extended ids are marked with an x";
   EXPECT_EQ("8", rc_Data.c_CanDlc);
   EXPECT_EQ("07 12 34 56 00 00 00 00", rc_Data.c_CanDataHex);
   EXPECT_EQ("1", rc_Data.c_Counter);
   EXPECT_FALSE(rc_Data.q_CanDlcError);
   ASSERT_EQ(2U, rc_Data.c_Signals.size()) << "the multiplexer and the one signal selected by value 7";
   EXPECT_EQ("EngineStatusMux (Multiplexer)", rc_Data.c_Signals[0].c_Name);
   EXPECT_EQ("7", rc_Data.c_Signals[0].c_Value);
   //Motorola, start bit 12, 16 bits over 12 34 56: bits 12..8 of byte 1, byte 2, top three bits of byte 3
   EXPECT_EQ("EngineSpeed", rc_Data.c_Signals[1].c_Name);
   EXPECT_EQ("37282", rc_Data.c_Signals[1].c_Value);
   EXPECT_EQ("91A2", rc_Data.c_Signals[1].c_RawValueHex);
   EXPECT_EQ("rpm", rc_Data.c_Signals[1].c_Unit);
   EXPECT_FALSE(rc_Data.c_Signals[1].q_DlcError);

   //a different multiplexer value selects nothing: only the multiplexer itself is reported
   c_Msg.au8_Data[0] = 0x03U;
   ASSERT_FALSE(static_cast<bool>(c_Logger.HandleCanMessage(c_Msg, false)));
   EXPECT_EQ("EngineStatus", c_Logger.Last().c_Name);
   EXPECT_EQ(1U, c_Logger.Last().c_Signals.size());
   EXPECT_EQ("3", c_Logger.Last().c_Signals[0].c_Value);

   //an id the definition does not know is passed through undecoded
   c_Msg.u32_ID = 0x7FFU;
   c_Msg.u8_XTD = 0U;
   ASSERT_FALSE(static_cast<bool>(c_Logger.HandleCanMessage(c_Msg, false)));
   EXPECT_EQ("", c_Logger.Last().c_Name);
   EXPECT_TRUE(c_Logger.Last().c_Signals.empty());

   (void)std::filesystem::remove_all(c_Dir);
}
