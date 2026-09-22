//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Characterisation tests for the openSYDE IP transport protocol broadcast services

   Covers the Set-IP / Set-node-ID broadcast frame composition, in particular the
   node-ID-only overloads (mode flag 0x02) added to carry the DoIP-over-IP
   configuration flow: they must leave the IP/netmask/gateway fields zero and
   set only the node identifier bytes.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include "gtest/gtest.h"
#include "C_OscProtocolDriverOsyTpIp.hpp"
#include "C_OscProtocolSerialNumber.hpp"
#include "C_OscDcBasicSequences.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
// DoIP header size; the C_DoIpHeader class is a private nested type so the test
// uses the literal offsets the implementation writes into.
const std::size_t mhu_HEADER_SIZE = 8U;

/// dispatcher double that records broadcast UDP frames for inspection and never answers
class C_BroadcastRecorder :
   public C_OscIpDispatcher
{
public:
   C_BroadcastRecorder(void) :
      C_OscIpDispatcher(1U)
   {
   }

   std::vector<std::vector<uint8_t> > c_Broadcasts; ///< every datagram handed to SendUdp

   std::error_code InitTcp(const uint8_t (&)[4], uint32_t & oru32_Handle) override
   {
      oru32_Handle = 0U;
      return Errc::success;
   }
   std::error_code IsTcpConnected(const uint32_t) override { return Errc::success; }
   std::error_code ReConnectTcp(const uint32_t) override { return Errc::success; }
   std::error_code InitUdp(void) override { return Errc::success; }
   std::error_code CloseTcp(const uint32_t) override { return Errc::success; }
   std::error_code CloseUdp(void) override { return Errc::success; }
   std::error_code SendTcp(const uint32_t, const std::vector<uint8_t> &) override { return Errc::success; }
   std::error_code ReadTcp(const uint32_t, std::vector<uint8_t> &) override { return Errc::noact; }
   std::error_code ReadTcp(const uint32_t, const uint8_t, const uint8_t, const uint8_t, const uint8_t,
                           std::vector<uint8_t> &) override
   {
      return Errc::noact;
   }
   std::error_code ReadTcpBuffer(const uint8_t, const uint8_t, const uint8_t, const uint8_t,
                                 std::vector<uint8_t> &) override
   {
      return Errc::noact;
   }
   std::error_code SendUdp(const std::vector<uint8_t> & orc_Data) override
   {
      c_Broadcasts.push_back(orc_Data);
      return Errc::success;
   }
   std::error_code ReadUdp(std::vector<uint8_t> &, uint8_t (&)[4]) override { return Errc::noact; }
};

/// bind a driver to the recorder with a short broadcast timeout so the "no response"
/// loop exits quickly, returning the recorder the driver is bound to
C_BroadcastRecorder & mh_SetUpDriver(C_OscProtocolDriverOsyTpIp & orc_Driver, C_BroadcastRecorder & orc_Recorder)
{
   orc_Driver.SetBroadcastTimeout(50U);
   (void)orc_Driver.SetDispatcher(&orc_Recorder, 0U);
   return orc_Recorder;
}

bool mh_AllZero(const std::vector<uint8_t> & orc_Data, const std::size_t ou32_Start, const std::size_t ou32_Length)
{
   for (std::size_t u32_Index = ou32_Start; u32_Index < (ou32_Start + ou32_Length); ++u32_Index)
   {
      if (orc_Data[u32_Index] != 0U)
      {
         return false;
      }
   }
   return true;
}
}

/* -- Tests --------------------------------------------------------------------------------------------------------- */

/// The node-ID-only overload must set the mode flag to 0x02 (bit 2: node identifier,
/// no IP) and leave the IP/netmask/gateway fields zero, carrying only the node id.
TEST(ProtocolDriverOsyTp, SetIpAddressNodeIdOnly_ZeroesIpFieldsAndSetsModeTwo)
{
   C_OscProtocolDriverOsyTpIp c_Driver;
   C_BroadcastRecorder c_Recorder;
   (void)mh_SetUpDriver(c_Driver, c_Recorder);

   C_OscProtocolSerialNumber c_Serial;
   const uint8_t au8_Sn[6] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x12U};
   c_Serial.SetPosSerialNumber(au8_Sn);
   ASSERT_TRUE(c_Serial.q_IsValid);
   ASSERT_FALSE(c_Serial.q_ExtFormatUsed);

   C_OscProtocolDriverOsyNode c_NewNodeId;
   c_NewNodeId.u8_BusIdentifier = 3U;
   c_NewNodeId.u8_NodeIdentifier = 7U;
   uint8_t au8_ResponseIp[4] = {0U, 0U, 0U, 0U};

   const std::error_code c_Result = c_Driver.BroadcastSetIpAddress(c_Serial, c_NewNodeId, au8_ResponseIp);

   EXPECT_EQ(Errc::timeout, c_Result); // recorder never answers
   ASSERT_EQ(1U, c_Recorder.c_Broadcasts.size());
   const std::vector<uint8_t> & rc_Request = c_Recorder.c_Broadcasts[0];
   ASSERT_EQ(mhu_HEADER_SIZE + 21U, rc_Request.size());

   // serial number at the start of the payload
   for (std::size_t u32_Index = 0U; u32_Index < 6U; ++u32_Index)
   {
      EXPECT_EQ(au8_Sn[u32_Index], rc_Request[mhu_HEADER_SIZE + u32_Index]);
   }
   EXPECT_EQ(0x02U, rc_Request[mhu_HEADER_SIZE + 6U]);  // mode: node identifier only
   EXPECT_TRUE(mh_AllZero(rc_Request, mhu_HEADER_SIZE + 7U, 12U)); // IP, netmask, gateway all zero
   EXPECT_EQ(3U, rc_Request[mhu_HEADER_SIZE + 19U]); // bus identifier
   EXPECT_EQ(7U, rc_Request[mhu_HEADER_SIZE + 20U]); // node identifier
}

/// The full overload must set the mode flag to 0x03 (IP and node identifier) and
/// carry the supplied IP address.
TEST(ProtocolDriverOsyTp, SetIpAddressFull_SetsModeThreeAndTheGivenIp)
{
   C_OscProtocolDriverOsyTpIp c_Driver;
   C_BroadcastRecorder c_Recorder;
   (void)mh_SetUpDriver(c_Driver, c_Recorder);

   C_OscProtocolSerialNumber c_Serial;
   const uint8_t au8_Sn[6] = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U};
   c_Serial.SetPosSerialNumber(au8_Sn);

   const uint8_t au8_Ip[4] = {10U, 0U, 0U, 5U};
   const uint8_t au8_NetMask[4] = {255U, 255U, 255U, 0U};
   const uint8_t au8_Gateway[4] = {10U, 0U, 0U, 1U};
   C_OscProtocolDriverOsyNode c_NewNodeId;
   c_NewNodeId.u8_BusIdentifier = 1U;
   c_NewNodeId.u8_NodeIdentifier = 2U;
   uint8_t au8_ResponseIp[4] = {0U, 0U, 0U, 0U};

   const std::error_code c_Result =
      c_Driver.BroadcastSetIpAddress(c_Serial, au8_Ip, au8_NetMask, au8_Gateway, c_NewNodeId, au8_ResponseIp);

   EXPECT_EQ(Errc::timeout, c_Result);
   ASSERT_EQ(1U, c_Recorder.c_Broadcasts.size());
   const std::vector<uint8_t> & rc_Request = c_Recorder.c_Broadcasts[0];
   ASSERT_EQ(mhu_HEADER_SIZE + 21U, rc_Request.size());

   EXPECT_EQ(0x03U, rc_Request[mhu_HEADER_SIZE + 6U]);
   for (std::size_t u32_Index = 0U; u32_Index < 4U; ++u32_Index)
   {
      EXPECT_EQ(au8_Ip[u32_Index], rc_Request[mhu_HEADER_SIZE + 7U + u32_Index]) << "ip byte " << u32_Index;
      EXPECT_EQ(au8_NetMask[u32_Index], rc_Request[mhu_HEADER_SIZE + 11U + u32_Index]) << "netmask byte " << u32_Index;
      EXPECT_EQ(au8_Gateway[u32_Index], rc_Request[mhu_HEADER_SIZE + 15U + u32_Index]) << "gateway byte " << u32_Index;
   }
}

/// The extended node-ID-only overload uses the extended serial number layout (mode
/// flag at offset 0 of the payload) and must also zero the IP/netmask/gateway.
TEST(ProtocolDriverOsyTp, SetIpAddressExtendedNodeIdOnly_ZeroesIpAndSetsModeTwo)
{
   C_OscProtocolDriverOsyTpIp c_Driver;
   C_BroadcastRecorder c_Recorder;
   (void)mh_SetUpDriver(c_Driver, c_Recorder);

   C_OscProtocolSerialNumber c_Serial;
   ASSERT_EQ(Errc::success, c_Serial.SetExtSerialNumber(std::string("012345678912"), 0U));
   ASSERT_TRUE(c_Serial.q_ExtFormatUsed);

   C_OscProtocolDriverOsyNode c_NewNodeId;
   c_NewNodeId.u8_BusIdentifier = 2U;
   c_NewNodeId.u8_NodeIdentifier = 9U;
   uint8_t au8_ResponseIp[4] = {0U, 0U, 0U, 0U};

   const std::error_code c_Result =
      c_Driver.BroadcastSetIpAddressExtended(c_Serial, c_NewNodeId, 5U, au8_ResponseIp);

   EXPECT_EQ(Errc::timeout, c_Result);
   ASSERT_EQ(1U, c_Recorder.c_Broadcasts.size());
   const std::vector<uint8_t> & rc_Request = c_Recorder.c_Broadcasts[0];
   ASSERT_EQ(mhu_HEADER_SIZE + 18U + static_cast<std::size_t>(c_Serial.u8_SerialNumberByteLength), rc_Request.size());

   EXPECT_EQ(0x02U, rc_Request[mhu_HEADER_SIZE + 0U]);  // mode: node identifier only
   EXPECT_TRUE(mh_AllZero(rc_Request, mhu_HEADER_SIZE + 1U, 12U)); // IP, netmask, gateway all zero
   EXPECT_EQ(2U, rc_Request[mhu_HEADER_SIZE + 13U]); // bus identifier
   EXPECT_EQ(9U, rc_Request[mhu_HEADER_SIZE + 14U]); // node identifier
   EXPECT_EQ(5U, rc_Request[mhu_HEADER_SIZE + 15U]); // sub node id
   EXPECT_EQ(0U, rc_Request[mhu_HEADER_SIZE + 16U]); // manufacturer format
   EXPECT_EQ(c_Serial.u8_SerialNumberByteLength, rc_Request[mhu_HEADER_SIZE + 17U]);
}

/// The extended full overload must set the mode flag to 0x03 and carry the IP address.
TEST(ProtocolDriverOsyTp, SetIpAddressExtendedFull_SetsModeThreeAndTheGivenIp)
{
   C_OscProtocolDriverOsyTpIp c_Driver;
   C_BroadcastRecorder c_Recorder;
   (void)mh_SetUpDriver(c_Driver, c_Recorder);

   C_OscProtocolSerialNumber c_Serial;
   ASSERT_EQ(Errc::success, c_Serial.SetExtSerialNumber(std::string("AABBCCDDEEFF"), 0U));

   const uint8_t au8_Ip[4] = {192U, 168U, 0U, 10U};
   const uint8_t au8_NetMask[4] = {255U, 255U, 255U, 0U};
   const uint8_t au8_Gateway[4] = {192U, 168U, 0U, 1U};
   C_OscProtocolDriverOsyNode c_NewNodeId;
   c_NewNodeId.u8_BusIdentifier = 0U;
   c_NewNodeId.u8_NodeIdentifier = 4U;
   uint8_t au8_ResponseIp[4] = {0U, 0U, 0U, 0U};

   const std::error_code c_Result = c_Driver.BroadcastSetIpAddressExtended(
      c_Serial, au8_Ip, au8_NetMask, au8_Gateway, c_NewNodeId, 0U, au8_ResponseIp);

   EXPECT_EQ(Errc::timeout, c_Result);
   ASSERT_EQ(1U, c_Recorder.c_Broadcasts.size());
   const std::vector<uint8_t> & rc_Request = c_Recorder.c_Broadcasts[0];

   EXPECT_EQ(0x03U, rc_Request[mhu_HEADER_SIZE + 0U]);
   for (std::size_t u32_Index = 0U; u32_Index < 4U; ++u32_Index)
   {
      EXPECT_EQ(au8_Ip[u32_Index], rc_Request[mhu_HEADER_SIZE + 1U + u32_Index]) << "ip byte " << u32_Index;
      EXPECT_EQ(au8_NetMask[u32_Index], rc_Request[mhu_HEADER_SIZE + 5U + u32_Index]) << "netmask byte " << u32_Index;
      EXPECT_EQ(au8_Gateway[u32_Index], rc_Request[mhu_HEADER_SIZE + 9U + u32_Index]) << "gateway byte " << u32_Index;
   }
}

/// ConfigureDeviceBySerialNumber over an IP dispatcher must route the node-ID-only
/// SetIpAddress broadcast (mode 0x02, zero IP fields) through the IP transport.
TEST(DcBasicSequencesIp, ConfigureDeviceBySerialNumber_SendsNodeIdOnlyBroadcast)
{
   C_OscDcBasicSequences c_Sequences;
   C_BroadcastRecorder c_Recorder;

   ASSERT_EQ(Errc::success, c_Sequences.Init(nullptr, &c_Recorder));

   C_OscProtocolSerialNumber c_Serial;
   const uint8_t au8_Sn[6] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x12U};
   c_Serial.SetPosSerialNumber(au8_Sn);
   ASSERT_TRUE(c_Serial.q_IsValid);

   // the recorder never answers; the broadcast times out but the frame must still go out
   EXPECT_EQ(Errc::timeout, c_Sequences.ConfigureDeviceBySerialNumber(c_Serial, 7U));

   // For IP the "enter default session" step is a no-op, so the only broadcast sent
   // is the node-ID SetIpAddress request.
   ASSERT_FALSE(c_Recorder.c_Broadcasts.empty());
   const std::vector<uint8_t> & rc_Request = c_Recorder.c_Broadcasts.back();
   ASSERT_EQ(mhu_HEADER_SIZE + 21U, rc_Request.size());
   EXPECT_EQ(0x02U, rc_Request[mhu_HEADER_SIZE + 6U]);  // mode: node identifier only
   EXPECT_TRUE(mh_AllZero(rc_Request, mhu_HEADER_SIZE + 7U, 12U)); // IP, netmask, gateway all zero
   EXPECT_EQ(7U, rc_Request[mhu_HEADER_SIZE + 20U]); // new node identifier
}
