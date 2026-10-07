//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the CAN Monitor's UDS (ISO 14229) frame interpreter

   The interpreter is per-frame: it sees single, first, consecutive and flow-control frames one at a time and
   renders what it can from each. The first-frame case is the regression pin: its 12-bit length was once stored in
   a byte, which made every first frame render as a broken single frame.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <string>

#include "C_CanMonProtocolUds.hpp"
#include "stw_can.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::cmon_protocol;
using stw::can::T_STWCAN_Msg_RX;

/* -- Module Global Functions --------------------------------------------------------------------------------------- */
namespace
{
T_STWCAN_Msg_RX h_Frame(const std::initializer_list<uint8_t> & orc_Bytes)
{
   T_STWCAN_Msg_RX c_Msg = {};

   c_Msg.u32_ID = 0x7E0U;
   c_Msg.u8_DLC = static_cast<uint8_t>(orc_Bytes.size());
   uint8_t u8_Index = 0U;
   for (const uint8_t u8_Byte : orc_Bytes)
   {
      c_Msg.au8_Data[u8_Index] = u8_Byte;
      ++u8_Index;
   }
   return c_Msg;
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(CmonProtocolUds, SingleFrameRequestNamesServiceAndDid)
{
   const C_CanMonProtocolUds c_Uds;
   //SF, 3 bytes: ReadDataByIdentifier F190 (VIN)
   const std::string c_Text = c_Uds.MessageToString(h_Frame({0x03U, 0x22U, 0xF1U, 0x90U}));

   EXPECT_EQ(0U, c_Text.rfind("SF ", 0U)) << c_Text;
   EXPECT_NE(std::string::npos, c_Text.find("ReadDataByIdentifier")) << c_Text;
   EXPECT_NE(std::string::npos, c_Text.find("F190")) << c_Text;
}

TEST(CmonProtocolUds, NegativeResponseUsesTheSharedNrcTable)
{
   const C_CanMonProtocolUds c_Uds;
   const std::string c_Text = c_Uds.MessageToString(h_Frame({0x03U, 0x7FU, 0x22U, 0x31U}));

   EXPECT_NE(std::string::npos, c_Text.find("NRC:requestOutOfRange")) << c_Text;
}

TEST(CmonProtocolUds, FirstFrameCarriesTheFullTwelveBitLength)
{
   const C_CanMonProtocolUds c_Uds;
   //FF, total length 0x123 = 291 bytes: positive response to ReadDataByIdentifier F190 with the first data bytes
   const std::string c_Text =
      c_Uds.MessageToString(h_Frame({0x11U, 0x23U, 0x62U, 0xF1U, 0x90U, 0x57U, 0x41U, 0x55U}));

   //Was "SF(DLC too short)": 0x123 truncated to a byte is 0x23 = 35, and 2 + 35 > 8
   EXPECT_EQ(0U, c_Text.rfind("FF(291) ", 0U)) << c_Text;
   EXPECT_NE(std::string::npos, c_Text.find("ReadDataByIdentifier")) << c_Text;
   EXPECT_NE(std::string::npos, c_Text.find("F190")) << c_Text;
   //and the three payload bytes present in this frame are shown, nothing beyond the frame is read
   EXPECT_NE(std::string::npos, c_Text.find("DATA[")) << c_Text;
}

TEST(CmonProtocolUds, FirstFrameWithLengthThatTruncatesToZeroIsNotEmpty)
{
   const C_CanMonProtocolUds c_Uds;
   //total length 0x100 = 256: the low byte is 0, which the old code reported as "SF(empty)"
   const std::string c_Text =
      c_Uds.MessageToString(h_Frame({0x11U, 0x00U, 0x62U, 0xF1U, 0x8CU, 0x30U, 0x31U, 0x32U}));

   EXPECT_EQ(0U, c_Text.rfind("FF(256) ", 0U)) << c_Text;
}

TEST(CmonProtocolUds, FirstFrameWithSingleFrameLengthIsFlagged)
{
   const C_CanMonProtocolUds c_Uds;
   const std::string c_Text = c_Uds.MessageToString(h_Frame({0x10U, 0x05U, 0x62U, 0xF1U, 0x90U, 0x00U, 0x00U, 0x00U}));

   EXPECT_EQ("FF(length 5 fits a single frame)", c_Text);
}

TEST(CmonProtocolUds, ConsecutiveAndFlowControlFramesRenderTheirPci)
{
   const C_CanMonProtocolUds c_Uds;

   EXPECT_EQ("CF(seq=5)", c_Uds.MessageToString(h_Frame({0x25U, 0x01U, 0x02U})));
   EXPECT_EQ("FC(sts=0,bs=8,stmin=20)", c_Uds.MessageToString(h_Frame({0x30U, 0x08U, 0x14U})));
}
