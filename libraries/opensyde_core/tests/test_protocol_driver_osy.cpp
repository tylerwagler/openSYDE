//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The openSYDE UDS driver over a scripted transport: every byte on the wire, without a device

   C_OscProtocolDriverOsy encodes each service request, polls the transport for the matching
   response and decodes it. The transport is an abstract queue pair, so a test double that
   answers from a script exercises the whole encode / poll / decode path in-process: the bytes
   the driver puts on the wire, what it makes of the bytes that come back, and how it treats
   negative responses, wrong lengths, wrong echoes, unsolicited events and silence.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <deque>
#include <initializer_list>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"
#include "C_OscProtocolSerialNumber.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
using T_Bytes = std::vector<uint8_t>;

T_Bytes mh_Bytes(const std::initializer_list<uint8_t> oc_Values)
{
   return T_Bytes(oc_Values);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A transport that answers from a script

   Cycle() is where a real transport talks to a bus. This one moves every queued request into the
   record, and for each request hands out the next scripted reply (if any) as if the server had
   answered at once. Injected services are unsolicited: they land in the Rx queue on the next cycle
   whether or not anything was sent.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_ScriptedTransport :
   public C_OscProtocolDriverOsyTpBase
{
public:
   C_ScriptedTransport(void) :
      C_OscProtocolDriverOsyTpBase(50U),
      u32_Cycles(0U)
   {
   }

   std::error_code Cycle(void) override
   {
      ++u32_Cycles;
      for (const T_Bytes & rc_Unsolicited : c_Injected)
      {
         C_OscProtocolDriverOsyService c_Service;
         c_Service.c_Data = rc_Unsolicited;
         (void)m_AddToRxQueue(c_Service);
      }
      c_Injected.clear();

      C_OscProtocolDriverOsyService c_Request;
      while (!m_GetFromTxQueue(c_Request))
      {
         c_Requests.push_back(c_Request.c_Data);
         if (c_Replies.empty() == false)
         {
            for (const T_Bytes & rc_Reply : c_Replies.front())
            {
               C_OscProtocolDriverOsyService c_Response;
               c_Response.c_Data = rc_Reply;
               (void)m_AddToRxQueue(c_Response);
            }
            c_Replies.pop_front();
         }
      }
      return Errc::success;
   }

   ///the next request gets exactly these services back, in this order
   void Reply(const std::initializer_list<T_Bytes> oc_Services)
   {
      c_Replies.emplace_back(oc_Services);
   }

   ///arrives on the next cycle, unasked
   void Inject(const T_Bytes & orc_Service)
   {
      c_Injected.push_back(orc_Service);
   }

   const T_Bytes & LastRequest(void) const
   {
      return c_Requests.back();
   }

   std::vector<T_Bytes> c_Requests;
   uint32_t u32_Cycles;

private:
   std::deque<std::vector<T_Bytes> > c_Replies;
   std::vector<T_Bytes> c_Injected;
};

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Driver that records the asynchronous datapool events it is handed
*/
//----------------------------------------------------------------------------------------------------------------------
class C_RecordingDriver :
   public C_OscProtocolDriverOsy
{
public:
   struct T_Event
   {
      uint8_t u8_DataPool;
      uint16_t u16_List;
      uint16_t u16_Element;
      T_Bytes c_Value;
   };
   struct T_ErrorEvent
   {
      uint8_t u8_DataPool;
      uint16_t u16_List;
      uint16_t u16_Element;
      uint8_t u8_NrCode;
   };

   std::vector<T_Event> c_Events;
   std::vector<T_ErrorEvent> c_ErrorEvents;

protected:
   void m_OsyReadDataPoolDataEventReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                           const uint16_t ou16_ElementIndex,
                                           const std::vector<uint8_t> & orc_Value) override
   {
      c_Events.push_back({ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_Value});
   }

   void m_OsyReadDataPoolDataEventErrorReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                const uint16_t ou16_ElementIndex, const uint8_t ou8_NrCode) override
   {
      c_ErrorEvents.push_back({ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, ou8_NrCode});
   }
};

class ProtocolDriverOsy :
   public ::testing::Test
{
protected:
   void SetUp(void) override
   {
      //silence is a 20 ms wait, not the one second a device gets
      mc_Driver.SetTimeoutPolling(20U);
      ASSERT_FALSE(static_cast<bool>(mc_Driver.SetNodeIdentifiers(C_OscProtocolDriverOsyNode(0U, 1U),
                                                                   C_OscProtocolDriverOsyNode(0U, 5U))));
      ASSERT_FALSE(static_cast<bool>(mc_Driver.SetTransportProtocol(&mc_Transport)));
   }

   C_ScriptedTransport mc_Transport;
   C_RecordingDriver mc_Driver;
};
}

/* -- ReadDataByIdentifier family ------------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverOsy, ReadHardwareNumber_EncodesTheRequestAndDecodesBigEndian)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xF1U, 0x92U, 0x12U, 0x34U, 0x56U, 0x78U})});

   uint32_t u32_Number = 0U;
   uint8_t u8_Nrc = 0xEEU;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadHardwareNumber(u32_Number, &u8_Nrc));

   EXPECT_EQ(mh_Bytes({0x22U, 0xF1U, 0x92U}), mc_Transport.LastRequest());
   EXPECT_EQ(0x12345678U, u32_Number);
   EXPECT_EQ(0U, u8_Nrc);
}

TEST_F(ProtocolDriverOsy, ReadDeviceName_IsTheUnterminatedPayloadAsText)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xA8U, 0x1AU, 'E', 'S', 'X', '-', '3'})});

   std::string c_Name;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadDeviceName(c_Name));
   EXPECT_EQ("ESX-3", c_Name);
   EXPECT_EQ(mh_Bytes({0x22U, 0xA8U, 0x1AU}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverOsy, ReadEcuSerialNumber_SixBytesInPosFormat)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xF1U, 0x8CU, 0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x0AU})});

   C_OscProtocolSerialNumber c_Serial;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadEcuSerialNumber(c_Serial));
   EXPECT_TRUE(c_Serial.q_IsValid);
   EXPECT_FALSE(c_Serial.q_ExtFormatUsed);
   const uint8_t au8_Expected[6] = {0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0x0AU};
   for (uint32_t u32_Index = 0U; u32_Index < 6U; ++u32_Index)
   {
      EXPECT_EQ(au8_Expected[u32_Index], c_Serial.au8_SerialNumber[u32_Index]) << u32_Index;
   }
}

TEST_F(ProtocolDriverOsy, ReadListOfFeatures_BitsMapToTheFlags)
{
   //byte 7 carries eight flags, byte 6 bit 0 the ninth
   mc_Transport.Reply({mh_Bytes({0x62U, 0xA8U, 0x00U, 0U, 0U, 0U, 0U, 0U, 0U, 0x01U, 0x93U})});

   C_OscProtocolDriverOsy::C_ListOfFeatures c_Features;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadListOfFeatures(c_Features));
   EXPECT_TRUE(c_Features.q_FlashloaderCanWriteToNvm);              //0x01
   EXPECT_TRUE(c_Features.q_MaxNumberOfBlockLengthAvailable);       //0x02
   EXPECT_FALSE(c_Features.q_EthernetToEthernetRoutingSupported);   //0x04
   EXPECT_FALSE(c_Features.q_FileBasedTransferExitResultAvailable); //0x08
   EXPECT_TRUE(c_Features.q_ExtendedSerialNumberModeImplemented);   //0x10
   EXPECT_FALSE(c_Features.q_SupportsSecurityAuthentication);       //0x20
   EXPECT_FALSE(c_Features.q_SupportsDebuggerOff);                  //0x40
   EXPECT_TRUE(c_Features.q_SupportsDebuggerOn);                    //0x80
   EXPECT_TRUE(c_Features.q_SupportsSecurityTrafficEncryption);     //byte 6, 0x01
}

TEST_F(ProtocolDriverOsy, ReadMaxNumberOfBlockLength_IsBigEndian16)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xA8U, 0x01U, 0x04U, 0x00U})});

   uint16_t u16_Length = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadMaxNumberOfBlockLength(u16_Length));
   EXPECT_EQ(1024U, u16_Length);
}

TEST_F(ProtocolDriverOsy, ReadApplicationSoftwareFingerprint_SplitsDateTimeAndUser)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xF1U, 0x84U, 26U, 9U, 18U, 12U, 34U, 56U, 4U, 'u', 's', 'e', 'r'})});

   uint8_t au8_Date[3] = {0U};
   uint8_t au8_Time[3] = {0U};
   std::string c_User;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadApplicationSoftwareFingerprint(au8_Date, au8_Time, c_User));
   EXPECT_EQ(26U, au8_Date[0]);
   EXPECT_EQ(18U, au8_Date[2]);
   EXPECT_EQ(12U, au8_Time[0]);
   EXPECT_EQ(56U, au8_Time[2]);
   EXPECT_EQ("user", c_User);
}

TEST_F(ProtocolDriverOsy, WriteApplicationSoftwareFingerprint_TruncatesTheUserToTwentyCharacters)
{
   mc_Transport.Reply({mh_Bytes({0x6EU, 0xF1U, 0x84U})});

   const uint8_t au8_Date[3] = {26U, 9U, 18U};
   const uint8_t au8_Time[3] = {1U, 2U, 3U};
   EXPECT_EQ(Errc::success,
             mc_Driver.OsyWriteApplicationSoftwareFingerprint(au8_Date, au8_Time, "abcdefghijklmnopqrstuvwxyz"));

   const T_Bytes & rc_Request = mc_Transport.LastRequest();
   ASSERT_EQ(3U + 7U + 20U, rc_Request.size());
   EXPECT_EQ(0x2EU, rc_Request[0]);
   EXPECT_EQ(0xF1U, rc_Request[1]);
   EXPECT_EQ(0x84U, rc_Request[2]);
   EXPECT_EQ(26U, rc_Request[3]);
   EXPECT_EQ(3U, rc_Request[8]);
   EXPECT_EQ(20U, rc_Request[9]); //length byte
   EXPECT_EQ('a', rc_Request[10]);
   EXPECT_EQ('t', rc_Request[29]);
}

/* -- What the driver does with a response it did not ask for ------------------------------------------------------ */

TEST_F(ProtocolDriverOsy, NegativeResponse_IsWarnWithTheCode)
{
   mc_Transport.Reply({mh_Bytes({0x7FU, 0x22U, 0x31U})});

   uint32_t u32_Number = 0U;
   uint8_t u8_Nrc = 0U;
   EXPECT_EQ(Errc::warn, mc_Driver.OsyReadHardwareNumber(u32_Number, &u8_Nrc));
   EXPECT_EQ(C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE, u8_Nrc);
}

TEST_F(ProtocolDriverOsy, ResponsePending_IsWaitedOutUntilThePositiveResponse)
{
   mc_Transport.Reply({mh_Bytes({0x7FU, 0x22U, 0x78U}), mh_Bytes({0x62U, 0xF1U, 0x92U, 0U, 0U, 0U, 7U})});

   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadHardwareNumber(u32_Number));
   EXPECT_EQ(7U, u32_Number);
}

TEST_F(ProtocolDriverOsy, WrongDataIdentifierInResponse_IsRdWr)
{
   mc_Transport.Reply({mh_Bytes({0x62U, 0xF1U, 0x93U, 1U, 2U, 3U, 4U})});

   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyReadHardwareNumber(u32_Number));
}

TEST_F(ProtocolDriverOsy, ResponseOfTheWrongLength_IsIgnoredAndTheCallTimesOut)
{
   //three payload bytes where exactly four are required
   mc_Transport.Reply({mh_Bytes({0x62U, 0xF1U, 0x92U, 1U, 2U, 3U})});

   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::timeout, mc_Driver.OsyReadHardwareNumber(u32_Number));
}

TEST_F(ProtocolDriverOsy, Silence_IsTimeout)
{
   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::timeout, mc_Driver.OsyReadHardwareNumber(u32_Number));
   EXPECT_GT(mc_Transport.u32_Cycles, 1U); //it kept polling until the deadline
}

TEST_F(ProtocolDriverOsy, NoTransport_IsConfigWithoutTouchingTheWire)
{
   C_OscProtocolDriverOsy c_Bare;
   uint32_t u32_Number = 0U;
   EXPECT_EQ(Errc::config, c_Bare.OsyReadHardwareNumber(u32_Number));
}

TEST_F(ProtocolDriverOsy, UnsolicitedEventsDuringAPoll_AreDispatchedAndThePollStillCompletes)
{
   //a cyclic datapool value and a datapool error arrive while we wait for TesterPresent
   mc_Transport.Inject(mh_Bytes({0xFAU, 0x0CU, 0x28U, 0x07U, 0xAAU, 0xBBU}));
   mc_Transport.Inject(mh_Bytes({0x7FU, 0xBAU, 0x31U, 0x0CU, 0x28U, 0x07U}));
   mc_Transport.Reply({mh_Bytes({0x7EU, 0x00U})});

   EXPECT_EQ(Errc::success, mc_Driver.OsyTesterPresent(0U));

   ASSERT_EQ(1U, mc_Driver.c_Events.size());
   EXPECT_EQ(3U, mc_Driver.c_Events[0].u8_DataPool);
   EXPECT_EQ(5U, mc_Driver.c_Events[0].u16_List);
   EXPECT_EQ(7U, mc_Driver.c_Events[0].u16_Element);
   EXPECT_EQ(mh_Bytes({0xAAU, 0xBBU}), mc_Driver.c_Events[0].c_Value);

   ASSERT_EQ(1U, mc_Driver.c_ErrorEvents.size());
   EXPECT_EQ(3U, mc_Driver.c_ErrorEvents[0].u8_DataPool);
   EXPECT_EQ(7U, mc_Driver.c_ErrorEvents[0].u16_Element);
   EXPECT_EQ(0x31U, mc_Driver.c_ErrorEvents[0].u8_NrCode);
}

/* -- Datapool access: the packed 3 byte identifier ------------------------------------------------------------------ */

TEST_F(ProtocolDriverOsy, ReadDataPoolData_PacksDatapoolListAndElementIntoThreeBytes)
{
   //(3 << 18) | (5 << 11) | 7 = 0x0C2807
   mc_Transport.Reply({mh_Bytes({0xFBU, 0x0CU, 0x28U, 0x07U, 0xDEU, 0xADU})});

   T_Bytes c_Data;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadDataPoolData(3U, 5U, 7U, c_Data));
   EXPECT_EQ(mh_Bytes({0xBBU, 0x0CU, 0x28U, 0x07U}), mc_Transport.LastRequest());
   EXPECT_EQ(mh_Bytes({0xDEU, 0xADU}), c_Data);
}

TEST_F(ProtocolDriverOsy, ReadDataPoolData_UsesTheServerIndexFromTheMapping)
{
   std::map<uint8_t, uint8_t> c_Mapping;
   c_Mapping[3U] = 9U;
   mc_Driver.RegisterDataPoolMapping(c_Mapping);
   //(9 << 18) | (5 << 11) | 7 = 0x242807
   mc_Transport.Reply({mh_Bytes({0xFBU, 0x24U, 0x28U, 0x07U, 0x01U})});

   T_Bytes c_Data;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadDataPoolData(3U, 5U, 7U, c_Data));
   EXPECT_EQ(mh_Bytes({0xBBU, 0x24U, 0x28U, 0x07U}), mc_Transport.LastRequest());

   //and an event for server datapool 9 comes back as client datapool 3
   mc_Transport.Inject(mh_Bytes({0xFAU, 0x24U, 0x28U, 0x07U, 0x55U}));
   mc_Transport.Reply({mh_Bytes({0x7EU, 0x00U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyTesterPresent(0U));
   ASSERT_EQ(1U, mc_Driver.c_Events.size());
   EXPECT_EQ(3U, mc_Driver.c_Events[0].u8_DataPool);
}

TEST_F(ProtocolDriverOsy, ReadDataPoolData_IndexOutOfRange_IsRangeAndNothingIsSent)
{
   T_Bytes c_Data;
   EXPECT_EQ(Errc::range, mc_Driver.OsyReadDataPoolData(3U, 128U, 7U, c_Data));   //128 lists max
   EXPECT_EQ(Errc::range, mc_Driver.OsyReadDataPoolData(32U, 0U, 0U, c_Data));    //32 datapools max
   EXPECT_EQ(Errc::range, mc_Driver.OsyReadDataPoolData(0U, 0U, 2048U, c_Data)); //2048 elements max
   EXPECT_TRUE(mc_Transport.c_Requests.empty());
}

TEST_F(ProtocolDriverOsy, ReadDataPoolData_EchoOfAnotherElement_IsRdWr)
{
   mc_Transport.Reply({mh_Bytes({0xFBU, 0x0CU, 0x28U, 0x08U, 0x00U})});

   T_Bytes c_Data;
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyReadDataPoolData(3U, 5U, 7U, c_Data));
}

TEST_F(ProtocolDriverOsy, WriteDataPoolData_AppendsTheValueAndExpectsTheEcho)
{
   mc_Transport.Reply({mh_Bytes({0xFCU, 0x0CU, 0x28U, 0x07U})});

   EXPECT_EQ(Errc::success, mc_Driver.OsyWriteDataPoolData(3U, 5U, 7U, mh_Bytes({0x11U, 0x22U, 0x33U})));
   EXPECT_EQ(mh_Bytes({0xBCU, 0x0CU, 0x28U, 0x07U, 0x11U, 0x22U, 0x33U}), mc_Transport.LastRequest());

   EXPECT_EQ(Errc::range, mc_Driver.OsyWriteDataPoolData(3U, 5U, 7U, T_Bytes()));
}

TEST_F(ProtocolDriverOsy, ReadDataPoolDataCyclic_RailIsOneBased_AndTheNegativeResponseCarriesTheIdentifier)
{
   mc_Transport.Reply({mh_Bytes({0xFAU, 0x0CU, 0x28U, 0x07U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadDataPoolDataCyclic(3U, 5U, 7U, 1U));
   EXPECT_EQ(mh_Bytes({0xBAU, 0x02U, 0x0CU, 0x28U, 0x07U}), mc_Transport.LastRequest());

   //rail 3 does not exist
   EXPECT_EQ(Errc::range, mc_Driver.OsyReadDataPoolDataCyclic(3U, 5U, 7U, 3U));

   //the server rejects this element by name
   mc_Transport.Reply({mh_Bytes({0x7FU, 0xBAU, 0x31U, 0x0CU, 0x28U, 0x07U})});
   uint8_t u8_Nrc = 0U;
   EXPECT_EQ(Errc::warn, mc_Driver.OsyReadDataPoolDataCyclic(3U, 5U, 7U, 0U, &u8_Nrc));
   EXPECT_EQ(0x31U, u8_Nrc);
}

/* -- RoutineControl family ------------------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverOsy, CheckFlashMemoryAvailable_IsStartRoutine0208WithAddressAndSize)
{
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x08U})});

   EXPECT_EQ(Errc::success, mc_Driver.OsyCheckFlashMemoryAvailable(0x80001000U, 0x00020000U));
   EXPECT_EQ(mh_Bytes({0x31U, 0x01U, 0x02U, 0x08U, 0x80U, 0x00U, 0x10U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U}),
             mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverOsy, RequestProgramming_TakesNoPayload)
{
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x06U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyRequestProgramming());
   EXPECT_EQ(mh_Bytes({0x31U, 0x01U, 0x02U, 0x06U}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverOsy, VerifyDataPool_ChecksumIsBigEndian_AndBitZeroMeansMismatch)
{
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x12U, 4U, 0x00U})});
   bool q_Match = false;
   EXPECT_EQ(Errc::success, mc_Driver.OsyVerifyDataPool(4U, 0xCAFEBABEU, q_Match));
   EXPECT_TRUE(q_Match);
   EXPECT_EQ(mh_Bytes({0x31U, 0x01U, 0x02U, 0x12U, 4U, 0xCAU, 0xFEU, 0xBAU, 0xBEU}), mc_Transport.LastRequest());

   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x12U, 4U, 0x01U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyVerifyDataPool(4U, 0xCAFEBABEU, q_Match));
   EXPECT_FALSE(q_Match);

   //answer about a different datapool
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x12U, 5U, 0x00U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyVerifyDataPool(4U, 0xCAFEBABEU, q_Match));
}

TEST_F(ProtocolDriverOsy, ReadDataPoolMetaData_ParsesVersionAndNameTlvs)
{
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x11U, 2U, 1U, 1U, 4U, 9U, 2U, 3U, 'D', 'P', 'x'})});

   C_OscProtocolDriverOsy::C_DataPoolMetaData c_Meta;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadDataPoolMetaData(2U, c_Meta));
   EXPECT_EQ(1U, c_Meta.au8_Version[0]);
   EXPECT_EQ(4U, c_Meta.au8_Version[1]);
   EXPECT_EQ(9U, c_Meta.au8_Version[2]);
   EXPECT_EQ("DPx", c_Meta.c_Name);
}

TEST_F(ProtocolDriverOsy, ReadFlashBlockData_ParsesEveryTlvInOrder)
{
   T_Bytes c_Reply = mh_Bytes({0x71U, 0x01U, 0x02U, 0x09U});
   //1: addresses
   for (const uint8_t u8_Byte : mh_Bytes({1U, 0x00U, 0x10U, 0x00U, 0x00U, 0x00U, 0x1FU, 0xFFU, 0xFFU}))
   {
      c_Reply.push_back(u8_Byte);
   }
   //2: signature result
   c_Reply.push_back(2U);
   c_Reply.push_back(0U);
   //3: application version
   for (const uint8_t u8_Byte : mh_Bytes({3U, 5U, 'V', '1', '.', '2', 'a'}))
   {
      c_Reply.push_back(u8_Byte);
   }
   //4: build timestamp, 11 + 8 characters, no length bytes
   c_Reply.push_back(4U);
   for (const char cn_Char : std::string("Sep 18 2026" "12:34:56"))
   {
      c_Reply.push_back(static_cast<uint8_t>(cn_Char));
   }
   //5: application name
   for (const uint8_t u8_Byte : mh_Bytes({5U, 3U, 'A', 'p', 'p'}))
   {
      c_Reply.push_back(u8_Byte);
   }
   //6: additional information
   for (const uint8_t u8_Byte : mh_Bytes({6U, 4U, 'i', 'n', 'f', 'o'}))
   {
      c_Reply.push_back(u8_Byte);
   }
   mc_Transport.Reply({c_Reply});

   C_OscProtocolDriverOsy::C_FlashBlockInfo c_Info;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadFlashBlockData(1U, c_Info));
   EXPECT_EQ(mh_Bytes({0x31U, 0x01U, 0x02U, 0x09U, 1U}), mc_Transport.LastRequest());
   EXPECT_EQ(0x00100000U, c_Info.u32_BlockStartAddress);
   EXPECT_EQ(0x001FFFFFU, c_Info.u32_BlockEndAddress);
   EXPECT_EQ(0U, c_Info.u8_SignatureValid);
   EXPECT_EQ("V1.2a", c_Info.c_ApplicationVersion);
   EXPECT_EQ("Sep 18 2026", c_Info.c_BuildDate);
   EXPECT_EQ("12:34:56", c_Info.c_BuildTime);
   EXPECT_EQ("App", c_Info.c_ApplicationName);
   EXPECT_EQ("info", c_Info.c_AdditionalInformation);
}

TEST_F(ProtocolDriverOsy, ReadFlashBlockData_FieldsAreOptionalAndTheResponseMayEndAfterAnyOfThem)
{
   //addresses, signature and name only: no version, no timestamp, no additional information. Before
   //2026-09-18 the parser tested for each next tag by indexing one past the end of a response that
   //ended early, which Ubuntu's assertion-enabled libstdc++ turned into an abort in CI.
   T_Bytes c_Reply = mh_Bytes({0x71U, 0x01U, 0x02U, 0x09U, 1U, 0x00U, 0x00U, 0x10U, 0x00U, 0x00U, 0x00U, 0x1FU, 0xFFU,
                               2U, 1U, 5U, 3U, 'A', 'p', 'p'});
   mc_Transport.Reply({c_Reply});

   C_OscProtocolDriverOsy::C_FlashBlockInfo c_Info;
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadFlashBlockData(0U, c_Info));
   EXPECT_EQ(0x00001000U, c_Info.u32_BlockStartAddress);
   EXPECT_EQ(1U, c_Info.u8_SignatureValid);
   EXPECT_EQ("App", c_Info.c_ApplicationName);
   EXPECT_TRUE(c_Info.c_ApplicationVersion.empty());
   EXPECT_TRUE(c_Info.c_BuildDate.empty());
   EXPECT_TRUE(c_Info.c_AdditionalInformation.empty());

   //a response with no fields at all is fine too
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x09U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadFlashBlockData(0U, c_Info));
   EXPECT_TRUE(c_Info.c_ApplicationName.empty());
}

TEST_F(ProtocolDriverOsy, ReadFlashBlockData_AFieldCutShortIsReportedNotReadPast)
{
   //the name claims five characters and the response carries two
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x09U, 2U, 0U, 5U, 5U, 'A', 'p'})});
   C_OscProtocolDriverOsy::C_FlashBlockInfo c_Info;
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyReadFlashBlockData(0U, c_Info));
   EXPECT_EQ(0U, c_Info.u8_SignatureValid); //what came before the cut is kept

   //the addresses field needs eight bytes and gets three
   mc_Transport.Reply({mh_Bytes({0x71U, 0x01U, 0x02U, 0x09U, 1U, 0U, 0U, 0U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyReadFlashBlockData(0U, c_Info));
}

/* -- Security access ------------------------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverOsy, SecurityAccessRequestSeed_NonSecure_FourByteSeed)
{
   mc_Transport.Reply({mh_Bytes({0x67U, 0x01U, 0x12U, 0x34U, 0x56U, 0x78U})});

   bool q_Secure = true;
   bool q_Auth = true;
   bool q_Encrypt = true;
   uint64_t u64_Seed = 0U;
   T_Bytes c_Iv;
   EXPECT_EQ(Errc::success, mc_Driver.OsySecurityAccessRequestSeed(1U, q_Secure, u64_Seed, q_Auth, q_Encrypt, c_Iv));
   EXPECT_EQ(mh_Bytes({0x27U, 0x01U}), mc_Transport.LastRequest());
   EXPECT_FALSE(q_Secure);
   EXPECT_FALSE(q_Auth);
   EXPECT_FALSE(q_Encrypt);
   EXPECT_EQ(0x12345678ULL, u64_Seed);
   EXPECT_TRUE(c_Iv.empty());
}

TEST_F(ProtocolDriverOsy, SecurityAccessRequestSeed_SecureWithAuthenticationAndEncryption)
{
   //algorithms 0x10: low nibble 0 = RSA authentication active, high nibble 1 = AES traffic encryption;
   //then a reserved byte, an 8 byte seed and a 16 byte init vector
   T_Bytes c_Reply = mh_Bytes({0x67U, 0x01U, 0x10U, 0x00U, 0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU});
   for (uint8_t u8_Index = 0U; u8_Index < 16U; ++u8_Index)
   {
      c_Reply.push_back(static_cast<uint8_t>(0xA0U + u8_Index));
   }
   mc_Transport.Reply({c_Reply});

   bool q_Secure = false;
   bool q_Auth = false;
   bool q_Encrypt = false;
   uint64_t u64_Seed = 0U;
   T_Bytes c_Iv;
   EXPECT_EQ(Errc::success, mc_Driver.OsySecurityAccessRequestSeed(1U, q_Secure, u64_Seed, q_Auth, q_Encrypt, c_Iv));
   EXPECT_TRUE(q_Secure);
   EXPECT_TRUE(q_Auth);
   EXPECT_TRUE(q_Encrypt);
   EXPECT_EQ(0x0123456789ABCDEFULL, u64_Seed);
   ASSERT_EQ(16U, c_Iv.size());
   EXPECT_EQ(0xA0U, c_Iv[0]);
   EXPECT_EQ(0xAFU, c_Iv[15]);
}

TEST_F(ProtocolDriverOsy, SecurityAccessRequestSeed_SecureWithoutAuthentication_FourByteSeedAfterTheHeader)
{
   //algorithms 0x01: no authentication, no encryption; seed is four bytes at offset 2
   mc_Transport.Reply({mh_Bytes({0x67U, 0x03U, 0x01U, 0x00U, 0xDEU, 0xADU, 0xBEU, 0xEFU})});

   bool q_Secure = false;
   bool q_Auth = true;
   bool q_Encrypt = true;
   uint64_t u64_Seed = 0U;
   T_Bytes c_Iv;
   EXPECT_EQ(Errc::success, mc_Driver.OsySecurityAccessRequestSeed(3U, q_Secure, u64_Seed, q_Auth, q_Encrypt, c_Iv));
   EXPECT_TRUE(q_Secure);
   EXPECT_FALSE(q_Auth);
   EXPECT_FALSE(q_Encrypt);
   EXPECT_EQ(0xDEADBEEFULL, u64_Seed);

   //a size that matches no combination is rd_wr
   mc_Transport.Reply({mh_Bytes({0x67U, 0x03U, 0x01U, 0x00U, 0xDEU, 0xADU, 0xBEU})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsySecurityAccessRequestSeed(3U, q_Secure, u64_Seed, q_Auth, q_Encrypt, c_Iv));
}

TEST_F(ProtocolDriverOsy, SecurityAccessSendKey_UsesTheNextSubFunctionAndBigEndianKey)
{
   mc_Transport.Reply({mh_Bytes({0x67U, 0x02U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsySecurityAccessSendKey(1U, 0xA1B2C3D4U));
   EXPECT_EQ(mh_Bytes({0x27U, 0x02U, 0xA1U, 0xB2U, 0xC3U, 0xD4U}), mc_Transport.LastRequest());

   mc_Transport.Reply({mh_Bytes({0x7FU, 0x27U, 0x35U})});
   uint8_t u8_Nrc = 0U;
   EXPECT_EQ(Errc::warn, mc_Driver.OsySecurityAccessSendKey(1U, 0U, &u8_Nrc));
   EXPECT_EQ(C_OscProtocolDriverOsy::hu8_NR_CODE_INVALID_KEY, u8_Nrc);
}

/* -- Download sequence ---------------------------------------------------------------------------------------------- */

TEST_F(ProtocolDriverOsy, RequestDownload_EncodesAddressAndSize_AndDecodesTheBlockLengthByItsFormat)
{
   //length format nibble 2: two bytes of max block length follow
   mc_Transport.Reply({mh_Bytes({0x74U, 0x20U, 0x04U, 0x00U})});

   uint32_t u32_MaxBlock = 0U;
   EXPECT_EQ(Errc::success, mc_Driver.OsyRequestDownload(0x08000000U, 0x00001234U, u32_MaxBlock));
   EXPECT_EQ(mh_Bytes({0x34U, 0x00U, 0x44U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x12U, 0x34U}),
             mc_Transport.LastRequest());
   EXPECT_EQ(0x0400U, u32_MaxBlock);

   //four byte format
   mc_Transport.Reply({mh_Bytes({0x74U, 0x40U, 0x00U, 0x01U, 0x00U, 0x00U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyRequestDownload(0U, 0U, u32_MaxBlock));
   EXPECT_EQ(0x00010000U, u32_MaxBlock);

   //a format of zero, or one that does not match the length, is rd_wr
   mc_Transport.Reply({mh_Bytes({0x74U, 0x00U, 0x04U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyRequestDownload(0U, 0U, u32_MaxBlock));
   mc_Transport.Reply({mh_Bytes({0x74U, 0x20U, 0x04U, 0x00U, 0x00U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyRequestDownload(0U, 0U, u32_MaxBlock));
}

TEST_F(ProtocolDriverOsy, TransferData_EchoesTheSequenceCounter)
{
   mc_Transport.Reply({mh_Bytes({0x76U, 0x05U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyTransferData(5U, mh_Bytes({0x10U, 0x20U})));
   EXPECT_EQ(mh_Bytes({0x36U, 0x05U, 0x10U, 0x20U}), mc_Transport.LastRequest());

   mc_Transport.Reply({mh_Bytes({0x76U, 0x06U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyTransferData(5U, mh_Bytes({0x10U})));
}

TEST_F(ProtocolDriverOsy, RequestTransferExitAddressBased_WithAndWithoutSignatureAddress)
{
   mc_Transport.Reply({mh_Bytes({0x77U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyRequestTransferExitAddressBased(true, 0x0800FF00U));
   EXPECT_EQ(mh_Bytes({0x37U, 0x08U, 0x00U, 0xFFU, 0x00U}), mc_Transport.LastRequest());

   mc_Transport.Reply({mh_Bytes({0x77U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyRequestTransferExitAddressBased(false, 0U));
   EXPECT_EQ(mh_Bytes({0x37U}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverOsy, ReadMemoryByAddress_ShrinksAddressAndSizeToTheBytesTheyNeed)
{
   //address 0x1000 takes two bytes, size 5 takes one: format byte 0x12
   mc_Transport.Reply({mh_Bytes({0x63U, 1U, 2U, 3U, 4U, 5U})});

   T_Bytes c_Data(5U);
   EXPECT_EQ(Errc::success, mc_Driver.OsyReadMemoryByAddress(0x1000U, c_Data));
   EXPECT_EQ(mh_Bytes({0x23U, 0x12U, 0x10U, 0x00U, 0x05U}), mc_Transport.LastRequest());
   EXPECT_EQ(mh_Bytes({1U, 2U, 3U, 4U, 5U}), c_Data);
}

TEST_F(ProtocolDriverOsy, WriteMemoryByAddress_SplitsAtTheServiceSizeAndExpectsTheHeaderEchoed)
{
   //service size 20 leaves 10 data bytes per request; 25 bytes take three requests at 0x1000, 0x100A, 0x1014
   mc_Driver.SetMaxServiceSize(20U);
   std::vector<uint8_t> c_Data(25U);
   for (uint8_t u8_Index = 0U; u8_Index < 25U; ++u8_Index)
   {
      c_Data[u8_Index] = static_cast<uint8_t>(0x30U + u8_Index);
   }
   mc_Transport.Reply({mh_Bytes({0x7DU, 0x12U, 0x10U, 0x00U, 0x0AU})});
   mc_Transport.Reply({mh_Bytes({0x7DU, 0x12U, 0x10U, 0x0AU, 0x0AU})});
   mc_Transport.Reply({mh_Bytes({0x7DU, 0x12U, 0x10U, 0x14U, 0x05U})});

   EXPECT_EQ(Errc::success, mc_Driver.OsyWriteMemoryByAddress(0x1000U, c_Data));

   ASSERT_EQ(3U, mc_Transport.c_Requests.size());
   EXPECT_EQ(mh_Bytes({0x3DU, 0x12U, 0x10U, 0x00U, 0x0AU, 0x30U, 0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U, 0x38U,
                       0x39U}),
             mc_Transport.c_Requests[0]);
   EXPECT_EQ(0x14U, mc_Transport.c_Requests[2][3]); //third block starts at 0x1014
   EXPECT_EQ(0x05U, mc_Transport.c_Requests[2][4]); //and carries the last five bytes
   EXPECT_EQ(10U, mc_Transport.c_Requests[2].size());

   //an echo of another address is rd_wr
   mc_Transport.Reply({mh_Bytes({0x7DU, 0x12U, 0x10U, 0x01U, 0x0AU})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyWriteMemoryByAddress(0x1000U, std::vector<uint8_t>(10U, 0U)));
}

/* -- Session, reset, keep-alive ------------------------------------------------------------------------------------- */

TEST_F(ProtocolDriverOsy, DiagnosticSessionControl_ExpectsTheSessionEchoed)
{
   mc_Transport.Reply({mh_Bytes({0x50U, 0x03U, 0U, 0U, 0U, 0U})});
   EXPECT_EQ(Errc::success, mc_Driver.OsyDiagnosticSessionControl(0x03U));
   EXPECT_EQ(mh_Bytes({0x10U, 0x03U}), mc_Transport.LastRequest());

   mc_Transport.Reply({mh_Bytes({0x50U, 0x01U, 0U, 0U, 0U, 0U})});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.OsyDiagnosticSessionControl(0x03U));
}

TEST_F(ProtocolDriverOsy, TesterPresent_SuppressedNeedsNoAnswer)
{
   EXPECT_EQ(Errc::success, mc_Driver.OsyTesterPresent(1U));
   EXPECT_EQ(mh_Bytes({0x3EU, 0x80U}), mc_Transport.LastRequest());
   EXPECT_EQ(1U, mc_Transport.u32_Cycles); //one cycle to push it out, no polling
}

TEST_F(ProtocolDriverOsy, EcuReset_IsFireAndForget)
{
   EXPECT_EQ(Errc::success, mc_Driver.OsyEcuReset(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON));
   EXPECT_EQ(mh_Bytes({0x11U, 0x02U}), mc_Transport.LastRequest());
}
