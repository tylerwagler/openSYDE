//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class to handle crypto agent access
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCRYPTOAGENTACCESSUTIL_HPP
#define C_OSCCRYPTOAGENTACCESSUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNode.hpp"
#include "C_OscIpDispatcher.hpp"
#include "C_OscCryptoAgentSettings.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCryptoAgentAccessUtil
{
public:
   static void h_SetCryptoAgentSettings(const C_OscCryptoAgentSettings & orc_Settings);
   static void h_HandleCryptoAgentAutostart(void);
   static void h_HandleCryptoAgentAutostop(void);
   static int32_t h_GetRsaSignatureBySerialNumber(const std::vector<uint8_t> & orc_SerialNumber,
                                                  const uint8_t ou8_SecurityLevel,
                                                  const std::vector<uint8_t> & orc_ServerChallengeValue,
                                                  std::vector<uint8_t> & orc_RsaSignature,
                                                  const stw::scl::C_SclString & orc_LastLoadedSystemDefinitionFilePath,
                                                  const C_OscNode & orc_Node, const uint8_t ou8_NodeIdentifier,
                                                  const stw::scl::C_SclString & orc_SerialNumberExtended,
                                                  const uint8_t ou8_SerialNumberManufacturerFormat);

private:
   C_OscCryptoAgentAccessUtil();
   static const stw::scl::C_SclString mhc_LOG_HEADING;
   static const uint8_t mhu8_REQUEST_SUCCESS;
   static const uint8_t mhu8_REQUEST_FAILED;
   static const uint32_t mhu32_TIMEOUT_REQUEST_100MS;
   static C_OscCryptoAgentSettings mhc_CurrentSettings;

   static int32_t mh_PingCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher);
   static int32_t mh_ShutdownCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher);
   static void mh_StartCryptoAgent(void);
   static int32_t mh_SendAndReceiveFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                   const std::vector<uint8_t> & orc_Request,
                                                   std::vector<uint8_t> & orc_Response);
   static int32_t mh_WaitForResponseFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                    std::vector<uint8_t> & orc_Response, const uint32_t ou32_Handle);
   static std::vector<uint8_t> mh_GetPingRequest(void);
   static std::vector<uint8_t> mh_GetPingDataOnly(void);
   static std::vector<uint8_t> mh_GetShutdownRequest(void);
   static std::vector<uint8_t> mh_GetShutdownDataOnly(void);
   static std::vector<uint8_t> mh_GetRsaSignatureBySerialNumberRequest(const std::vector<uint8_t> & orc_SerialNumber,
                                                                       const uint8_t ou8_SecurityLevel,
                                                                       const std::vector<uint8_t> & orc_ServerChallengeValue, const stw::scl::C_SclString & orc_LastLoadedSystemDefinitionFilePath, const C_OscNode & orc_Node, const uint8_t ou8_NodeIdentifier, const stw::scl::C_SclString & orc_SerialNumberExtended, const uint8_t ou8_SerialNumberManufacturerFormat);
   static void mh_AppendStringAndSize(std::vector<uint8_t> & orc_Request, const std::string & orc_Data);
   static void mh_AppendDataAndSize(std::vector<uint8_t> & orc_Request, const std::vector<uint8_t> & orc_Data);
   static void mh_AppendSize(std::vector<uint8_t> & orc_Request, const uint32_t ou32_Size);
   static void mh_PrependDataSize(std::vector<uint8_t> & orc_Data);
   static int32_t mh_CheckResponse(const std::vector<uint8_t> & orc_Response,
                                   uint8_t * const opu8_ResponseValue = NULL);
   static int32_t mh_HandlePingResponse(const std::vector<uint8_t> & orc_Response);
   static int32_t mh_HandleRsaSignatureResponse(const std::vector<uint8_t> & orc_Response,
                                                std::vector<uint8_t> & orc_RsaSignature);
   static int32_t mh_ReadSize(const std::vector<uint8_t> & orc_Response, const uint32_t ou32_StartPosition,
                              const uint32_t ou32_NumBytes, uint32_t & oru32_Result);
   static int32_t mh_ReadString(const std::vector<uint8_t> & orc_Response, const uint32_t ou32_StartPosition,
                                const uint32_t ou32_Length, stw::scl::C_SclString & orc_Result);
   static int32_t mh_ReportString(const std::vector<uint8_t> & orc_Response, const uint32_t ou32_StartPosition,
                                  const uint32_t ou32_SizeLength, const stw::scl::C_SclString & orc_Category,
                                  uint32_t * const opu32_EndPos = NULL);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
