//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generic UDS (ISO 14229-1) client

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdint>
#include <format>
#include <string>
#include <system_error>
#include <vector>

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscEndian.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscProtocolDriverUds.hpp"
#include "C_OscUdsNrc.hpp"
#include "TglTime.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
namespace
{
constexpr char hacn_LOG_ACTIVITY[] = "UDS client";
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverUds::C_OscProtocolDriverUds(void) :
   mpc_TransportProtocol(nullptr),
   mpc_SeedKey(&mc_DefaultSeedKey),
   mu32_P2Ms(hu32_DEFAULT_P2_MS),
   mu32_P2StarMs(hu32_DEFAULT_P2_STAR_MS)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverUds::~C_OscProtocolDriverUds(void)
{
   mpc_TransportProtocol = nullptr;
   mpc_SeedKey = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Install the transport protocol to talk through

   The caller keeps ownership and has already addressed it (SetNodeIdentifiers for openSYDE's scheme, or
   SetExplicitIdentifiers on the CAN transport for a fixed identifier pair).

   \param[in]  opc_TransportProtocol   transport protocol; nullptr to detach

   \return
   Errc::success   no problems
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::SetTransportProtocol(C_OscProtocolDriverOsyTpBase * const opc_TransportProtocol)
{
   mpc_TransportProtocol = opc_TransportProtocol;
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpBase * C_OscProtocolDriverUds::GetTransportProtocol(void) const
{
   return mpc_TransportProtocol;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Install the seed-to-key algorithm SecurityAccess() uses

   \param[in]  opc_SeedKey   algorithm, not owned; nullptr goes back to the built-in constant key
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverUds::SetSeedKey(const C_OscUdsSeedKey * const opc_SeedKey)
{
   mpc_SeedKey = (opc_SeedKey != nullptr) ? opc_SeedKey : &mc_DefaultSeedKey;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set the response timing

   Normally taken over from the DiagnosticSessionControl response; set here for a server that does not report it,
   or to shorten the wait in tests.

   \param[in]  ou32_P2Ms       time to wait for a response
   \param[in]  ou32_P2StarMs   time to wait after a ResponsePending
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverUds::SetTimings(const uint32_t ou32_P2Ms, const uint32_t ou32_P2StarMs)
{
   mu32_P2Ms = ou32_P2Ms;
   mu32_P2StarMs = ou32_P2StarMs;
}

//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscProtocolDriverUds::GetP2Ms(void) const
{
   return mu32_P2Ms;
}

//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscProtocolDriverUds::GetP2StarMs(void) const
{
   return mu32_P2StarMs;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Drive the transport and drop whatever arrived unasked

   Plain UDS has no event-driven services, so anything in the Rx queue outside a transaction is a late or
   unsolicited response; it is logged and discarded.

   \return
   Errc::success   cycle finished
   Errc::config    no transport protocol installed
   Errc::com       transport reported an error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::Cycle(void)
{
   std::error_code c_Return = Errc::success;

   if (mpc_TransportProtocol == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      const std::lock_guard<std::mutex> c_Lock(mc_LockReception);
      c_Return = mpc_TransportProtocol->Cycle();
      if (c_Return == Errc::success)
      {
         C_OscProtocolDriverOsyService c_Service;
         while (mpc_TransportProtocol->ReadResponse(c_Service) == Errc::success)
         {
            osc_write_log_warning(hacn_LOG_ACTIVITY, "Unsolicited service received outside a request. Ignoring.");
         }
      }
      else
      {
         c_Return = Errc::com;
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   DiagnosticSessionControl (0x10)

   Switches the session and takes over the P2 and P2* the server reports in its response.

   \param[in]   ou8_Session    session to switch to (hu8_SESSION_*)
   \param[out]  opu8_NrCode    if != nullptr: negative response code in case of an error response

   \return
   Errc::success   positive response received; timings updated
   Errc::warn      negative response (code in *opu8_NrCode)
   Errc::timeout   no response within P2 (or P2* after a ResponsePending)
   Errc::rd_wr     positive response with an unexpected content
   Errc::noact     could not send the request
   Errc::config    no transport protocol installed
   Errc::com       transport reported an error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::DiagnosticSessionControl(const uint8_t ou8_Session,
                                                                 uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return = m_Transact({hu8_SI_DIAGNOSTIC_SESSION_CONTROL, ou8_Session}, 2U, c_Response, u8_NrCode);

   if (c_Return == Errc::success)
   {
      if (c_Response[1] != ou8_Session)
      {
         c_Return = Errc::rd_wr;
      }
      else if (c_Response.size() >= 6U)
      {
         //sessionParameterRecord: P2 in ms, P2* in units of 10 ms
         mu32_P2Ms = C_OscEndian::h_GetU16Big(&c_Response[2]);
         mu32_P2StarMs = static_cast<uint32_t>(C_OscEndian::h_GetU16Big(&c_Response[4])) * 10U;
      }
      else
      {
         //a server may leave the record out; keep what we have
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("DiagnosticSessionControl", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ECUReset (0x11)

   \param[in]   ou8_ResetType         hu8_RESET_*
   \param[in]   oq_SuppressResponse   true: set the suppressPosRspMsgIndicationBit and do not wait for an answer
   \param[out]  opu8_NrCode           if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::EcuReset(const uint8_t ou8_ResetType, const bool oq_SuppressResponse,
                                                 uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   const uint8_t u8_SubFunction =
      static_cast<uint8_t>(ou8_ResetType | (oq_SuppressResponse ? mhu8_SUPPRESS_POSITIVE_RESPONSE : 0U));
   std::error_code c_Return = m_Transact({hu8_SI_ECU_RESET, u8_SubFunction}, 2U, c_Response, u8_NrCode,
                                         oq_SuppressResponse);

   if ((c_Return == Errc::success) && (oq_SuppressResponse == false) && (c_Response[1] != ou8_ResetType))
   {
      c_Return = Errc::rd_wr;
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ECUReset", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   TesterPresent (0x3E)

   \param[in]   oq_SuppressResponse   true (the usual keep-alive form): no answer is expected
   \param[out]  opu8_NrCode           if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::TesterPresent(const bool oq_SuppressResponse, uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   const uint8_t u8_SubFunction = oq_SuppressResponse ? mhu8_SUPPRESS_POSITIVE_RESPONSE : 0x00U;
   const std::error_code c_Return = m_Transact({hu8_SI_TESTER_PRESENT, u8_SubFunction}, 2U, c_Response, u8_NrCode,
                                               oq_SuppressResponse);

   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("TesterPresent", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ReadDataByIdentifier (0x22), one identifier

   \param[in]   ou16_Identifier   DID
   \param[out]  orc_Data          data record (without the echoed DID)
   \param[out]  opu8_NrCode       if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the response echoes another DID
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ReadDataByIdentifier(const uint16_t ou16_Identifier,
                                                             std::vector<uint8_t> & orc_Data,
                                                             uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return = m_Transact({hu8_SI_READ_DATA_BY_IDENTIFIER, static_cast<uint8_t>(ou16_Identifier >> 8U),
                                          static_cast<uint8_t>(ou16_Identifier)}, 3U, c_Response, u8_NrCode);

   orc_Data.clear();
   if (c_Return == Errc::success)
   {
      if (C_OscEndian::h_GetU16Big(&c_Response[1]) != ou16_Identifier)
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         orc_Data.assign(c_Response.begin() + 3, c_Response.end());
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ReadDataByIdentifier", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   WriteDataByIdentifier (0x2E)

   \param[in]   ou16_Identifier   DID
   \param[in]   orc_Data          data record to write
   \param[out]  opu8_NrCode       if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the response echoes another DID
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::WriteDataByIdentifier(const uint16_t ou16_Identifier,
                                                              const std::vector<uint8_t> & orc_Data,
                                                              uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Request{hu8_SI_WRITE_DATA_BY_IDENTIFIER, static_cast<uint8_t>(ou16_Identifier >> 8U),
                                  static_cast<uint8_t>(ou16_Identifier)};
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   c_Request.insert(c_Request.end(), orc_Data.begin(), orc_Data.end());
   c_Return = m_Transact(c_Request, 3U, c_Response, u8_NrCode);
   if ((c_Return == Errc::success) && (C_OscEndian::h_GetU16Big(&c_Response[1]) != ou16_Identifier))
   {
      c_Return = Errc::rd_wr;
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("WriteDataByIdentifier", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   SecurityAccess (0x27) requestSeed

   \param[in]   ou8_Level     security level: the odd sub-function (0x01, 0x03, ...)
   \param[out]  orc_Seed      seed from the server; all zeros means the level is already unlocked
   \param[out]  opu8_NrCode   if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::range for an even level
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::SecurityAccessRequestSeed(const uint8_t ou8_Level,
                                                                  std::vector<uint8_t> & orc_Seed,
                                                                  uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   orc_Seed.clear();
   if ((ou8_Level % 2U) == 0U)
   {
      c_Return = Errc::range;
   }
   else
   {
      c_Return = m_Transact({hu8_SI_SECURITY_ACCESS, ou8_Level}, 2U, c_Response, u8_NrCode);
      if (c_Return == Errc::success)
      {
         if (c_Response[1] != ou8_Level)
         {
            c_Return = Errc::rd_wr;
         }
         else
         {
            orc_Seed.assign(c_Response.begin() + 2, c_Response.end());
         }
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("SecurityAccess requestSeed", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   SecurityAccess (0x27) sendKey

   \param[in]   ou8_Level     security level the seed was requested for (the odd sub-function)
   \param[in]   orc_Key       key
   \param[out]  opu8_NrCode   if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::range for an even level
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::SecurityAccessSendKey(const uint8_t ou8_Level,
                                                              const std::vector<uint8_t> & orc_Key,
                                                              uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   if ((ou8_Level % 2U) == 0U)
   {
      c_Return = Errc::range;
   }
   else
   {
      const uint8_t u8_SubFunction = static_cast<uint8_t>(ou8_Level + 1U);
      std::vector<uint8_t> c_Request{hu8_SI_SECURITY_ACCESS, u8_SubFunction};
      c_Request.insert(c_Request.end(), orc_Key.begin(), orc_Key.end());
      c_Return = m_Transact(c_Request, 2U, c_Response, u8_NrCode);
      if ((c_Return == Errc::success) && (c_Response[1] != u8_SubFunction))
      {
         c_Return = Errc::rd_wr;
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("SecurityAccess sendKey", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   SecurityAccess (0x27): unlock a level with the installed seed-to-key algorithm

   Requests the seed; if it is all zeros the level is already unlocked and nothing more is sent. Otherwise the key
   comes from the installed C_OscUdsSeedKey and goes back.

   \param[in]   ou8_Level     security level: the odd sub-function (0x01, 0x03, ...)
   \param[out]  opu8_NrCode   if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::checksum if the seed-to-key algorithm has no key for this seed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::SecurityAccess(const uint8_t ou8_Level, uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Seed;
   std::error_code c_Return = SecurityAccessRequestSeed(ou8_Level, c_Seed, opu8_NrCode);

   if (c_Return == Errc::success)
   {
      bool q_AlreadyUnlocked = true;
      for (const uint8_t u8_Byte : c_Seed)
      {
         if (u8_Byte != 0U)
         {
            q_AlreadyUnlocked = false;
            break;
         }
      }
      if (q_AlreadyUnlocked == false)
      {
         std::vector<uint8_t> c_Key;
         if (mpc_SeedKey->CalculateKey(ou8_Level, c_Seed, c_Key))
         {
            osc_write_log_error(hacn_LOG_ACTIVITY, "SecurityAccess: the seed-to-key algorithm has no key for level " +
                                std::to_string(ou8_Level) + ".");
            c_Return = Errc::checksum;
         }
         else
         {
            c_Return = SecurityAccessSendKey(ou8_Level, c_Key, opu8_NrCode);
         }
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   RoutineControl (0x31)

   \param[in]   ou8_SubFunction          hu8_ROUTINE_*
   \param[in]   ou16_RoutineIdentifier   RID
   \param[in]   orc_RequestRecord        routineControlOptionRecord
   \param[out]  orc_ResponseRecord       routineStatusRecord (without the echoed sub-function and RID)
   \param[out]  opu8_NrCode              if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the response echoes another sub-function or RID
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::RoutineControl(const uint8_t ou8_SubFunction,
                                                       const uint16_t ou16_RoutineIdentifier,
                                                       const std::vector<uint8_t> & orc_RequestRecord,
                                                       std::vector<uint8_t> & orc_ResponseRecord,
                                                       uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Request{hu8_SI_ROUTINE_CONTROL, ou8_SubFunction,
                                  static_cast<uint8_t>(ou16_RoutineIdentifier >> 8U),
                                  static_cast<uint8_t>(ou16_RoutineIdentifier)};
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   c_Request.insert(c_Request.end(), orc_RequestRecord.begin(), orc_RequestRecord.end());
   orc_ResponseRecord.clear();
   c_Return = m_Transact(c_Request, 4U, c_Response, u8_NrCode);
   if (c_Return == Errc::success)
   {
      if ((c_Response[1] != ou8_SubFunction) || (C_OscEndian::h_GetU16Big(&c_Response[2]) != ou16_RoutineIdentifier))
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         orc_ResponseRecord.assign(c_Response.begin() + 4, c_Response.end());
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("RoutineControl", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   CommunicationControl (0x28)

   \param[in]   ou8_ControlType         hu8_COMM_*
   \param[in]   ou8_CommunicationType   hu8_COMM_TYPE_*
   \param[out]  opu8_NrCode             if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::CommunicationControl(const uint8_t ou8_ControlType,
                                                             const uint8_t ou8_CommunicationType,
                                                             uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return = m_Transact({hu8_SI_COMMUNICATION_CONTROL, ou8_ControlType, ou8_CommunicationType}, 2U,
                                         c_Response, u8_NrCode);

   if ((c_Return == Errc::success) && (c_Response[1] != ou8_ControlType))
   {
      c_Return = Errc::rd_wr;
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("CommunicationControl", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ControlDTCSetting (0x85)

   \param[in]   ou8_SettingType   hu8_DTC_SETTING_ON / OFF
   \param[out]  opu8_NrCode       if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ControlDtcSetting(const uint8_t ou8_SettingType, uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return = m_Transact({hu8_SI_CONTROL_DTC_SETTING, ou8_SettingType}, 2U, c_Response, u8_NrCode);

   if ((c_Return == Errc::success) && (c_Response[1] != ou8_SettingType))
   {
      c_Return = Errc::rd_wr;
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ControlDTCSetting", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ClearDiagnosticInformation (0x14)

   \param[in]   ou32_GroupOfDtc   24 bit DTC group; hu32_DTC_GROUP_ALL for everything
   \param[out]  opu8_NrCode       if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ClearDiagnosticInformation(const uint32_t ou32_GroupOfDtc,
                                                                   uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   const std::error_code c_Return = m_Transact({hu8_SI_CLEAR_DIAGNOSTIC_INFORMATION,
                                                static_cast<uint8_t>(ou32_GroupOfDtc >> 16U),
                                                static_cast<uint8_t>(ou32_GroupOfDtc >> 8U),
                                                static_cast<uint8_t>(ou32_GroupOfDtc)}, 1U, c_Response, u8_NrCode);

   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ClearDiagnosticInformation", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ReadDTCInformation (0x19) reportNumberOfDTCByStatusMask

   \param[in]   ou8_StatusMask           status bits a DTC has to match
   \param[out]  oru8_AvailabilityMask    status bits the server supports
   \param[out]  oru16_Count              number of matching DTCs
   \param[out]  opu8_NrCode              if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ReadNumberOfDtcByStatusMask(const uint8_t ou8_StatusMask,
                                                                    uint8_t & oru8_AvailabilityMask,
                                                                    uint16_t & oru16_Count,
                                                                    uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   //response: 59 01 availabilityMask formatIdentifier count(2)
   std::error_code c_Return = m_Transact({hu8_SI_READ_DTC_INFORMATION, hu8_DTC_REPORT_NUMBER_BY_STATUS_MASK,
                                          ou8_StatusMask}, 6U, c_Response, u8_NrCode);

   oru8_AvailabilityMask = 0U;
   oru16_Count = 0U;
   if (c_Return == Errc::success)
   {
      if (c_Response[1] != hu8_DTC_REPORT_NUMBER_BY_STATUS_MASK)
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         oru8_AvailabilityMask = c_Response[2];
         oru16_Count = C_OscEndian::h_GetU16Big(&c_Response[4]);
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ReadDTCInformation reportNumberOfDTCByStatusMask", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ReadDTCInformation (0x19) reportDTCByStatusMask

   \param[in]   ou8_StatusMask           status bits a DTC has to match
   \param[out]  oru8_AvailabilityMask    status bits the server supports
   \param[out]  orc_Dtcs                 matching DTCs with their status
   \param[out]  opu8_NrCode              if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ReadDtcByStatusMask(const uint8_t ou8_StatusMask,
                                                            uint8_t & oru8_AvailabilityMask,
                                                            std::vector<C_DtcRecord> & orc_Dtcs,
                                                            uint8_t * const opu8_NrCode)
{
   return m_ReadDtcRecords(hu8_DTC_REPORT_BY_STATUS_MASK, ou8_StatusMask, oru8_AvailabilityMask, orc_Dtcs,
                           opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ReadDTCInformation (0x19) reportSupportedDTC

   \param[out]  oru8_AvailabilityMask    status bits the server supports
   \param[out]  orc_Dtcs                 every DTC the server knows, with its status
   \param[out]  opu8_NrCode              if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::ReadSupportedDtc(uint8_t & oru8_AvailabilityMask,
                                                         std::vector<C_DtcRecord> & orc_Dtcs,
                                                         uint8_t * const opu8_NrCode)
{
   return m_ReadDtcRecords(hu8_DTC_REPORT_SUPPORTED, 0U, oru8_AvailabilityMask, orc_Dtcs, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   RequestDownload (0x34), four byte address and size

   \param[in]   ou8_DataFormatIdentifier   compression (high nibble) and encryption (low nibble); 0x00 for none
   \param[in]   ou32_Address               start address
   \param[in]   ou32_Size                  number of bytes
   \param[out]  oru32_MaxBlockLength       largest TransferData request the server takes, including its two header
                                           bytes
   \param[out]  opu8_NrCode                if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the response's length format is unusable
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::RequestDownload(const uint8_t ou8_DataFormatIdentifier,
                                                        const uint32_t ou32_Address, const uint32_t ou32_Size,
                                                        uint32_t & oru32_MaxBlockLength, uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Request(11U);
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   c_Request[0] = hu8_SI_REQUEST_DOWNLOAD;
   c_Request[1] = ou8_DataFormatIdentifier;
   c_Request[2] = mhu8_ADDRESS_AND_LENGTH_FORMAT_4_4;
   C_OscEndian::h_SetU32Big(ou32_Address, &c_Request[3]);
   C_OscEndian::h_SetU32Big(ou32_Size, &c_Request[7]);

   oru32_MaxBlockLength = 0U;
   //response: 74 lengthFormatIdentifier maxNumberOfBlockLength(n bytes, n = high nibble)
   c_Return = m_Transact(c_Request, 2U, c_Response, u8_NrCode);
   if (c_Return == Errc::success)
   {
      const uint8_t u8_NumLengthBytes = static_cast<uint8_t>(c_Response[1] >> 4U);
      if ((u8_NumLengthBytes == 0U) || (u8_NumLengthBytes > 4U) || (c_Response.size() < (2U + u8_NumLengthBytes)))
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         for (uint8_t u8_Index = 0U; u8_Index < u8_NumLengthBytes; ++u8_Index)
         {
            oru32_MaxBlockLength = (oru32_MaxBlockLength << 8U) | c_Response[2U + u8_Index];
         }
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("RequestDownload", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   TransferData (0x36)

   \param[in]   ou8_BlockSequenceCounter   1 for the first block after RequestDownload, then counting up and wrapping
   \param[in]   orc_Data                   block
   \param[out]  opu8_NrCode                if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the response echoes another counter
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::TransferData(const uint8_t ou8_BlockSequenceCounter,
                                                     const std::vector<uint8_t> & orc_Data,
                                                     uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Request{hu8_SI_TRANSFER_DATA, ou8_BlockSequenceCounter};
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   c_Request.insert(c_Request.end(), orc_Data.begin(), orc_Data.end());
   c_Return = m_Transact(c_Request, 2U, c_Response, u8_NrCode);
   if ((c_Return == Errc::success) && (c_Response[1] != ou8_BlockSequenceCounter))
   {
      c_Return = Errc::rd_wr;
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("TransferData", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   RequestTransferExit (0x37)

   \param[out]  opu8_NrCode   if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::RequestTransferExit(uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   const std::error_code c_Return = m_Transact({hu8_SI_REQUEST_TRANSFER_EXIT}, 1U, c_Response, u8_NrCode);

   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("RequestTransferExit", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Any request, with the same response handling as the typed services

   \param[in]   orc_Request           request, service identifier first
   \param[out]  orc_Response          positive response, service identifier first
   \param[out]  opu8_NrCode           if != nullptr: negative response code in case of an error response
   \param[in]   oq_SuppressResponse   true: the request carries the suppress bit; return after sending

   \return
   see DiagnosticSessionControl; Errc::range for an empty request
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::SendRequest(const std::vector<uint8_t> & orc_Request,
                                                    std::vector<uint8_t> & orc_Response, uint8_t * const opu8_NrCode,
                                                    const bool oq_SuppressResponse)
{
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   if (orc_Request.empty())
   {
      orc_Response.clear();
      c_Return = Errc::range;
   }
   else
   {
      c_Return = m_Transact(orc_Request, 1U, orc_Response, u8_NrCode, oq_SuppressResponse);
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("Request", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: textual representation of a service result

   \param[in]  orc_FunctionResult    result of a service function
   \param[in]  ou8_NrCode            negative response code received (used for Errc::warn)

   \return
   text
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscProtocolDriverUds::h_GetServiceErrorDetails(const std::error_code & orc_FunctionResult,
                                                             const uint8_t ou8_NrCode)
{
   std::string c_Text;

   if (orc_FunctionResult == Errc::success)
   {
      c_Text = "No problem";
   }
   else if (orc_FunctionResult == Errc::warn)
   {
      c_Text = "Negative response received (" + C_OscUdsNrc::h_ToText(ou8_NrCode) + ")";
   }
   else if (orc_FunctionResult == Errc::timeout)
   {
      c_Text = "No response received within timeout";
   }
   else if (orc_FunctionResult == Errc::rd_wr)
   {
      c_Text = "Unexpected content in positive response";
   }
   else if (orc_FunctionResult == Errc::noact)
   {
      c_Text = "Could not send request";
   }
   else if (orc_FunctionResult == Errc::config)
   {
      c_Text = "No transport protocol installed";
   }
   else if (orc_FunctionResult == Errc::com)
   {
      c_Text = "Communication error";
   }
   else if (orc_FunctionResult == Errc::checksum)
   {
      c_Text = "Seed-to-key calculation failed";
   }
   else if (orc_FunctionResult == Errc::range)
   {
      c_Text = "Invalid parameter";
   }
   else
   {
      c_Text = "Undefined error code " + std::to_string(orc_FunctionResult.value());
   }
   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send one request and wait for its response

   A busyRepeatRequest sends the request again, up to mhu8_MAX_BUSY_REPEATS times.

   \param[in]   orc_Request           request, service identifier first
   \param[in]   ox_MinResponseSize    shortest acceptable positive response
   \param[out]  orc_Response          positive response (empty with oq_SuppressResponse)
   \param[out]  oru8_NrCode           negative response code (0 if none)
   \param[in]   oq_SuppressResponse   true: do not wait for a response

   \return
   Errc::success   positive response of acceptable length received, or request sent with oq_SuppressResponse
   Errc::warn      negative response (code in oru8_NrCode)
   Errc::timeout   no response within P2 (or P2* after a ResponsePending)
   Errc::rd_wr     positive response too short
   Errc::noact     could not send the request
   Errc::config    no transport protocol installed
   Errc::com       transport reported an error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::m_Transact(const std::vector<uint8_t> & orc_Request,
                                                   const size_t ox_MinResponseSize, std::vector<uint8_t> & orc_Response,
                                                   uint8_t & oru8_NrCode, const bool oq_SuppressResponse)
{
   std::error_code c_Return = Errc::success;

   orc_Response.clear();
   oru8_NrCode = 0U;
   if (mpc_TransportProtocol == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      const std::lock_guard<std::mutex> c_Lock(mc_LockReception);
      uint8_t u8_Attempt = 0U;
      bool q_Finished = false;

      while (q_Finished == false)
      {
         C_OscProtocolDriverOsyService c_Service;
         c_Service.c_Data = orc_Request;
         ++u8_Attempt;
         if (mpc_TransportProtocol->SendRequest(c_Service) != Errc::success)
         {
            c_Return = Errc::noact;
            q_Finished = true;
         }
         else if (oq_SuppressResponse == true)
         {
            //push it out; the server will not answer
            c_Return = mpc_TransportProtocol->Cycle();
            if (c_Return != Errc::success)
            {
               c_Return = Errc::com;
            }
            q_Finished = true;
         }
         else
         {
            c_Return = m_WaitForResponse(orc_Request[0], orc_Response, oru8_NrCode);
            if ((c_Return == Errc::warn) && (oru8_NrCode == C_OscUdsNrc::hu8_BUSY_REPEAT_REQUEST) &&
                (u8_Attempt <= mhu8_MAX_BUSY_REPEATS))
            {
               osc_write_log_info(hacn_LOG_ACTIVITY, "busyRepeatRequest received; sending the request again.");
               stw::tgl::TglSleepPolling();
            }
            else
            {
               if ((c_Return == Errc::success) && (orc_Response.size() < ox_MinResponseSize))
               {
                  osc_write_log_error(hacn_LOG_ACTIVITY, "Positive response shorter than the service defines.");
                  c_Return = Errc::rd_wr;
               }
               q_Finished = true;
            }
         }
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Wait for the response to one service

   Drives the transport until the positive or negative response to ou8_ServiceId arrives or P2 runs out. P2 is
   counted from the end of the request's transmission: while the transport still has the request on its way out
   (a segmented transfer waiting for flow control, say) the deadline keeps moving. The transport's own N_Bs and
   consecutive-frame timeouts bound that phase. A requestCorrectlyReceived-ResponsePending extends the deadline to
   P2* from its arrival. Services that are neither are late or unsolicited and are dropped.

   \param[in]   ou8_ServiceId   service identifier the request carried
   \param[out]  orc_Response    positive response, service identifier first
   \param[out]  oru8_NrCode     negative response code if Errc::warn

   \return
   Errc::success   positive response received
   Errc::warn      negative response received
   Errc::timeout   nothing matching within the deadline
   Errc::com       transport reported an error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::m_WaitForResponse(const uint8_t ou8_ServiceId,
                                                          std::vector<uint8_t> & orc_Response, uint8_t & oru8_NrCode)
{
   std::error_code c_Return = Errc::timeout;
   const uint8_t u8_PositiveServiceId = static_cast<uint8_t>(ou8_ServiceId + mhu8_POSITIVE_RESPONSE_OFFSET);
   uint32_t u32_Deadline = stw::tgl::TglGetTickCount() + mu32_P2Ms;
   bool q_Finished = false;

   while (q_Finished == false)
   {
      C_OscProtocolDriverOsyService c_Service;
      if (mpc_TransportProtocol->Cycle() != Errc::success)
      {
         c_Return = Errc::com;
         q_Finished = true;
      }
      else
      {
         if (mpc_TransportProtocol->IsTransmissionPending() == true)
         {
            //the request is not out yet: P2 has not started
            const uint32_t u32_FromNow = stw::tgl::TglGetTickCount() + mu32_P2Ms;
            if (static_cast<int32_t>(u32_FromNow - u32_Deadline) > 0)
            {
               u32_Deadline = u32_FromNow;
            }
         }
         while ((q_Finished == false) && (mpc_TransportProtocol->ReadResponse(c_Service) == Errc::success))
         {
            const std::vector<uint8_t> & rc_Data = c_Service.c_Data;
            if ((rc_Data.size() >= 3U) && (rc_Data[0] == hu8_SI_NEGATIVE_RESPONSE) && (rc_Data[1] == ou8_ServiceId))
            {
               if (rc_Data[2] == C_OscUdsNrc::hu8_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING)
               {
                  osc_write_log_info(hacn_LOG_ACTIVITY, "ResponsePending received; waiting P2* for the answer.");
                  u32_Deadline = stw::tgl::TglGetTickCount() + mu32_P2StarMs;
               }
               else
               {
                  oru8_NrCode = rc_Data[2];
                  c_Return = Errc::warn;
                  q_Finished = true;
               }
            }
            else if ((rc_Data.empty() == false) && (rc_Data[0] == u8_PositiveServiceId))
            {
               orc_Response = rc_Data;
               c_Return = Errc::success;
               q_Finished = true;
            }
            else
            {
               osc_write_log_warning(hacn_LOG_ACTIVITY, "Response to another service received while waiting. Ignoring.");
            }
         }
         if (q_Finished == false)
         {
            if (static_cast<int32_t>(stw::tgl::TglGetTickCount() - u32_Deadline) >= 0)
            {
               c_Return = Errc::timeout;
               q_Finished = true;
            }
            else
            {
               stw::tgl::TglSleepPolling();
            }
         }
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   ReadDTCInformation with a DTC record list in the response

   \param[in]   ou8_SubFunction          reportDTCByStatusMask or reportSupportedDTC
   \param[in]   ou8_StatusMask           status mask (only sent for reportDTCByStatusMask)
   \param[out]  oru8_AvailabilityMask    status bits the server supports
   \param[out]  orc_Dtcs                 records
   \param[out]  opu8_NrCode              if != nullptr: negative response code in case of an error response

   \return
   see DiagnosticSessionControl; Errc::rd_wr if the record list is not a whole number of four byte records
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverUds::m_ReadDtcRecords(const uint8_t ou8_SubFunction, const uint8_t ou8_StatusMask,
                                                         uint8_t & oru8_AvailabilityMask,
                                                         std::vector<C_DtcRecord> & orc_Dtcs,
                                                         uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Request{hu8_SI_READ_DTC_INFORMATION, ou8_SubFunction};
   std::vector<uint8_t> c_Response;
   uint8_t u8_NrCode = 0U;
   std::error_code c_Return;

   if (ou8_SubFunction == hu8_DTC_REPORT_BY_STATUS_MASK)
   {
      c_Request.push_back(ou8_StatusMask);
   }
   oru8_AvailabilityMask = 0U;
   orc_Dtcs.clear();
   //response: 59 subFunction availabilityMask (dtc(3) status)*
   c_Return = m_Transact(c_Request, 3U, c_Response, u8_NrCode);
   if (c_Return == Errc::success)
   {
      const size_t x_RecordBytes = c_Response.size() - 3U;
      if ((c_Response[1] != ou8_SubFunction) || ((x_RecordBytes % 4U) != 0U))
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         oru8_AvailabilityMask = c_Response[2];
         for (size_t x_Index = 3U; x_Index < c_Response.size(); x_Index += 4U)
         {
            C_DtcRecord c_Record;
            c_Record.u32_Dtc = (static_cast<uint32_t>(c_Response[x_Index]) << 16U) |
                               (static_cast<uint32_t>(c_Response[x_Index + 1U]) << 8U) |
                               c_Response[x_Index + 2U];
            c_Record.u8_Status = c_Response[x_Index + 3U];
            orc_Dtcs.push_back(c_Record);
         }
      }
   }
   mh_ReportNrCode(opu8_NrCode, u8_NrCode);
   m_LogServiceError("ReadDTCInformation", c_Return, u8_NrCode);
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverUds::mh_ReportNrCode(uint8_t * const opu8_NrCode, const uint8_t ou8_NrCode)
{
   if (opu8_NrCode != nullptr)
   {
      (*opu8_NrCode) = ou8_NrCode;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Log a failed service

   \param[in]  opcn_Service   service name
   \param[in]  orc_Result     result
   \param[in]  ou8_NrCode     negative response code if any
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverUds::m_LogServiceError(const char * const opcn_Service, const std::error_code & orc_Result,
                                               const uint8_t ou8_NrCode) const
{
   if (orc_Result != Errc::success)
   {
      osc_write_log_error(hacn_LOG_ACTIVITY, std::string(opcn_Service) + ": " +
                          h_GetServiceErrorDetails(orc_Result, ou8_NrCode));
   }
}
