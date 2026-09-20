//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The CAN transport against a virtual ECU: segmentation, flow control and the update sequences on CAN

   C_OscProtocolDriverOsyTpCan puts services on the bus in ISO 15765-2 style frames (single, first,
   consecutive, flow control) plus openSYDE's own frame types, through the abstract C_CanDispatcher.
   A dispatcher double that owns a bus with virtual ECUs reassembles what the client sends, hands the
   payload to the same UDS server the Ethernet suite uses, and segments the answer back. That
   exercises the transport's state machines in both directions, which nothing else does, and lets
   C_OscSuSequences run a whole update on a CAN system definition.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <filesystem>
#include <string>
#include <vector>

#include "C_CanDispatcher.hpp"
#include "C_OscApplicationInfoBlock.hpp"
#include "C_OscDcBasicSequences.hpp"
#include "C_OscDcDeviceInformation.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscEndian.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscHexFile.hpp"
#include "C_OscNode.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscProtocolDriverOsyTpCan.hpp"
#include "C_OscSecurityPemDatabase.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscSystemDefinition.hpp"
#include "osy_virtual_ecu.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace osy_virtual_ecu;
using stw::can::T_STWCAN_Msg_RX;
using stw::can::T_STWCAN_Msg_TX;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
constexpr uint8_t mhu8_CLIENT_NODE_ID = 126U; //what C_OscComDriverProtocol uses for itself
//mhu8_ECU_NODE_ID and mhu8_BUS_ID come from osy_virtual_ecu.hpp

uint32_t mh_PhysicalId(const uint8_t ou8_Target, const uint8_t ou8_Source)
{
   return 0x18DA0000U | (static_cast<uint32_t>(ou8_Target) << 8U) | ou8_Source;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A CAN bus with virtual ECUs on it, behind the dispatcher interface

   The client's frames arrive through CAN_Send_Msg and are reassembled per device (single frame,
   first + consecutive with a flow control sent back, or openSYDE's multi frame without flow
   control). The device's answer is segmented the same way and queued for the client to read
   through m_CAN_Read_Msg. Everything that went either way is kept.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_VirtualCanBus :
   public stw::can::C_CanDispatcher
{
public:
   std::vector<T_STWCAN_Msg_TX> c_SentByClient;
   std::vector<T_STWCAN_Msg_RX> c_SentByDevices;
   int32_t s32_Bitrate;
   uint32_t u32_CorruptSequenceOfConsecutiveFrame; ///< 1-based index of a device CF to send with a wrong number, 0 = none
   bool q_DevicesWithholdFlowControl;              ///< true: no device answers a first frame with flow control

   C_VirtualCanBus(void) :
      s32_Bitrate(0),
      u32_CorruptSequenceOfConsecutiveFrame(0U),
      q_DevicesWithholdFlowControl(false),
      mu64_Time(0U)
   {
   }

   void Attach(const uint8_t ou8_NodeId, C_VirtualEcu & orc_Ecu)
   {
      T_Device c_Device;
      c_Device.u8_NodeId = ou8_NodeId;
      c_Device.pc_Ecu = &orc_Ecu;
      mc_Devices.push_back(c_Device);
   }

   ///a frame from a device the client did not ask anything, e.g. an event
   void InjectFromDevice(const uint8_t ou8_NodeId, const uint8_t ou8_Target, const std::vector<uint8_t> & orc_Data)
   {
      m_Queue(mh_PhysicalId(ou8_Target, ou8_NodeId), orc_Data);
   }

   std::vector<T_STWCAN_Msg_TX> SentTo(const uint32_t ou32_Id) const
   {
      std::vector<T_STWCAN_Msg_TX> c_Result;
      for (const T_STWCAN_Msg_TX & rc_Msg : c_SentByClient)
      {
         if (rc_Msg.u32_ID == ou32_Id)
         {
            c_Result.push_back(rc_Msg);
         }
      }
      return c_Result;
   }

   //C_CanBase
   std::error_code CAN_Init(void) override
   {
      return Errc::success;
   }

   std::error_code CAN_Init(const int32_t os32_BitrateKBitS) override
   {
      s32_Bitrate = os32_BitrateKBitS;
      return Errc::success;
   }

   std::error_code CAN_Exit(void) override
   {
      return Errc::success;
   }

   std::error_code CAN_Reset(void) override
   {
      return Errc::success;
   }

   std::error_code CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message) override
   {
      c_SentByClient.push_back(orc_Message);
      if (orc_Message.u8_XTD == 1U)
      {
         const uint32_t u32_Family = orc_Message.u32_ID & 0x1FFF0000U;
         const uint8_t u8_Source = static_cast<uint8_t>(orc_Message.u32_ID & 0xFFU);
         const uint8_t u8_Target = static_cast<uint8_t>((orc_Message.u32_ID >> 8U) & 0xFFU);
         const bool q_Broadcast = (u32_Family == 0x18DB0000U);
         if ((u32_Family == 0x18DA0000U) || q_Broadcast)
         {
            for (T_Device & rc_Device : mc_Devices)
            {
               if (q_Broadcast || (rc_Device.u8_NodeId == u8_Target))
               {
                  m_DeviceReceive(rc_Device, u8_Source, orc_Message, q_Broadcast);
               }
            }
         }
      }
      return Errc::success;
   }

   std::error_code CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const override
   {
      oru64_SystemTimeUs = mu64_Time;
      return Errc::success;
   }

protected:
   std::error_code m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message) override
   {
      if (mc_ToClient.empty())
      {
         return Errc::noact;
      }
      orc_Message = mc_ToClient.front();
      mc_ToClient.pop_front();
      return Errc::success;
   }

private:
   struct T_Device
   {
      uint8_t u8_NodeId;
      C_VirtualEcu * pc_Ecu;
      //receive side
      std::vector<uint8_t> c_Assembly;
      uint16_t u16_Expected = 0U;
      uint8_t u8_RxSequence = 0U;
      bool q_Receiving = false;
      //transmit side
      std::vector<uint8_t> c_Pending;
      uint16_t u16_TxIndex = 0U;
      uint8_t u8_TxSequence = 0U;
      uint8_t u8_TxTarget = 0U;
      bool q_WaitingForFlowControl = false;
      uint32_t u32_ConsecutiveFramesSent = 0U;
   };

   std::vector<T_Device> mc_Devices;
   std::deque<T_STWCAN_Msg_RX> mc_ToClient;
   uint64_t mu64_Time;

   void m_Queue(const uint32_t ou32_Id, const std::vector<uint8_t> & orc_Data)
   {
      T_STWCAN_Msg_RX c_Msg;
      c_Msg.u32_ID = ou32_Id;
      c_Msg.u8_XTD = 1U;
      c_Msg.u8_RTR = 0U;
      c_Msg.u8_DLC = static_cast<uint8_t>(orc_Data.size());
      (void)std::memset(&c_Msg.au8_Data[0], 0, 8U);
      (void)std::memcpy(&c_Msg.au8_Data[0], orc_Data.data(), orc_Data.size());
      mu64_Time += 1000U;
      c_Msg.u64_TimeStamp = mu64_Time;
      mc_ToClient.push_back(c_Msg);
      c_SentByDevices.push_back(c_Msg);
   }

   void m_DeviceReceive(T_Device & orc_Device, const uint8_t ou8_Source, const T_STWCAN_Msg_TX & orc_Msg,
                        const bool oq_Broadcast)
   {
      const uint8_t u8_Pci = orc_Msg.au8_Data[0] & 0xF0U;
      const uint8_t u8_Low = orc_Msg.au8_Data[0] & 0x0FU;
      switch (u8_Pci)
      {
      case 0x00U: //single frame
         m_DeviceHandle(orc_Device, ou8_Source,
                        std::vector<uint8_t>(&orc_Msg.au8_Data[1], &orc_Msg.au8_Data[1] + u8_Low), oq_Broadcast);
         break;
      case 0x10U: //first frame: 12 bit length, six bytes, answer with flow control
         orc_Device.u16_Expected = static_cast<uint16_t>((static_cast<uint16_t>(u8_Low) << 8U) | orc_Msg.au8_Data[1]);
         orc_Device.c_Assembly.assign(&orc_Msg.au8_Data[2], &orc_Msg.au8_Data[8]);
         orc_Device.u8_RxSequence = 1U;
         orc_Device.q_Receiving = true;
         if (q_DevicesWithholdFlowControl == false)
         {
            m_Queue(mh_PhysicalId(ou8_Source, orc_Device.u8_NodeId), {0x30U, 0x00U, 0x00U});
         }
         break;
      case 0x20U: //consecutive frame
         if (orc_Device.q_Receiving && (u8_Low == orc_Device.u8_RxSequence))
         {
            m_DeviceAppend(orc_Device, ou8_Source, orc_Msg, 0U);
         }
         else
         {
            orc_Device.q_Receiving = false;
         }
         break;
      case 0x30U: //flow control for something we are sending
         if (orc_Device.q_WaitingForFlowControl)
         {
            m_DeviceSendConsecutive(orc_Device);
         }
         break;
      case 0xE0U: //openSYDE multi frame: no flow control, sequence starts again at 1 after 15
         if (u8_Low == 0U)
         {
            orc_Device.u16_Expected = orc_Msg.au8_Data[1];
            orc_Device.c_Assembly.assign(&orc_Msg.au8_Data[2], &orc_Msg.au8_Data[8]);
            orc_Device.u8_RxSequence = 1U;
            orc_Device.q_Receiving = true;
         }
         else if (orc_Device.q_Receiving && (u8_Low == orc_Device.u8_RxSequence))
         {
            m_DeviceAppend(orc_Device, ou8_Source, orc_Msg, 1U);
         }
         else
         {
            orc_Device.q_Receiving = false;
         }
         break;
      default:
         break;
      }
   }

   void m_DeviceAppend(T_Device & orc_Device, const uint8_t ou8_Source, const T_STWCAN_Msg_TX & orc_Msg,
                       const uint8_t ou8_SequenceAfterFifteen)
   {
      const size_t x_Remaining = orc_Device.u16_Expected - orc_Device.c_Assembly.size();
      const size_t x_Take = std::min<size_t>(x_Remaining, static_cast<size_t>(orc_Msg.u8_DLC) - 1U);
      orc_Device.c_Assembly.insert(orc_Device.c_Assembly.end(), &orc_Msg.au8_Data[1], &orc_Msg.au8_Data[1] + x_Take);
      orc_Device.u8_RxSequence = static_cast<uint8_t>((orc_Device.u8_RxSequence + 1U) & 0x0FU);
      if (orc_Device.u8_RxSequence == 0U)
      {
         orc_Device.u8_RxSequence = ou8_SequenceAfterFifteen;
      }
      if (orc_Device.c_Assembly.size() >= orc_Device.u16_Expected)
      {
         orc_Device.q_Receiving = false;
         m_DeviceHandle(orc_Device, ou8_Source, orc_Device.c_Assembly, false);
      }
   }

   void m_DeviceHandle(T_Device & orc_Device, const uint8_t ou8_Source, const std::vector<uint8_t> & orc_Payload,
                       const bool oq_Broadcast)
   {
      if (orc_Payload.empty())
      {
         return;
      }
      if (oq_Broadcast && (orc_Payload[0] == 0xBDU))
      {
         //ReadSerialNumber broadcast: the plain form is answered, the extended one is not implemented here
         if (orc_Payload.size() == 1U)
         {
            std::vector<uint8_t> c_Response{0xFDU};
            c_Response.insert(c_Response.end(), &orc_Device.pc_Ecu->au8_SerialNumber[0],
                              &orc_Device.pc_Ecu->au8_SerialNumber[6]);
            m_DeviceSend(orc_Device, ou8_Source, c_Response);
         }
         return;
      }
      const std::optional<std::vector<uint8_t> > c_Answer = orc_Device.pc_Ecu->Handle(orc_Payload);
      //a broadcast session change carries the suppress-positive-response bit: nothing goes back
      const bool q_Suppressed = oq_Broadcast && (orc_Payload[0] == 0x10U) && ((orc_Payload[1] & 0x80U) != 0U);
      if (c_Answer.has_value() && (q_Suppressed == false))
      {
         m_DeviceSend(orc_Device, ou8_Source, *c_Answer);
      }
   }

   void m_DeviceSend(T_Device & orc_Device, const uint8_t ou8_Target, const std::vector<uint8_t> & orc_Payload)
   {
      const uint32_t u32_Id = mh_PhysicalId(ou8_Target, orc_Device.u8_NodeId);
      if (orc_Payload.size() <= 7U)
      {
         std::vector<uint8_t> c_Frame{static_cast<uint8_t>(orc_Payload.size())};
         c_Frame.insert(c_Frame.end(), orc_Payload.begin(), orc_Payload.end());
         m_Queue(u32_Id, c_Frame);
      }
      else
      {
         std::vector<uint8_t> c_Frame{static_cast<uint8_t>(0x10U | ((orc_Payload.size() >> 8U) & 0x0FU)),
                                      static_cast<uint8_t>(orc_Payload.size() & 0xFFU)};
         c_Frame.insert(c_Frame.end(), orc_Payload.begin(), orc_Payload.begin() + 6);
         m_Queue(u32_Id, c_Frame);
         orc_Device.c_Pending = orc_Payload;
         orc_Device.u16_TxIndex = 6U;
         orc_Device.u8_TxSequence = 1U;
         orc_Device.u8_TxTarget = ou8_Target;
         orc_Device.q_WaitingForFlowControl = true;
      }
   }

   void m_DeviceSendConsecutive(T_Device & orc_Device)
   {
      orc_Device.q_WaitingForFlowControl = false;
      while (orc_Device.u16_TxIndex < orc_Device.c_Pending.size())
      {
         const size_t x_Take = std::min<size_t>(7U, orc_Device.c_Pending.size() - orc_Device.u16_TxIndex);
         ++orc_Device.u32_ConsecutiveFramesSent;
         uint8_t u8_Sequence = orc_Device.u8_TxSequence;
         if (orc_Device.u32_ConsecutiveFramesSent == u32_CorruptSequenceOfConsecutiveFrame)
         {
            u8_Sequence = static_cast<uint8_t>((u8_Sequence + 1U) & 0x0FU);
         }
         std::vector<uint8_t> c_Frame{static_cast<uint8_t>(0x20U | u8_Sequence)};
         c_Frame.insert(c_Frame.end(), orc_Device.c_Pending.begin() + orc_Device.u16_TxIndex,
                        orc_Device.c_Pending.begin() + orc_Device.u16_TxIndex + static_cast<long>(x_Take));
         m_Queue(mh_PhysicalId(orc_Device.u8_TxTarget, orc_Device.u8_NodeId), c_Frame);
         orc_Device.u16_TxIndex = static_cast<uint16_t>(orc_Device.u16_TxIndex + x_Take);
         orc_Device.u8_TxSequence = static_cast<uint8_t>((orc_Device.u8_TxSequence + 1U) & 0x0FU);
      }
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Driver that records the asynchronous datapool events it is handed
*/
//----------------------------------------------------------------------------------------------------------------------
class C_EventRecordingDriver :
   public C_OscProtocolDriverOsy
{
public:
   std::vector<std::vector<uint8_t> > c_EventValues;

protected:
   void m_OsyReadDataPoolDataEventReceived(const uint8_t, const uint16_t, const uint16_t,
                                           const std::vector<uint8_t> & orc_Value) override
   {
      c_EventValues.push_back(orc_Value);
   }
};

class CanTransportVirtualEcu :
   public ::testing::Test
{
protected:
   void SetUp(void) override
   {
      mc_Bus.Attach(mhu8_ECU_NODE_ID, mc_Ecu);
      ASSERT_FALSE(static_cast<bool>(mc_Tp.SetNodeIdentifiers(C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_CLIENT_NODE_ID),
                                                               C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_ECU_NODE_ID))));
      ASSERT_FALSE(static_cast<bool>(mc_Tp.SetDispatcher(&mc_Bus)));
      mc_Driver.SetTimeoutPolling(200U);
      ASSERT_FALSE(static_cast<bool>(mc_Driver.SetNodeIdentifiers(C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_CLIENT_NODE_ID),
                                                                   C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_ECU_NODE_ID))));
      ASSERT_FALSE(static_cast<bool>(mc_Driver.SetTransportProtocol(&mc_Tp)));
   }

   C_VirtualEcu mc_Ecu;
   C_VirtualCanBus mc_Bus;
   C_OscProtocolDriverOsyTpCan mc_Tp;
   C_EventRecordingDriver mc_Driver;
};

const uint32_t mhu32_REQUEST_ID = mh_PhysicalId(mhu8_ECU_NODE_ID, mhu8_CLIENT_NODE_ID);  //0x18DA057E
const uint32_t mhu32_RESPONSE_ID = mh_PhysicalId(mhu8_CLIENT_NODE_ID, mhu8_ECU_NODE_ID); //0x18DA7E05
}

/* -- Transport level ------------------------------------------------------------------------------------------------ */

TEST_F(CanTransportVirtualEcu, SingleFrame_GoesOutOnThePhysicalIdWithTheLengthNibble)
{
   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadHardwareNumber(u32_Number));
   EXPECT_EQ(4711U, u32_Number);

   ASSERT_EQ(1U, mc_Bus.c_SentByClient.size());
   const T_STWCAN_Msg_TX & rc_Frame = mc_Bus.c_SentByClient[0];
   EXPECT_EQ(mhu32_REQUEST_ID, rc_Frame.u32_ID);
   EXPECT_EQ(1U, rc_Frame.u8_XTD);
   EXPECT_EQ(4U, rc_Frame.u8_DLC);
   EXPECT_EQ(0x03U, rc_Frame.au8_Data[0]); //single frame, three bytes
   EXPECT_EQ(0x22U, rc_Frame.au8_Data[1]);
   EXPECT_EQ(0xF1U, rc_Frame.au8_Data[2]);
   EXPECT_EQ(0x92U, rc_Frame.au8_Data[3]);

   //and the answer came back as one frame on the mirrored id
   ASSERT_EQ(1U, mc_Bus.c_SentByDevices.size());
   EXPECT_EQ(mhu32_RESPONSE_ID, mc_Bus.c_SentByDevices[0].u32_ID);
   EXPECT_EQ(0x07U, mc_Bus.c_SentByDevices[0].au8_Data[0]);
}

TEST_F(CanTransportVirtualEcu, MultiFrameRequest_FirstFrameWaitsForFlowControlThenConsecutiveFramesFollow)
{
   //4 + 20 = 24 bytes: first frame carries 6, three consecutive frames carry 7, 7, 4
   std::vector<uint8_t> c_Value(20U);
   for (uint8_t u8_Index = 0U; u8_Index < 20U; ++u8_Index)
   {
      c_Value[u8_Index] = static_cast<uint8_t>(0x40U + u8_Index);
   }
   EXPECT_EQ(Errc::success, mc_Driver.OsyWriteDataPoolData(1U, 2U, 3U, c_Value));

   const std::vector<T_STWCAN_Msg_TX> c_Frames = mc_Bus.SentTo(mhu32_REQUEST_ID);
   ASSERT_EQ(4U, c_Frames.size());
   EXPECT_EQ(0x10U, c_Frames[0].au8_Data[0]); //first frame, length 0x018
   EXPECT_EQ(24U, c_Frames[0].au8_Data[1]);
   EXPECT_EQ(0xBCU, c_Frames[0].au8_Data[2]); //service id is the first payload byte
   EXPECT_EQ(8U, c_Frames[0].u8_DLC);
   EXPECT_EQ(0x21U, c_Frames[1].au8_Data[0]);
   EXPECT_EQ(0x22U, c_Frames[2].au8_Data[0]);
   EXPECT_EQ(0x23U, c_Frames[3].au8_Data[0]);
   EXPECT_EQ(5U, c_Frames[3].u8_DLC); //4 bytes left

   //the device answered the first frame with flow control before the consecutive frames were sent
   ASSERT_GE(mc_Bus.c_SentByDevices.size(), 2U);
   EXPECT_EQ(0x30U, mc_Bus.c_SentByDevices[0].au8_Data[0]);
   EXPECT_EQ(3U, mc_Bus.c_SentByDevices[0].u8_DLC);

   //and it got the whole service
   const std::vector<std::vector<uint8_t> > c_Writes = mc_Ecu.RequestsFor(0xBCU);
   ASSERT_EQ(1U, c_Writes.size());
   ASSERT_EQ(24U, c_Writes[0].size());
   EXPECT_EQ(0x40U, c_Writes[0][4]);
   EXPECT_EQ(0x53U, c_Writes[0][23]);
}

TEST_F(CanTransportVirtualEcu, MultiFrameResponse_IsAnsweredWithFlowControlAndReassembled)
{
   //the flash block description is well over seven bytes
   C_OscProtocolDriverOsy::C_FlashBlockInfo c_Info;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadFlashBlockData(1U, c_Info));
   EXPECT_EQ("OldApp", c_Info.c_ApplicationName);
   EXPECT_EQ("V0.9", c_Info.c_ApplicationVersion);
   EXPECT_EQ(mhu32_APPLICATION_BASE, c_Info.u32_BlockStartAddress);

   //client: the request, then a flow control for the device's first frame
   const std::vector<T_STWCAN_Msg_TX> c_Frames = mc_Bus.SentTo(mhu32_REQUEST_ID);
   ASSERT_EQ(2U, c_Frames.size());
   EXPECT_EQ(0x05U, c_Frames[0].au8_Data[0]);
   EXPECT_EQ(0x30U, c_Frames[1].au8_Data[0]);
   EXPECT_EQ(0U, c_Frames[1].au8_Data[1]); //no block size limit
   EXPECT_EQ(0U, c_Frames[1].au8_Data[2]); //no separation time
   //device: first frame, then consecutive frames numbered from 1
   ASSERT_GE(mc_Bus.c_SentByDevices.size(), 3U);
   EXPECT_EQ(0x10U, mc_Bus.c_SentByDevices[0].au8_Data[0] & 0xF0U);
   EXPECT_EQ(0x21U, mc_Bus.c_SentByDevices[1].au8_Data[0]);
   EXPECT_EQ(0x22U, mc_Bus.c_SentByDevices[2].au8_Data[0]);
}

TEST_F(CanTransportVirtualEcu, LongTransfer_SequenceNumbersWrapFromFifteenToZero)
{
   //2 + 200 bytes: first frame 6, then 28 consecutive frames; the sequence nibble runs 1..15, 0, 1..
   std::vector<uint8_t> c_Data(200U);
   for (uint32_t u32_Index = 0U; u32_Index < 200U; ++u32_Index)
   {
      c_Data[u32_Index] = static_cast<uint8_t>(u32_Index);
   }
   mc_Ecu.Handle({0x34U, 0x00U, 0x44U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0xC8U}); //open a download so TransferData is accepted
   EXPECT_EQ(Errc::success, mc_Driver.OsyTransferData(1U, c_Data));

   const std::vector<T_STWCAN_Msg_TX> c_Frames = mc_Bus.SentTo(mhu32_REQUEST_ID);
   ASSERT_EQ(1U + 28U, c_Frames.size());
   EXPECT_EQ(0x2FU, c_Frames[15].au8_Data[0]);
   EXPECT_EQ(0x20U, c_Frames[16].au8_Data[0]);
   EXPECT_EQ(0x21U, c_Frames[17].au8_Data[0]);
   EXPECT_EQ(0x2CU, c_Frames[28].au8_Data[0]);
   EXPECT_EQ(8U, c_Frames[28].u8_DLC); //202 - 6 - 27 * 7 = 7 bytes: the last frame is full too

   const std::vector<std::vector<uint8_t> > c_Transfers = mc_Ecu.RequestsFor(0x36U);
   ASSERT_EQ(1U, c_Transfers.size());
   ASSERT_EQ(202U, c_Transfers[0].size());
   EXPECT_EQ(std::vector<uint8_t>(c_Transfers[0].begin() + 2, c_Transfers[0].end()), c_Data);
}

TEST_F(CanTransportVirtualEcu, ConsecutiveFrameOutOfSequence_DropsTheTransferAndTheCallTimesOut)
{
   //a name long enough for three consecutive frames; the second one carries the wrong number
   mc_Ecu.c_DeviceName = "VIRTUAL-ECU-WITH-A-LONG-NAME";
   mc_Bus.u32_CorruptSequenceOfConsecutiveFrame = 2U;

   std::string c_Name;
   EXPECT_EQ(Errc::timeout, mc_Driver.OsyReadDeviceName(c_Name));
   EXPECT_TRUE(c_Name.empty());

   //the device did send all its frames; the transport dropped the transfer at the bad one
   EXPECT_GE(mc_Bus.c_SentByDevices.size(), 4U);
}

TEST_F(CanTransportVirtualEcu, NoFlowControlFromTheDevice_RequestIsNeverCompleted)
{
   mc_Bus.q_DevicesWithholdFlowControl = true;
   std::vector<uint8_t> c_Value(20U, 0x11U);

   EXPECT_EQ(Errc::timeout, mc_Driver.OsyWriteDataPoolData(1U, 2U, 3U, c_Value));
   //only the first frame went out; no consecutive frame without flow control
   const std::vector<T_STWCAN_Msg_TX> c_Frames = mc_Bus.SentTo(mhu32_REQUEST_ID);
   ASSERT_EQ(1U, c_Frames.size());
   EXPECT_EQ(0x10U, c_Frames[0].au8_Data[0]);
   EXPECT_TRUE(mc_Ecu.RequestsFor(0xBCU).empty());
}

TEST_F(CanTransportVirtualEcu, FramesFromAnotherNode_DoNotReachThisDriver)
{
   //a device at node 6 answers something to us; our transport is bound to node 5 and must not take it
   mc_Bus.InjectFromDevice(6U, mhu8_CLIENT_NODE_ID, {0x07U, 0x62U, 0xF1U, 0x92U, 0U, 0U, 0U, 9U});

   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadHardwareNumber(u32_Number));
   EXPECT_EQ(4711U, u32_Number); //node 5's answer, not node 6's 9
}

TEST_F(CanTransportVirtualEcu, EventDrivenSingleFrame_IsDeliveredAsADatapoolEvent)
{
   //openSYDE single frame 0xF1: event-driven datapool value, packed id then value bytes
   mc_Bus.InjectFromDevice(mhu8_ECU_NODE_ID, mhu8_CLIENT_NODE_ID, {0xF1U, 0x0CU, 0x28U, 0x07U, 0xAAU, 0xBBU});

   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadHardwareNumber(u32_Number));
   ASSERT_EQ(1U, mc_Driver.c_EventValues.size());
   EXPECT_EQ(std::vector<uint8_t>({0xAAU, 0xBBU}), mc_Driver.c_EventValues[0]);
}

TEST_F(CanTransportVirtualEcu, BroadcastEnterPreProgrammingSession_UsesTheFunctionalIdAndSuppressesTheAnswer)
{
   EXPECT_EQ(Errc::success, mc_Tp.BroadcastSendEnterPreProgrammingSession());

   ASSERT_EQ(1U, mc_Bus.c_SentByClient.size());
   EXPECT_EQ(0x18DB7F00U | mhu8_CLIENT_NODE_ID, mc_Bus.c_SentByClient[0].u32_ID);
   EXPECT_EQ(3U, mc_Bus.c_SentByClient[0].u8_DLC);
   EXPECT_EQ(0x02U, mc_Bus.c_SentByClient[0].au8_Data[0]);
   EXPECT_EQ(0x10U, mc_Bus.c_SentByClient[0].au8_Data[1]);
   EXPECT_EQ(0xE0U, mc_Bus.c_SentByClient[0].au8_Data[2]); //0x60 with the suppress bit
   EXPECT_EQ(std::vector<uint8_t>({0xE0U}), mc_Ecu.c_Sessions);
   EXPECT_TRUE(mc_Bus.c_SentByDevices.empty());
}

TEST_F(CanTransportVirtualEcu, BroadcastReadSerialNumber_HearsEveryNodeOnTheBus)
{
   C_VirtualEcu c_Second;
   c_Second.au8_SerialNumber[0] = 0x99U;
   mc_Bus.Attach(6U, c_Second);

   C_OscProtocolDriverOsyTpCan c_Broadcast;
   ASSERT_FALSE(static_cast<bool>(c_Broadcast.SetNodeIdentifiersForBroadcasts(
                                     C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_CLIENT_NODE_ID))));
   ASSERT_FALSE(static_cast<bool>(c_Broadcast.SetDispatcher(&mc_Bus)));
   c_Broadcast.SetBroadcastTimeout(100U);

   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> c_Results;
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> c_Extended;
   EXPECT_EQ(Errc::success, c_Broadcast.BroadcastReadSerialNumber(c_Results, c_Extended));

   ASSERT_EQ(2U, c_Results.size());
   EXPECT_EQ(mhu8_ECU_NODE_ID, c_Results[0].c_SenderId.u8_NodeIdentifier);
   EXPECT_EQ(0x12U, c_Results[0].c_SerialNumber.au8_SerialNumber[0]);
   EXPECT_EQ(6U, c_Results[1].c_SenderId.u8_NodeIdentifier);
   EXPECT_EQ(0x99U, c_Results[1].c_SerialNumber.au8_SerialNumber[0]);
   EXPECT_TRUE(c_Extended.empty()); //the virtual device does not implement the extended form
}

/* -- The update sequences on a CAN system definition ---------------------------------------------------------------- */

namespace
{
std::vector<uint8_t> mh_MakeImage(const std::string & orc_DeviceId, uint32_t & oru32_SignatureOffset)
{
   std::vector<uint8_t> c_Image;
   for (uint8_t u8_Index = 0U; u8_Index < APPLICATION_INFO_MAGIC_LENGTH_V2; ++u8_Index)
   {
      c_Image.push_back(static_cast<uint8_t>(APPLICATION_INFO_MAGIC_V2[u8_Index]));
   }
   c_Image.push_back(2U);
   c_Image.push_back(0x01U);
   for (uint8_t u8_Index = 0U; u8_Index < 17U; ++u8_Index)
   {
      c_Image.push_back((u8_Index < orc_DeviceId.size()) ? static_cast<uint8_t>(orc_DeviceId[u8_Index]) : 0x20U);
   }
   while (c_Image.size() < 300U)
   {
      c_Image.push_back(static_cast<uint8_t>((c_Image.size() * 13U) & 0xFFU));
   }
   oru32_SignatureOffset = static_cast<uint32_t>(c_Image.size());
   for (const char cn_Char : std::string(";zwm2KgUZ!"))
   {
      c_Image.push_back(static_cast<uint8_t>(cn_Char));
   }
   return c_Image;
}

class SuSequencesVirtualEcuCan :
   public ::testing::Test
{
protected:
   void SetUp(void) override
   {
      C_OscSystemBus c_Bus;
      c_Bus.e_Type = C_OscSystemBus::eCAN;
      c_Bus.c_Name = "CAN1";
      c_Bus.u8_BusId = mhu8_BUS_ID;
      c_Bus.u64_BitRate = 125000U;
      mc_SystemDefinition.c_Buses.push_back(c_Bus);

      C_OscSubDeviceDefinition c_Sub;
      c_Sub.c_SubDeviceName = "VIRTUAL-ECU";
      c_Sub.q_ProgrammingSupport = true;
      c_Sub.q_DiagnosticProtocolOpenSydeCan = true;
      c_Sub.q_FlashloaderOpenSydeCan = true;
      c_Sub.q_FlashloaderOpenSydeIsFileBased = false;
      c_Sub.u32_FlashloaderResetWaitTimeNoChangesCan = 10U;
      c_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesCan = 10U;
      c_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesCan = 10U;
      c_Sub.u32_FlashloaderOpenSydeRequestDownloadTimeout = 200U;
      c_Sub.u32_FlashloaderOpenSydeTransferDataTimeout = 200U;
      mc_DeviceDefinition.c_DeviceName = "VIRTUAL-ECU";
      mc_DeviceDefinition.u8_NumCanBusses = 1U;
      mc_DeviceDefinition.c_SubDevices.push_back(c_Sub);

      C_OscNode c_Node;
      c_Node.c_DeviceType = "VIRTUAL-ECU";
      c_Node.pc_DeviceDefinition = &mc_DeviceDefinition;
      c_Node.u32_SubDeviceIndex = 0U;
      c_Node.c_Properties.c_Name = "Node1";
      c_Node.c_Properties.e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
      c_Node.c_Properties.e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;
      C_OscNodeComInterfaceSettings c_Can;
      c_Can.e_InterfaceType = C_OscSystemBus::eCAN;
      c_Can.u8_InterfaceNumber = 0U;
      c_Can.u8_NodeId = mhu8_ECU_NODE_ID;
      c_Can.q_IsUpdateEnabled = true;
      c_Can.q_IsDiagnosisEnabled = true;
      c_Can.q_IsRoutingEnabled = false;
      c_Can.AddConnection(0U);
      c_Can.SetInterfaceConnectedInDevice(true);
      c_Node.c_Properties.c_ComInterfaces.push_back(c_Can);
      mc_SystemDefinition.c_Nodes.push_back(c_Node);

      mc_Bus.Attach(mhu8_ECU_NODE_ID, mc_Ecu);
   }

   C_OscSystemDefinition mc_SystemDefinition;
   C_OscDeviceDefinition mc_DeviceDefinition;
   C_OscSecurityPemDatabase mc_PemDatabase;
   C_VirtualEcu mc_Ecu;
   C_VirtualCanBus mc_Bus;
   C_RecordingSequences mc_Sequences;
};
}

TEST_F(SuSequencesVirtualEcuCan, WholeUpdate_ActivateReadFlashReset)
{
   const std::vector<uint8_t> c_ActiveNodes(1U, 1U);
   ASSERT_EQ(Errc::success, mc_Sequences.Init(mc_SystemDefinition, 0U, c_ActiveNodes, &mc_Bus, nullptr, &mc_PemDatabase));

   uint32_t u32_SignatureOffset = 0U;
   const std::vector<uint8_t> c_Image = mh_MakeImage("VIRTUAL-ECU", u32_SignatureOffset);
   stw::hex_file::C_HexDataDump c_Dump;
   stw::hex_file::C_HexDataDumpBlock c_Block;
   c_Block.u32_AddressOffset = mhu32_APPLICATION_BASE;
   c_Block.au8_Data = c_Image;
   c_Dump.at_Blocks.push_back(c_Block);
   const std::string c_HexPath = (std::filesystem::temp_directory_path() / "osy_vecu_can_app.hex").string();
   {
      C_OscHexFile c_File;
      ASSERT_FALSE(static_cast<bool>(c_File.CreateHexFile(c_Dump, 32U)));
      ASSERT_FALSE(static_cast<bool>(c_File.SaveToFile(c_HexPath.c_str())));
   }
   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_HexPath);
   const std::vector<uint32_t> c_Order(1U, 0U);

   //ActivateFlashloader on CAN keeps broadcasting "enter pre-programming" for its five second scan window
   EXPECT_EQ(Errc::success, mc_Sequences.ActivateFlashloader());
   EXPECT_GT(mc_Bus.SentTo(0x18DB7F00U | mhu8_CLIENT_NODE_ID).size(), 50U);
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER}),
             mc_Ecu.c_ResetTypes);

   EXPECT_EQ(Errc::success, mc_Sequences.ReadDeviceInformation());
   ASSERT_EQ(1U, mc_Sequences.c_DeviceInformation.count(0U));
   EXPECT_EQ("VIRTUAL-ECU", mc_Sequences.c_DeviceInformation[0U].c_DeviceName);
   EXPECT_EQ(2U, mc_Sequences.c_DeviceInformation[0U].c_Applications.size());

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();
   ASSERT_EQ(1U, mc_Ecu.c_Flash.count(mhu32_APPLICATION_BASE));
   EXPECT_EQ(c_Image, mc_Ecu.c_Flash[mhu32_APPLICATION_BASE]);
   ASSERT_EQ(1U, mc_Ecu.c_SignatureAddresses.size());
   EXPECT_EQ(mhu32_APPLICATION_BASE + u32_SignatureOffset, mc_Ecu.c_SignatureAddresses[0]);

   EXPECT_EQ(Errc::success, mc_Sequences.ResetSystem());
   EXPECT_EQ(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON, mc_Ecu.c_ResetTypes.back());

   (void)std::remove(c_HexPath.c_str());
}

/* -- Device configuration: the scan-and-assign-node-ids sequences on a CAN bus ------------------------------------- */

namespace
{
class C_RecordingDcSequences :
   public C_OscDcBasicSequences
{
public:
   std::vector<C_OscDcDeviceInformation> c_Devices;
   bool q_SecurityFeatureUsed = false;
   std::vector<std::string> c_Errors;

protected:
   void m_ReportProgress(const int32_t os32_Result, const std::string & orc_Information) override
   {
      if (os32_Result != 0)
      {
         c_Errors.push_back(orc_Information);
      }
   }

   void m_ReportDevicesInfoRead(const std::vector<C_OscDcDeviceInformation> & orc_DeviceInfoResult,
                                const bool oq_SecurityFeatureUsed) override
   {
      c_Devices = orc_DeviceInfoResult;
      q_SecurityFeatureUsed = oq_SecurityFeatureUsed;
   }
};
}

TEST(DcBasicSequencesVirtualEcu, ScanConfigureAndResetTwoDevicesOnTheBus)
{
   //two devices, no system definition: this is how a fresh bus is brought up
   C_VirtualEcu c_First;
   C_VirtualEcu c_Second;
   c_Second.c_DeviceName = "OTHER-BOX";
   c_Second.au8_SerialNumber[0] = 0x77U;
   C_VirtualCanBus c_Bus;
   c_Bus.Attach(5U, c_First);
   c_Bus.Attach(6U, c_Second);
   C_RecordingDcSequences c_Sequences;

   ASSERT_EQ(Errc::success, c_Sequences.Init(&c_Bus));

   //activation: request programming answered by both, reset to flashloader, five seconds of "enter pre-programming"
   EXPECT_EQ(Errc::success, c_Sequences.ScanEnterFlashloader(10U));
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER}),
             c_First.c_ResetTypes);
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER}),
             c_Second.c_ResetTypes);
   EXPECT_GT(c_Bus.SentTo(0x18DB7F00U | mhu8_CLIENT_NODE_ID).size(), 50U);

   //the scan hears both serial numbers and reads both names
   EXPECT_EQ(Errc::success, c_Sequences.ScanGetInfo());
   ASSERT_EQ(2U, c_Sequences.c_Devices.size());
   EXPECT_EQ(5U, c_Sequences.c_Devices[0].u8_NodeId);
   EXPECT_EQ("VIRTUAL-ECU", c_Sequences.c_Devices[0].c_DeviceName);
   EXPECT_EQ(6U, c_Sequences.c_Devices[1].u8_NodeId);
   EXPECT_EQ("OTHER-BOX", c_Sequences.c_Devices[1].c_DeviceName);
   EXPECT_EQ(0x77U, c_Sequences.c_Devices[1].c_SerialNumber.au8_SerialNumber[0]);
   EXPECT_FALSE(c_Sequences.q_SecurityFeatureUsed);

   //the second device gets node id 9 and 250 kbit/s on its first CAN interface
   EXPECT_EQ(Errc::success, c_Sequences.ConfigureDevice(6U, 9U, 250U, 0U));
   ASSERT_EQ(1U, c_Second.c_NodeIdsSetForChannel.size());
   EXPECT_EQ(std::vector<uint8_t>({0U, 0U, mhu8_BUS_ID, 9U}), c_Second.c_NodeIdsSetForChannel[0]);
   ASSERT_EQ(1U, c_Second.c_BitratesSetForChannel.size());
   EXPECT_EQ(std::vector<uint8_t>({0U, 0U, 0x00U, 0x03U, 0xD0U, 0x90U}), c_Second.c_BitratesSetForChannel[0]);
   EXPECT_TRUE(c_First.c_NodeIdsSetForChannel.empty());
   //in the programming session, unlocked at level 1 with the fixed non-secure key
   EXPECT_NE(c_Second.c_Sessions.end(), std::find(c_Second.c_Sessions.begin(), c_Second.c_Sessions.end(), 0x02U));
   const std::vector<std::vector<uint8_t> > c_Keys = c_Second.RequestsFor(0x27U);
   ASSERT_EQ(2U, c_Keys.size());
   EXPECT_EQ(std::vector<uint8_t>({0x27U, 0x02U, 0U, 0U, 0U, 23U}), c_Keys[1]);

   //and everybody is reset for the new configuration to take effect
   EXPECT_EQ(Errc::success, c_Sequences.ResetSystem());
   EXPECT_EQ(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON, c_First.c_ResetTypes.back());
   EXPECT_EQ(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON, c_Second.c_ResetTypes.back());
   EXPECT_TRUE(c_Sequences.c_Errors.empty());
}
