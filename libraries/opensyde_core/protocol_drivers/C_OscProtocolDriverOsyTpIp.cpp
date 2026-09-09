//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol IP transport-protocol

   For details cf. documentation in .h file.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include "C_SclStringUtil.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <system_error>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "TglTime.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscProtocolSerialNumber.hpp"
#include "C_OscProtocolDriverOsyTpIp.hpp"
#include "C_OscIpDispatcher.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Helper -------------------------------------------------------------------------------------------------------- */
namespace
{
   template <typename T>
   std::string mh_IntToHex(const T orc_Val, const uint32_t ou32_Digits)
   {
      std::stringstream c_Stream;
      c_Stream << std::hex << std::uppercase << std::setw(ou32_Digits) << std::setfill('0') << orc_Val;
      return c_Stream.str();
   }
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize class fields with defaults
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::C_DoIpHeader::C_DoIpHeader(void) :
   u16_PayloadType(0U),
   u32_PayloadSize(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize class fields with specified values

   \param[in]  ou16_PayloadType   DoIp payload type
   \param[in]  ou32_PayloadSize   number of bytes in payload
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::C_DoIpHeader::C_DoIpHeader(const uint16_t ou16_PayloadType,
                                                       const uint32_t ou32_PayloadSize) :
   u16_PayloadType(ou16_PayloadType),
   u32_PayloadSize(ou32_PayloadSize)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Decode DoIp request header

   Decode DoIp request header into individual elements.
   - DoIp header with payload type, payload size

   \param[in]    orc_Header         header to decode; must contain >= 8 bytes

   \return
   Errc::success   decoded; data stored in instance data
   Errc::range     not enough data
   Errc::config    incorrect version
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::C_DoIpHeader::DecodeHeader(const std::vector<uint8_t> & orc_Header)
{
   std::error_code c_Return = Errc::success;

   if (orc_Header.size() < hu8_DOIP_HEADER_SIZE)
   {
      c_Return = Errc::range;
   }
   else if ((orc_Header[0] != 0x02U) || (orc_Header[1] != 0xFDU))
   {
      c_Return = Errc::config;
   }
   else
   {
      //this is the one we know how to handle ...
      u16_PayloadType = static_cast<uint16_t>(static_cast<uint16_t>(orc_Header[2]) << 8U) +
                        static_cast<uint16_t>(orc_Header[3]);
      u32_PayloadSize = (static_cast<uint32_t>(orc_Header[4]) << 24U) +
                        (static_cast<uint32_t>(orc_Header[5]) << 16U) +
                        (static_cast<uint32_t>(orc_Header[6]) << 8U) +
                        orc_Header[7];
      c_Return = Errc::success;
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compose DoIp request header

   Set up DoIp request header:
   - DoIp header with version, payload type, payload size

   \param[out]   orc_Request        composed header (length will be set to accommodate header and payload)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::C_DoIpHeader::ComposeHeader(std::vector<uint8_t> & orc_Header) const
{
   orc_Header.resize(static_cast<uint32_t>(hu8_DOIP_HEADER_SIZE + u32_PayloadSize));
   orc_Header[0] = 0x02U;         //protocol version DoIP ISO 13400-2:2012
   orc_Header[1] = 0x02U ^ 0xFFU; //inverted protocol version
   orc_Header[2] = static_cast<uint8_t>(u16_PayloadType >> 8U);
   orc_Header[3] = static_cast<uint8_t>(u16_PayloadType & 0xFFU);
   orc_Header[4] = static_cast<uint8_t>(u32_PayloadSize >> 24U);
   orc_Header[5] = static_cast<uint8_t>(u32_PayloadSize >> 16U);
   orc_Header[6] = static_cast<uint8_t>(u32_PayloadSize >> 8U);
   orc_Header[7] = static_cast<uint8_t>(u32_PayloadSize);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize class fields with defaults
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::C_TcpRxServiceState::C_TcpRxServiceState(void) :
   e_Status(eIDLE)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Make IP communication dispatcher to use known

   The caller is responsible to connect and disconnect.
   Here we simply remember the installed dispatcher to use for communication.

   \param[in]    opc_Dispatcher         IP dispatcher to use for communication
   \param[in]    ou32_DispatcherHandle  handle to use for dispatcher TCP functions

   \return
   Errc::success   no problems
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::SetDispatcher(C_OscIpDispatcher * const opc_Dispatcher,
                                                          const uint32_t ou32_DispatcherHandle)
{
   //no need to reconfigure anything: application is responsible for connection
   this->mpc_Dispatcher = opc_Dispatcher;
   this->mu32_DispatcherHandle = ou32_DispatcherHandle;

   return make_error_code(Errc::success);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks the connection of the TCP socket

   \return
   Errc::success   is connected
   Errc::noact     is not connected
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::IsConnected(void)
{
   std::error_code c_Return = Errc::success;

   if (this->mpc_Dispatcher == nullptr)
   {
      tgl_assert(false); //misuse ...
      c_Return = Errc::noact;
   }
   else
   {
      c_Return = this->mpc_Dispatcher->IsTcpConnected(this->mu32_DispatcherHandle);
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reconnect to server

   Here: re-connect via TCP

   \return
   Errc::success   no problems
   Errc::busy      could not establish connection
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::ReConnect(void)
{
   std::error_code c_Return = Errc::success;

   if (this->mpc_Dispatcher == nullptr)
   {
      tgl_assert(false); //misuse ...
      c_Return = Errc::noact;
   }
   else
   {
      c_Return = this->mpc_Dispatcher->IsTcpConnected(this->mu32_DispatcherHandle);
      if (c_Return == Errc::noact)
      {
         c_Return = this->mpc_Dispatcher->ReConnectTcp(this->mu32_DispatcherHandle);
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Disconnect from server

   Here: disconnect via TCP

   \return
   Errc::success   no problems
   Errc::noact     could not disconnect
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::Disconnect(void)
{
   std::error_code c_Return = Errc::success;

   if (this->mpc_Dispatcher == nullptr)
   {
      tgl_assert(false); //misuse ...
      c_Return = Errc::noact;
   }
   else
   {
      c_Return = this->IsConnected();

      if (!c_Return)
      {
         c_Return = this->mpc_Dispatcher->CloseTcp(this->mu32_DispatcherHandle);
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get device information from all devices on local bus

   Send broadcast to read device information.
   Report back list of nodes that sent a response (with the sent serial number and node IDs).
   The function will wait for responses for the time configured with SetBroadcastTimeout()

   Incoming UDP responses to other services will be dumped: we are strictly handshaking here ...

   \param[out]    orc_DeviceInfos           information about all nodes that sent a response
   \param[out]    orc_DeviceExtendedInfos   information about all nodes that sent an extended response

   \return
   Errc::success   no problems; zero or more responses received; data placed in orc_DeviceInfos
   Errc::com       could not send request
   Errc::config    no dispatcher installed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::BroadcastGetDeviceInfo(
   std::vector<C_BroadcastGetDeviceInfoResults> & orc_DeviceInfos,
   std::vector<C_BroadcastGetDeviceInfoExtendedResults> & orc_DeviceExtendedInfos)
const
{
   const uint32_t u32_PAYLOAD_SIZE_STD = 37U;
   const uint32_t u32_PAYLOAD_SIZE_EXT_MIN = 36U;
   const uint32_t u32_PAYLOAD_SIZE_EXT_MAX = 64U;
   std::error_code c_Return = Errc::success;
   C_DoIpHeader c_Header(C_DoIpHeader::hu16_PAYLOAD_TYPE_OSY_GET_DEVICE_INFO_REQ, 0U);

   orc_DeviceInfos.clear();
   orc_DeviceExtendedInfos.clear();
   if (mpc_Dispatcher == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      std::vector<uint8_t> c_Request;
      c_Header.ComposeHeader(c_Request);

      if (mpc_Dispatcher->SendUdp(c_Request))
      {
         m_LogWarningWithHeader("Could not send UDP broadcast request.", TGL_UTIL_FUNC_ID);
         c_Return = Errc::com;
      }
      else
      {
         //check for responses
         const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();
         uint8_t au8_Ip[4];
         std::vector<uint8_t> c_Response;
         std::vector<C_BroadcastGetDeviceInfoResults>::iterator c_ItDeviceInfo;
         std::vector<C_BroadcastGetDeviceInfoExtendedResults>::iterator c_ItDeviceExtInfo;

         while ((stw::tgl::TglGetTickCount() - mu32_BroadcastTimeoutMs) < u32_StartTime)
         {
            const std::error_code c_ReturnLocal = mpc_Dispatcher->ReadUdp(c_Response, au8_Ip);
            if (!c_ReturnLocal)
            {
               if (((c_Response.size() >= (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + u32_PAYLOAD_SIZE_EXT_MIN)) &&
                    (c_Response.size() <= (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + u32_PAYLOAD_SIZE_EXT_MAX))))
               {
                  //header OK ?
                  const std::error_code c_HeaderResult = c_Header.DecodeHeader(c_Response);
                  if (!c_HeaderResult)
                  {
                     //sanity check: does payload size match response size ?
                     if (c_Header.u32_PayloadSize ==
                         static_cast<uint32_t>((c_Response.size() - C_DoIpHeader::hu8_DOIP_HEADER_SIZE)))
                     {
                        if ((c_Header.u16_PayloadType == C_DoIpHeader::hu16_PAYLOAD_TYPE_OSY_GET_DEVICE_INFO_RES) &&
                            (c_Response.size() == (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + u32_PAYLOAD_SIZE_STD)))
                        {
                           //looks legit for the standard response; extract payload ...
                           C_BroadcastGetDeviceInfoResults c_Info;
                           c_Info.ParseFromArray(c_Response, C_DoIpHeader::hu8_DOIP_HEADER_SIZE);
                           (void)std::memcpy(&c_Info.au8_IpAddress[0], &au8_Ip[0], 4U);
                           orc_DeviceInfos.push_back(c_Info);
                        }
                        else if ((c_Header.u16_PayloadType ==
                                  C_DoIpHeader::hu16_PAYLOAD_TYPE_OSY_GET_DEVICE_INFO_EXT_RES) &&
                                 ((c_Response.size() >=
                                   (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + u32_PAYLOAD_SIZE_EXT_MIN)) &&
                                  (c_Response.size() <=
                                   (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + u32_PAYLOAD_SIZE_EXT_MAX))))
                        {
                           //looks legit for the extended response; extract payload ...
                           C_BroadcastGetDeviceInfoExtendedResults c_Info;
                           c_Info.ParseFromArray(c_Response, C_DoIpHeader::hu8_DOIP_HEADER_SIZE);
                           (void)std::memcpy(&c_Info.au8_IpAddress[0], &au8_Ip[0], 4U);
                           orc_DeviceExtendedInfos.push_back(c_Info);
                        }
                        else
                        {
                           m_LogWarningWithHeaderAndIp(
                              "UDP response with not matching payload type value received. Ignoring.",
                              TGL_UTIL_FUNC_ID, au8_Ip);
                        }
                     }
                     else
                     {
                        m_LogWarningWithHeaderAndIp("UDP response with unexpected payload size received. Ignoring.",
                                                    TGL_UTIL_FUNC_ID, au8_Ip);
                     }
                  }
                  else if (c_HeaderResult == Errc::config)
                  {
                     m_LogWarningWithHeaderAndIp("UDP response with unexpected version received. Ignoring.",
                                                 TGL_UTIL_FUNC_ID, au8_Ip);
                  }
                  else
                  {
                     //unexpected ...
                     m_LogWarningWithHeaderAndIp("Internal error parsing DoIp header.", TGL_UTIL_FUNC_ID, au8_Ip);
                  }
               }
               else
               {
                  m_LogWarningWithHeaderAndIp("UDP response with incorrect payload size (" +
                                              std::to_string(c_Response.size()) + ") received. Ignoring.",
                                              TGL_UTIL_FUNC_ID, au8_Ip);
               }
            }
         }
         c_Return = Errc::success;

         // Remove duplicates. It is possible to get the responses multiple times, if the server and the client
         // are connected to at least two same subnets.
         std::sort(orc_DeviceInfos.begin(), orc_DeviceInfos.end());
         c_ItDeviceInfo = std::unique(orc_DeviceInfos.begin(), orc_DeviceInfos.end());
         if (c_ItDeviceInfo != orc_DeviceInfos.end())
         {
            osc_write_log_info("openSYDE IP-TP",
                               "At least one server sent multiple device information responses");
         }
         orc_DeviceInfos.erase(c_ItDeviceInfo, orc_DeviceInfos.end());

         std::sort(orc_DeviceExtendedInfos.begin(), orc_DeviceExtendedInfos.end());
         c_ItDeviceExtInfo = std::unique(orc_DeviceExtendedInfos.begin(), orc_DeviceExtendedInfos.end());
         if (c_ItDeviceExtInfo != orc_DeviceExtendedInfos.end())
         {
            osc_write_log_info("openSYDE IP-TP",
                               "At least one server sent multiple device information extended responses");
         }
         orc_DeviceExtendedInfos.erase(c_ItDeviceExtInfo, orc_DeviceExtendedInfos.end());
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set IP address of one node

   Send broadcast to change the IP address of one specific node.
   Only the node with a specified serial number is expected to send a response and change its IP address.
   The function will return as soon as it has received one response.

   Incoming UDP responses to other services will be dumped: we are strictly handshaking here ...

   \param[in]    orc_SerialNumber      serial number of server to change IP on
   \param[in]    orau8_NewIpAddress    IP address to set
   \param[in]    orau8_NetMask         Net mask to set
   \param[in]    orau8_DefaultGateway  Default gateway to set
   \param[in]    orc_NewNodeId         New bus id and node id for the interface
   \param[out]   orau8_ResponseIp      IP address the response was received from
   \param[out]   opu8_ErrorResult      if not NULL: code of error response (if Errc::warn is returned)

   \return
   Errc::success   no problems
   Errc::warn      error response
   Errc::com       could not send request
   Errc::config    no dispatcher installed
   Errc::range     serial number is invalid or wrong format of serial number is configured
   Errc::timeout   no response within timeout
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::BroadcastSetIpAddress(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                          const uint8_t (&orau8_NewIpAddress)[4],
                                                          const uint8_t (&orau8_NetMask)[4],
                                                          const uint8_t (&orau8_DefaultGateway)[4],
                                                          const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                          uint8_t (&orau8_ResponseIp)[4],
                                                          uint8_t * const opu8_ErrorResult) const
{
   std::error_code c_Return = Errc::timeout;

   std::vector<uint8_t> c_Request;
   if (mpc_Dispatcher == nullptr)
   {
      c_Return = Errc::config;
   }
   else if ((orc_SerialNumber.q_IsValid == false) ||
            (orc_SerialNumber.q_ExtFormatUsed == true))
   {
      c_Return = Errc::range;
   }
   else
   {
      C_DoIpHeader c_Header(C_DoIpHeader::hu16_PAYLOAD_TYPE_SET_IP_ADDRESS_MESSAGE_REQ, 21U);
      std::error_code c_ReturnLocal = Errc::success;
      c_Header.ComposeHeader(c_Request);
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE], &orc_SerialNumber.au8_SerialNumber[0], 6);
      // Mode flag. Bit 1 is IP address, bit 2 is node identifier
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 6] = 0x03;
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 7], &orau8_NewIpAddress[0], 4);
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 11], &orau8_NetMask[0], 4);
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 15], &orau8_DefaultGateway[0], 4);
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 19] = orc_NewNodeId.u8_BusIdentifier;
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 20] = orc_NewNodeId.u8_NodeIdentifier;

      c_ReturnLocal = mpc_Dispatcher->SendUdp(c_Request);
      if (c_ReturnLocal)
      {
         m_LogWarningWithHeader("Could not send UDP broadcast request.", TGL_UTIL_FUNC_ID);
         c_Return = Errc::com;
      }
      else
      {
         const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();
         std::vector<uint8_t> c_Response;
         bool q_Done = false;

         while (((stw::tgl::TglGetTickCount() - mu32_BroadcastTimeoutMs) < u32_StartTime) && (q_Done == false))
         {
            //check for response
            c_ReturnLocal = mpc_Dispatcher->ReadUdp(c_Response, orau8_ResponseIp);
            if (!c_ReturnLocal)
            {
               if (c_Response.size() == (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 9U))
               {
                  //header OK ?
                  const std::error_code c_HeaderResult = c_Header.DecodeHeader(c_Response);
                  if (!c_HeaderResult)
                  {
                     //sanity check: does payload size match response size ?
                     if ((c_Header.u32_PayloadSize ==
                          static_cast<uint32_t>((c_Response.size() - C_DoIpHeader::hu8_DOIP_HEADER_SIZE))) &&
                         (c_Header.u16_PayloadType == C_DoIpHeader::hu16_PAYLOAD_TYPE_SET_IP_ADDRESS_MESSAGE_RES))
                     {
                        //looks legit; check payload ...
                        const int x_SnrOk = //lint !e970 !e8080 //using type to match library interface
                                            std::memcmp(&orc_SerialNumber.au8_SerialNumber[0],
                                                        &c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2], 6U);
                        if (x_SnrOk != 0)
                        {
                           m_LogWarningWithHeaderAndIp(
                              "SetIpAddress: response with unexpected serial number. Ignoring.",
                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                        }
                        else
                        {
                           if (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 8U] == 0U)
                           {
                              c_Return = Errc::success;
                           }
                           else if (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 8U] == 3U)
                           {
                              m_LogWarningWithHeaderAndIp(
                                 "SetIpAddress: could not perform action. Node has security feature activated.",
                                 TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                              if (opu8_ErrorResult != nullptr)
                              {
                                 (*opu8_ErrorResult) = c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 8U];
                              }
                           }
                           else
                           {
                              c_Return = Errc::warn;
                              if (opu8_ErrorResult != nullptr)
                              {
                                 (*opu8_ErrorResult) = c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 8U];
                              }
                           }
                           q_Done = true;
                        }
                     }
                     else
                     {
                        m_LogWarningWithHeaderAndIp(
                           "UDP response with unexpected payload size or type received. Ignoring.", TGL_UTIL_FUNC_ID,
                           orau8_ResponseIp);
                     }
                  }
                  else if (c_HeaderResult == Errc::config)
                  {
                     m_LogWarningWithHeaderAndIp("UDP response with unexpected version received. Ignoring.",
                                                 TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                  }
                  else
                  {
                     //unexpected ...
                     m_LogWarningWithHeaderAndIp("Internal error parsing DoIp header.", TGL_UTIL_FUNC_ID,
                                                 orau8_ResponseIp);
                  }
               }
               else
               {
                  m_LogWarningWithHeaderAndIp("UDP response with incorrect payload size (" +
                                              std::to_string(c_Response.size()) + ") received. Ignoring.",
                                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
               }
            }
         }
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set IP address extended of one node

   Send broadcast to change the IP address of one specific node.
   Only the node with a specified serial number is expected to send a response and change its IP address.
   The function will return as soon as it has received one response.

   Incoming UDP responses to other services will be dumped: we are strictly handshaking here ...

   \param[in]    orc_SerialNumber                   serial number of server to change IP on
   \param[in]    orau8_NewIpAddress                 IP address to set
   \param[in]    orau8_NetMask                      Net mask to set
   \param[in]    orau8_DefaultGateway               Default gateway to set
   \param[in]    orc_NewNodeId                      New bus id and node id for the interface
   \param[in]    ou8_SubNodeId                      Sub node id of node for identification in case of a multi CPU node
   \param[out]   orau8_ResponseIp                   IP address the response was received from
   \param[out]   opu8_ErrorResult                   if not NULL: code of error response (if Errc::warn is returned)

   \return
   Errc::success   no problems
   Errc::warn      error response
   Errc::com       could not send request
   Errc::config    no dispatcher installed
   Errc::timeout   no response within timeout
   Errc::range     serial number (orc_SerialNumber) is to long (maximum is 29 byte) or empty
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::BroadcastSetIpAddressExtended(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                                  const uint8_t(&orau8_NewIpAddress)[4],
                                                                  const uint8_t(&orau8_NetMask)[4],
                                                                  const uint8_t(&orau8_DefaultGateway)[4],
                                                                  const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                                  const uint8_t ou8_SubNodeId,
                                                                  uint8_t(&orau8_ResponseIp)[4],
                                                                  uint8_t * const opu8_ErrorResult) const
{
   std::error_code c_Return = Errc::timeout;

   std::vector<uint8_t> c_Request;
   if (mpc_Dispatcher == nullptr)
   {
      c_Return = Errc::config;
   }
   else if ((orc_SerialNumber.q_IsValid == false) ||
            (orc_SerialNumber.q_ExtFormatUsed == false))
   {
      c_Return = Errc::range;
   }
   else
   {
      const uint8_t u8_SerialNumberLength = orc_SerialNumber.u8_SerialNumberByteLength;
      const std::vector<uint8_t> c_SerialNumberRaw = orc_SerialNumber.GetSerialNumberAsRawData();
      C_DoIpHeader c_Header(C_DoIpHeader::hu16_PAYLOAD_TYPE_SET_IP_ADDRESS_MESSAGE_EXT_REQ,
                            (static_cast<uint32_t>(18U) + u8_SerialNumberLength));
      std::error_code c_ReturnLocal = Errc::success;

      c_Header.ComposeHeader(c_Request);
      // Mode flag. Bit 1 is IP address, bit 2 is node identifier
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE] = 0x03;
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 1], &orau8_NewIpAddress[0], 4);
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 5], &orau8_NetMask[0], 4);
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 9], &orau8_DefaultGateway[0], 4);
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 13] = orc_NewNodeId.u8_BusIdentifier;
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 14] = orc_NewNodeId.u8_NodeIdentifier;

      // Extended part
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 15] = ou8_SubNodeId;
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 16] = orc_SerialNumber.u8_SerialNumberManufacturerFormat;
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 17] = u8_SerialNumberLength;
      (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 18],
                        &c_SerialNumberRaw[0], u8_SerialNumberLength);

      c_ReturnLocal = mpc_Dispatcher->SendUdp(c_Request);
      if (c_ReturnLocal)
      {
         m_LogWarningWithHeader("Could not send UDP broadcast request.", TGL_UTIL_FUNC_ID);
         c_Return = Errc::com;
      }
      else
      {
         const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();
         std::vector<uint8_t> c_Response;
         bool q_Done = false;

         while (((stw::tgl::TglGetTickCount() - mu32_BroadcastTimeoutMs) < u32_StartTime) && (q_Done == false))
         {
            //check for response
            c_ReturnLocal = mpc_Dispatcher->ReadUdp(c_Response, orau8_ResponseIp);
            if (!c_ReturnLocal)
            {
               if (static_cast<uint32_t>(c_Response.size()) ==
                   (static_cast<uint32_t>(C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 6U) + u8_SerialNumberLength))
               {
                  //header OK ?
                  const std::error_code c_HeaderResult = c_Header.DecodeHeader(c_Response);
                  if (!c_HeaderResult)
                  {
                     //sanity check: does payload size match response size ?
                     if ((c_Header.u32_PayloadSize ==
                          static_cast<uint32_t>((c_Response.size() - C_DoIpHeader::hu8_DOIP_HEADER_SIZE))) &&
                         (c_Header.u16_PayloadType == C_DoIpHeader::hu16_PAYLOAD_TYPE_SET_IP_ADDRESS_MESSAGE_EXT_RES))
                     {
                        //looks legit; check payload ...
                        const bool q_SubNodeIdOk = (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 3] ==
                                                    ou8_SubNodeId);
                        const bool q_SnManufacturerFormatOk = (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 4] ==
                                                               orc_SerialNumber.u8_SerialNumberManufacturerFormat);
                        const bool q_SnLengthOk = (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 5] ==
                                                   u8_SerialNumberLength);
                        const int x_SnrOk = //lint !e970 !e8080 //using type to match library interface
                                            std::memcmp(&c_SerialNumberRaw[0],
                                                        &c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 6U],
                                                        u8_SerialNumberLength);

                        if (q_SubNodeIdOk == false)
                        {
                           m_LogWarningWithHeaderAndIp(
                              "SetIpAddress: response with unexpected sub node id. Ignoring.",
                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                        }
                        else if (q_SnManufacturerFormatOk == false)
                        {
                           m_LogWarningWithHeaderAndIp(
                              "SetIpAddress: response with unexpected manufacturer format. Ignoring.",
                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                        }
                        else if (q_SnLengthOk == false)
                        {
                           m_LogWarningWithHeaderAndIp(
                              "SetIpAddress: response with unexpected serial number length. Ignoring.",
                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                        }
                        else if (x_SnrOk != 0)
                        {
                           m_LogWarningWithHeaderAndIp(
                              "SetIpAddress: response with unexpected serial number. Ignoring.",
                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                        }
                        else
                        {
                           if (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2U] == 0U)
                           {
                              c_Return = Errc::success;
                           }
                           else if (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2U] == 3U)
                           {
                              m_LogWarningWithHeaderAndIp(
                                 "SetIpAddress: could not perform action. Node has security feature activated.",
                                 TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                              if (opu8_ErrorResult != nullptr)
                              {
                                 (*opu8_ErrorResult) = c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2U];
                              }
                           }
                           else
                           {
                              c_Return = Errc::warn;
                              if (opu8_ErrorResult != nullptr)
                              {
                                 (*opu8_ErrorResult) = c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2U];
                              }
                           }
                           q_Done = true;
                        }
                     }
                     else
                     {
                        m_LogWarningWithHeaderAndIp(
                           "UDP response with unexpected payload size or type received. Ignoring.", TGL_UTIL_FUNC_ID,
                           orau8_ResponseIp);
                     }
                  }
                  else if (c_HeaderResult == Errc::config)
                  {
                     m_LogWarningWithHeaderAndIp("UDP response with unexpected version received. Ignoring.",
                                                 TGL_UTIL_FUNC_ID, orau8_ResponseIp);
                  }
                  else
                  {
                     //unexpected ...
                     m_LogWarningWithHeaderAndIp("Internal error parsing DoIp header.", TGL_UTIL_FUNC_ID,
                                                 orau8_ResponseIp);
                  }
               }
               else
               {
                  m_LogWarningWithHeaderAndIp("UDP response with incorrect payload size (" +
                                              std::to_string(c_Response.size()) + ") received. Ignoring.",
                                              TGL_UTIL_FUNC_ID, orau8_ResponseIp);
               }
            }
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Request that all devices stay in the flashloader after the next reset

   Send broadcast to set "stay-in-flashloader" flag.
   Report back list of nodes that sent a response (with pass/fail information).
   The function will wait for responses for the time configured with SetBroadcastTimeout()

   Incoming UDP responses to other services will be dumped: we are strictly handshaking here ...

   \param[out]    orc_Results   information about all nodes that sent a response

   \return
   Errc::success   no problems; zero or more responses received; data placed in orc_Results
   Errc::com       could not send request
   Errc::config    no dispatcher installed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::BroadcastRequestProgramming(
   std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastRequestProgrammingResults> & orc_Results) const
{
   std::error_code c_Return = Errc::success;
   C_DoIpHeader c_Header(C_DoIpHeader::hu16_PAYLOAD_TYPE_REQUEST_PROGRAMMING_REQ, 0U);

   orc_Results.clear();
   if (mpc_Dispatcher == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      std::vector<uint8_t> c_Request;

      c_Header.ComposeHeader(c_Request);

      if (mpc_Dispatcher->SendUdp(c_Request))
      {
         m_LogWarningWithHeader("Could not send UDP broadcast request.", TGL_UTIL_FUNC_ID);
         c_Return = Errc::com;
      }
      else
      {
         //check for responses
         const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();
         uint8_t au8_Ip[4];
         std::vector<uint8_t> c_Response;

         while ((stw::tgl::TglGetTickCount() - mu32_BroadcastTimeoutMs) < u32_StartTime)
         {
            const std::error_code c_ReturnLocal = mpc_Dispatcher->ReadUdp(c_Response, au8_Ip);
            if (!c_ReturnLocal)
            {
               if (c_Response.size() == (C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2 + 1))
               {
                  //header OK ?
                  const std::error_code c_HeaderResult = c_Header.DecodeHeader(c_Response);
                  if (!c_HeaderResult)
                  {
                     //sanity check: does payload size match response size ?
                     if ((c_Header.u32_PayloadSize ==
                          static_cast<uint32_t>((c_Response.size() - C_DoIpHeader::hu8_DOIP_HEADER_SIZE))) &&
                         (c_Header.u16_PayloadType == C_DoIpHeader::hu16_PAYLOAD_TYPE_REQUEST_PROGRAMMING_RES))
                     {
                        //looks legit; extract payload ...
                        C_BroadcastRequestProgrammingResults c_Info;
                        const uint16_t u16_SourceAddress =
                           (static_cast<uint16_t>(static_cast<uint16_t>(c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE])
                                                  <<
                                                  8U) + c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 1]) - 1U;
                        c_Info.c_NodeId.u8_NodeIdentifier = static_cast<uint8_t>(u16_SourceAddress & 0x7FU);
                        c_Info.c_NodeId.u8_BusIdentifier = static_cast<uint8_t>((u16_SourceAddress >> 7U) & 0x0FU);
                        c_Info.q_RequestAccepted =
                           (c_Response[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 2] == 0U) ? true : false;
                        (void)std::memcpy(&c_Info.au8_IpAddress[0], &au8_Ip[0], 4U);
                        orc_Results.push_back(c_Info);
                     }
                     else
                     {
                        m_LogWarningWithHeaderAndIp("UDP response with unexpected payload size received. Ignoring.",
                                                    TGL_UTIL_FUNC_ID, au8_Ip);
                     }
                  }
                  else if (c_HeaderResult == Errc::config)
                  {
                     m_LogWarningWithHeaderAndIp("UDP response with unexpected version received. Ignoring.",
                                                 TGL_UTIL_FUNC_ID, au8_Ip);
                  }
                  else
                  {
                     //unexpected ...
                     m_LogWarningWithHeaderAndIp("Internal error parsing DoIp header.", TGL_UTIL_FUNC_ID, au8_Ip);
                  }
               }
               else
               {
                  m_LogWarningWithHeaderAndIp("UDP response with incorrect payload size (" +
                                              std::to_string(c_Response.size()) + ") received. Ignoring.",
                                              TGL_UTIL_FUNC_ID, au8_Ip);
               }
            }
         }
         c_Return = Errc::success;
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send network reset broadcast

   Send broadcast to reset all nodes.
   Optionally only the node with a specified serial number is expected to reset itself.
   There will be no response to this service.

   \param[in]    ou8_ResetType                Reset type (0x02: keyOffOnReset, 0x60: resetToFlashloader)
   \param[in]    oq_SpecificSerialNumberOnly  false: reset all nodes
   \param[in]    opau8_SerialNumber           if oq_SpecificSerialNumberOnly is true: only the server with this SNR
                                               shall reset

   \return
   Errc::success   no problems
   Errc::com       could not send request
   Errc::config    no dispatcher installed
   Errc::range     oq_SpecificSerialNumberOnly is true but opau8_SerialNumber is NULL
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::BroadcastNetReset(const uint8_t ou8_ResetType,
                                                      const bool oq_SpecificSerialNumberOnly,
                                                      const uint8_t (*const opau8_SerialNumber)[6]) const
{
   std::error_code c_Return = Errc::success;
   const C_DoIpHeader c_Header(C_DoIpHeader::hu16_PAYLOAD_TYPE_NET_RESET_MESSAGE_REQ, 8U);

   std::vector<uint8_t> c_Request;

   if (mpc_Dispatcher == nullptr)
   {
      c_Return = Errc::config;
   }
   else if ((oq_SpecificSerialNumberOnly == true) && (opau8_SerialNumber == nullptr))
   {
      c_Return = Errc::range;
   }
   else
   {
      c_Header.ComposeHeader(c_Request);
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 6] = (oq_SpecificSerialNumberOnly == true) ? 1U : 0U;
      if (oq_SpecificSerialNumberOnly == true)
      {
         (void)std::memcpy(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE], opau8_SerialNumber, 6U);
      }
      else
      {
         (void)std::memset(&c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE], 0x00U, 6U);
      }
      // set reset type
      c_Request[C_DoIpHeader::hu8_DOIP_HEADER_SIZE + 7] = ou8_ResetType;

      if (mpc_Dispatcher->SendUdp(c_Request))
      {
         m_LogWarningWithHeader("Could not send UDP broadcast request.", TGL_UTIL_FUNC_ID);
         c_Return = Errc::com;
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Set up class

   \param[in]  ou16_MaxServiceQueueSize   maximum number of entries in Tx/Rx queue
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::C_OscProtocolDriverOsyTpIp(const uint16_t ou16_MaxServiceQueueSize) :
   C_OscProtocolDriverOsyTpBase(ou16_MaxServiceQueueSize),
   mpc_Dispatcher(nullptr),
   mu32_DispatcherHandle(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   destructor

   clean up ...
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::~C_OscProtocolDriverOsyTpIp(void)
{
   mpc_Dispatcher = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compose DoIp request

   Set up one DoIp frame:
   - DoIp header with payload type "DiagnosticMessage"
   - Service data as payload

   \param[in]    orc_Service    service to put into request
   \param[out]   orc_Request    composed data, ready for sending via IP
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::m_ComposeRequest(const C_OscProtocolDriverOsyService & orc_Service,
                                                  std::vector<uint8_t> & orc_Request) const
{
   const uint16_t u16_SourceAddress =
      (static_cast<uint16_t>(static_cast<uint16_t>(mc_ClientId.u8_BusIdentifier) << 7U) +
       mc_ClientId.u8_NodeIdentifier) + 1U;
   const uint16_t u16_TargetAddress =
      (static_cast<uint16_t>(static_cast<uint16_t>(mc_ServerId.u8_BusIdentifier) << 7U) +
       mc_ServerId.u8_NodeIdentifier) + 1U;
   //DoIP header + source + target + service
   const uint32_t u32_RequestSizePayload = 2U + 2U + static_cast<uint32_t>(orc_Service.c_Data.size());

   C_DoIpHeader c_Header;

   c_Header.u32_PayloadSize = u32_RequestSizePayload;

   if (orc_Service.q_CanTransferWithoutFlowControl == false)
   {
      c_Header.u16_PayloadType = C_DoIpHeader::hu16_PAYLOAD_TYPE_DIAGNOSTIC_MESSAGE;
   }
   else
   {
      c_Header.u16_PayloadType = C_DoIpHeader::hu16_PAYLOAD_TYPE_CAN_ROUTING_WITHOUT_FC_REQ;
   }

   c_Header.ComposeHeader(orc_Request);

   //add payload:
   //source and target address:
   orc_Request[8]  = static_cast<uint8_t>(u16_SourceAddress >> 8U);
   orc_Request[9]  = static_cast<uint8_t>(u16_SourceAddress);
   orc_Request[10] = static_cast<uint8_t>(u16_TargetAddress >> 8U);
   orc_Request[11] = static_cast<uint8_t>(u16_TargetAddress);
   //service:
   if (orc_Service.c_Data.size() > 0)
   {
      (void)std::memcpy(&orc_Request[12], &orc_Service.c_Data[0], orc_Service.c_Data.size());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Perform cyclic communication tasks for IP-TP

   Perform cyclic communication tasks for IP-TP.
   - send services from Tx queue via TCP connection
   - read incoming TCP services and place in Rx queue

   Configuration problems will be reported via the function return value.
   Ongoing communication problems will be written to the class's log text.

   \return
   Errc::success   cycle finished
   Errc::config    no dispatcher installed
   Errc::com       error on sending service
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolDriverOsyTpIp::Cycle(void)
{
   std::error_code c_ReturnFunc = Errc::success;
   C_OscProtocolDriverOsyService c_Service;

   if (mpc_Dispatcher == nullptr)
   {
      c_ReturnFunc = Errc::config;
   }
   else
   {
      //doubles as the loop control: a failed send stops the loop just as an empty queue does
      std::error_code c_TxResult = Errc::success;

      //the Rx pass has its own control variable: it doubles as the read-loop condition
      std::error_code c_RxResult = Errc::success;

      //do we have requests to send ?
      while (!c_TxResult)
      {
         c_TxResult = this->m_GetFromTxQueue(c_Service);
         if (!c_TxResult)
         {
            //send data on TCP:
            std::vector<uint8_t> c_Request;
            m_ComposeRequest(c_Service, c_Request);

            if (mpc_Dispatcher->SendTcp(this->mu32_DispatcherHandle, c_Request))
            {
               m_LogWarningWithHeader("Could not send TCP request. Service data lost.", TGL_UTIL_FUNC_ID);
               c_TxResult = Errc::com;
            }
         }
      }

      if (c_TxResult == Errc::com)
      {
         c_ReturnFunc = Errc::com;
         c_RxResult = Errc::com; //as before, a failed send skips the Rx pass entirely
      }
      else
      {
         c_RxResult = Errc::success;
      }

      //read all incoming messages:
      while (!c_RxResult)
      {
         std::vector<uint8_t> c_Data;
         bool q_DataFromBuffer = false;

         //As the TCP transfer is stream based we must consider that we might not get "exactly" one service here.
         //So we need to "aggregate" multiple blocks.
         //We need at least the 8 DoIp header bytes to identify how many data bytes we need.
         // (size is part of this header)

         //Are we just starting ?
         if (mc_RxState.e_Status == C_TcpRxServiceState::eIDLE)
         {
            //look in the buffer first
            c_RxResult = this->mpc_Dispatcher->ReadTcpBuffer(this->mc_ClientId.u8_BusIdentifier,
                                                            this->mc_ClientId.u8_NodeIdentifier,
                                                            this->mc_ServerId.u8_BusIdentifier,
                                                            this->mc_ServerId.u8_NodeIdentifier,
                                                            c_Data);

            if (c_RxResult)
            {
               // No data in buffer, search for new data
               //try to get header ...
               c_Data.resize(C_DoIpHeader::hu8_DOIP_HEADER_SIZE);
               c_RxResult = mpc_Dispatcher->ReadTcp(this->mu32_DispatcherHandle, c_Data);
               if (!c_RxResult)
               {
                  //we got the header ...
                  const std::error_code c_HeaderResult = mc_RxState.c_ServiceHeader.DecodeHeader(c_Data);
                  if (!c_HeaderResult)
                  {
                     //sanity check: is the payload size realistic ?
                     //effectively CAN-TP limits the size of one service to 4kB
                     if ((mc_RxState.c_ServiceHeader.u32_PayloadSize < 4200U) &&
                         (mc_RxState.c_ServiceHeader.u16_PayloadType ==
                          C_DoIpHeader::hu16_PAYLOAD_TYPE_DIAGNOSTIC_MESSAGE))
                     {
                        mc_RxState.e_Status = C_TcpRxServiceState::ePARTIAL;
                     }
                     else
                     {
                        m_LogWarningWithHeader("TCP response with unexpected payload size received. Ignoring.",
                                               TGL_UTIL_FUNC_ID);
                     }
                  }
                  else if (c_HeaderResult == Errc::config)
                  {
                     m_LogWarningWithHeader("TCP response with unexpected version received. Ignoring.",
                                            TGL_UTIL_FUNC_ID);
                  }
                  else
                  {
                     //unexpected ...
                     m_LogWarningWithHeader("Internal error parsing DoIp header.", TGL_UTIL_FUNC_ID);
                  }
                  c_RxResult = Errc::success; //maybe there's more ...
               }
            }
            else
            {
               // Data from dispatcher buffer
               q_DataFromBuffer = true;
               mc_RxState.e_Status = C_TcpRxServiceState::ePARTIAL;
               mc_RxState.c_ServiceHeader.u16_PayloadType = C_DoIpHeader::hu16_PAYLOAD_TYPE_DIAGNOSTIC_MESSAGE;
            }
         }
         //"wait" for payload:
         if (mc_RxState.e_Status == C_TcpRxServiceState::ePARTIAL)
         {
            if (q_DataFromBuffer == false)
            {
               c_Data.resize(mc_RxState.c_ServiceHeader.u32_PayloadSize);

               c_RxResult = mpc_Dispatcher->ReadTcp(this->mu32_DispatcherHandle,
                                                   this->mc_ClientId.u8_BusIdentifier,
                                                   this->mc_ClientId.u8_NodeIdentifier,
                                                   this->mc_ServerId.u8_BusIdentifier,
                                                   this->mc_ServerId.u8_NodeIdentifier,
                                                   c_Data);
            }

            if (!c_RxResult)
            {
               //we got the payload ...
               if (c_Data.size() < 4)
               {
                  m_LogWarningWithHeader("TCP response with not enough payload data. Ignoring.", TGL_UTIL_FUNC_ID);
               }
               else
               {
                  if (mc_RxState.c_ServiceHeader.u16_PayloadType == C_DoIpHeader::hu16_PAYLOAD_TYPE_DIAGNOSTIC_MESSAGE)
                  {
                     c_Service.c_Data.resize(c_Data.size() - 4U);
                     (void)std::memcpy(&c_Service.c_Data[0], &c_Data[4], c_Data.size() - 4U);
                     //add to queue:
                     const std::error_code c_AddResult = m_AddToRxQueue(c_Service);
                     if (c_AddResult)
                     {
                        m_LogWarningWithHeader("Rx Queue overflow. Incoming TCP response dumped.", TGL_UTIL_FUNC_ID);
                        //a full queue ended the read loop before; keep that
                        c_RxResult = Errc::noact;
                     }
                  }
                  else
                  {
                     //nothing we can handle ...
                     m_LogWarningWithHeader("Unexpected payload type received: 0x" +
                                            mh_IntToHex(mc_RxState.c_ServiceHeader.u16_PayloadType, 4) +
                                            "Incoming TCP response dumped.", TGL_UTIL_FUNC_ID);
                  }
               }
               mc_RxState.e_Status = C_TcpRxServiceState::eIDLE;
            }
            else if (c_RxResult == Errc::warn)
            {
               // Receiving of an other server. Check further messages
               mc_RxState.e_Status = C_TcpRxServiceState::eIDLE;
               c_RxResult = Errc::success;
            }
            else
            {
               //No new data -> nothing to do
               //Note that we do not perform timeout handling here
               //As long as the connection is open we'll receive data.
               //If it gets dropped for some reason we either
               //- will detect an error when trying to send another TCP request
               //- will detect a timeout when waiting for a response (one layer above)
               //- will detect that there are no more expected incoming event driven responses
               //In any of these cases it's the application's job to handle this (e.g. reconnect or cry).
            }
         }
      }
   }
   return c_ReturnFunc;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Print logging text

   Add information about this class and the current server-id configuration and call logging handler function
    to perform the actual logging.

   \param[in]     orc_Information     text to log
   \param[in]     opcn_Function       function name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::m_LogWarningWithHeader(const std::string & orc_Information,
                                                        const char_t * const opcn_Function) const
{
   C_OscLoggingHandler::h_WriteLogError("openSYDE IP-TP",
                                        "openSYDE IP-TP node " + std::to_string(
                                           mc_ServerId.u8_BusIdentifier) + "." +
                                        std::to_string(
                                           mc_ServerId.u8_NodeIdentifier) + ": " + orc_Information, __FILE__,
                                        opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Print logging text with IP

   Add
   * information about this class
   * and the current server-id configuration
   * IP information
   and call logging handler function  to perform the actual logging.

   \param[in]     orc_Information     text to log
   \param[in]     opcn_Function       function name
   \param[in]     orau8_Ip            IP address to report
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::m_LogWarningWithHeaderAndIp(const std::string & orc_Information,
                                                             const char_t * const opcn_Function,
                                                             const uint8_t (&orau8_Ip)[4]) const
{
   std::string c_Text;

   c_Text = PrintFormattedCompat("[IP: %d.%d.%d.%d]: ", orau8_Ip[0], orau8_Ip[1], orau8_Ip[2], orau8_Ip[3]);
   c_Text += orc_Information;

   m_LogWarningWithHeader(c_Text, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults::~C_BroadcastGetDeviceInfoResults(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current equal to orc_Cmp

   IP Address will not be checked. It is not an unique identifier for a device.

   \param[in] orc_Cmp Compared instance

   \return
   Current equal to orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults::operator ==(
   const C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults & orc_Cmp) const
{
   bool q_Return = false;

   if ((this->c_SerialNumber == orc_Cmp.c_SerialNumber)      &&
       (this->c_DeviceName == orc_Cmp.c_DeviceName) &&
       (this->c_NodeId == orc_Cmp.c_NodeId))
   {
      q_Return = true;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current smaller than orc_Cmp

   IP Address will not be checked. It is not an unique identifier for a device.

   \param[in] orc_Cmp Compared instance

   \return
   Current smaller than orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults::operator <(
   const C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults & orc_Cmp) const
{
   bool q_Return = false;

   if (this->c_SerialNumber < orc_Cmp.c_SerialNumber)
   {
      q_Return = true;
   }
   else if (this->c_SerialNumber == orc_Cmp.c_SerialNumber)
   {
      if (this->c_DeviceName < orc_Cmp.c_DeviceName)
      {
         q_Return = true;
      }
      else if (this->c_DeviceName == orc_Cmp.c_DeviceName)
      {
         if (this->c_NodeId < orc_Cmp.c_NodeId)
         {
            q_Return = true;
         }
      }
      else
      {
         // Nothing to do
      }
   }
   else
   {
      // Nothing to do
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract device information from binary data

   Extract device information from binary data stream.
   Caller is responsible to provide data with the correct size.
   Extracts serial number and device name.

   \param[in]  orc_Data            binary data
   \param[in]  ou8_DataStartIndex  offset index within orc_Data where data to parse starts
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults::ParseFromArray(const std::vector<uint8_t> & orc_Data,
                                                                                 const uint8_t ou8_DataStartIndex)
{
   uint8_t au8_DeviceName[29];
   uint8_t au8_SerialNumber[6];

   const uint16_t u16_SourceAddress =
      (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[ou8_DataStartIndex]) << 8U) +
       orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 1U]) - 1U;

   this->c_NodeId.u8_NodeIdentifier = static_cast<uint8_t>(u16_SourceAddress & 0x7FU);
   this->c_NodeId.u8_BusIdentifier = static_cast<uint8_t>((u16_SourceAddress >> 7U) & 0x0FU);

   (void)std::memcpy(&au8_SerialNumber, &orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 2U], 6U);
   this->c_SerialNumber.SetPosSerialNumber(au8_SerialNumber);

   //defensive approach: spec says it's always zero terminated: make sure
   au8_DeviceName[28] = 0U;
   (void)std::memcpy(&au8_DeviceName[0], &orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 8U], 28U);

   //lint -e{9176} //no problems as long as charn has the same size as uint8; if not we'd be in deep !"=?& anyway
   this->c_DeviceName = reinterpret_cast<const char_t *>(&au8_DeviceName[0]);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current equal to orc_Cmp

   IP Address will not be checked. It is not an unique identifier for a device.

   \param[in] orc_Cmp Compared instance

   \return
   Current equal to orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults::operator ==(
   const C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults & orc_Cmp) const
{
   bool q_Return = false;

   if ((this->c_DeviceName == orc_Cmp.c_DeviceName) &&
       (this->c_NodeId == orc_Cmp.c_NodeId) &&
       (this->c_SerialNumber == orc_Cmp.c_SerialNumber) &&
       (this->u8_SubNodeId == orc_Cmp.u8_SubNodeId))
   {
      q_Return = true;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current smaller than orc_Cmp

   IP Address will not be checked. It is not an unique identifier for a device.

   \param[in] orc_Cmp Compared instance

   \return
   Current smaller than orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults::operator <(
   const C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults & orc_Cmp) const
{
   bool q_Return = false;

   if (this->c_SerialNumber < orc_Cmp.c_SerialNumber)
   {
      q_Return = true;
   }
   else if (this->c_SerialNumber == orc_Cmp.c_SerialNumber)
   {
      if (this->u8_SubNodeId < orc_Cmp.u8_SubNodeId)
      {
         q_Return = true;
      }
      else if (this->u8_SubNodeId == orc_Cmp.u8_SubNodeId)
      {
         if (this->c_DeviceName < orc_Cmp.c_DeviceName)
         {
            q_Return = true;
         }
         else if (this->c_DeviceName == orc_Cmp.c_DeviceName)
         {
            if (this->c_NodeId < orc_Cmp.c_NodeId)
            {
               q_Return = true;
            }
         }
         else
         {
            // Nothing to do
         }
      }
      else
      {
         // Nothing to do
      }
   }
   else
   {
      // Nothing to do
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract device information extended from binary data

   Extract device information extended from binary data stream.
   Caller is responsible to provide data with the correct size.
   Extracts serial number and device name.

   \param[in]  orc_Data            binary data
   \param[in]  ou8_DataStartIndex  offset index within orc_Data where data to parse starts
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults::ParseFromArray(
   const std::vector<uint8_t> & orc_Data, const uint8_t ou8_DataStartIndex)
{
   uint8_t au8_DeviceName[29];
   uint8_t u8_SerialNumberLength;

   const uint16_t u16_SourceAddress =
      (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[ou8_DataStartIndex]) << 8U) +
       orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 1U]) - 1U;

   this->c_NodeId.u8_NodeIdentifier = static_cast<uint8_t>(u16_SourceAddress & 0x7FU);
   this->c_NodeId.u8_BusIdentifier = static_cast<uint8_t>((u16_SourceAddress >> 7U) & 0x0FU);

   // Security activated flag (Bit 0 security flag, all other bits are reserved)
   this->q_SecurityActivated = ((orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 2U] & 0x01U) == 0x01U);

   //defensive approach: spec says it's always zero terminated: make sure
   au8_DeviceName[28] = 0U;
   (void)std::memcpy(&au8_DeviceName[0], &orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 3U], 28U);

   //lint -e{9176} //no problems as long as charn has the same size as uint8; if not we'd be in deep !"=?& anyway
   this->c_DeviceName = reinterpret_cast<const char_t *>(&au8_DeviceName[0]);

   this->u8_SubNodeId = orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 32];

   u8_SerialNumberLength = orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 34];
   if (u8_SerialNumberLength <= 29)
   {
      std::vector<uint8_t> c_CurSerialNumber;
      //defensive approach: variable length, so set all to 0 first
      c_CurSerialNumber.resize(u8_SerialNumberLength);
      (void)std::memcpy(&c_CurSerialNumber[0], &orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 35U],
                        u8_SerialNumberLength);

      this->c_SerialNumber.SetExtSerialNumber(c_CurSerialNumber,
                                              orc_Data[static_cast<size_t>(ou8_DataStartIndex) + 33]);
   }
}
