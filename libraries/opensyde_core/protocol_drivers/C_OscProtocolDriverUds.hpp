//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generic UDS (ISO 14229-1) client

   \class       stw::opensyde_core::C_OscProtocolDriverUds
   \brief       Generic UDS (ISO 14229-1) client

   A client for a server that speaks plain UDS, with none of openSYDE's supplier-specific services, identifiers,
   routines or sessions. It runs over the same transport protocols as the openSYDE driver
   (C_OscProtocolDriverOsyTpBase: CAN with ISO 15765-2, DoIP-style IP), so it reuses the segmentation and the
   test doubles, but shares no code with C_OscProtocolDriverOsy: that driver is 6,500 lines of openSYDE server
   assumptions and this one is deliberately small.

   Every service is "polled": the request goes out, the client waits up to P2 for the matching response, a
   requestCorrectlyReceived-ResponsePending (0x78) negative response extends the wait to P2*, and a
   busyRepeatRequest (0x21) makes it send the request again a bounded number of times. P2 and P2* start at the
   ISO defaults and are taken over from every DiagnosticSessionControl response.

   Negative responses come back as Errc::warn with the code in the optional opu8_NrCode parameter, the way the
   openSYDE driver reports them, so sequences can treat both drivers alike.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPROTOCOLDRIVERUDS_HPP
#define C_OSCPROTOCOLDRIVERUDS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <mutex>
#include <string>
#include <system_error>
#include <vector>

#include "C_OscProtocolDriverOsyTpBase.hpp"
#include "C_OscUdsSeedKey.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscProtocolDriverUds
{
public:
   ///one diagnostic trouble code with its status byte, as ReadDTCInformation reports it
   class C_DtcRecord
   {
   public:
      uint32_t u32_Dtc;  ///< 24 bit DTC (high byte, middle byte, low byte of the record)
      uint8_t u8_Status; ///< DTC status byte (ISO 14229-1 D.2)
   };

   //service identifiers
   static constexpr uint8_t hu8_SI_DIAGNOSTIC_SESSION_CONTROL  = 0x10U;
   static constexpr uint8_t hu8_SI_ECU_RESET                   = 0x11U;
   static constexpr uint8_t hu8_SI_CLEAR_DIAGNOSTIC_INFORMATION = 0x14U;
   static constexpr uint8_t hu8_SI_READ_DTC_INFORMATION        = 0x19U;
   static constexpr uint8_t hu8_SI_READ_DATA_BY_IDENTIFIER     = 0x22U;
   static constexpr uint8_t hu8_SI_SECURITY_ACCESS             = 0x27U;
   static constexpr uint8_t hu8_SI_COMMUNICATION_CONTROL       = 0x28U;
   static constexpr uint8_t hu8_SI_WRITE_DATA_BY_IDENTIFIER    = 0x2EU;
   static constexpr uint8_t hu8_SI_ROUTINE_CONTROL             = 0x31U;
   static constexpr uint8_t hu8_SI_REQUEST_DOWNLOAD            = 0x34U;
   static constexpr uint8_t hu8_SI_TRANSFER_DATA               = 0x36U;
   static constexpr uint8_t hu8_SI_REQUEST_TRANSFER_EXIT       = 0x37U;
   static constexpr uint8_t hu8_SI_TESTER_PRESENT              = 0x3EU;
   static constexpr uint8_t hu8_SI_CONTROL_DTC_SETTING         = 0x85U;
   static constexpr uint8_t hu8_SI_NEGATIVE_RESPONSE           = 0x7FU;

   //sessions (DiagnosticSessionControl sub-functions)
   static constexpr uint8_t hu8_SESSION_DEFAULT                = 0x01U;
   static constexpr uint8_t hu8_SESSION_PROGRAMMING            = 0x02U;
   static constexpr uint8_t hu8_SESSION_EXTENDED_DIAGNOSTIC    = 0x03U;
   static constexpr uint8_t hu8_SESSION_SAFETY_SYSTEM_DIAGNOSTIC = 0x04U;

   //reset types (ECUReset sub-functions)
   static constexpr uint8_t hu8_RESET_HARD                     = 0x01U;
   static constexpr uint8_t hu8_RESET_KEY_OFF_ON               = 0x02U;
   static constexpr uint8_t hu8_RESET_SOFT                     = 0x03U;

   //RoutineControl sub-functions
   static constexpr uint8_t hu8_ROUTINE_START                  = 0x01U;
   static constexpr uint8_t hu8_ROUTINE_STOP                   = 0x02U;
   static constexpr uint8_t hu8_ROUTINE_REQUEST_RESULTS        = 0x03U;

   //CommunicationControl control types and communication types
   static constexpr uint8_t hu8_COMM_ENABLE_RX_AND_TX          = 0x00U;
   static constexpr uint8_t hu8_COMM_ENABLE_RX_DISABLE_TX      = 0x01U;
   static constexpr uint8_t hu8_COMM_DISABLE_RX_ENABLE_TX      = 0x02U;
   static constexpr uint8_t hu8_COMM_DISABLE_RX_AND_TX         = 0x03U;
   static constexpr uint8_t hu8_COMM_TYPE_NORMAL               = 0x01U;
   static constexpr uint8_t hu8_COMM_TYPE_NETWORK_MANAGEMENT   = 0x02U;
   static constexpr uint8_t hu8_COMM_TYPE_NORMAL_AND_NETWORK_MANAGEMENT = 0x03U;

   //ControlDTCSetting sub-functions
   static constexpr uint8_t hu8_DTC_SETTING_ON                 = 0x01U;
   static constexpr uint8_t hu8_DTC_SETTING_OFF                = 0x02U;

   //ReadDTCInformation sub-functions used here
   static constexpr uint8_t hu8_DTC_REPORT_NUMBER_BY_STATUS_MASK = 0x01U;
   static constexpr uint8_t hu8_DTC_REPORT_BY_STATUS_MASK      = 0x02U;
   static constexpr uint8_t hu8_DTC_REPORT_SUPPORTED           = 0x0AU;

   ///ClearDiagnosticInformation: every DTC
   static constexpr uint32_t hu32_DTC_GROUP_ALL                = 0xFFFFFFU;

   ///ISO 14229-2 default timing until a session response says otherwise
   static constexpr uint32_t hu32_DEFAULT_P2_MS                = 50U;
   static constexpr uint32_t hu32_DEFAULT_P2_STAR_MS           = 5000U;

   C_OscProtocolDriverUds(void);
   virtual ~C_OscProtocolDriverUds(void);

   [[nodiscard]] std::error_code SetTransportProtocol(C_OscProtocolDriverOsyTpBase * const opc_TransportProtocol);
   C_OscProtocolDriverOsyTpBase * GetTransportProtocol(void) const;
   void SetSeedKey(const C_OscUdsSeedKey * const opc_SeedKey);
   void SetTimings(const uint32_t ou32_P2Ms, const uint32_t ou32_P2StarMs);
   uint32_t GetP2Ms(void) const;
   uint32_t GetP2StarMs(void) const;

   //Deliberately not [[nodiscard]] -- a dispatcher poll, called from a read loop, as on the openSYDE driver.
   std::error_code Cycle(void);

   //services:
   [[nodiscard]] std::error_code DiagnosticSessionControl(const uint8_t ou8_Session,
                                                          uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code EcuReset(const uint8_t ou8_ResetType, const bool oq_SuppressResponse = false,
                                          uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code TesterPresent(const bool oq_SuppressResponse = true,
                                               uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ReadDataByIdentifier(const uint16_t ou16_Identifier, std::vector<uint8_t> & orc_Data,
                                                      uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code WriteDataByIdentifier(const uint16_t ou16_Identifier,
                                                       const std::vector<uint8_t> & orc_Data,
                                                       uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code SecurityAccessRequestSeed(const uint8_t ou8_Level, std::vector<uint8_t> & orc_Seed,
                                                           uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code SecurityAccessSendKey(const uint8_t ou8_Level, const std::vector<uint8_t> & orc_Key,
                                                       uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code SecurityAccess(const uint8_t ou8_Level, uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code RoutineControl(const uint8_t ou8_SubFunction, const uint16_t ou16_RoutineIdentifier,
                                                const std::vector<uint8_t> & orc_RequestRecord,
                                                std::vector<uint8_t> & orc_ResponseRecord,
                                                uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code CommunicationControl(const uint8_t ou8_ControlType, const uint8_t ou8_CommunicationType,
                                                      uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ControlDtcSetting(const uint8_t ou8_SettingType, uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ClearDiagnosticInformation(const uint32_t ou32_GroupOfDtc,
                                                            uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ReadNumberOfDtcByStatusMask(const uint8_t ou8_StatusMask,
                                                             uint8_t & oru8_AvailabilityMask, uint16_t & oru16_Count,
                                                             uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ReadDtcByStatusMask(const uint8_t ou8_StatusMask, uint8_t & oru8_AvailabilityMask,
                                                     std::vector<C_DtcRecord> & orc_Dtcs,
                                                     uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code ReadSupportedDtc(uint8_t & oru8_AvailabilityMask, std::vector<C_DtcRecord> & orc_Dtcs,
                                                  uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code RequestDownload(const uint8_t ou8_DataFormatIdentifier, const uint32_t ou32_Address,
                                                 const uint32_t ou32_Size, uint32_t & oru32_MaxBlockLength,
                                                 uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code TransferData(const uint8_t ou8_BlockSequenceCounter,
                                              const std::vector<uint8_t> & orc_Data,
                                              uint8_t * const opu8_NrCode = nullptr);
   [[nodiscard]] std::error_code RequestTransferExit(uint8_t * const opu8_NrCode = nullptr);

   //any service, for what the typed ones do not cover:
   [[nodiscard]] std::error_code SendRequest(const std::vector<uint8_t> & orc_Request,
                                             std::vector<uint8_t> & orc_Response, uint8_t * const opu8_NrCode = nullptr,
                                             const bool oq_SuppressResponse = false);

   static std::string h_GetServiceErrorDetails(const std::error_code & orc_FunctionResult, const uint8_t ou8_NrCode);

private:
   static constexpr uint8_t mhu8_SUPPRESS_POSITIVE_RESPONSE = 0x80U;
   static constexpr uint8_t mhu8_POSITIVE_RESPONSE_OFFSET   = 0x40U;
   static constexpr uint8_t mhu8_MAX_BUSY_REPEATS           = 3U;
   static constexpr uint8_t mhu8_ADDRESS_AND_LENGTH_FORMAT_4_4 = 0x44U; ///< four byte address, four byte size

   C_OscProtocolDriverOsyTpBase * mpc_TransportProtocol;
   const C_OscUdsSeedKey * mpc_SeedKey;
   C_OscUdsSeedKeyConstant mc_DefaultSeedKey;
   uint32_t mu32_P2Ms;
   uint32_t mu32_P2StarMs;
   std::mutex mc_LockReception;

   [[nodiscard]] std::error_code m_Transact(const std::vector<uint8_t> & orc_Request, const size_t ox_MinResponseSize,
                                            std::vector<uint8_t> & orc_Response, uint8_t & oru8_NrCode,
                                            const bool oq_SuppressResponse = false);
   [[nodiscard]] std::error_code m_WaitForResponse(const uint8_t ou8_ServiceId, std::vector<uint8_t> & orc_Response,
                                                   uint8_t & oru8_NrCode);
   [[nodiscard]] std::error_code m_ReadDtcRecords(const uint8_t ou8_SubFunction, const uint8_t ou8_StatusMask,
                                                  uint8_t & oru8_AvailabilityMask, std::vector<C_DtcRecord> & orc_Dtcs,
                                                  uint8_t * const opu8_NrCode);
   static void mh_ReportNrCode(uint8_t * const opu8_NrCode, const uint8_t ou8_NrCode);
   void m_LogServiceError(const char * const opcn_Service, const std::error_code & orc_Result,
                          const uint8_t ou8_NrCode) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
