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

#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "TglTasks.hpp"
#include "stwerrors.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscIpDispatcherImpl.hpp"
#include "C_OscCryptoAgentAccessUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const stw::scl::C_SclString C_OscCryptoAgentAccessUtil::mhc_LOG_HEADING = "Crypto Agent access";
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
      const int32_t s32_Retval = mh_PingCryptoAgent(&c_Dispatcher);
      if (s32_Retval == C_NOACT)
      {
         mh_StartCryptoAgent();
      }
      else if (s32_Retval == C_NO_ERR)
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
      const int32_t s32_Retval = mh_ShutdownCryptoAgent(&c_Dispatcher);
      if (s32_Retval == C_NO_ERR)
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

   \retval   C_NO_ERR   Rsa signature valid
   \retval   C_NOACT    No connection
   \retval   C_COM      Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::h_GetRsaSignatureBySerialNumber(const std::vector<uint8_t> & orc_SerialNumber,
                                                                    const uint8_t ou8_SecurityLevel,
                                                                    const std::vector<uint8_t> & orc_ServerChallengeValue, std::vector<uint8_t> & orc_RsaSignature, const stw::scl::C_SclString &  orc_LastLoadedSystemDefinitionFilePath, const C_OscNode & orc_Node, const uint8_t ou8_NodeIdentifier, const stw::scl::C_SclString & orc_SerialNumberExtended,
                                                                    const uint8_t ou8_SerialNumberManufacturerFormat)
{
   C_OscIpDispatcherImpl c_Dispatcher;
   const std::vector<uint8_t> c_Request = mh_GetRsaSignatureBySerialNumberRequest(orc_SerialNumber, ou8_SecurityLevel,
                                                                                  orc_ServerChallengeValue,
                                                                                  orc_LastLoadedSystemDefinitionFilePath, orc_Node,
                                                                                  ou8_NodeIdentifier, orc_SerialNumberExtended,
                                                                                  ou8_SerialNumberManufacturerFormat);

   std::vector<uint8_t> c_Response;
   int32_t s32_Retval;
   c_Response.resize(136);
   s32_Retval = mh_SendAndReceiveFromCryptoAgent(&c_Dispatcher, c_Request, c_Response);
   if (s32_Retval == C_NO_ERR)
   {
      s32_Retval = mh_HandleRsaSignatureResponse(c_Response, orc_RsaSignature);
   }

   return s32_Retval;
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

   \retval   C_NO_ERR   Data send
   \retval   C_NOACT    No connection
   \retval   C_COM      Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_PingCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher)
{
   const std::vector<uint8_t> c_Request = mh_GetPingRequest();

   std::vector<uint8_t> c_Response;
   int32_t s32_Retval;
   c_Response.resize(37);
   s32_Retval = mh_SendAndReceiveFromCryptoAgent(opc_Dispatcher, c_Request, c_Response);
   if (s32_Retval == C_NO_ERR)
   {
      s32_Retval = mh_HandlePingResponse(c_Response);
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Shutdown crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher

   \return
   STW error codes

   \retval   C_NO_ERR   Data send
   \retval   C_NOACT    No connection
   \retval   C_COM      Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_ShutdownCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher)
{
   const std::vector<uint8_t> c_Request = mh_GetShutdownRequest();

   std::vector<uint8_t> c_Response;
   int32_t s32_Retval;
   c_Response.resize(6);
   s32_Retval = mh_SendAndReceiveFromCryptoAgent(opc_Dispatcher, c_Request, c_Response);
   if (s32_Retval == C_NO_ERR)
   {
      s32_Retval = mh_CheckResponse(c_Response);
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Start crypto agent
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentAccessUtil::mh_StartCryptoAgent(void)
{
   if (TglFileExists(mhc_CurrentSettings.c_CryptoAgentExecutablePath))
   {
      const stw::scl::C_SclString c_Parameters = "--configfile \"" + mhc_CurrentSettings.c_CryptoAgentConfigFilePath +
                                                 "\"";
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
      const stw::scl::C_SclString c_Warning = "could not find binary at \"" +
                                              mhc_CurrentSettings.c_CryptoAgentExecutablePath + "\"";
      osc_write_log_warning(mhc_LOG_HEADING, c_Warning.c_str());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Send and receive from crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher
   \param[in]      orc_Request      Request
   \param[in,out]  orc_Response     Response

   \return
   STW error codes

   \retval   C_NO_ERR   Data send
   \retval   C_NOACT    No connection
   \retval   C_COM      Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_SendAndReceiveFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                                     const std::vector<uint8_t> & orc_Request,
                                                                     std::vector<uint8_t> & orc_Response)
{
   int32_t s32_Retval = C_NO_ERR;

   if (opc_Dispatcher != NULL)
   {
      uint32_t u32_Handle;
      if (opc_Dispatcher->InitTcp(mhc_CurrentSettings.au8_CryptoAgentIp, u32_Handle,
                                  mhc_CurrentSettings.u16_CryptoAgentPort) == C_NO_ERR)
      {
         if (opc_Dispatcher->ReConnectTcp(u32_Handle) == C_NO_ERR)
         {
            if (opc_Dispatcher->SendTcp(u32_Handle, orc_Request) == C_NO_ERR)
            {
               s32_Retval = mh_WaitForResponseFromCryptoAgent(opc_Dispatcher, orc_Response, u32_Handle);
            }
            else
            {
               s32_Retval = C_COM;
               osc_write_log_warning(mhc_LOG_HEADING, "Failed to send to Crypto Agent");
            }
         }
         else
         {
            s32_Retval = C_NOACT;
            osc_write_log_info(mhc_LOG_HEADING, "Failed to connect to Crypto Agent");
         }
      }
      else
      {
         s32_Retval = C_NOACT;
         osc_write_log_warning(mhc_LOG_HEADING, "InitTcp failed");
      }
   }
   else
   {
      s32_Retval = C_NOACT;
      osc_write_log_warning(mhc_LOG_HEADING, "dispatcher invalid");
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Wait for response from crypto agent

   \param[in,out]  opc_Dispatcher   Dispatcher
   \param[in,out]  orc_Response     Response
   \param[in]      ou32_Handle      Handle

   \return
   STW error codes

   \retval   C_NO_ERR   Data send
   \retval   C_NOACT    No connection
   \retval   C_COM      Error during communication
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_WaitForResponseFromCryptoAgent(C_OscIpDispatcher * const opc_Dispatcher,
                                                                      std::vector<uint8_t> & orc_Response,
                                                                      const uint32_t ou32_Handle)
{
   int32_t s32_Retval = opc_Dispatcher->ReadTcp(ou32_Handle, orc_Response);

   // Wait for response
   for (uint32_t u32_CurTime = 0UL; (u32_CurTime < mhu32_TIMEOUT_REQUEST_100MS) && (s32_Retval == C_NOACT);
        ++u32_CurTime)
   {
      TglSleep(100);
      s32_Retval = opc_Dispatcher->ReadTcp(ou32_Handle, orc_Response);
   }
   if (s32_Retval == C_NO_ERR)
   {
      //Close done by delete (caller)
   }
   else
   {
      s32_Retval = C_COM;
      osc_write_log_warning(mhc_LOG_HEADING, "Failed to receive from Crypto Agent");
   }
   return s32_Retval;
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
   const stw::scl::C_SclString &  orc_LastLoadedSystemDefinitionFilePath, const C_OscNode & orc_Node,
   const uint8_t ou8_NodeIdentifier, const stw::scl::C_SclString & orc_SerialNumberExtended,
   const uint8_t ou8_SerialNumberManufacturerFormat)
{
   const stw::scl::C_SclString c_FileName = TglExtractFileName(orc_LastLoadedSystemDefinitionFilePath);
   const stw::scl::C_SclString c_FileExtension = TglExtractFileExtension(orc_LastLoadedSystemDefinitionFilePath);

   stw::scl::C_SclString c_ProjectName = c_FileName;
   const uint32_t u32_ExtPos = c_ProjectName.LastPos(c_FileExtension);

   if (u32_ExtPos != 0U)
   {
      c_ProjectName = c_ProjectName.SubString(1U, u32_ExtPos - 1UL);
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
   mh_AppendStringAndSize(c_Data, c_ProjectName.c_str());
   //device type
   if (orc_Node.pc_DeviceDefinition != NULL)
   {
      mh_AppendStringAndSize(c_Data, orc_Node.pc_DeviceDefinition->c_DeviceName.c_str());
   }
   else
   {
      mh_AppendStringAndSize(c_Data, "");
   }
   //node name
   mh_AppendStringAndSize(c_Data, orc_Node.c_Properties.c_Name.c_str());
   //node id
   c_Data.push_back(ou8_NodeIdentifier);
   //manufacturer id
   c_Data.push_back(ou8_SerialNumberManufacturerFormat);
   //ECU serial number
   mh_AppendStringAndSize(c_Data, orc_SerialNumberExtended.c_str());
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
      orc_Request.push_back(orc_Data[u32_It]);
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

   \retval   C_NO_ERR   Response success
   \retval   C_COM      Response failure
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_CheckResponse(const std::vector<uint8_t> & orc_Response,
                                                     uint8_t * const opu8_ResponseValue)
{
   int32_t s32_Retval;

   if (opu8_ResponseValue != NULL)
   {
      *opu8_ResponseValue = mhu8_REQUEST_SUCCESS;
   }

   if (orc_Response.size() >= 6UL)
   {
      if (opu8_ResponseValue != NULL)
      {
         *opu8_ResponseValue = orc_Response[5UL];
      }
      if (orc_Response[5UL] == mhu8_REQUEST_SUCCESS)
      {
         s32_Retval = C_NO_ERR;
      }
      else
      {
         s32_Retval = C_COM;
      }
   }
   else
   {
      s32_Retval = C_COM;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle ping response

   \param[in]  orc_Response   Response

   \return
   STW error codes

   \retval   C_NO_ERR   Response success
   \retval   C_COM      Response failure
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_HandlePingResponse(const std::vector<uint8_t> & orc_Response)
{
   int32_t s32_Retval = mh_CheckResponse(orc_Response);

   if (s32_Retval == C_NO_ERR)
   {
      if (orc_Response.size() >= 8UL)
      {
         if ((orc_Response[6UL] == 0x00U) &&
             (orc_Response[7UL] == 0x01U))
         {
            uint32_t u32_PosVersion = 0U;
            s32_Retval = C_NO_ERR;
            // Ignore index 8,9 (reserved)
            if (mh_ReportString(orc_Response, 10UL, 2UL, "name", &u32_PosVersion) == C_NO_ERR)
            {
               mh_ReportString(orc_Response, u32_PosVersion, 2UL, "version");
            }
         }
         else
         {
            s32_Retval = C_COM;
            osc_write_log_warning(mhc_LOG_HEADING, "Ping returned unknown protocol version");
         }
      }
      else
      {
         s32_Retval = C_COM;
         osc_write_log_warning(mhc_LOG_HEADING, "Ping missing protocol version");
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle rsa signature response

   \param[in]      orc_Response        Response
   \param[in,out]  orc_RsaSignature    Rsa signature

   \return
   STW error codes

   \retval   C_NO_ERR   Rsa signature valid
   \retval   C_COM      Response failure
   \retval   C_RANGE    Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_HandleRsaSignatureResponse(const std::vector<uint8_t> & orc_Response,
                                                                  std::vector<uint8_t> & orc_RsaSignature)
{
   uint8_t u8_ResponseValue;
   int32_t s32_Retval = mh_CheckResponse(orc_Response, &u8_ResponseValue);

   if (s32_Retval == C_NO_ERR)
   {
      uint32_t u32_Size = 0UL;
      s32_Retval = mh_ReadSize(orc_Response, 6UL, 2UL, u32_Size);
      if (s32_Retval == C_NO_ERR)
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
               s32_Retval = C_RANGE;
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

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read size

   \param[in]   orc_Response        Response
   \param[in]   ou32_StartPosition  Start position
   \param[in]   ou32_NumBytes       Num bytes
   \param[out]  oru32_Result        Result

   \return
   STW error codes

   \retval   C_NO_ERR   Size read
   \retval   C_RANGE    Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_ReadSize(const std::vector<uint8_t> & orc_Response,
                                                const uint32_t ou32_StartPosition, const uint32_t ou32_NumBytes,
                                                uint32_t & oru32_Result)
{
   int32_t s32_Retval = C_NO_ERR;

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
      s32_Retval = C_RANGE;
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read string

   \param[in]      orc_Response        Response
   \param[in]      ou32_StartPosition  Start position
   \param[in]      ou32_Length         Length
   \param[in,out]  orc_Result          Result

   \return
   STW error codes

   \retval   C_NO_ERR   String read
   \retval   C_RANGE    Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_ReadString(const std::vector<uint8_t> & orc_Response,
                                                  const uint32_t ou32_StartPosition, const uint32_t ou32_Length,
                                                  stw::scl::C_SclString & orc_Result)
{
   int32_t s32_Retval = C_NO_ERR;

   if (orc_Response.size() >=
       (static_cast<std::vector<uint8_t>::size_type>(ou32_StartPosition) +
        static_cast<std::vector<uint8_t>::size_type>(ou32_Length)))
   {
      orc_Result = "";
      for (uint32_t u32_It = ou32_StartPosition; u32_It < (ou32_StartPosition + ou32_Length); ++u32_It)
      {
         orc_Result += static_cast<char_t>(orc_Response[u32_It]);
      }
   }
   else
   {
      s32_Retval = C_RANGE;
   }

   return s32_Retval;
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

   \retval   C_NO_ERR   String reported
   \retval   C_RANGE    Response too short
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCryptoAgentAccessUtil::mh_ReportString(const std::vector<uint8_t> & orc_Response,
                                                    const uint32_t ou32_StartPosition, const uint32_t ou32_SizeLength,
                                                    const stw::scl::C_SclString & orc_Category,
                                                    uint32_t * const opu32_EndPos)
{
   uint32_t u32_SizeName = 0UL;
   int32_t s32_Retval = mh_ReadSize(orc_Response, ou32_StartPosition, ou32_SizeLength, u32_SizeName);

   if (s32_Retval == C_NO_ERR)
   {
      stw::scl::C_SclString c_Name;
      s32_Retval = mh_ReadString(orc_Response, ou32_StartPosition + ou32_SizeLength, u32_SizeName, c_Name);
      if (s32_Retval == C_NO_ERR)
      {
         const stw::scl::C_SclString c_Message = "Crypto Agent " + orc_Category + ": \"" + c_Name + "\"";
         if (opu32_EndPos != NULL)
         {
            *opu32_EndPos = ou32_StartPosition + ou32_SizeLength + u32_SizeName;
         }
         osc_write_log_info(mhc_LOG_HEADING, c_Message.c_str());
      }
   }
   return s32_Retval;
}
