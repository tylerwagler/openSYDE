//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the openSYDE transport protocol base and node identifier

   Groundwork for the protocol_drivers error-handling migration. That subsystem is
   ECU-facing, is the largest remaining wave, and had essentially no coverage - so
   the behaviour worth preserving is pinned here before it is rewritten.

   Everything covered is pure logic: no bus, no device. The transport base is an
   interface class, so the queue behaviour is driven through a minimal subclass
   that implements only the pure virtuals.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <vector>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"
#include "C_OscUpdateUtil.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

namespace
{
/// Smallest possible concrete transport: the base class does the queueing, and
/// the pure virtuals are the parts that would talk to a bus.
class C_TestTp :
   public C_OscProtocolDriverOsyTpBase
{
public:
   explicit C_TestTp(const uint16_t ou16_QueueSize) :
      C_OscProtocolDriverOsyTpBase(ou16_QueueSize)
   {
   }

   int32_t Cycle(void) override
   {
      return C_NO_ERR;
   }

   int32_t IsConnected(void) override
   {
      return C_NO_ERR;
   }

   int32_t ReConnect(void) override
   {
      return C_NO_ERR;
   }

   int32_t Disconnect(void) override
   {
      return C_NO_ERR;
   }

   /// expose the protected queue entry point so a response can be staged
   int32_t StageResponse(const C_OscProtocolDriverOsyService & orc_Service)
   {
      return this->m_AddToRxQueue(orc_Service);
   }
};

C_OscProtocolDriverOsyService h_MakeService(const size_t oun_Size)
{
   C_OscProtocolDriverOsyService c_Service;

   c_Service.c_Data.resize(oun_Size, 0xAAU);
   return c_Service;
}
}

// -- node identifier ---------------------------------------------------------

TEST(ProtocolDriverNode, ConstructionAndEquality)
{
   const C_OscProtocolDriverOsyNode c_First(1U, 5U);
   const C_OscProtocolDriverOsyNode c_Same(1U, 5U);
   const C_OscProtocolDriverOsyNode c_OtherNode(1U, 6U);
   const C_OscProtocolDriverOsyNode c_OtherBus(2U, 5U);

   EXPECT_EQ(1U, c_First.u8_BusIdentifier);
   EXPECT_EQ(5U, c_First.u8_NodeIdentifier);

   EXPECT_TRUE(c_First == c_Same);
   EXPECT_FALSE(c_First != c_Same);

   // both halves of the identifier have to participate, or two distinct ECUs
   // compare equal
   EXPECT_TRUE(c_First != c_OtherNode);
   EXPECT_TRUE(c_First != c_OtherBus);
}

TEST(ProtocolDriverNode, OrderingIsStrictWeak)
{
   const C_OscProtocolDriverOsyNode c_Low(1U, 5U);
   const C_OscProtocolDriverOsyNode c_HighNode(1U, 6U);
   const C_OscProtocolDriverOsyNode c_HighBus(2U, 0U);

   EXPECT_TRUE(c_Low < c_HighNode);
   EXPECT_FALSE(c_HighNode < c_Low);

   // bus is the more significant half
   EXPECT_TRUE(c_Low < c_HighBus);
   EXPECT_FALSE(c_HighBus < c_Low);

   EXPECT_FALSE(c_Low < c_Low) << "irreflexivity violated";
}

/// 0x7F is the broadcast node id and is deliberately not a usable node address.
TEST(ProtocolDriverNode, BroadcastIdIsTheReservedMaximum)
{
   EXPECT_EQ(0x7FU, C_OscProtocolDriverOsyNode::mhu8_NODE_ID_BROADCASTS);
   EXPECT_EQ(0x7FU, C_OscProtocolDriverOsyNode::mhu8_MAX_NODE);
   EXPECT_EQ(0x0FU, C_OscProtocolDriverOsyNode::mhu8_MAX_BUS);
}

// -- transport queueing ------------------------------------------------------

TEST(ProtocolDriverTpBase, SendRequestAcceptsServiceUpToMaximumSize)
{
   C_TestTp c_Tp(10U);

   EXPECT_EQ(C_NO_ERR, c_Tp.SendRequest(h_MakeService(1U)));
   EXPECT_EQ(C_NO_ERR,
             c_Tp.SendRequest(h_MakeService(C_OscProtocolDriverOsyTpBase::hu16_OSY_MAXIMUM_SERVICE_SIZE)));
}

/// A service larger than the protocol maximum must be refused rather than
/// truncated - it would be silently malformed on the wire.
TEST(ProtocolDriverTpBase, SendRequestRejectsOversizedService)
{
   C_TestTp c_Tp(10U);

   EXPECT_EQ(C_RANGE,
             c_Tp.SendRequest(h_MakeService(C_OscProtocolDriverOsyTpBase::hu16_OSY_MAXIMUM_SERVICE_SIZE + 1U)));
}

/// The queue is bounded, and overflow has to be reported. Silently dropping a
/// request would leave a flash sequence waiting for a response to a service
/// that was never queued.
TEST(ProtocolDriverTpBase, SendRequestReportsQueueOverflow)
{
   const uint16_t u16_QueueSize = 3U;
   C_TestTp c_Tp(u16_QueueSize);

   for (uint16_t u16_Index = 0U; u16_Index < u16_QueueSize; u16_Index++)
   {
      EXPECT_EQ(C_NO_ERR, c_Tp.SendRequest(h_MakeService(4U))) << "entry " << u16_Index;
   }

   EXPECT_EQ(C_OVERFLOW, c_Tp.SendRequest(h_MakeService(4U)));
}

TEST(ProtocolDriverTpBase, ReadResponseReportsEmptyQueue)
{
   C_TestTp c_Tp(10U);
   C_OscProtocolDriverOsyService c_Response;

   EXPECT_NE(C_NO_ERR, c_Tp.ReadResponse(c_Response)) << "an empty queue reported a response";
}

TEST(ProtocolDriverTpBase, ReadResponseReturnsStagedServiceFifo)
{
   C_TestTp c_Tp(10U);
   C_OscProtocolDriverOsyService c_First = h_MakeService(2U);
   C_OscProtocolDriverOsyService c_Second = h_MakeService(2U);

   c_First.c_Data[0] = 0x11U;
   c_Second.c_Data[0] = 0x22U;

   ASSERT_EQ(C_NO_ERR, c_Tp.StageResponse(c_First));
   ASSERT_EQ(C_NO_ERR, c_Tp.StageResponse(c_Second));

   C_OscProtocolDriverOsyService c_Read;
   ASSERT_EQ(C_NO_ERR, c_Tp.ReadResponse(c_Read));
   EXPECT_EQ(0x11U, c_Read.c_Data[0]) << "responses came back out of order";

   ASSERT_EQ(C_NO_ERR, c_Tp.ReadResponse(c_Read));
   EXPECT_EQ(0x22U, c_Read.c_Data[0]);

   EXPECT_NE(C_NO_ERR, c_Tp.ReadResponse(c_Read)) << "queue should now be empty";
}

// -- flash transfer timeout --------------------------------------------------

/// This value decides how long the tool waits for a device to accept a block of
/// firmware. Too short and a legitimate flash aborts partway through.
TEST(UpdateUtil, TransferTimeoutIsNeverBelowTheDeviceTimeout)
{
   const uint32_t u32_Device = 1000U;

   EXPECT_GE(C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(u32_Device, 512U, 500U), u32_Device);
   EXPECT_GE(C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(u32_Device, 0U, 500U), u32_Device);
}

/// A zero bitrate would divide by zero; the code substitutes the slowest
/// supported bus speed instead.
TEST(UpdateUtil, ZeroBitrateIsTreatedAsSlowestSupported)
{
   const uint32_t u32_Zero = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 512U, 0U);
   const uint32_t u32_Hundred = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 512U, 100U);

   EXPECT_EQ(u32_Hundred, u32_Zero) << "zero bitrate did not fall back to 100 kBit/s";
   EXPECT_GT(u32_Zero, 1000U) << "no offset was added at all";
}

/// A faster bus needs less slack, so the offset shrinks as bitrate rises.
TEST(UpdateUtil, FasterBitrateYieldsShorterTimeout)
{
   const uint32_t u32_Slow = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 4096U, 100U);
   const uint32_t u32_Medium = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 4096U, 500U);
   const uint32_t u32_Fast = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 4096U, 1000U);

   EXPECT_GT(u32_Slow, u32_Medium);
   EXPECT_GT(u32_Medium, u32_Fast);
   EXPECT_GE(u32_Fast, 1000U);
}

/// Pins the documented formula: ((block * 10 / 7) * 100) / bitrate, added to the
/// device timeout.
TEST(UpdateUtil, TimeoutMatchesDocumentedFormula)
{
   const uint32_t u32_Device = 1000U;
   const uint32_t u32_Block = 700U;
   const uint32_t u32_Bitrate = 100U;
   const uint32_t u32_Expected = u32_Device + ((((u32_Block * 10U) / 7U) * 100U) / u32_Bitrate);

   EXPECT_EQ(u32_Expected, C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(u32_Device, u32_Block, u32_Bitrate));
}

TEST(UpdateUtil, ZeroBlockLengthAddsNoOffset)
{
   EXPECT_EQ(1000U, C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(1000U, 0U, 500U));
}
