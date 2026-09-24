//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class to handle crypto agent access

   Utility class to handle crypto agent access

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <vector>
#include <cstdint>

#include "TglFile.hpp"
#include "TglTime.hpp"
#include "TglTasks.hpp"
#include "stwerrors.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscIpDispatcherPlatform.hpp"
#include "C_OscCryptoAgentAccessUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscCryptoAgentAccessUtil::mhc_LOG_HEADING = "Crypto Agent access";
const uint8_t C_OscCryptoAgentAccessUtil::mhu8_REQUEST_SUCCESS = 0U;
const uint8_t C_OscCryptoAgentAccessUtil::mhu8_REQUEST_FAILED = 3U;
const uint32_t C_OscCryptoAgentAccessUtil::mhu32_TIMEOUT_REQUEST_100MS = 35U;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
C_OscCryptoAgentSettings C_OscCryptoAgentAccessUtil::mhc_CurrentSettings;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set crypto agent settings

   \param[in]  orc_Settings   Settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::h_SetCryptoAgentSettings(const C_OscCryptoAgentSettings & orc_Settings)
{
   mhc_CurrentSettings = orc_Settings;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle crypto agent autostart
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::h_HandleCryptoAgentAutostart(void)
{
   if (mhc_CurrentSettings.q_CryptoAgentAutoStart)
   {
      C_OscIpDispatcherImpl c_Dispatcher;
      const std::error_code c_Retval = mh_PingCryptoAgent(&c_Dispatcher);
      if (c_Retval == Errc::noact)
      {
         mh_StartCryptoAgent();
      }
      else if (c_Retval == Errc::success)
      {
         osc_write_log_info(mhc_LOG_HEADING, "Crypto Agent already running");
      }
      else
      {
         osc_write_log_warning(mhc_LOG_HEADING, "communication error to Crypto Agent");
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle crypto agent autostop
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::h_HandleCryptoAgentAutostop(void)
{
   if (mhc_CurrentSettings.q_CryptoAgentAutoStop)
   {
      C_OscIpDispatcherImpl c_Dispatcher;
      const std::error_code c_Retval = mh_ShutdownCryptoAgent(&c_Dispatcher);
      if (c_Retval == Errc::success)
      {
         osc_write_log_info(mhc_LOG_HEADING, "stopped");
      }
      else
      {
         osc_write_log_warning(mhc_LOG_HEADING, "could not stop");
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get rsa signature by serial number

   \param[in]      orc_SerialNumber                         Serial number
   \param[in]      ou8_SecurityLevel                        Security level
   \param[in]      orc_ServerChallengeValue                 Server challenge value
   \param[in,out]  orc_RsaSignature                         Rsa signature
   \param[in]      orc_LastLoadedSystemDefinitionFilePath   Last loaded system definition file path
   \param[in]      orc_Node                                 Node
   \param[in]      ou8_NodeIdentifier                       Node identifier
   \param[in]      orc_SerialNumberExtended                 Serial number extended
   \param[in]      ou8_SerialNumberManufacturerFormat       Serial number manufacturer format

   \return
   STW error codes

   \retval   Errc::success   Rsa signature valid
   \retval   Errc::noact     No connection
   \retval   Errc::com       Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::h_GetRsaSignatureBySerialNumber(
   const std::vector<uint8_t> & orc_SerialNumber, const uint8_t ou8_SecurityLevel,
   const std::vector<uint8_t> & orc_ServerChallengeValue, std::vector<uint8_t> & orc_RsaSignature,
   const std::string & orc_LastLoadedSystemDefinitionFilePath, const C_OscNode & orc_Node,
   const uint8_t ou8_NodeIdentifier, const std::string & orc_SerialNumberExtended,
   const uint8_t ou8_SerialNumberManufacturerFormat)
{
   C_OscIpDispatcherImpl c_Dispatcher;
   const std::vector<uint8_t> c_Request = mh_GetRsaSignatureBySerialNumberRequest(
      orc_SerialNumber, ou8_SecurityLevel, orc_ServerChallengeValue,
      orc_LastLoadedSystemDefinitionFilePath, orc_Node, ou8_NodeIdentifier, orc_SerialNumberExtended,
      ou8_SerialNumberManufacturerFormat);

   std::vector<uint8_t> c_Response;
   std::error_code c_Retval;
   c_Response.resize(136);
   c_Retval = mh_SendAndReceiveFromCryptoAgent(&c_Dispatcher, c_Request, c_Response);
   if (c_Retval == Errc::success)
   {
      c_Retval = mh_HandleRsaSignatureResponse(c_Response, orc_RsaSignature);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCryptoAgentAccessUtil::C_OscCryptoAgentAccessUtil()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Ping crypto agent

   \param[in]  opc_Dispatcher    Dispatcher

   \return
   STW error codes

   \retval   Errc::success   Data send
   \retval   Errc::noact     No connection
   \retval   Errc::com       Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_PingCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher)
{
   const std::vector<uint8_t> c_Request = mh_GetPingRequest();

   std::vector<uint8_t> c_Response;
   std::error_code c_Retval;
   c_Response.resize(37);
   c_Retval = mh_SendAndReceiveFromCryptoAgent(opc_Dispatcher, c_Request, c_Response);
   if (c_Retval == Errc::success)
   {
      c_Retval = mh_HandlePingResponse(c_Response);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Shutdown crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher

   \return
   STW error codes

   \retval   Errc::success   Data send
   \retval   Errc::noact     No connection
   \retval   Errc::com       Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_ShutdownCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher)
{
   const std::vector<uint8_t> c_Request = mh_GetShutdownRequest();

   std::vector<uint8_t> c_Response;
   std::error_code c_Retval;
   c_Response.resize(6);
   c_Retval = mh_SendAndReceiveFromCryptoAgent(opc_Dispatcher, c_Request, c_Response);
   if (c_Retval == Errc::success)
   {
      c_Retval = mh_CheckResponse(c_Response);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Start crypto agent
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_StartCryptoAgent(void)
{
   if (TglFileExists(mhc_CurrentSettings.c_CryptoAgentExecutablePath))
   {
      const std::string c_Parameters =
         "--configfile \"" + mhc_CurrentSettings.c_CryptoAgentConfigFilePath + "\"";
      if (TglStartProcessDetached(mhc_CurrentSettings.c_CryptoAgentExecutablePath, c_Parameters) == C_NO_ERR)
      {
         osc_write_log_info(mhc_LOG_HEADING, "started");
      }
      else
      {
         osc_write_log_warning(mhc_LOG_HEADING, "could not start");
      }
   }
   else
   {
      const std::string c_Warning =
         "could not find binary at \"" + mhc_CurrentSettings.c_CryptoAgentExecutablePath + "\"";
      osc_write_log_warning(mhc_LOG_HEADING, c_Warning);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Send and receive from crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher
   \param[in]      orc_Request      Request
   \param[in,out]  orc_Response     Response

   \return
   STW error codes

   \retval   Errc::success   Data send
   \retval   Errc::noact     No connection
   \retval   Errc::com       Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_SendAndReceiveFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                                             const std::vector<uint8_t> & orc_Request,
                                                                             std::vector<uint8_t> & orc_Response)
{
   std::error_code c_Retval = Errc::success;

   if (opc_Dispatcher != NULL)
   {
      uint32_t u32_Handle;
      if (opc_Dispatcher->InitTcp(mhc_CurrentSettings.au8_CryptoAgentIp, u32_Handle,
                                  mhc_CurrentSettings.u16_CryptoAgentPort) == Errc::success)
      {
         if (opc_Dispatcher->ReConnectTcp(u32_Handle) == Errc::success)
         {
            if (opc_Dispatcher->SendTcp(u32_Handle, orc_Request) == Errc::success)
            {
               c_Retval = mh_WaitForResponseFromCryptoAgent(opc_Dispatcher, orc_Response, u32_Handle);
            }
            else
            {
               c_Retval = Errc::com;
               osc_write_log_warning(mhc_LOG_HEADING, "Failed to send to Crypto Agent");
            }
         }
         else
         {
            c_Retval = Errc::noact;
            osc_write_log_info(mhc_LOG_HEADING, "Failed to connect to Crypto Agent");
         }
      }
      else
      {
         c_Retval = Errc::noact;
         osc_write_log_warning(mhc_LOG_HEADING, "InitTcp failed");
      }
   }
   else
   {
      c_Retval = Errc::noact;
      osc_write_log_warning(mhc_LOG_HEADING, "dispatcher invalid");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Wait for response from crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher
   \param[in,out]  orc_Response     Response
   \param[in]      ou32_Handle      Handle

   \return
   STW error codes

   \retval   Errc::success   Data send
   \retval   Errc::noact     No connection
   \retval   Errc::com       Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_WaitForResponseFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                                              std::vector<uint8_t> & orc_Response,
                                                                              const uint32_t ou32_Handle)
{
   std::error_code c_Retval = opc_Dispatcher->ReadTcp(ou32_Handle, orc_Response);

   // Wait for response
   for (uint32_t u32_CurTime = 0UL; (u32_CurTime < mhu32_TIMEOUT_REQUEST_100MS) && (c_Retval == Errc::noact);
        ++u32_CurTime)
   {
      TglSleep(100);
      c_Retval = opc_Dispatcher->ReadTcp(ou32_Handle, orc_Response);
   }
   if (c_Retval == Errc::success)
   {
      //Close done by delete (caller)
   }
   else
   {
      c_Retval = Errc::com;
      osc_write_log_warning(mhc_LOG_HEADING, "Failed to receive from Crypto Agent");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get ping data only

   \return
   Ping data only
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_OscCryptoAgentAccessUtil::mh_GetPingDataOnly()
{
   std::vector<uint8_t> c_Data;
   c_Data.push_back(0U);
   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get ping request

   \return
   Ping request
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_OscCryptoAgentAccessUtil::mh_GetPingRequest()
{
   std::vector<uint8_t> c_Request = mh_GetPingDataOnly();
   mh_PrependDataSize(c_Request);
   return c_Request;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get shutdown data only

   \return
   Shutdown data only
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_OscCryptoAgentAccessUtil::mh_GetShutdownDataOnly()
{
   std::vector<uint8_t> c_Data;
   c_Data.push_back(1U);
   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get rsa signature by serial number request

   \param[in]  orc_SerialNumber                          Serial number
   \param[in]  ou8_SecurityLevel                         Security level
   \param[in]  orc_ServerChallengeValue                  Server challenge value
   \param[in]  orc_LastLoadedSystemDefinitionFilePath    Last loaded system definition file path
   \param[in]  orc_Node                                  Node
   \param[in]  ou8_NodeIdentifier                        Node identifier
   \param[in]  orc_SerialNumberExtended                  Serial number extended
   \param[in]  ou8_SerialNumberManufacturerFormat        Serial number manufacturer format

   \return
   Rsa signature by serial number request
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_OscCryptoAgentAccessUtil::mh_GetRsaSignatureBySerialNumberRequest(
   const std::vector<uint8_t> & orc_SerialNumber, const uint8_t ou8_SecurityLevel,
   const std::vector<uint8_t> & orc_ServerChallengeValue,
   const std::string & orc_LastLoadedSystemDefinitionFilePath, const C_OscNode & orc_Node,
   const uint8_t ou8_NodeIdentifier, const std::string & orc_SerialNumberExtended,
   const uint8_t ou8_SerialNumberManufacturerFormat)
{
   const std::string c_FileName = TglExtractFileName(orc_LastLoadedSystemDefinitionFilePath);
   const std::string c_FileExtension = TglExtractFileExtension(orc_LastLoadedSystemDefinitionFilePath);

   std::string c_ProjectName = c_FileName;
   if (c_FileExtension.empty() == false)
   {
      const std::size_t u32_ExtPos = c_ProjectName.rfind(c_FileExtension);
      if (u32_ExtPos != std::string::npos)
      {
         c_ProjectName = c_ProjectName.substr(0U, u32_ExtPos);
      }
   }

   std::vector<uint8_t> c_Data;
   //service
   c_Data.push_back(2U);
   //request version
   c_Data.push_back(1U);
   //auth algo RSA 1024
   c_Data.push_back(0U);
   //security access level
   c_Data.push_back(ou8_SecurityLevel);
   //project name
   mh_AppendStringAndSize(c_Data, c_ProjectName);
   //device type
   if (orc_Node.pc_DeviceDefinition != NULL)
   {
      mh_AppendStringAndSize(c_Data, orc_Node.pc_DeviceDefinition->c_DeviceName);
   }
   else
   {
      mh_AppendStringAndSize(c_Data, "");
   }
   //node name
   mh_AppendStringAndSize(c_Data, orc_Node.c_Properties.c_Name);
   //node id
   c_Data.push_back(ou8_NodeIdentifier);
   //manufacturer id
   c_Data.push_back(ou8_SerialNumberManufacturerFormat);
   //ECU serial number
   mh_AppendStringAndSize(c_Data, orc_SerialNumberExtended);
   //Server certificate serial number
   mh_AppendDataAndSize(c_Data, orc_SerialNumber);
   //Server challenge value
   mh_AppendDataAndSize(c_Data, orc_ServerChallengeValue);
   //Size of request
   mh_PrependDataSize(c_Data);
   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Append string and size

   \param[in,out]  orc_Request   Request
   \param[in]      orc_Data      Data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_AppendStringAndSize(std::vector<uint8_t> & orc_Request,
                                                        const std::string & orc_Data)
{
   mh_AppendSize(orc_Request, static_cast<uint32_t>(orc_Data.size()));
   for (uint32_t u32_It = 0UL; u32_It < orc_Data.size(); ++u32_It)
   {
      orc_Request.push_back(static_cast<uint8_t>(orc_Data[u32_It]));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Append data and size

   \param[in,out]  orc_Request   Request
   \param[in]      orc_Data      Data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_AppendDataAndSize(std::vector<uint8_t> & orc_Request,
                                                      const std::vector<uint8_t> & orc_Data)
{
   mh_AppendSize(orc_Request, static_cast<uint32_t>(orc_Data.size()));
   for (uint32_t u32_It = 0UL; u32_It < orc_Data.size(); ++u32_It)
   {
      orc_Request.push_back(orc_Data[u32_It]);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Append size

   \param[in,out]  orc_Request   Request
   \param[in]      ou32_Size     Size
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_AppendSize(std::vector<uint8_t> & orc_Request, const uint32_t ou32_Size)
{
   orc_Request.push_back(static_cast<uint8_t>(ou32_Size >> 8U));
   orc_Request.push_back(static_cast<uint8_t>(ou32_Size & 0xFFU));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get shutdown request

   \return
   Shutdown request
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> C_OscCryptoAgentAccessUtil::mh_GetShutdownRequest()
{
   std::vector<uint8_t> c_Request = mh_GetShutdownDataOnly();
   mh_PrependDataSize(c_Request);
   return c_Request;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Prepend data size

   \param[in,out]  orc_Data   Data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_PrependDataSize(std::vector<uint8_t> & orc_Data)
{
   std::vector<uint8_t> c_ServiceSize;
   const uint32_t u32_ServiceSize = static_cast<uint32_t>(orc_Data.size());
   c_ServiceSize.resize(4);
   c_ServiceSize[0] = static_cast<uint8_t>(u32_ServiceSize >> 24U);
   c_ServiceSize[1] = static_cast<uint8_t>(u32_ServiceSize >> 16U);
   c_ServiceSize[2] = static_cast<uint8_t>(u32_ServiceSize >> 8U);
   c_ServiceSize[3] = static_cast<uint8_t>(u32_ServiceSize & 0xFFU);

   orc_Data.insert(orc_Data.begin(), c_ServiceSize.begin(), c_ServiceSize.end());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check response

   \param[in]   orc_Response        Response
   \param[out]  opu8_ResponseValue  Response value

   \return
   STW error codes

   \retval   Errc::success   Response success
   \retval   Errc::com       Response failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_CheckResponse(const std::vector<uint8_t> & orc_Response,
                                                              uint8_t * const opu8_ResponseValue)
{
   if (opu8_ResponseValue != NULL)
   {
      *opu8_ResponseValue = mhu8_REQUEST_SUCCESS;
   }

   std::error_code c_Retval;
   if (orc_Response.size() >= 6UL)
   {
      if (opu8_ResponseValue != NULL)
      {
         *opu8_ResponseValue = orc_Response[5UL];
      }
      if (orc_Response[5UL] == mhu8_REQUEST_SUCCESS)
      {
         c_Retval = Errc::success;
      }
      else
      {
         c_Retval = Errc::com;
      }
   }
   else
   {
      c_Retval = Errc::com;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle ping response

   \param[in]  orc_Response   Response

   \return
   STW error codes

   \retval   Errc::success   Response success
   \retval   Errc::com       Response failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_HandlePingResponse(const std::vector<uint8_t> & orc_Response)
{
   std::error_code c_Retval = mh_CheckResponse(orc_Response);

   if (c_Retval == Errc::success)
   {
      if (orc_Response.size() >= 8UL)
      {
         if ((orc_Response[6UL] == 0x00U) &&
             (orc_Response[7UL] == 0x01U))
         {
            uint32_t u32_PosVersion = 0U;
            c_Retval = Errc::success;
            // Ignore index 8,9 (reserved)
            if (mh_ReportString(orc_Response, 10UL, 2UL, "name", &u32_PosVersion) == Errc::success)
            {
               mh_ReportString(orc_Response, u32_PosVersion, 2UL, "version");
            }
         }
         else
         {
            c_Retval = Errc::com;
            osc_write_log_warning(mhc_LOG_HEADING, "Ping returned unknown protocol version");
         }
      }
      else
      {
         c_Retval = Errc::com;
         osc_write_log_warning(mhc_LOG_HEADING, "Ping missing protocol version");
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle rsa signature response

   \param[in]      orc_Response        Response
   \param[in,out]  orc_RsaSignature    Rsa signature

   \return
   STW error codes

   \retval   Errc::success   Rsa signature valid
   \retval   Errc::com       Response failure
   \retval   Errc::range     Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_HandleRsaSignatureResponse(const std::vector<uint8_t> & orc_Response,
                                                                           std::vector<uint8_t> & orc_RsaSignature)
{
   uint8_t u8_ResponseValue;
   std::error_code c_Retval = mh_CheckResponse(orc_Response, &u8_ResponseValue);

   if (c_Retval == Errc::success)
   {
      uint32_t u32_Size = 0UL;
      c_Retval = mh_ReadSize(orc_Response, 6UL, 2UL, u32_Size);
      if (c_Retval == Errc::success)
      {
         const uint32_t u32_OFFSET = 8UL;
         orc_RsaSignature.reserve(u32_Size);
         for (uint32_t u32_It = 0UL; u32_It < u32_Size; ++u32_It)
         {
            if ((static_cast<std::vector<uint8_t>::size_type>(u32_OFFSET) +
                 static_cast<std::vector<uint8_t>::size_type>(u32_It)) < orc_Response.size())
            {
               orc_RsaSignature.push_back(orc_Response[static_cast<std::vector<uint8_t>::size_type>(u32_OFFSET) +
                                                         static_cast<std::vector<uint8_t>::size_type>(u32_It)]);
            }
            else
            {
               c_Retval = Errc::range;
               break;
            }
         }
         osc_write_log_info(mhc_LOG_HEADING, "Received solution to authentication challenge from Crypto Agent");
      }
   }
   else
   {
      if (u8_ResponseValue == mhu8_REQUEST_FAILED)
      {
         mh_ReportString(orc_Response, 6UL, 2UL, "error details");
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read size

   \param[in]   orc_Response        Response
   \param[in]   ou32_StartPosition  Start position
   \param[in]   ou32_NumBytes       Num bytes
   \param[out]  oru32_Result        Result

   \return
   STW error codes

   \retval   Errc::success   Size read
   \retval   Errc::range     Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_ReadSize(const std::vector<uint8_t> & orc_Response,
                                                        const uint32_t ou32_StartPosition, const uint32_t ou32_NumBytes,
                                                        uint32_t & oru32_Result)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Response.size() >=
       (static_cast<std::vector<uint8_t>::size_type>(ou32_StartPosition) +
        static_cast<std::vector<uint8_t>::size_type>(ou32_NumBytes)))
   {
      oru32_Result = 0UL;
      for (uint32_t u32_It = ou32_StartPosition; u32_It < (ou32_StartPosition + ou32_NumBytes); ++u32_It)
      {
         oru32_Result += static_cast<uint32_t>(orc_Response[u32_It]) <<
                         (8U * ((ou32_StartPosition + ou32_NumBytes) - (u32_It + 1UL)));
      }
   }
   else
   {
      c_Retval = Errc::range;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read string

   \param[in]      orc_Response        Response
   \param[in]      ou32_StartPosition  Start position
   \param[in]      ou32_Length         Length
   \param[in,out]  orc_Result          Result

   \return
   STW error codes

   \retval   Errc::success   String read
   \retval   Errc::range     Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_ReadString(const std::vector<uint8_t> & orc_Response,
                                                          const uint32_t ou32_StartPosition,
                                                          const uint32_t ou32_Length, std::string & orc_Result)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Response.size() >=
       (static_cast<std::vector<uint8_t>::size_type>(ou32_StartPosition) +
        static_cast<std::vector<uint8_t>::size_type>(ou32_Length)))
   {
      orc_Result = "";
      for (uint32_t u32_It = ou32_StartPosition; u32_It < (ou32_StartPosition + ou32_Length); ++u32_It)
      {
         orc_Result += static_cast<char>(orc_Response[u32_It]);
      }
   }
   else
   {
      c_Retval = Errc::range;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report string

   \param[in]   orc_Response        Response
   \param[in]   ou32_StartPosition  Start position
   \param[in]   ou32_SizeLength     Size length
   \param[in]   orc_Category        Category
   \param[out]  opu32_EndPos        End pos

   \return
   STW error codes

   \retval   Errc::success   String reported
   \retval   Errc::range     Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCryptoAgentAccessUtil::mh_ReportString(const std::vector<uint8_t> & orc_Response,
                                                            const uint32_t ou32_StartPosition,
                                                            const uint32_t ou32_SizeLength,
                                                            const std::string & orc_Category,
                                                            uint32_t * const opu32_EndPos)
{
   uint32_t u32_SizeName = 0UL;
   std::error_code c_Retval = mh_ReadSize(orc_Response, ou32_StartPosition, ou32_SizeLength, u32_SizeName);

   if (c_Retval == Errc::success)
   {
      std::string c_Name;
      c_Retval = mh_ReadString(orc_Response, ou32_StartPosition + ou32_SizeLength, u32_SizeName, c_Name);
      if (c_Retval == Errc::success)
      {
         const std::string c_Message = "Crypto Agent " + orc_Category + ": \"" + c_Name + "\"";
         if (opu32_EndPos != NULL)
         {
            *opu32_EndPos = ou32_StartPosition + ou32_SizeLength + u32_SizeName;
         }
         osc_write_log_info(mhc_LOG_HEADING, c_Message);
      }
   }
   return c_Retval;
}
