//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to UDS (ISO 14229) L7 interpretation

   Translate L2 CAN message to Unified Diagnostic Services (ISO 14229) L7 interpretation

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <sstream>
#include <iomanip>
#include <cstdint>
#include "C_CanMonProtocolUds.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::cmon_protocol;
using namespace stw::can;

/* -- Anonymous Helpers --------------------------------------------------------------------------------------------- */
namespace {
   template <typename T>
   std::string mh_IntToHex(T val, uint32_t digits) {
      std::stringstream ss;
      ss << std::hex << std::uppercase << std::setw(digits) << std::setfill('0') << val;
      return ss.str();
   }
}

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

// ISO 15765-2 PCI types
#define PCI_SF  (0x00U) ///< Single Frame
#define PCI_FF  (0x10U) ///< First Frame
#define PCI_CF  (0x20U) ///< Consecutive Frame
#define PCI_FC  (0x30U) ///< Flow Control

// UDS service IDs (ISO 14229)
#define SID_DIAGNOSTIC_SESSION_CONTROL         (0x10U)
#define SID_ECU_RESET                          (0x11U)
#define SID_READ_DATA_BY_IDENTIFIER            (0x22U)
#define SID_READ_MEMORY_BY_ADDRESS             (0x23U)
#define SID_READ_SCALING_DATA_BY_IDENTIFIER    (0x24U)
#define SID_READ_DATA_BY_PERIODIC_ID           (0x2AU)
#define SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER (0x2CU)
#define SID_WRITE_DATA_BY_IDENTIFIER           (0x2EU)
#define SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER (0x2FU)
#define SID_SECURITY_ACCESS                    (0x27U)
#define SID_COMMUNICATION_CONTROL              (0x28U)
#define SID_ROUTINE_CONTROL                    (0x31U)
#define SID_REQUEST_DOWNLOAD                   (0x34U)
#define SID_REQUEST_UPLOAD                     (0x35U)
#define SID_TRANSFER_DATA                      (0x36U)
#define SID_REQUEST_TRANSFER_EXIT              (0x37U)
#define SID_REQUEST_FILE_TRANSFER              (0x38U)
#define SID_WRITE_MEMORY_BY_ADDRESS            (0x3DU)
#define SID_TESTER_PRESENT                     (0x3EU)
#define SID_NEGATIVE_RESPONSE                  (0x7FU)
#define SID_ACCESS_TIMING_PARAMETER            (0x83U)
#define SID_SECURED_DATA_TRANSMISSION          (0x84U)
#define SID_CONTROL_DTC_SETTINGS               (0x85U)
#define SID_RESPONSE_EVENT_ON_DTC_STATUS       (0x86U)
#define SID_LINK_CONTROL                       (0x87U)

// Negative response codes (ISO 14229)
#define NRC_NO_ERROR                                   (0x00U)
#define NRC_SERVICE_NOT_SUPPORTED                      (0x11U)
#define NRC_SUB_FUNCTION_NOT_SUPPORTED                 (0x12U)
#define NRC_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT (0x13U)
#define NRC_RESPONSE_TOO_LONG                          (0x14U)
#define NRC_BUSY_REPEAT_REQUEST                        (0x21U)
#define NRC_CONDITIONS_NOT_CORRECT                     (0x22U)
#define NRC_REQUEST_SEQUENCE_ERROR                     (0x24U)
#define NRC_NO_RESPONSE_FROM_SUBNET_COMPONENT          (0x25U)
#define NRC_FAILURE_PREVENTS_EXECUTION_OF_REQUEST      (0x26U)
#define NRC_REQUEST_OUT_OF_RANGE                       (0x31U)
#define NRC_SECURITY_ACCESS_DENIED                     (0x33U)
#define NRC_INVALID_KEY                                (0x35U)
#define NRC_EXCEEDED_NUMBER_OF_ATTEMPTS                (0x36U)
#define NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED            (0x37U)
#define NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED               (0x70U)
#define NRC_TRANSFER_DATA_SUSPENDED                    (0x71U)
#define NRC_GENERAL_PROGRAMMING_FAILURE                (0x72U)
#define NRC_WRONG_BLOCK_SEQUENCE_COUNTER               (0x73U)
#define NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING (0x78U)
#define NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION (0x7EU)
#define NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION    (0x7FU)
#define NRC_VOLTAGE_TOO_HIGH                           (0x92U)
#define NRC_VOLTAGE_TOO_LOW                            (0x93U)

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CanMonProtocolUds::C_CanMonProtocolUds(void) :
   C_CanMonProtocolBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS service identifier to text

   \param[in]  ou8_Sid               Service ID
   \param[in]  oq_IsResponse         true: response (SID + 0x40)
   \param[in]  oq_IsNegativeResponse true: negative response (0x7F)

   \return
   Text interpretation of service identifier
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_ServiceIdToText(const uint8_t ou8_Sid, const bool oq_IsResponse,
                                                     const bool oq_IsNegativeResponse)
{
   std::string c_Text;

   if (oq_IsNegativeResponse == true)
   {
      c_Text = "NEG_RES ";
   }
   else if (oq_IsResponse == true)
   {
      c_Text = "RES ";
   }
   else
   {
      c_Text = "REQ ";
   }

   switch (ou8_Sid)
   {
   case SID_DIAGNOSTIC_SESSION_CONTROL:
      c_Text += "DiagnosticSessionControl";
      break;
   case SID_ECU_RESET:
      c_Text += "ECUReset";
      break;
   case SID_READ_DATA_BY_IDENTIFIER:
      c_Text += "ReadDataByIdentifier";
      break;
   case SID_READ_MEMORY_BY_ADDRESS:
      c_Text += "ReadMemoryByAddress";
      break;
   case SID_READ_SCALING_DATA_BY_IDENTIFIER:
      c_Text += "ReadScalingDataByIdentifier";
      break;
   case SID_READ_DATA_BY_PERIODIC_ID:
      c_Text += "ReadDataByPeriodicIdentifier";
      break;
   case SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER:
      c_Text += "DynamicallyDefineDataIdentifier";
      break;
   case SID_WRITE_DATA_BY_IDENTIFIER:
      c_Text += "WriteDataByIdentifier";
      break;
   case SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
      c_Text += "InputOutputControlByIdentifier";
      break;
   case SID_SECURITY_ACCESS:
      c_Text += "SecurityAccess";
      break;
   case SID_COMMUNICATION_CONTROL:
      c_Text += "CommunicationControl";
      break;
   case SID_ROUTINE_CONTROL:
      c_Text += "RoutineControl";
      break;
   case SID_REQUEST_DOWNLOAD:
      c_Text += "RequestDownload";
      break;
   case SID_REQUEST_UPLOAD:
      c_Text += "RequestUpload";
      break;
   case SID_TRANSFER_DATA:
      c_Text += "TransferData";
      break;
   case SID_REQUEST_TRANSFER_EXIT:
      c_Text += "RequestTransferExit";
      break;
   case SID_REQUEST_FILE_TRANSFER:
      c_Text += "RequestFileTransfer";
      break;
   case SID_WRITE_MEMORY_BY_ADDRESS:
      c_Text += "WriteMemoryByAddress";
      break;
   case SID_TESTER_PRESENT:
      c_Text += "TesterPresent";
      break;
   case SID_ACCESS_TIMING_PARAMETER:
      c_Text += "AccessTimingParameter";
      break;
   case SID_SECURED_DATA_TRANSMISSION:
      c_Text += "SecuredDataTransmission";
      break;
   case SID_CONTROL_DTC_SETTINGS:
      c_Text += "ControlDTCSettings";
      break;
   case SID_RESPONSE_EVENT_ON_DTC_STATUS:
      c_Text += "ResponseOnEvent";
      break;
   case SID_LINK_CONTROL:
      c_Text += "LinkControl";
      break;
   default:
      c_Text += "UnknownService(0x" + mh_IntToHex(ou8_Sid, 2) + ")";
      break;
   }

   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS sub-function to text

   \param[in]  ou8_ServiceId  Service ID
   \param[in]  ou8_SubFunc    Sub-function byte

   \return
   Text interpretation of sub-function
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_SubFunctionToText(const uint8_t ou8_ServiceId,
                                                       const uint8_t ou8_SubFunc)
{
   std::string c_Text;
   const bool q_SuppressResponse = ((ou8_SubFunc & 0x80U) != 0U) ? true : false;
   const uint8_t u8_SubFuncVal = (ou8_SubFunc & 0x7FU);

   if (q_SuppressResponse == true)
   {
      c_Text = " SUPPRESS_RSP";
   }

   switch (ou8_ServiceId)
   {
   case SID_DIAGNOSTIC_SESSION_CONTROL:
      c_Text += " SESSION:";
      switch (u8_SubFuncVal)
      {
      case 0x01U:
         c_Text += "default";
         break;
      case 0x02U:
         c_Text += "programming";
         break;
      case 0x03U:
         c_Text += "extended";
         break;
      case 0x60U:
         c_Text += "safe";
         break;
      case 0x61U:
         c_Text += "systemSupplierSpecific1";
         break;
      case 0x62U:
         c_Text += "systemSupplierSpecific2";
         break;
      case 0x63U:
         c_Text += "systemSupplierSpecific3";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   case SID_ECU_RESET:
      c_Text += " TYPE:";
      switch (u8_SubFuncVal)
      {
      case 0x01U:
         c_Text += "hardReset";
         break;
      case 0x02U:
         c_Text += "keyOffOnReset";
         break;
      case 0x03U:
         c_Text += "softReset";
         break;
      case 0x04U:
         c_Text += "enableRapidPowerShutdown";
         break;
      case 0x05U:
         c_Text += "disableRapidPowerShutdown";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   case SID_SECURITY_ACCESS:
      c_Text += " ACCESS:";
      if ((u8_SubFuncVal % 2U) == 1U)
      {
         c_Text += "requestSeed(level=" + std::to_string(static_cast<int32_t>(u8_SubFuncVal)) + ")";
      }
      else
      {
         c_Text += "sendKey(level=" + std::to_string(static_cast<int32_t>(u8_SubFuncVal)) + ")";
      }
      break;
   case SID_COMMUNICATION_CONTROL:
      c_Text += " CTRL:";
      switch (u8_SubFuncVal)
      {
      case 0x00U:
         c_Text += "enableRxAndTx";
         break;
      case 0x01U:
         c_Text += "enableRxAndDisableTx";
         break;
      case 0x02U:
         c_Text += "disableRxAndEnableTx";
         break;
      case 0x03U:
         c_Text += "disableRxAndTx";
         break;
      case 0x04U:
         c_Text += "enableRxAndDisableTxWithEnhancedAddress";
         break;
      case 0x05U:
         c_Text += "enableRxAndTxWithEnhancedAddress";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   case SID_TESTER_PRESENT:
      c_Text += " SUBFUNC:0x" + mh_IntToHex(u8_SubFuncVal, 2);
      break;
   case SID_ACCESS_TIMING_PARAMETER:
      c_Text += " TIMING:";
      switch (u8_SubFuncVal)
      {
      case 0x00U:
         c_Text += "readExtendedTimingParams";
         break;
      case 0x01U:
         c_Text += "setTimingParamsToDefault";
         break;
      case 0x02U:
         c_Text += "readCurrentlyActiveTimingParams";
         break;
      case 0x03U:
         c_Text += "setTimingParamsToGivenValues";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   case SID_CONTROL_DTC_SETTINGS:
      c_Text += " DTC:";
      switch (u8_SubFuncVal)
      {
      case 0x01U:
         c_Text += "on";
         break;
      case 0x02U:
         c_Text += "off";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   case SID_LINK_CONTROL:
      c_Text += " LINK:";
      switch (u8_SubFuncVal)
      {
      case 0x01U:
         c_Text += "verifyBaudrateTransition";
         break;
      case 0x02U:
         c_Text += "transitionBaudrate";
         break;
      default:
         c_Text += "unknown(0x" + mh_IntToHex(u8_SubFuncVal, 2) + ")";
         break;
      }
      break;
   default:
      c_Text += " SUBFUNC:0x" + mh_IntToHex(u8_SubFuncVal, 2);
      break;
   }

   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS negative response code to text

   \param[in]  ou8_Nrc  Negative response code

   \return
   Text interpretation of NRC
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_NegativeResponseCodeToText(const uint8_t ou8_Nrc)
{
   std::string c_Text;

   switch (ou8_Nrc)
   {
   case NRC_NO_ERROR:
      c_Text = "positiveResponse";
      break;
   case NRC_SERVICE_NOT_SUPPORTED:
      c_Text = "serviceNotSupported";
      break;
   case NRC_SUB_FUNCTION_NOT_SUPPORTED:
      c_Text = "subFunctionNotSupported";
      break;
   case NRC_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT:
      c_Text = "incorrectMessageLengthOrInvalidFormat";
      break;
   case NRC_RESPONSE_TOO_LONG:
      c_Text = "responseTooLong";
      break;
   case NRC_BUSY_REPEAT_REQUEST:
      c_Text = "busyRepeatRequest";
      break;
   case NRC_CONDITIONS_NOT_CORRECT:
      c_Text = "conditionsNotCorrect";
      break;
   case NRC_REQUEST_SEQUENCE_ERROR:
      c_Text = "requestSequenceError";
      break;
   case NRC_NO_RESPONSE_FROM_SUBNET_COMPONENT:
      c_Text = "noResponseFromSubnetComponent";
      break;
   case NRC_FAILURE_PREVENTS_EXECUTION_OF_REQUEST:
      c_Text = "failurePreventsExecutionOfRequest";
      break;
   case NRC_REQUEST_OUT_OF_RANGE:
      c_Text = "requestOutOfRange";
      break;
   case NRC_SECURITY_ACCESS_DENIED:
      c_Text = "securityAccessDenied";
      break;
   case NRC_INVALID_KEY:
      c_Text = "invalidKey";
      break;
   case NRC_EXCEEDED_NUMBER_OF_ATTEMPTS:
      c_Text = "exceededNumberOfAttempts";
      break;
   case NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED:
      c_Text = "requiredTimeDelayNotExpired";
      break;
   case NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED:
      c_Text = "uploadDownloadNotAccepted";
      break;
   case NRC_TRANSFER_DATA_SUSPENDED:
      c_Text = "transferDataSuspended";
      break;
   case NRC_GENERAL_PROGRAMMING_FAILURE:
      c_Text = "generalProgrammingFailure";
      break;
   case NRC_WRONG_BLOCK_SEQUENCE_COUNTER:
      c_Text = "wrongBlockSequenceCounter";
      break;
   case NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING:
      c_Text = "responsePending";
      break;
   case NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION:
      c_Text = "subFunctionNotSupportedInActiveSession";
      break;
   case NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION:
      c_Text = "serviceNotSupportedInActiveSession";
      break;
   case NRC_VOLTAGE_TOO_HIGH:
      c_Text = "voltageTooHigh";
      break;
   case NRC_VOLTAGE_TOO_LOW:
      c_Text = "voltageTooLow";
      break;
   default:
      c_Text = "unknownNRC(0x" + mh_IntToHex(ou8_Nrc, 2) + ")";
      break;
   }

   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS session to text

   \param[in]  ou8_Session  Session ID

   \return
   Text interpretation
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_SessionToText(const uint8_t ou8_Session)
{
   switch (ou8_Session)
   {
   case 0x01U:
      return "default";
   case 0x02U:
      return "programming";
   case 0x03U:
      return "extended";
   case 0x60U:
      return "safe";
   default:
      return "unknown(0x" + mh_IntToHex(ou8_Session, 2) + ")";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS reset type to text

   \param[in]  ou8_ResetType  Reset type

   \return
   Text interpretation
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_ResetTypeToText(const uint8_t ou8_ResetType)
{
   switch (ou8_ResetType)
   {
   case 0x01U:
      return "hardReset";
   case 0x02U:
      return "keyOffOnReset";
   case 0x03U:
      return "softReset";
   case 0x04U:
      return "enableRapidPowerShutdown";
   case 0x05U:
      return "disableRapidPowerShutdown";
   default:
      return "unknown(0x" + mh_IntToHex(ou8_ResetType, 2) + ")";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS data identifier to text

   \param[in]  ou16_Did  Data identifier

   \return
   Text interpretation
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_DataIdentifierToText(const uint16_t ou16_Did)
{
   switch (ou16_Did)
   {
   // ISO 14229-1 defined DIDs
   case 0xF180U:
      return "bootSoftwareIdentification";
   case 0xF181U:
      return "applicationSoftwareIdentification";
   case 0xF182U:
      return "applicationDataIdentification";
   case 0xF183U:
      return "bootSoftwareFingerprint";
   case 0xF184U:
      return "applicationSoftwareFingerprint";
   case 0xF185U:
      return "applicationDataFingerprint";
   case 0xF186U:
      return "activeDiagnosticSession";
   case 0xF187U:
      return "softwareModuleIdentification";
   case 0xF18CU:
      return "ecuSerialNumber";
   case 0xF18DU:
      return "systemSupplierSpecificData";
   case 0xF192U:
      return "systemSupplierEcuHardwareNumber";
   case 0xF193U:
      return "systemSupplierEcuHardwareVersionNumber";
   case 0xF194U:
      return "systemSupplierEcuManufacturingDate";
   case 0xF195U:
      return "systemSupplierEcuManufacturingPlant";
   case 0xF1A0U:
      return "vin";
   case 0xFF00U:
      return "udsVersion";
   default:
      return "DID(0x" + mh_IntToHex(ou16_Did, 4) + ")";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS routine identifier to text

   \param[in]  ou16_Rid  Routine identifier

   \return
   Text interpretation
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_RoutineIdentifierToText(const uint16_t ou16_Rid)
{
   switch (ou16_Rid)
   {
   case 0xFF00U:
      return "RoutineCtrlUdsVersion";
   default:
      return "RID(0x" + mh_IntToHex(ou16_Rid, 4) + ")";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert UDS security access type to text

   \param[in]  ou8_AccessType  Access type

   \return
   Text interpretation
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::mh_AccessTypeToText(const uint8_t ou8_AccessType)
{
   if ((ou8_AccessType % 2U) == 1U)
   {
      return "requestSeed(level=" + std::to_string(static_cast<int32_t>(ou8_AccessType)) + ")";
   }
   return "sendKey(level=" + std::to_string(static_cast<int32_t>(ou8_AccessType)) + ")";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert CAN message to UDS text representation

   \param[in]  orc_Msg  CAN message to convert

   \return
   Text interpretation of CAN message ("" if the message cannot be interpreted)
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::MessageToString(const T_STWCAN_Msg_RX & orc_Msg) const
{
   std::string c_Text;

   if (orc_Msg.u8_DLC < 1U)
   {
      return "";
   }

   // Determine PCI type from first nibble
   const uint8_t u8_Pci = (orc_Msg.au8_Data[0] & 0xF0U);
   uint8_t u8_NumDataBytes = 0U;
   uint8_t u8_ServiceStart = 1U; // byte index where service data starts

   switch (u8_Pci)
   {
   case PCI_SF:
      // Single Frame: lower nibble is data length
      u8_NumDataBytes = (orc_Msg.au8_Data[0] & 0x0FU);
      u8_ServiceStart = 1U;
      break;
   case PCI_FF:
      // First Frame: lower nibble + next byte = 12-bit data length
      u8_NumDataBytes = static_cast<uint8_t>(((static_cast<uint16_t>(orc_Msg.au8_Data[0] & 0x0FU) << 8U) |
                                                orc_Msg.au8_Data[1]) & 0x0FFFU);
      u8_ServiceStart = 2U;
      break;
   case PCI_CF:
      // Consecutive Frame: sequence number in lower nibble
      return "CF(seq=" + std::to_string(static_cast<int32_t>(orc_Msg.au8_Data[0] & 0x0FU)) + ")";
   case PCI_FC:
      // Flow Control
      return "FC(sts=" + std::to_string(static_cast<int32_t>((orc_Msg.au8_Data[0] & 0x0FU))) +
             ",bs=" + std::to_string(static_cast<int32_t>(orc_Msg.au8_Data[1])) +
             ",stmin=" + std::to_string(static_cast<int32_t>(orc_Msg.au8_Data[2])) + ")";
   default:
      return "";
   }

   // Check if we have enough data
   if ((static_cast<uint16_t>(u8_ServiceStart) + static_cast<uint16_t>(u8_NumDataBytes)) >
       static_cast<uint16_t>(orc_Msg.u8_DLC))
   {
      return "SF(DLC too short)";
   }

   if (u8_NumDataBytes < 1U)
   {
      return "SF(empty)";
   }

   // Get the service ID (first byte after PCI)
   const uint8_t u8_Sid = orc_Msg.au8_Data[u8_ServiceStart];

   // Check for negative response
   if (u8_Sid == SID_NEGATIVE_RESPONSE)
   {
      if (u8_NumDataBytes < 3U)
      {
         return "NEGRES(invalid length)";
      }
      const uint8_t u8_ReqSid = orc_Msg.au8_Data[u8_ServiceStart + 1U];
      const uint8_t u8_Nrc = orc_Msg.au8_Data[u8_ServiceStart + 2U];
      c_Text = mh_ServiceIdToText(u8_ReqSid, false, true);
      c_Text += " NRC:" + mh_NegativeResponseCodeToText(u8_Nrc);
      return c_Text;
   }

   // Determine if this is a response (SID has bit 6 set = response)
   const bool q_IsResponse = ((u8_Sid & 0x40U) == 0x40U) ? true : false;
   const uint8_t u8_BaseSid = (u8_Sid & 0xBFU);

   // Format: "SF/FF REQ/RES ServiceName [subfunc] [data]"
   if (u8_Pci == PCI_SF)
   {
      c_Text = "SF ";
   }
   else
   {
      c_Text = "FF ";
   }

   c_Text += mh_ServiceIdToText(u8_BaseSid, q_IsResponse, false);

   // Decode service-specific data
   const uint8_t * const pu8_ServiceData = &orc_Msg.au8_Data[u8_ServiceStart];
   const uint8_t u8_ServiceLen = u8_NumDataBytes;
   uint8_t u8_Consumed = 1U; // bytes consumed by service-specific decoding

   switch (u8_BaseSid)
   {
   case SID_DIAGNOSTIC_SESSION_CONTROL:
   case SID_ECU_RESET:
   case SID_SECURITY_ACCESS:
   case SID_COMMUNICATION_CONTROL:
   case SID_TESTER_PRESENT:
   case SID_ACCESS_TIMING_PARAMETER:
   case SID_CONTROL_DTC_SETTINGS:
   case SID_LINK_CONTROL:
      // Services with sub-function byte
      if (u8_ServiceLen >= 2U)
      {
         c_Text += mh_SubFunctionToText(u8_BaseSid, pu8_ServiceData[1]);
         u8_Consumed = 2U;
      }
      break;
   case SID_READ_DATA_BY_IDENTIFIER:
   case SID_WRITE_DATA_BY_IDENTIFIER:
   case SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
      // Services with 2-byte data identifier
      if (u8_ServiceLen >= 3U)
      {
         const uint16_t u16_Did = (static_cast<uint16_t>(pu8_ServiceData[1]) << 8U) |
                                   static_cast<uint16_t>(pu8_ServiceData[2]);
         c_Text += " DID:" + mh_DataIdentifierToText(u16_Did);
         u8_Consumed = 3U;
      }
      break;
   case SID_READ_MEMORY_BY_ADDRESS:
   case SID_WRITE_MEMORY_BY_ADDRESS:
      // Memory address + size (format byte + address + size)
      if (u8_ServiceLen >= 2U)
      {
         const uint8_t u8_AddrFormat = pu8_ServiceData[1];
         const uint8_t u8_NumAddrBytes = (u8_AddrFormat & 0x0FU);
         const uint8_t u8_NumSizeBytes = (u8_AddrFormat >> 4U);
         c_Text += " ADDR_FORMAT:0x" + mh_IntToHex(u8_AddrFormat, 2);
         u8_Consumed = 2U + u8_NumAddrBytes + u8_NumSizeBytes;
         if (u8_Consumed > u8_ServiceLen)
         {
            u8_Consumed = u8_ServiceLen;
         }
      }
      break;
   case SID_ROUTINE_CONTROL:
      // Sub-function + 2-byte routine identifier
      if (u8_ServiceLen >= 4U)
      {
         c_Text += mh_SubFunctionToText(u8_BaseSid, pu8_ServiceData[1]);
         const uint16_t u16_Rid = (static_cast<uint16_t>(pu8_ServiceData[2]) << 8U) |
                                   static_cast<uint16_t>(pu8_ServiceData[3]);
         c_Text += " RID:" + mh_RoutineIdentifierToText(u16_Rid);
         u8_Consumed = 4U;
      }
      break;
   case SID_REQUEST_DOWNLOAD:
   case SID_REQUEST_UPLOAD:
      // Data format identifier + address + size
      if (u8_ServiceLen >= 2U)
      {
         const uint8_t u8_DataFormat = pu8_ServiceData[1];
         c_Text += " DFMT:0x" + mh_IntToHex(u8_DataFormat, 2);
         u8_Consumed = 2U;
         // Remaining bytes are address + size (format-dependent)
      }
      break;
   case SID_TRANSFER_DATA:
      // Block sequence counter
      if (u8_ServiceLen >= 2U)
      {
         c_Text += " BLKSEQ:" + std::to_string(static_cast<int32_t>(pu8_ServiceData[1]));
         u8_Consumed = 2U;
      }
      break;
   case SID_REQUEST_TRANSFER_EXIT:
      // No additional parameters required
      u8_Consumed = 1U;
      break;
   default:
      u8_Consumed = 1U;
      break;
   }

   // Append remaining raw data
   if (u8_Consumed < u8_ServiceLen)
   {
      c_Text += " DATA[";
      for (uint8_t u8_i = u8_Consumed; u8_i < u8_ServiceLen; u8_i++)
      {
         if (u8_i > u8_Consumed)
         {
            c_Text += ",";
         }
         c_Text += m_GetByteAsStringFormat(pu8_ServiceData[u8_i]);
      }
      c_Text += "]";
   }

   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get protocol name

   \return
   Protocol name
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CanMonProtocolUds::GetProtocolName(void) const
{
   return "UDS (ISO 14229)";
}
