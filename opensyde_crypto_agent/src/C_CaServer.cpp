//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class

   For details see documentation in .hpp file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdio>
#include <string>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "version_config.hpp"

#include "TglFile.hpp"
#include "C_CaServer.hpp"
#include "C_SclString.hpp"
#include "C_OscLoggingHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::osy_crypto_agent;
using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
const uint8_t C_CaServer::mhu8_SERVICE_RESULT_NO_ERROR;
const uint8_t C_CaServer::mhu8_SERVICE_RESULT_SERVCE_NOT_SUPPORTED;
const uint8_t C_CaServer::mhu8_SERVICE_RESULT_MALFORMED_REQUEST;
const uint8_t C_CaServer::mhu8_SERVICE_RESULT_FAILED;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

bool C_CaServer::mh_ReadUint8(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos, uint8_t & oru8_Value)
{
   bool q_Return = false;

   if (oru32_Pos < orc_Data.size())
   {
      oru8_Value = orc_Data[oru32_Pos];
      ++oru32_Pos;
      q_Return = true;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_CaServer::mh_ReadUint16BigEndian(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos,
                                        uint16_t & oru16_Value)
{
   bool q_Return = false;

   if ((oru32_Pos + 1) < static_cast<uint32_t>(orc_Data.size()))
   {
      oru16_Value = static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[oru32_Pos]) << 8U) +
                    (orc_Data[static_cast<size_t>(oru32_Pos) + 1]);
      oru32_Pos += 2U;
      q_Return = true;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_CaServer::mh_ReadByteVector(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos,
                                   std::vector<uint8_t> & orc_Value, const uint16_t ou16_MaxSize)
{
   bool q_Return;
   uint16_t u16_Length = 0U;

   q_Return = C_CaServer::mh_ReadUint16BigEndian(orc_Data, oru32_Pos, u16_Length);
   if ((q_Return == true) && (u16_Length <= ou16_MaxSize))
   {
      const uint32_t u32_NewPos = oru32_Pos + u16_Length;
      if ((u32_NewPos >= oru32_Pos) && (u32_NewPos <= orc_Data.size()))
      {
         orc_Value.assign(orc_Data.begin() + oru32_Pos, orc_Data.begin() + u32_NewPos);
         oru32_Pos = u32_NewPos;
      }
      else
      {
         q_Return = false;
      }
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_CaServer::mh_ReadString(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos, std::string & orc_Value,
                               const uint16_t ou16_MaxSize)
{
   std::vector<uint8_t> c_Data;
   bool q_Return;

   q_Return = C_CaServer::mh_ReadByteVector(orc_Data, oru32_Pos, c_Data, ou16_MaxSize);
   if (q_Return == true)
   {
      orc_Value.assign(c_Data.begin(), c_Data.end());
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------

void C_CaServer::mh_AppendString(const std::string & orc_Text, std::vector<uint8_t> & orc_Data)
{
   std::vector<uint8_t> c_Data;
   c_Data.assign(orc_Text.begin(), orc_Text.end());

   mh_AppendByteVector(c_Data, orc_Data);
}
//----------------------------------------------------------------------------------------------------------------------

void C_CaServer::mh_AppendByteVector(const std::vector<uint8_t> & orc_Value, std::vector<uint8_t> & orc_Data)
{
   tgl_assert(orc_Value.size() <= 0xFFFF);
   mh_AppendUint16BigEndian(static_cast<uint16_t>(orc_Value.size()), orc_Data);
   orc_Data.insert(orc_Data.end(), orc_Value.begin(), orc_Value.end());
}

//----------------------------------------------------------------------------------------------------------------------

void C_CaServer::mh_AppendUint16BigEndian(const uint16_t ou16_Value, std::vector<uint8_t> & orc_Data)
{
   orc_Data.push_back(static_cast<uint8_t>(ou16_Value >> 8U));
   orc_Data.push_back(static_cast<uint8_t>(ou16_Value));
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_CaServer::C_CaServer()
{
   C_SclString c_Version;
   const C_SclString c_ExePath = stw::tgl::TglGetExePath();

   c_Version.StringPrintFormatted("V%d.%02dr%d", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_RELEASE);
   //just get name without possibly existing extension
   mc_ApplicationName = stw::tgl::TglChangeFileExtension(stw::tgl::TglExtractFileName(c_ExePath), "").c_str();
   mc_ApplicationVersion = c_Version.c_str();
}

//----------------------------------------------------------------------------------------------------------------------

C_CaServer::~C_CaServer()
{
}

//----------------------------------------------------------------------------------------------------------------------

C_CaServer::C_InputParametersSolveAuthChallenge::C_InputParametersSolveAuthChallenge() :
   u8_RequestVersion(0U),
   u8_AuthAlgorithm(0U),
   u8_SaLevel(0U),
   u8_SubNodeId(0U),
   u8_EcuSerialNumberManufacturerId(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decode request parameters from binary data

   Binary format:
   * simple bytes for bytes
   * strings and BLOBs: big endian 16bit size value; then data with that size
   ** technically for now 8bit size would suffice, but we want to be defensive for possible future expansion

   \param[in]    orc_BinaryData   received request

   \return  C_NO_ERR: everything ok
            C_RANGE: error decoding
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServer::C_InputParametersSolveAuthChallenge::FromBinary(const std::vector<uint8_t> & orc_BinaryData)
{
   int32_t s32_Result = C_RANGE;
   uint32_t u32_Pos = 1U; //payload starts with byte1; byte one contains service ID
   C_InputParametersSolveAuthChallenge c_RequestData;

   if (mh_ReadUint8(orc_BinaryData, u32_Pos, c_RequestData.u8_RequestVersion) == true)
   {
      if (c_RequestData.u8_RequestVersion == C_CaServer::mhu8_AUTH_VERSION)
      {
         if ((mh_ReadUint8(orc_BinaryData, u32_Pos, c_RequestData.u8_AuthAlgorithm) == true) &&
             (mh_ReadUint8(orc_BinaryData, u32_Pos, c_RequestData.u8_SaLevel) == true) &&
             (mh_ReadString(orc_BinaryData, u32_Pos, c_RequestData.c_ProjectName, 255) == true) &&
             (mh_ReadString(orc_BinaryData, u32_Pos, c_RequestData.c_DeviceType, 28) == true) &&
             (mh_ReadString(orc_BinaryData, u32_Pos, c_RequestData.c_NodeName, 31) == true) &&
             (mh_ReadUint8(orc_BinaryData, u32_Pos, c_RequestData.u8_SubNodeId) == true) &&
             (mh_ReadUint8(orc_BinaryData, u32_Pos, c_RequestData.u8_EcuSerialNumberManufacturerId) == true) &&
             (mh_ReadByteVector(orc_BinaryData, u32_Pos, c_RequestData.c_EcuSerialNumber, 29) == true) &&
             (mh_ReadByteVector(orc_BinaryData, u32_Pos, c_RequestData.c_ServerCertificateSerialNumber, 20) == true) &&
             (mh_ReadByteVector(orc_BinaryData, u32_Pos, c_RequestData.c_ServerChallengeValue, 8) == true) &&
             (u32_Pos == orc_BinaryData.size()))
         {
            (*this) = c_RequestData;
            s32_Result = C_NO_ERR;
         }
      }
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle incoming request and compose response

   \param[in]    orc_BinaryData   received request
   \param[out]   orc_Response     response to send
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaServer::m_HandleRequest(const std::vector<uint8_t> & orc_Request, std::vector<uint8_t> & orc_Response)
{
   orc_Response.clear();

   if (orc_Request.size() > 0)
   {
      const uint8_t u8_ServiceId = orc_Request[0]; //first byte is always service ID
      orc_Response.push_back(u8_ServiceId);
      switch (u8_ServiceId)
      {
      case mhu8_SERVICE_ID_PING:
         osc_write_log_info("CryptoAgent", "Ping request received.");
         orc_Response.push_back(mhu8_SERVICE_RESULT_NO_ERROR);
         mh_AppendUint16BigEndian(mhu16_PROTOCOL_VERSION, orc_Response);
         mh_AppendUint16BigEndian(0x0000U, orc_Response); //reserved bytes; can never hurt ...
         mh_AppendString(mc_ApplicationName, orc_Response);
         mh_AppendString(mc_ApplicationVersion, orc_Response);

         //Nothing else to do. Just to inform client that we are alive.
         break;
      case mhu8_SERVICE_ID_SHUTDOWN:
         osc_write_log_info("CryptoAgent", "Shutdown request received.");
         orc_Response.push_back(mhu8_SERVICE_RESULT_NO_ERROR);
         //let platform implementation know it should shut down:
         this->m_Shutdown();
         break;
      case mhu8_SERVICE_ID_SOLVE_AUTH_CHALLENGE:
         {
            C_InputParametersSolveAuthChallenge c_Parameters;
            int32_t s32_Return = c_Parameters.FromBinary(orc_Request);
            if (s32_Return == C_NO_ERR)
            {
               std::vector<uint8_t> c_ChallengeResponse;
               std::string c_ErrorDetails;

               std::string c_LogText = "SolveAuthChallenge request received for certificate serial number [";
               for (uint8_t u8_Byte = 0U; u8_Byte < c_Parameters.c_ServerCertificateSerialNumber.size(); u8_Byte++)
               {
                  c_LogText +=
                     stw::scl::C_SclString::IntToHex(c_Parameters.c_ServerCertificateSerialNumber[u8_Byte], 2).c_str();
                  if (u8_Byte != (c_Parameters.c_ServerCertificateSerialNumber.size() - 1))
                  {
                     c_LogText += ":";
                  }
               }
               c_LogText += "]";
               osc_write_log_info("CryptoAgent", c_LogText.c_str());

               s32_Return = this->m_ServiceHandlerSolveAuthChallenge(c_Parameters, c_ChallengeResponse, c_ErrorDetails);
               if (s32_Return == C_NO_ERR)
               {
                  //challenge solved: return value
                  orc_Response.push_back(mhu8_SERVICE_RESULT_NO_ERROR);

                  mh_AppendByteVector(c_ChallengeResponse, orc_Response);
               }
               else
               {
                  //could not resolve challenge
                  orc_Response.push_back(mhu8_SERVICE_RESULT_FAILED);
                  mh_AppendString(c_ErrorDetails, orc_Response);
               }
            }
            else
            {
               orc_Response.push_back(mhu8_SERVICE_RESULT_MALFORMED_REQUEST);
               mh_AppendString("SolveAuthChallenge: Malformed request", orc_Response);
            }
            break;
         }
      default:
         orc_Response.push_back(mhu8_SERVICE_RESULT_SERVCE_NOT_SUPPORTED);
         mh_AppendString("Unknown service", orc_Response);
         break;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Run server

   Start server operation.
   * install ctrl+c handler for handling process termination
   * initialize server socket
   * start listening in thread
   * stop listening and shutdown if application is terminated

   \param[in]   ou16_ServerPort    port number to run TCP server on
   \param[in]   orau8_BindAddress  bind address for server

   \return C_NO_ERR  no problems
           else      failed to execute
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServer::Execute(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4])
{
   return this->m_ExecuteListener(ou16_ServerPort, orau8_BindAddress);
}
