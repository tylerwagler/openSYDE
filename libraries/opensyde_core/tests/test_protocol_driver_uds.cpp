//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The generic UDS client over a scripted transport: every byte on the wire, without a device

   C_OscProtocolDriverUds encodes each ISO 14229-1 request, waits for the matching response within P2 (P2* after
   a ResponsePending), retries on busyRepeatRequest and decodes what comes back. The scripted transport answers
   from a list, immediately or a number of cycles later, so the timing rules are observable as well as the bytes.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscProtocolDriverUds.hpp"
#include "C_OscUdsNrc.hpp"
#include "C_OscUdsSeedKey.hpp"
#include "TglTime.hpp"
#include "osy_scripted_transport.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using osy_scripted_transport::C_ScriptedTransport;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
using T_Bytes = std::vector<uint8_t>;

///key = every seed byte plus one; enough to prove the hook is used
class C_PlusOneSeedKey :
   public C_OscUdsSeedKey
{
public:
   std::error_code CalculateKey(const uint8_t ou8_Level, const std::vector<uint8_t> & orc_Seed,
                                std::vector<uint8_t> & orc_Key) const override
   {
      (void)ou8_Level;
      orc_Key.clear();
      for (const uint8_t u8_Byte : orc_Seed)
      {
         orc_Key.push_back(static_cast<uint8_t>(u8_Byte + 1U));
      }
      return Errc::success;
   }
};

class C_RefusingSeedKey :
   public C_OscUdsSeedKey
{
public:
   std::error_code CalculateKey(const uint8_t, const std::vector<uint8_t> &, std::vector<uint8_t> &) const override
   {
      return Errc::config;
   }
};

class ProtocolDriverUds :
   public ::testing::Test
{
protected:
   void SetUp(void) override
   {
      //silence is 20 ms, a pending answer 200 ms; not the ISO defaults of 50 ms and 5 s
      mc_Driver.SetTimings(20U, 200U);
      ASSERT_FALSE(static_cast<bool>(mc_Driver.SetTransportProtocol(&mc_Transport)));
   }

   C_ScriptedTransport mc_Transport;
   C_OscProtocolDriverUds mc_Driver;
};
}

/* -- Request / response mechanics ---------------------------------------------------------------------------------- */

TEST_F(ProtocolDriverUds, ReadDataByIdentifier_EncodesTheRequestAndStripsTheEcho)
{
   mc_Transport.Reply({{0x62U, 0xF1U, 0x90U, 'W', 'A', 'U'}});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::success, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_EQ((T_Bytes{0x22U, 0xF1U, 0x90U}), mc_Transport.LastRequest());
   EXPECT_EQ((T_Bytes{'W', 'A', 'U'}), c_Data);
}

TEST_F(ProtocolDriverUds, NegativeResponse_IsWarnWithTheCode)
{
   mc_Transport.Reply({{0x7FU, 0x22U, 0x31U}});
   T_Bytes c_Data;
   uint8_t u8_Nrc = 0U;

   EXPECT_EQ(Errc::warn, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data, &u8_Nrc));
   EXPECT_EQ(C_OscUdsNrc::hu8_REQUEST_OUT_OF_RANGE, u8_Nrc);
   EXPECT_TRUE(c_Data.empty());
}

TEST_F(ProtocolDriverUds, NoResponse_TimesOutAfterP2)
{
   T_Bytes c_Data;
   const uint32_t u32_Start = stw::tgl::TglGetTickCount();

   EXPECT_EQ(Errc::timeout, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   const uint32_t u32_Elapsed = stw::tgl::TglGetTickCount() - u32_Start;
   EXPECT_GE(u32_Elapsed, 19U);
   EXPECT_LT(u32_Elapsed, 150U); //P2 is 20 ms; P2* (200 ms) must not have been used
}

TEST_F(ProtocolDriverUds, ResponsePending_ExtendsTheWaitToP2Star)
{
   //the answer comes about 60 cycles (one ms each) after the request: past P2, within P2*
   mc_Transport.Reply({{0x7FU, 0x22U, 0x78U}});
   mc_Transport.InjectAfterCycles(60U, {0x62U, 0xF1U, 0x90U, 0x01U});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::success, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_EQ((T_Bytes{0x01U}), c_Data);
}

TEST_F(ProtocolDriverUds, LateAnswerWithoutResponsePending_IsATimeout)
{
   //the same late answer, but nobody said "pending": P2 rules
   mc_Transport.InjectAfterCycles(60U, {0x62U, 0xF1U, 0x90U, 0x01U});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::timeout, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
}

TEST_F(ProtocolDriverUds, BusyRepeatRequest_SendsTheRequestAgain)
{
   mc_Transport.Reply({{0x7FU, 0x22U, 0x21U}});
   mc_Transport.Reply({{0x62U, 0xF1U, 0x90U, 0x02U}});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::success, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_EQ(2U, mc_Transport.c_Requests.size());
   EXPECT_EQ(mc_Transport.c_Requests[0], mc_Transport.c_Requests[1]);
}

TEST_F(ProtocolDriverUds, BusyRepeatRequest_GivesUpAfterThreeRepeats)
{
   for (uint32_t u32_Index = 0U; u32_Index < 4U; ++u32_Index)
   {
      mc_Transport.Reply({{0x7FU, 0x22U, 0x21U}});
   }
   T_Bytes c_Data;
   uint8_t u8_Nrc = 0U;

   EXPECT_EQ(Errc::warn, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data, &u8_Nrc));
   EXPECT_EQ(C_OscUdsNrc::hu8_BUSY_REPEAT_REQUEST, u8_Nrc);
   EXPECT_EQ(4U, mc_Transport.c_Requests.size()); //the original and three repeats
}

TEST_F(ProtocolDriverUds, WrongIdentifierEcho_IsRejected)
{
   mc_Transport.Reply({{0x62U, 0xF1U, 0x91U, 0x01U}});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::rd_wr, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_TRUE(c_Data.empty());
}

TEST_F(ProtocolDriverUds, PositiveResponseShorterThanTheService_IsRejected)
{
   mc_Transport.Reply({{0x62U, 0xF1U}});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::rd_wr, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
}

TEST_F(ProtocolDriverUds, ResponseToAnotherService_IsSkippedWhileWaiting)
{
   mc_Transport.Reply({{0x50U, 0x03U, 0x00U, 0x32U, 0x01U, 0xF4U}, {0x62U, 0xF1U, 0x90U, 0x03U}});
   T_Bytes c_Data;

   EXPECT_EQ(Errc::success, mc_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_EQ((T_Bytes{0x03U}), c_Data);
}

TEST_F(ProtocolDriverUds, NoTransport_IsConfig)
{
   C_OscProtocolDriverUds c_Driver;
   T_Bytes c_Data;

   EXPECT_EQ(Errc::config, c_Driver.ReadDataByIdentifier(0xF190U, c_Data));
   EXPECT_EQ(Errc::config, c_Driver.Cycle());
}

/* -- Sessions, resets, keep-alive ---------------------------------------------------------------------------------- */

TEST_F(ProtocolDriverUds, DiagnosticSessionControl_AdoptsP2AndP2StarFromTheResponse)
{
   //P2 = 0x0032 ms, P2* = 0x01F4 * 10 ms
   mc_Transport.Reply({{0x50U, 0x03U, 0x00U, 0x32U, 0x01U, 0xF4U}});

   EXPECT_EQ(Errc::success, mc_Driver.DiagnosticSessionControl(C_OscProtocolDriverUds::hu8_SESSION_EXTENDED_DIAGNOSTIC));
   EXPECT_EQ((T_Bytes{0x10U, 0x03U}), mc_Transport.LastRequest());
   EXPECT_EQ(50U, mc_Driver.GetP2Ms());
   EXPECT_EQ(5000U, mc_Driver.GetP2StarMs());
}

TEST_F(ProtocolDriverUds, DiagnosticSessionControl_WrongSessionEchoIsRejected)
{
   mc_Transport.Reply({{0x50U, 0x01U, 0x00U, 0x32U, 0x01U, 0xF4U}});

   EXPECT_EQ(Errc::rd_wr, mc_Driver.DiagnosticSessionControl(C_OscProtocolDriverUds::hu8_SESSION_PROGRAMMING));
   EXPECT_EQ(20U, mc_Driver.GetP2Ms()); //untouched
}

TEST_F(ProtocolDriverUds, TesterPresentSuppressed_SendsAndDoesNotWait)
{
   const uint32_t u32_Start = stw::tgl::TglGetTickCount();

   EXPECT_EQ(Errc::success, mc_Driver.TesterPresent(true));
   EXPECT_EQ((T_Bytes{0x3EU, 0x80U}), mc_Transport.LastRequest());
   EXPECT_LT(stw::tgl::TglGetTickCount() - u32_Start, 15U); //well under P2: nothing was waited for
}

TEST_F(ProtocolDriverUds, TesterPresentAnswered_WaitsForTheResponse)
{
   mc_Transport.Reply({{0x7EU, 0x00U}});

   EXPECT_EQ(Errc::success, mc_Driver.TesterPresent(false));
   EXPECT_EQ((T_Bytes{0x3EU, 0x00U}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverUds, EcuReset_EncodesTheTypeAndTheSuppressBit)
{
   mc_Transport.Reply({{0x51U, 0x01U}});
   EXPECT_EQ(Errc::success, mc_Driver.EcuReset(C_OscProtocolDriverUds::hu8_RESET_HARD));
   EXPECT_EQ((T_Bytes{0x11U, 0x01U}), mc_Transport.LastRequest());

   EXPECT_EQ(Errc::success, mc_Driver.EcuReset(C_OscProtocolDriverUds::hu8_RESET_SOFT, true));
   EXPECT_EQ((T_Bytes{0x11U, 0x83U}), mc_Transport.LastRequest());
}

/* -- SecurityAccess ------------------------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverUds, SecurityAccess_UsesTheInstalledSeedKey)
{
   const C_PlusOneSeedKey c_SeedKey;
   mc_Driver.SetSeedKey(&c_SeedKey);
   mc_Transport.Reply({{0x67U, 0x01U, 0x0AU, 0x0BU}});
   mc_Transport.Reply({{0x67U, 0x02U}});

   EXPECT_EQ(Errc::success, mc_Driver.SecurityAccess(0x01U));
   ASSERT_EQ(2U, mc_Transport.c_Requests.size());
   EXPECT_EQ((T_Bytes{0x27U, 0x01U}), mc_Transport.c_Requests[0]);
   EXPECT_EQ((T_Bytes{0x27U, 0x02U, 0x0BU, 0x0CU}), mc_Transport.c_Requests[1]);
}

TEST_F(ProtocolDriverUds, SecurityAccess_DefaultKeyIsOpenSydesConstant)
{
   mc_Transport.Reply({{0x67U, 0x03U, 0x00U, 0x00U, 0x00U, 0x2AU}});
   mc_Transport.Reply({{0x67U, 0x04U}});

   EXPECT_EQ(Errc::success, mc_Driver.SecurityAccess(0x03U));
   EXPECT_EQ((T_Bytes{0x27U, 0x04U, 0x00U, 0x00U, 0x00U, 0x17U}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverUds, SecurityAccess_AllZeroSeedMeansAlreadyUnlocked)
{
   mc_Transport.Reply({{0x67U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U}});

   EXPECT_EQ(Errc::success, mc_Driver.SecurityAccess(0x01U));
   EXPECT_EQ(1U, mc_Transport.c_Requests.size()); //no key sent
}

TEST_F(ProtocolDriverUds, SecurityAccess_InvalidKeyIsReported)
{
   mc_Transport.Reply({{0x67U, 0x01U, 0x0AU, 0x0BU}});
   mc_Transport.Reply({{0x7FU, 0x27U, 0x35U}});
   uint8_t u8_Nrc = 0U;

   EXPECT_EQ(Errc::warn, mc_Driver.SecurityAccess(0x01U, &u8_Nrc));
   EXPECT_EQ(C_OscUdsNrc::hu8_INVALID_KEY, u8_Nrc);
}

TEST_F(ProtocolDriverUds, SecurityAccess_SeedKeyWithoutAnAnswerIsChecksum)
{
   const C_RefusingSeedKey c_SeedKey;
   mc_Driver.SetSeedKey(&c_SeedKey);
   mc_Transport.Reply({{0x67U, 0x01U, 0x0AU, 0x0BU}});

   EXPECT_EQ(Errc::checksum, mc_Driver.SecurityAccess(0x01U));
   EXPECT_EQ(1U, mc_Transport.c_Requests.size());
}

TEST_F(ProtocolDriverUds, SecurityAccess_EvenLevelIsRange)
{
   T_Bytes c_Seed;

   EXPECT_EQ(Errc::range, mc_Driver.SecurityAccessRequestSeed(0x02U, c_Seed));
   EXPECT_EQ(Errc::range, mc_Driver.SecurityAccessSendKey(0x02U, {0x01U}));
   EXPECT_TRUE(mc_Transport.c_Requests.empty());
}

/* -- Routines, control services, DTCs ------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverUds, RoutineControl_EncodesAndReturnsTheStatusRecord)
{
   mc_Transport.Reply({{0x71U, 0x01U, 0x02U, 0x08U, 0x00U, 0x11U}});
   T_Bytes c_Record;

   EXPECT_EQ(Errc::success, mc_Driver.RoutineControl(C_OscProtocolDriverUds::hu8_ROUTINE_START, 0x0208U, {0xAAU},
                                                     c_Record));
   EXPECT_EQ((T_Bytes{0x31U, 0x01U, 0x02U, 0x08U, 0xAAU}), mc_Transport.LastRequest());
   EXPECT_EQ((T_Bytes{0x00U, 0x11U}), c_Record);
}

TEST_F(ProtocolDriverUds, RoutineControl_WrongRoutineEchoIsRejected)
{
   mc_Transport.Reply({{0x71U, 0x01U, 0x02U, 0x09U}});
   T_Bytes c_Record;

   EXPECT_EQ(Errc::rd_wr, mc_Driver.RoutineControl(C_OscProtocolDriverUds::hu8_ROUTINE_START, 0x0208U, {}, c_Record));
}

TEST_F(ProtocolDriverUds, CommunicationControlAndControlDtcSetting_EncodeTheirSubFunctions)
{
   mc_Transport.Reply({{0x68U, 0x03U}});
   EXPECT_EQ(Errc::success, mc_Driver.CommunicationControl(C_OscProtocolDriverUds::hu8_COMM_DISABLE_RX_AND_TX,
                                                           C_OscProtocolDriverUds::hu8_COMM_TYPE_NORMAL_AND_NETWORK_MANAGEMENT));
   EXPECT_EQ((T_Bytes{0x28U, 0x03U, 0x03U}), mc_Transport.LastRequest());

   mc_Transport.Reply({{0xC5U, 0x02U}});
   EXPECT_EQ(Errc::success, mc_Driver.ControlDtcSetting(C_OscProtocolDriverUds::hu8_DTC_SETTING_OFF));
   EXPECT_EQ((T_Bytes{0x85U, 0x02U}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverUds, ClearDiagnosticInformation_EncodesTheGroup)
{
   mc_Transport.Reply({{0x54U}});

   EXPECT_EQ(Errc::success, mc_Driver.ClearDiagnosticInformation(C_OscProtocolDriverUds::hu32_DTC_GROUP_ALL));
   EXPECT_EQ((T_Bytes{0x14U, 0xFFU, 0xFFU, 0xFFU}), mc_Transport.LastRequest());
}

TEST_F(ProtocolDriverUds, ReadDtcByStatusMask_DecodesTheRecords)
{
   mc_Transport.Reply({{0x59U, 0x02U, 0xFFU, 0x12U, 0x34U, 0x56U, 0x09U, 0xABU, 0xCDU, 0xEFU, 0x2FU}});
   uint8_t u8_Availability = 0U;
   std::vector<C_OscProtocolDriverUds::C_DtcRecord> c_Dtcs;

   EXPECT_EQ(Errc::success, mc_Driver.ReadDtcByStatusMask(0x08U, u8_Availability, c_Dtcs));
   EXPECT_EQ((T_Bytes{0x19U, 0x02U, 0x08U}), mc_Transport.LastRequest());
   EXPECT_EQ(0xFFU, u8_Availability);
   ASSERT_EQ(2U, c_Dtcs.size());
   EXPECT_EQ(0x123456U, c_Dtcs[0].u32_Dtc);
   EXPECT_EQ(0x09U, c_Dtcs[0].u8_Status);
   EXPECT_EQ(0xABCDEFU, c_Dtcs[1].u32_Dtc);
   EXPECT_EQ(0x2FU, c_Dtcs[1].u8_Status);
}

TEST_F(ProtocolDriverUds, ReadDtc_RecordListNotAMultipleOfFour_IsRejected)
{
   mc_Transport.Reply({{0x59U, 0x0AU, 0xFFU, 0x12U, 0x34U, 0x56U}});
   uint8_t u8_Availability = 0U;
   std::vector<C_OscProtocolDriverUds::C_DtcRecord> c_Dtcs;

   EXPECT_EQ(Errc::rd_wr, mc_Driver.ReadSupportedDtc(u8_Availability, c_Dtcs));
   EXPECT_EQ((T_Bytes{0x19U, 0x0AU}), mc_Transport.LastRequest());
   EXPECT_TRUE(c_Dtcs.empty());
}

TEST_F(ProtocolDriverUds, ReadNumberOfDtcByStatusMask_DecodesTheCount)
{
   mc_Transport.Reply({{0x59U, 0x01U, 0x7FU, 0x01U, 0x00U, 0x02U}});
   uint8_t u8_Availability = 0U;
   uint16_t u16_Count = 0U;

   EXPECT_EQ(Errc::success, mc_Driver.ReadNumberOfDtcByStatusMask(0xFFU, u8_Availability, u16_Count));
   EXPECT_EQ((T_Bytes{0x19U, 0x01U, 0xFFU}), mc_Transport.LastRequest());
   EXPECT_EQ(0x7FU, u8_Availability);
   EXPECT_EQ(2U, u16_Count);
}

/* -- Download ------------------------------------------------------------------------------------------------------ */

TEST_F(ProtocolDriverUds, RequestDownload_EncodesFourByteAddressAndSizeAndDecodesTheBlockLength)
{
   mc_Transport.Reply({{0x74U, 0x20U, 0x0FU, 0xFFU}});
   uint32_t u32_MaxBlock = 0U;

   EXPECT_EQ(Errc::success, mc_Driver.RequestDownload(0x00U, 0x00010000U, 0x1000U, u32_MaxBlock));
   EXPECT_EQ((T_Bytes{0x34U, 0x00U, 0x44U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x10U, 0x00U}),
             mc_Transport.LastRequest());
   EXPECT_EQ(0x0FFFU, u32_MaxBlock);
}

TEST_F(ProtocolDriverUds, RequestDownload_UnusableLengthFormatIsRejected)
{
   mc_Transport.Reply({{0x74U, 0x00U}});
   uint32_t u32_MaxBlock = 0U;

   EXPECT_EQ(Errc::rd_wr, mc_Driver.RequestDownload(0x00U, 0U, 16U, u32_MaxBlock));
   EXPECT_EQ(0U, u32_MaxBlock);
}

TEST_F(ProtocolDriverUds, TransferDataAndExit_EncodeAndCheckTheCounter)
{
   mc_Transport.Reply({{0x76U, 0x01U}});
   EXPECT_EQ(Errc::success, mc_Driver.TransferData(0x01U, {0xDEU, 0xADU}));
   EXPECT_EQ((T_Bytes{0x36U, 0x01U, 0xDEU, 0xADU}), mc_Transport.LastRequest());

   mc_Transport.Reply({{0x76U, 0x03U}});
   EXPECT_EQ(Errc::rd_wr, mc_Driver.TransferData(0x02U, {0xBEU}));

   mc_Transport.Reply({{0x77U}});
   EXPECT_EQ(Errc::success, mc_Driver.RequestTransferExit());
   EXPECT_EQ((T_Bytes{0x37U}), mc_Transport.LastRequest());
}

/* -- Escape hatch and texts ---------------------------------------------------------------------------------------- */

TEST_F(ProtocolDriverUds, SendRequest_PassesAnyServiceThrough)
{
   mc_Transport.Reply({{0xE5U, 0x01U, 0x02U}});
   T_Bytes c_Response;

   EXPECT_EQ(Errc::success, mc_Driver.SendRequest({0xA5U, 0x01U}, c_Response));
   EXPECT_EQ((T_Bytes{0xE5U, 0x01U, 0x02U}), c_Response);
   EXPECT_EQ(Errc::range, mc_Driver.SendRequest({}, c_Response));
}

TEST(ProtocolDriverUdsText, ErrorDetailsNameTheNegativeResponse)
{
   EXPECT_EQ("Negative response received (securityAccessDenied)",
             C_OscProtocolDriverUds::h_GetServiceErrorDetails(Errc::warn, 0x33U));
   EXPECT_EQ("No response received within timeout", C_OscProtocolDriverUds::h_GetServiceErrorDetails(Errc::timeout, 0U));
   EXPECT_EQ("No problem", C_OscProtocolDriverUds::h_GetServiceErrorDetails(Errc::success, 0U));
}
