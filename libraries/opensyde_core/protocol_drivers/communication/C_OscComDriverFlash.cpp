//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Core communication driver for flashloader protocols (implementation)

   Adds functionality to the base class: flashloader protocol instance for the openSYDE protocol.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include "stwerrors.hpp"

#include "C_OscComDriverFlash.hpp"
#include "C_OscLoggingHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::can;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]  oq_RoutingActive              Flag for activating routing
   \param[in]  oq_UpdateRoutingMode          Flag for update specific routing or generic routing (m_GetRoutingMode)
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComDriverFlash::C_OscComDriverFlash(const bool oq_RoutingActive, const bool oq_UpdateRoutingMode) :
   C_OscComDriverProtocol(),
   mq_RoutingActive(oq_RoutingActive),
   mq_UpdateRoutingMode(oq_UpdateRoutingMode)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComDriverFlash::~C_OscComDriverFlash(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all members

   \param[in]  orc_SystemDefinition    Entire system definition
   \param[in]  ou32_ActiveBusIndex     Bus index of bus in system definition where we are connected to
   \param[in]  orc_ActiveNodes         Flags for all available nodes in the system
   \param[in]  opc_CanDispatcher       Pointer to concrete CAN dispatcher
   \param[in]  opc_IpDispatcher        Pointer to concrete IP dispatcher
   \param[in]  opc_SecurityPemDb       Pointer to PEM database (optional)
                                       Needed if nodes with enabled security are used in the system

   \return
   C_NO_ERR      Operation success
   C_NOACT       No active nodes
   C_CONFIG      Invalid system definition for parameters
   C_RD_WR       Configured communication DLL does not exist
   C_OVERFLOW    Unknown transport protocol or unknown diagnostic server for at least one node
   C_COM         CAN initialization failed or no route found for at least one node
   C_CHECKSUM    Internal buffer overflow detected
   C_DEFAULT     Parameter ou32_ActiveBusIndex invalid
   C_RANGE       Routing configuration failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::Init(const C_OscSystemDefinition & orc_SystemDefinition,
                                  const uint32_t ou32_ActiveBusIndex, const std::vector<uint8_t> & orc_ActiveNodes,
                                  C_CanDispatcher * const opc_CanDispatcher, C_OscIpDispatcher * const opc_IpDispatcher,
                                  C_OscSecurityPemDatabase * const opc_SecurityPemDb)
{
   int32_t s32_Return = C_OscComDriverProtocol::Init(orc_SystemDefinition, ou32_ActiveBusIndex, orc_ActiveNodes,
                                                     opc_CanDispatcher, opc_IpDispatcher, opc_SecurityPemDb);

   if (s32_Return == C_NO_ERR)
   {
      s32_Return = this->m_InitFlashProtocol();
      if (s32_Return == C_NO_ERR)
      {
         this->mq_Initialized = true;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the new CAN bitrate

   An old connection will be closed.

   \param[in] ou32_Bitrate      Bitrate in kbit/s

   \return
   C_NO_ERR    Bitrate set
   C_COM       Error on reading bitrate
   C_CONFIG    No dispatcher installed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::InitCanAndSetCanBitrate(const uint32_t ou32_Bitrate)
{
   C_CanDispatcher * const pc_CanDispatcher = this->m_GetCanDispatcher();
   int32_t s32_Return = C_CONFIG;

   if (pc_CanDispatcher != NULL)
   {
      pc_CanDispatcher->CAN_Exit();
      s32_Return = pc_CanDispatcher->CAN_Init(static_cast<int32_t>(ou32_Bitrate));

      if (s32_Return != C_NO_ERR)
      {
         s32_Return = C_COM;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Configure openSYDE protocol polling timeout

   \param[in]    orc_ServerId      Server id for communication
   \param[in]    ou32_TimeoutMs    Timeout in ms

   \return
   C_NO_ERR   new timeout configured
   C_RANGE    openSYDE protocol not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::OsySetPollingTimeout(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                  const uint32_t ou32_TimeoutMs) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      pc_ExistingProtocol->SetTimeoutPolling(ou32_TimeoutMs);
      s32_Return = C_NO_ERR;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reset the openSYDE protocol polling timeout

   \param[in]    orc_ServerId      Server id for communication

   \return
   C_NO_ERR   new timeout configured
   C_RANGE    openSYDE protocol not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::OsyResetPollingTimeout(const C_OscProtocolDriverOsyNode & orc_ServerId) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      pc_ExistingProtocol->ResetTimeoutPolling();
      s32_Return = C_NO_ERR;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the minimum Flashloader reset wait time in ms

   Default minimum value is 500 ms independent of the type.

   \param[in]  oe_Type  Type of minimum flashloader reset wait time

   \return
   Time in ms all nodes need at least to get from application to the Flashloader or from Flashloader to Flashloader
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscComDriverFlash::GetMinimumFlashloaderResetWaitTime(const E_MinimumFlashloaderResetWaitTimeType oe_Type)
const
{
   uint32_t u32_WaitTime = 500U;
   uint32_t u32_Counter;

   for (u32_Counter = 0U; u32_Counter < this->mc_ActiveNodesIndexes.size(); ++u32_Counter)
   {
      uint32_t u32_NodeWaitTime = 0;
      tgl_assert(m_GetMinimumFlashloaderResetWaitTime(oe_Type, this->mc_ActiveNodesIndexes[u32_Counter],
                                                      u32_NodeWaitTime) == C_NO_ERR);
      if (u32_NodeWaitTime > u32_WaitTime)
      {
         // The node needs a longer wait time
         u32_WaitTime = u32_NodeWaitTime;
      }
   }

   return u32_WaitTime;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the minimum Flashloader reset wait time in ms for a specific node

   Default minimum value is 500 ms independent of the type.

   \param[in]   oe_Type             Type of minimum flashloader reset wait time
   \param[in]   orc_ServerId        Server id to get the configured wait time
   \param[out]  oru32_TimeValue     Time in ms the node need at least to get from application to the Flashloader or
                                    from Flashloader to Flashloader

   \retval   C_NO_ERR   Time returned
   \retval   C_RANGE    Node with orc_ServerId does not exist or is not active
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::GetMinimumFlashloaderResetWaitTime(const E_MinimumFlashloaderResetWaitTimeType oe_Type,
                                                                const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                uint32_t & oru32_TimeValue) const
{
   int32_t s32_Return = C_RANGE;
   bool q_Found = false;
   const uint32_t u32_ActiveNodeIndex = this->m_GetActiveIndex(orc_ServerId, q_Found);

   if (q_Found == true)
   {
      s32_Return = this->m_GetMinimumFlashloaderResetWaitTime(oe_Type,
                                                              this->mc_ActiveNodesIndexes[u32_ActiveNodeIndex],
                                                              oru32_TimeValue);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize and prepare the temporary (transport) protocol and connect to a openSYDE node

   \param[in]       orc_ServerId            node to connect to
   \param[in]       orau8_IpAddress         IP address to connect to
   \param[in,out]   orc_TemporaryProtocol   Temporary protocol object for initialization and using
   \param[in,out]   orc_TpIp                Temporary transport protocol object for initialization and using

   \return
   C_NO_ERR   re-connected
   C_CONFIG   no transport protocol installed
   C_BUSY     re-connect failed
   C_RANGE    client and/or server identifier out of range
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::EthConnectNode(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                            const uint8_t (&orau8_IpAddress)[4],
                                            C_OscProtocolDriverOsy & orc_TemporaryProtocol,
                                            C_OscProtocolDriverOsyTpIp & orc_TpIp)
{
   int32_t s32_Return;

   // Prepare the temporary protocol and its tp
   s32_Return = orc_TpIp.SetNodeIdentifiers(this->GetClientId(), orc_ServerId);
   if (s32_Return == C_NO_ERR)
   {
      uint32_t u32_Handle;
      C_OscIpDispatcher * const pc_IpDispatcher = this->m_GetIpDispatcher();
      s32_Return = pc_IpDispatcher->InitTcp(orau8_IpAddress, u32_Handle);
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = orc_TpIp.SetDispatcher(pc_IpDispatcher, u32_Handle);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = orc_TemporaryProtocol.SetTransportProtocol(&orc_TpIp);
         if (s32_Return == C_NO_ERR)
         {
            s32_Return = orc_TemporaryProtocol.SetNodeIdentifiers(this->GetClientId(), orc_ServerId);
         }
      }
   }

   // The connect with the temporary protocol
   if (s32_Return == C_NO_ERR)
   {
      s32_Return = orc_TemporaryProtocol.ReConnect();
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Disconnects of a specific and temporary protocol connection

  \param[in]   orc_TemporaryProtocol   Temporary protocol object for initialization and using

   \return
   C_NO_ERR   disconnected
   C_CONFIG   no transport protocol installed
   C_NOACT    disconnect failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::h_EthDisconnectNode(C_OscProtocolDriverOsy & orc_TemporaryProtocol)
{
   return orc_TemporaryProtocol.Disconnect();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send request programming

   Sending service as broadcast via CAN or ETH TP.
   Responses from servers will be collected.

   \param[out]    orq_NotAccepted   true: at least one "not accepted" response was received
                                    false: no "not accepted" response was received

   \return
   C_NO_ERR   request sent, positive response received
   C_NOACT    could not put request in Tx queue ...
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_CONFIG   no transport protocol installed or broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyBroadcastRequestProgramming(bool & orq_NotAccepted) const
{
   int32_t s32_Return;

   orq_NotAccepted = false;

   if (this->mpc_CanTransportProtocolBroadcast != NULL)
   {
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastRequestProgrammingResults> c_Results;
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastRequestProgramming(c_Results);
      if (s32_Return == C_NO_ERR)
      {
         for (uint32_t u32_ResponseIndex = 0U; u32_ResponseIndex < c_Results.size(); u32_ResponseIndex++)
         {
            if (c_Results[u32_ResponseIndex].q_RequestAccepted == false)
            {
               orq_NotAccepted = true;
               break;
            }
         }
      }
   }
   else
   {
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastRequestProgrammingResults> c_Results;
      s32_Return = this->mpc_IpTransportProtocolBroadcast->BroadcastRequestProgramming(c_Results);
      if (s32_Return == C_NO_ERR)
      {
         for (uint32_t u32_ResponseIndex = 0U; u32_ResponseIndex < c_Results.size(); u32_ResponseIndex++)
         {
            if (c_Results[u32_ResponseIndex].q_RequestAccepted == false)
            {
               orq_NotAccepted = true;
               break;
            }
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send network reset broadcast

   Use the constants provided by C_OscProtocolDriverOsyTpBase for the parameter.

   \param[in]  ou8_ResetType        Reset type (0x02: keyOffOnReset, 0x60: resetToFlashloader)

   \return
   C_NO_ERR   no problems
   C_COM      could not send request
   C_CONFIG   no dispatcher installed or broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyBroadcastEcuReset(const uint8_t ou8_ResetType) const
{
   int32_t s32_Return;

   if ((this->mpc_CanTransportProtocolBroadcast == NULL) &&
       (this->mpc_IpTransportProtocolBroadcast == NULL))
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      if (this->mpc_CanTransportProtocolBroadcast != NULL)
      {
         s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastEcuReset(ou8_ResetType);
      }
      else
      {
         s32_Return = this->mpc_IpTransportProtocolBroadcast->BroadcastNetReset(ou8_ResetType);
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast service EnterDiagnosticSession(PreProgramming)

   Sending service as broadcast. Only available for CAN broadcast.
   Only sends the request, does not wait for response.

   \return
   C_NO_ERR   no problems
   C_COM      could not send request
   C_CONFIG   no CAN dispatcher installed or broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCanBroadcastEnterPreProgrammingSession(void) const
{
   int32_t s32_Return;

   if (this->mpc_CanTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastSendEnterPreProgrammingSession();
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast service EnterDiagnosticSession(Default)

   Sending service as broadcast. Only available for CAN broadcast.
   Only sends the request, does not wait for response.

   \return
   C_NO_ERR   no problems
   C_COM      could not send request
   C_CONFIG   no CAN dispatcher installed or broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCanBroadcastEnterDefaultSession(void) const
{
   int32_t s32_Return;

   if (this->mpc_CanTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastSendEnterDefaultSession();
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read serial numbers of all devices on local bus

   \param[out]    orc_Responses           information about all nodes that sent a response
   \param[out]    orc_ExtendedResponses   information about all nodes that sent an extended response
   \return
   C_NO_ERR   no problems; zero or more responses received; data placed in orc_Responses
   C_COM      could not send request
   C_CONFIG   no dispatcher installed or broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCanBroadcastReadSerialNumber(
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> & orc_Responses,
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> & orc_ExtendedResponses)
const
{
   int32_t s32_Return;

   if (this->mpc_CanTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastReadSerialNumber(orc_Responses,
                                                                                      orc_ExtendedResponses);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the device name by service ReadDataByIdentifier

   \param[in]     orc_ServerId      Server id for communication
   \param[out]    orc_DeviceName    Result device name of service
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR   request sent, positive response received
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      error on creating temporary needed protocol
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadDeviceName(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                   std::string & orc_DeviceName, uint8_t * const opu8_NrCode)
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   orc_DeviceName = "";

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadDeviceName(orc_DeviceName, opu8_NrCode);
   }
   else
   {
      // Only necessary in case of combination of CAN and device configuration initial scan
      C_OscProtocolDriverOsyTpCan c_TpCan;
      C_OscProtocolDriverOsy c_OsyProtocol;

      // No device with this server id with openSYDE protocol exist. We need a temporary protocol.
      s32_Return = this->m_PrepareTemporaryOsyProtocol(orc_ServerId, c_OsyProtocol, c_TpCan);

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = c_OsyProtocol.OsyReadDeviceName(orc_DeviceName, opu8_NrCode);
      }
      else
      {
         s32_Return = C_COM;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the serial number by service ReadDataByIdentifier

   \param[in]     orc_ServerId          Server id for communication
   \param[out]    orc_SerialNumberExt   Read serial number
   \param[out]    opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   request sent, positive response received
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      error on creating temporary needed protocol
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadSerialNumber(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                     C_OscProtocolSerialNumber & orc_SerialNumberExt,
                                                     uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadEcuSerialNumber(orc_SerialNumberExt, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the serial number ext by service ReadDataByIdentifier

   \param[in]  orc_ServerId                        Server id for communication
   \param[out] orc_SerialNumberExt                 Read extended serial number
   \param[out] opu8_NrCode                         if != NULL and error response: negative response code

   \return
   C_NO_ERR   request sent, positive response received
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      error on creating temporary needed protocol
   C_RANGE    length of read string or serial number does not match
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadSerialNumberExt(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                        C_OscProtocolSerialNumber & orc_SerialNumberExt,
                                                        uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadEcuSerialNumberExt(orc_SerialNumberExt, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node-id of node specified by serial number

   \param[in]    orc_SerialNumber     serial number of node to address
   \param[in]    orc_NewNodeId        node ID to set
   \param[out]   opu8_NrCode          if not NULL: code of error response (if C_WARN is returned or addressed
                                      node has security activated)

   \return
   C_NO_ERR    no problems; one positive response received
   C_RANGE     invalid node ID (bus id or node-ID out of range); "0x7F" is not permitted as node ID as it's reserved for
                broadcasts
               invalid serial number
   C_WARN      negative response received
   C_COM       could not send requests
   C_CONFIG    no dispatcher installed or broadcast protocol not initialized
   C_TIMEOUT   no response within timeout (was SetNodeIdentifiersForBroadcasts() called ?)
   C_OVERFLOW  multiple responses received
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCanBroadcastSetNodeIdBySerialNumber(
   const C_OscProtocolSerialNumber & orc_SerialNumber, const C_OscProtocolDriverOsyNode & orc_NewNodeId,
   uint8_t * const opu8_NrCode)
const
{
   int32_t s32_Return;

   if (this->mpc_CanTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastSetNodeIdBySerialNumber(orc_SerialNumber,
                                                                                             orc_NewNodeId,
                                                                                             opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node-id of node specified by serial number extended

   \param[in]    orc_SerialNumber    serial number of node to address (1 to 29 bytes allowed)
   \param[in]    ou8_SubNodeId       sub node id of sub node to address (in case of a device without sub nodes: 0)
   \param[in]    orc_NewNodeId       node ID to set
   \param[out]   opu8_NrCode         if not NULL: code of error response (if C_WARN is returned or addressed
                                     node has security activated)

   \return
   C_NO_ERR    no problems; one positive response received
   C_RANGE     invalid node ID (bus id or node-ID out of range); "0x7F" is not permitted as node ID as it's reserved for
                broadcasts
               invalid serial number
   C_WARN      negative response received
   C_COM       could not send requests
   C_CONFIG    no dispatcher installed or broadcast protocol not initialized
   C_TIMEOUT   no response within timeout (was SetNodeIdentifiersForBroadcasts() called ?)
   C_OVERFLOW  multiple responses received
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCanBroadcastSetNodeIdBySerialNumberExtended(
   const C_OscProtocolSerialNumber & orc_SerialNumber, const uint8_t ou8_SubNodeId,
   const C_OscProtocolDriverOsyNode & orc_NewNodeId, uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return;

   if (this->mpc_CanTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_CanTransportProtocolBroadcast->BroadcastSetNodeIdBySerialNumberExtended(
         orc_SerialNumber,
         ou8_SubNodeId,
         orc_NewNodeId,
         opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast to get device information via ETH-TP

   \param[out]    orc_ReadDeviceInfoResults           received responses
   \param[out]    orc_ReadDeviceInfoExtendedResults   received extended responses

   \return
   C_NO_ERR    no problems; zero or more responses received; data placed in c_ReadDeviceInfoResults
   C_COM       could not send request
   C_CONFIG    no dispatcher installed or ETH broadcast protocol not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyEthBroadcastGetDeviceInformation(
   std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults> & orc_ReadDeviceInfoResults,
   std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults> & orc_ReadDeviceInfoExtendedResults)
const
{
   int32_t s32_Return;

   if (this->mpc_IpTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_IpTransportProtocolBroadcast->BroadcastGetDeviceInfo(orc_ReadDeviceInfoResults,
                                                                                  orc_ReadDeviceInfoExtendedResults);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast to set IP address via ETH-TP

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
   \param[out]   opu8_ErrorResult      if not NULL: code of error response (if C_WARN is returned or addressed
                                       node has security activated))

   orau8_DefaultGateway

   \param[out]   orau8_ResponseIp     IP address the response was received from
   \param[out]   opu8_ErrorResult     if not NULL: code of error response (if C_WARN is returned)

   \return
   C_NO_ERR    no problems; one OK response received; response IP placed in orau8_ResponseIp
   C_WARN      error response
   C_COM       could not send request
   C_CONFIG    no dispatcher installed or ETH broadcast protocol not initialized
   C_RANGE     serial number is invalid or wrong format of serial number is configured
   C_TIMEOUT   no response within timeout
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyEthBroadcastSetIpAddress(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                             const uint8_t (&orau8_NewIpAddress)[4],
                                                             const uint8_t (&orau8_NetMask)[4],
                                                             const uint8_t(&orau8_DefaultGateway)[4],
                                                             const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                             uint8_t (&orau8_ResponseIp)[4],
                                                             uint8_t * const opu8_ErrorResult) const
{
   int32_t s32_Return;

   if (this->mpc_IpTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_IpTransportProtocolBroadcast->BroadcastSetIpAddress(orc_SerialNumber,
                                                                                 orau8_NewIpAddress,
                                                                                 orau8_NetMask,
                                                                                 orau8_DefaultGateway,
                                                                                 orc_NewNodeId,
                                                                                 orau8_ResponseIp,
                                                                                 opu8_ErrorResult);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast to set IP address extended via ETH-TP

   Send broadcast to change the IP address of one specific node.
   Only the node with a specified serial number is expected to send a response and change its IP address.
   The function will return as soon as it has received one response.

   Incoming UDP responses to other services will be dumped: we are strictly handshaking here ...

   \param[in]    orc_SerialNumber         serial number of server to change IP on
   \param[in]    orau8_NewIpAddress       IP address to set
   \param[in]    orau8_NetMask            Net mask to set
   \param[in]    orau8_DefaultGateway     Default gateway to set
   \param[in]    orc_NewNodeId            New bus id and node id for the interface
   \param[in]    ou8_SubNodeId            Sub node id of node for identification in case of a multi CPU node
   \param[out]   orau8_ResponseIp         IP address the response was received from
   \param[out]   opu8_ErrorResult         if not NULL: code of error response (if C_WARN is returned or addressed
                                          node has security activated))

   \return
   C_NO_ERR    no problems; one OK response received; response IP placed in orau8_ResponseIp
   C_WARN      error response
   C_COM       could not send request
   C_CONFIG    no dispatcher installed or ETH broadcast protocol not initialized
   C_RANGE     serial number is invalid or wrong format of serial number is configured
   C_TIMEOUT   no response within timeout
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyEthBroadcastSetIpAddressExtended(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                                     const uint8_t (&orau8_NewIpAddress)[4],
                                                                     const uint8_t (&orau8_NetMask)[4],
                                                                     const uint8_t(&orau8_DefaultGateway)[4],
                                                                     const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                                     const uint8_t ou8_SubNodeId,
                                                                     uint8_t (&orau8_ResponseIp)[4],
                                                                     uint8_t * const opu8_ErrorResult) const
{
   int32_t s32_Return;

   if (this->mpc_IpTransportProtocolBroadcast == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      s32_Return = this->mpc_IpTransportProtocolBroadcast->BroadcastSetIpAddressExtended(
         orc_SerialNumber,
         orau8_NewIpAddress,
         orau8_NetMask,
         orau8_DefaultGateway,
         orc_NewNodeId,
         ou8_SubNodeId,
         orau8_ResponseIp,
         opu8_ErrorResult);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send the request programming service to one node

   \param[in]     orc_ServerId      Server id for communication

   \return
   C_NO_ERR    Received positive response
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestProgramming(const C_OscProtocolDriverOsyNode & orc_ServerId) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      // If the device is already in flashloader, the preprogramming session is needed.
      // If the device is in the application, this session request will return with an error. This error can be ignored.
      pc_ExistingProtocol->OsyDiagnosticSessionControl(C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PREPROGRAMMING,
                                                       NULL);
      s32_Return = pc_ExistingProtocol->OsyRequestProgramming();
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send the ReadActiveDiagnosticSession service to one node and report response

   \param[in]     orc_ServerId      Server id for communication
   \param[out]    oru8_SessionId    reported session ID
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    Received positive response
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadActiveDiagnosticSession(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                uint8_t & oru8_SessionId,
                                                                uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadActiveDiagnosticSession(oru8_SessionId, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send the ReadFlashBlockData service to read information about all flash blocks

   Executes the service in a loop starting from 0.
   A "requestOutOfRange" NRC is interpreted as "this is the first block not available" and will terminate the loop.

   Technically block 0 (Flashloader) should always be present. As a defensive measure we will not fail in this case.
   Instead we will return an empty list of identified blocks. So the application can decide how to deal with this
    situation.

   \param[in]     orc_ServerId      Server id for communication
   \param[out]    orc_BlockInfo     read flash block information
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    Read information for at least one block
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received (except for requestOutOfRange)
   C_TIMEOUT   Expected response not received within timeout
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadAllFlashBlockData(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                          std::vector<C_OscProtocolDriverOsy::C_FlashBlockInfo> & orc_BlockInfo,
                                                          uint8_t * const opu8_NrCode)
const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   orc_BlockInfo.resize(0);

   if (pc_ExistingProtocol != NULL)
   {
      for (uint16_t u16_Block = 0U; u16_Block <= 0xFFU; u16_Block++)
      {
         uint8_t u8_NrCode;
         C_OscProtocolDriverOsy::C_FlashBlockInfo c_BlockInfo;
         s32_Return = pc_ExistingProtocol->OsyReadFlashBlockData(static_cast<uint8_t>(u16_Block), c_BlockInfo,
                                                                 &u8_NrCode);
         if (s32_Return == C_NO_ERR)
         {
            orc_BlockInfo.push_back(c_BlockInfo);
         }
         else
         {
            if (opu8_NrCode != NULL)
            {
               (*opu8_NrCode) = u8_NrCode;
            }

            if ((s32_Return == C_WARN) && (u8_NrCode == C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE))
            {
               //no more blocks
               //done here ... not a real problem
               s32_Return = C_NO_ERR;
            }
            break;
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read various information from flashloader

   Read information from flashloader and fill in structure.

   \param[in]     orc_ServerId        Server id for communication
   \param[out]    orc_Information     read server information
   \param[out]    opu8_NrCode         if != NULL and error response: negative response code

   \return
   C_NO_ERR    Read information (placed in orc_Information)
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadInformationFromFlashloader(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                   C_OscComFlashloaderInformation & orc_Information,
                                                                   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return =
         pc_ExistingProtocol->OsyReadBootSoftwareIdentification(orc_Information.au8_FlashloaderSoftwareVersion,
                                                                opu8_NrCode);
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadApplicationSoftwareFingerprint(
            orc_Information.au8_FlashFingerprintDate, orc_Information.au8_FlashFingerprintTime,
            orc_Information.c_FlashFingerprintUserName, opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadHardwareNumber(orc_Information.u32_EcuArticleNumber, opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadHardwareVersionNumber(orc_Information.c_EcuHardwareVersionNumber,
                                                                        opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadProtocolVersion(orc_Information.au8_ProtocolVersion, opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadFlashloaderProtocolVersion(
            orc_Information.au8_FlashloaderProtocolVersion, opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadFlashCount(orc_Information.u32_FlashCount, opu8_NrCode);
      }

      // Get available flashloader features
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = pc_ExistingProtocol->OsyReadListOfFeatures(orc_Information.c_AvailableFeatures, opu8_NrCode);
      }

      if (s32_Return == C_NO_ERR)
      {
         if (orc_Information.c_AvailableFeatures.q_ExtendedSerialNumberModeImplemented == false)
         {
            s32_Return = pc_ExistingProtocol->OsyReadEcuSerialNumber(orc_Information.c_SerialNumber, opu8_NrCode);
         }
         else
         {
            s32_Return = pc_ExistingProtocol->OsyReadEcuSerialNumberExt(orc_Information.c_SerialNumber, opu8_NrCode);
         }
      }

      if ((s32_Return == C_NO_ERR) &&
          (orc_Information.c_AvailableFeatures.q_MaxNumberOfBlockLengthAvailable == true))
      {
         s32_Return = pc_ExistingProtocol->OsyReadMaxNumberOfBlockLength(orc_Information.u16_MaxNumberOfBlockLength,
                                                                         opu8_NrCode);
         if (s32_Return != C_NO_ERR)
         {
            orc_Information.u16_MaxNumberOfBlockLength = 0U;
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute CheckFlashMemoryAvailable service

   \param[in]    orc_ServerId        Server id for communication
   \param[in]    ou32_StartAddress   Start address to check for
   \param[in]    ou32_Size           Data size to check for
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyCheckFlashMemoryAvailable(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                              const uint32_t ou32_StartAddress,
                                                              const uint32_t ou32_Size,
                                                              uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyCheckFlashMemoryAvailable(ou32_StartAddress, ou32_Size, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute WriteApplicationSoftwareFingerprint service

   If a user name with more than 20 characters is passed only the first 20 will be used.

   \param[in] orc_ServerId          Server id for communication
   \param[in] orau8_Date            date of programming (yy.mm.dd)
   \param[in] orau8_Time            time of programming (hh:mm:ss)
   \param[in] orc_Username          name of operator
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyWriteApplicationSoftwareFingerprint(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                        const uint8_t (&orau8_Date)[3],
                                                                        const uint8_t (&orau8_Time)[3],
                                                                        const std::string & orc_Username,
                                                                        uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyWriteApplicationSoftwareFingerprint(orau8_Date, orau8_Time, orc_Username,
                                                                               opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute RequestDownload service

   \param[in] orc_ServerId           Server id for communication
   \param[in] ou32_StartAddress      Start address to write to
   \param[in] ou32_Size              Number of bytes we want to write to the start address
   \param[out] oru32_MaxBlockLength  Maximum number of bytes we may send in one subsequent "TransferData" service
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestDownload(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                    const uint32_t ou32_StartAddress, const uint32_t ou32_Size,
                                                    uint32_t & oru32_MaxBlockLength, uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyRequestDownload(ou32_StartAddress, ou32_Size, oru32_MaxBlockLength,
                                                           opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute RequestFileTransfer service

   \param[in] orc_ServerId           Server id for communication
   \param[in] orc_FilePath           File to write
   \param[in] ou32_FileSize          Number of bytes we want to transfer
   \param[out] oru32_MaxBlockLength  Maximum number of bytes we may send in one subsequent "TransferData" service
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found; file path too long
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestFileTransfer(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                        const std::string & orc_FilePath, const uint32_t ou32_FileSize,
                                                        uint32_t & oru32_MaxBlockLength,
                                                        uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyRequestFileTransfer(orc_FilePath, ou32_FileSize, oru32_MaxBlockLength,
                                                               opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute TransferData service

   \param[in]  orc_ServerId               Server id for communication
   \param[in]  ou8_BlockSequenceCounter   sequence counter 0-255; first black starts with 1
   \param[in]  orc_Data                   data to be programmed to flash memory
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyTransferData(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                 const uint8_t ou8_BlockSequenceCounter,
                                                 const std::vector<uint8_t> & orc_Data,
                                                 uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyTransferData(ou8_BlockSequenceCounter, orc_Data, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute TransferExitAddressBased service

   \param[in]  orc_ServerId               Server id for communication
   \param[in]  oq_SendSignatureBlockAddress  true: it's the last area of a block; send the signature address
                                             false: more areas will follow; do not send the signature address
   \param[in]  ou32_SignatureBlockAddress    address of signature block in flash covering the application/data
                                             previously programmed by service TransferData.
                                             (only used if oq_SendSignatureBlockAddress is true)
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestTransferExitAddressBased(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                    const bool oq_SendSignatureBlockAddress,
                                                                    const uint32_t ou32_SignatureBlockAddress,
                                                                    uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyRequestTransferExitAddressBased(oq_SendSignatureBlockAddress,
                                                                           ou32_SignatureBlockAddress, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute TransferExitFileBased service

   \param[in]  orc_ServerId               Server id for communication
   \param[in]  ou32_CrcOverData           CRC32 over data sent with TransferData
   \param[out] opu8_NrCode                if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestTransferExitFileBased(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                 const uint32_t ou32_CrcOverData,
                                                                 uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      uint8_t au8_Signature[8];
      //place the CRC into the first four bytes; rest is reserved
      au8_Signature[0] = static_cast<uint8_t>(ou32_CrcOverData >> 24U);
      au8_Signature[1] = static_cast<uint8_t>(ou32_CrcOverData >> 16U);
      au8_Signature[2] = static_cast<uint8_t>(ou32_CrcOverData >> 8U);
      au8_Signature[3] = static_cast<uint8_t>(ou32_CrcOverData);
      au8_Signature[4] = 0U; //reserved: set to zero
      au8_Signature[5] = 0U; //reserved: set to zero
      au8_Signature[6] = 0U; //reserved: set to zero
      au8_Signature[7] = 0U; //reserved: set to zero
      s32_Return = pc_ExistingProtocol->OsyRequestTransferExitFileBased(au8_Signature, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute ReadFileBasedTransferExitResult service

   \param[in]   orc_ServerId      Server id for communication
   \param[out]  orc_Result        read information
   \param[out]  opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR    service finished without problems
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_COM       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyRequestFileBasedTransferExitResult(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                       std::string & orc_Result,
                                                                       uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadFileBasedTransferExitResult(orc_Result, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send the reset ECU service request to one node

   \param[in]     orc_ServerId      Server id for communication
   \param[in]     ou8_ResetType     Reset type (0x02: keyOffOnReset, 0x60: resetToFlashloader)

   \return
   C_NO_ERR    Request sent
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly.
   C_NOACT     Could not put request in Tx queue
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyEcuReset(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                             const uint8_t ou8_ResetType) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyEcuReset(ou8_ResetType);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the session to preprogramming and the necessary security access to level 1

   \param[in]     orc_Protocol      Protocol object for communication
   \param[in]     oq_SessionOnly    true: only set session; don't set security access
   \param[out]    opu8_NrCode       if != NULL: negative response code

   \return
   C_NO_ERR    Session and security access set successfully
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly or
               PEM database was needed but not set.
   C_NOACT     Nodes has no openSYDE protocol
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_CHECKSUM  Security related error (something went wrong while handshaking with the server)
               Detailed error codes are logged with opu8_NrCode
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetPreProgrammingMode(C_OscProtocolDriverOsy & orc_Protocol,
                                                          const bool oq_SessionOnly, uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = this->m_SetNodeSessionId(&orc_Protocol,
                                                 C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PREPROGRAMMING,
                                                 false, opu8_NrCode);

   if ((s32_Return == C_NO_ERR) && (oq_SessionOnly == false))
   {
      s32_Return = this->m_SetNodeSecurityAccess(&orc_Protocol, 1, opu8_NrCode);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the session to preprogramming and the necessary security access to level 1

   \param[in]     orc_ServerId      Server id for communication
   \param[in]     oq_SessionOnly    true: only set session; don't set security access
   \param[out]    opu8_NrCode       if != NULL: negative response code
   \param[out]    opq_SecureAuthenticationActive  if != NULL: true: secure authentication required by server
   \param[out]    opq_TrafficEncryptionActive     if != NULL: true: traffic encryption required by server

   \return
   C_NO_ERR    Session and security access set successfully
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly or
               PEM database was needed but not set.
   C_NOACT     Nodes has no openSYDE protocol
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_CHECKSUM  Security related error (something went wrong while handshaking with the server)
               Detailed error codes are logged with opu8_NrCode
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetPreProgrammingMode(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                          const bool oq_SessionOnly, uint8_t * const opu8_NrCode,
                                                          bool * const opq_SecureAuthenticationActive,
                                                          bool * const opq_TrafficEncryptionActive)
{
   int32_t s32_Return = C_RANGE;
   bool q_Found = false;
   const uint32_t u32_ActiveNodeIndex = this->m_GetActiveIndex(orc_ServerId, q_Found);

   if (q_Found == true)
   {
      s32_Return = this->m_SetNodeSessionId(u32_ActiveNodeIndex,
                                            C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PREPROGRAMMING,
                                            false, opu8_NrCode);

      if ((s32_Return == C_NO_ERR) && (oq_SessionOnly == false))
      {
         s32_Return = this->m_SetNodeSecurityAccess(u32_ActiveNodeIndex, 1, opu8_NrCode, opq_SecureAuthenticationActive,
                                                    opq_TrafficEncryptionActive);
      }
   }
   else
   {
      // Only necessary in case of combination of CAN and device configuration initial scan
      // In case of ETH and device configuration a ETH specific version of this function exists
      C_OscProtocolDriverOsyTpCan c_TpCan;
      C_OscProtocolDriverOsy c_OsyProtocol;

      // No device with this server id with openSYDE protocol exist. We need a temporary protocol.
      s32_Return = this->m_PrepareTemporaryOsyProtocol(orc_ServerId, c_OsyProtocol, c_TpCan);

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = this->m_SetNodeSessionId(&c_OsyProtocol,
                                               C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PREPROGRAMMING,
                                               false, opu8_NrCode);

         if ((s32_Return == C_NO_ERR) && (oq_SessionOnly == false))
         {
            s32_Return = this->m_SetNodeSecurityAccess(&c_OsyProtocol, 1, opu8_NrCode, opq_SecureAuthenticationActive,
                                                       opq_TrafficEncryptionActive);
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the session to programming and the necessary security access to level 3

   \param[in]     orc_ServerId        Server id for communication
   \param[in]     opu8_SecurityLevel  Optional security level. If not set the security level 3 is used
   \param[out]    opu8_NrCode         if != NULL: negative response code

   \return
   C_NO_ERR    Session and security access set successfully
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly or
               PEM database was needed but not set.
   C_NOACT     Nodes has no openSYDE protocol
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_CHECKSUM  Security related error (something went wrong while handshaking with the server)
               Detailed error codes are logged with opu8_NrCode
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetProgrammingMode(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                       const uint8_t * const opu8_SecurityLevel,
                                                       uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   bool q_Found = false;
   const uint32_t u32_ActiveNodeIndex = this->m_GetActiveIndex(orc_ServerId, q_Found);

   if (q_Found == true)
   {
      s32_Return = this->m_SetNodeSessionId(u32_ActiveNodeIndex,
                                            C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PROGRAMMING,
                                            false, opu8_NrCode);

      if (s32_Return == C_NO_ERR)
      {
         uint8_t u8_SecurityLevel = 3U;

         if (opu8_SecurityLevel != NULL)
         {
            u8_SecurityLevel = *opu8_SecurityLevel;
         }

         s32_Return = this->m_SetNodeSecurityAccess(u32_ActiveNodeIndex, u8_SecurityLevel, opu8_NrCode);
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set specified security level

   \param[in]     orc_ServerId      Server id for communication
   \param[in]     ou8_Level         security level to set
   \param[out]    opu8_NrCode       if != NULL: negative response code

   \return
   C_NO_ERR    Session and security access set successfully
   C_RANGE     openSYDE protocol not found
   C_CONFIG    Init function was not called or not successful or protocol was not initialized properly or
               PEM database was needed but not set.
   C_NOACT     Nodes has no openSYDE protocol
   C_WARN      Error response received
   C_TIMEOUT   Expected response not received within timeout
   C_CHECKSUM  Security related error (something went wrong while handshaking with the server)
               Detailed error codes are logged with opu8_NrCode
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetSecurityLevel(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                     const uint8_t ou8_Level, uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   bool q_Found = false;
   const uint32_t u32_ActiveNodeIndex = this->m_GetActiveIndex(orc_ServerId, q_Found);

   if (q_Found == true)
   {
      s32_Return = this->m_SetNodeSecurityAccess(u32_ActiveNodeIndex, ou8_Level, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets a new bitrate on an openSYDE server

   Only for CAN

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  ou8_ChannelIndex      selected channel index
   \param[in]  ou32_Bitrate          new bitrate configuration for the server in bit/s
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR   Bitrate was set successful
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetBitrate(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                               const uint8_t ou8_ChannelIndex, const uint32_t ou32_Bitrate,
                                               uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsySetBitrate(0U, ou8_ChannelIndex, ou32_Bitrate, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set the ip address for the referenced channel to the given value

   Only for Ethernet

   \param[in]     orc_Protocol          Protocol object for communication
   \param[in]     ou8_ChannelIndex      selected channel index
   \param[in]     orau8_IpAddress       IP V4 address
   \param[in]     orau8_NetMask         IP V4 net mask
   \param[in]     orau8_DefaultGateway  IP V4 default gateway
   \param[out]    opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Bitrate was set successful
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong routine identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::h_SendOsySetIpAddressForChannel(C_OscProtocolDriverOsy & orc_Protocol,
                                                             const uint8_t ou8_ChannelIndex,
                                                             const uint8_t (&orau8_IpAddress)[4],
                                                             const uint8_t (&orau8_NetMask)[4],
                                                             const uint8_t (&orau8_DefaultGateway)[4],
                                                             uint8_t * const opu8_NrCode)
{
   return orc_Protocol.OsySetIpAddressForChannel(1U, ou8_ChannelIndex, orau8_IpAddress, orau8_NetMask,
                                                 orau8_DefaultGateway, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set the ip address for the referenced channel to the given value

   Only for Ethernet

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  ou8_ChannelIndex      selected channel index
   \param[in]  orau8_IpAddress       IP V4 address
   \param[in]  orau8_NetMask         IP V4 net mask
   \param[in]  orau8_DefaultGateway  IP V4 default gateway
   \param[out]    opu8_NrCode       if != NULL and error response: negative response code

   \return
   C_NO_ERR   Bitrate was set successful
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong routine identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetIpAddressForChannel(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                           const uint8_t ou8_ChannelIndex,
                                                           const uint8_t (&orau8_IpAddress)[4],
                                                           const uint8_t (&orau8_NetMask)[4],
                                                           const uint8_t (&orau8_DefaultGateway)[4],
                                                           uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsySetIpAddressForChannel(1U, ou8_ChannelIndex, orau8_IpAddress, orau8_NetMask,
                                                                  orau8_DefaultGateway, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets node id and bus id for specific communication channel

   \param[in]  orc_Protocol          Protocol object for communication
   \param[in]  ou8_ChannelType       selected channel type (0 equals CAN, 1 equals Ethernet)
   \param[in]  ou8_ChannelIndex      selected channel index
   \param[in]  orc_NewNodeId         new bus id (0..15) and node id (0..126) configuration for the server
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Bitrate was set successful
   C_RANGE    ou8_BusId or ou8_NodeId is out of range
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::h_SendOsySetNodeIdForChannel(C_OscProtocolDriverOsy & orc_Protocol,
                                                          const uint8_t ou8_ChannelType, const uint8_t ou8_ChannelIndex,
                                                          const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                          uint8_t * const opu8_NrCode)
{
   return orc_Protocol.OsySetNodeIdForChannel(ou8_ChannelType, ou8_ChannelIndex,
                                              orc_NewNodeId, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets node id and bus id for specific communication channel

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  ou8_ChannelType       selected channel type (0 equals CAN, 1 equals Ethernet)
   \param[in]  ou8_ChannelIndex      selected channel index
   \param[in]  orc_NewNodeId         new bus id (0..15) and node id (0..126) configuration for the server
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Bitrate was set successful
   C_RANGE    openSYDE protocol not found
              ou8_BusId or ou8_NodeId is out of range
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsySetNodeIdForChannel(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                        const uint8_t ou8_ChannelType, const uint8_t ou8_ChannelIndex,
                                                        const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                        uint8_t * const opu8_NrCode)
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsySetNodeIdForChannel(ou8_ChannelType, ou8_ChannelIndex,
                                                               orc_NewNodeId, opu8_NrCode);
   }
   else
   {
      // Only necessary in case of combination of CAN and device configuration initial scan
      // In case of ETH and device configuration a ETH specific version of this function exists
      C_OscProtocolDriverOsyTpCan c_TpCan;
      C_OscProtocolDriverOsy c_OsyProtocol;

      // No device with this server id with openSYDE protocol exist. We need a temporary protocol.
      s32_Return = this->m_PrepareTemporaryOsyProtocol(orc_ServerId, c_OsyProtocol, c_TpCan);

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = c_OsyProtocol.OsySetNodeIdForChannel(ou8_ChannelType, ou8_ChannelIndex,
                                                           orc_NewNodeId, opu8_NrCode);
      }
      else
      {
         s32_Return = C_COM;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the list of supported features

   \param[in]  orc_ServerId          Server id for communication
   \param[out] orc_ListOfFeatures    List of server features
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   List of features was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadListOfFeatures(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                       C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ListOfFeatures,
                                                       uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadListOfFeatures(orc_ListOfFeatures, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the authentication certificate serial number

   Serial number is a byte array with maximum length of 20 Bytes.

   \param[in]  orc_ServerId          Server id for communication
   \param[out] orc_SerialNumber      read certificate serial number
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Certificate serial number was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
   C_RANGE    count of read bytes does not match the expectation (more than 20 bytes received)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadAuthenticationCertificateSerialNumber(
   const C_OscProtocolDriverOsyNode & orc_ServerId, std::vector<uint8_t> & orc_SerialNumber,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadAuthenticationCertificateSerialNumber(orc_SerialNumber, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the authentication certificate serial number for security access level 7

   Serial number is a byte array with maximum length of 20 Bytes.

   \param[in]  orc_ServerId          Server id for communication
   \param[out] orc_SerialNumber      read certificate serial number
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Certificate serial number was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
   C_RANGE    count of read bytes does not match the expectation (more than 20 bytes received)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadAuthenticationCertificateSerialNumberL7(
   const C_OscProtocolDriverOsyNode & orc_ServerId, std::vector<uint8_t> & orc_SerialNumber,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadAuthenticationCertificateSerialNumberL7(orc_SerialNumber, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Writes the public security authentication key with its certificate serial number

   Public key modulus is a byte array with a length of 128 Bytes.
   Public key exponent is a byte array with a length of 1 to 4 Bytes containing the exponent (high byte first).
   Serial number is a byte array with maximum length of 20 Bytes.

   \param[in]  orc_ServerId                Server id for communication
   \param[in]  orc_PublicKeyModulus        new public key modulus
   \param[in]  orc_PublicKeyExponent       new public key exponent
   \param[in]  orc_CertificateSerialNumber new certificate serial number
   \param[out] opu8_NrCode                 if != NULL and error response: negative response code

   \return
   C_NO_ERR   Certificate serial number was write successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
   C_RANGE    parameter orc_SerialNumber, orc_PublicKeyModulus, orc_PublicKeyExponent does not match the size
               expectation
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyWriteSecurityAuthenticationKey(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                   const std::vector<uint8_t> & orc_PublicKeyModulus,
                                                                   const std::vector<uint8_t> & orc_PublicKeyExponent,
                                                                   const std::vector<uint8_t> & orc_CertificateSerialNumber,
                                                                   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyWriteSecurityAuthenticationKey(orc_PublicKeyModulus, orc_PublicKeyExponent,
                                                                          orc_CertificateSerialNumber, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the security authentication status

   \param[in]  orc_ServerId                 Server id for communication
   \param[out] orq_SecurityOn               read flag if security is on or off on the node
   \param[out] oru8_SecurityAlgorithm       read used security algorithm of the node
   \param[out] opu8_NrCode                  if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadSecurityAuthenticationActivation(
   const C_OscProtocolDriverOsyNode & orc_ServerId, bool & orq_SecurityOn, uint8_t & oru8_SecurityAlgorithm,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadSecurityAuthenticationActivation(orq_SecurityOn, oru8_SecurityAlgorithm,
                                                                                opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Writes the security authentication status

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  oq_SecurityOn         new flag if security is on or off on the node
   \param[in]  ou8_SecurityAlgorithm new used security algorithm of the node
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was write successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyWriteSecurityAuthenticationActivation(
   const C_OscProtocolDriverOsyNode & orc_ServerId, const bool oq_SecurityOn, const uint8_t ou8_SecurityAlgorithm,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyWriteSecurityAuthenticationActivation(oq_SecurityOn, ou8_SecurityAlgorithm,
                                                                                 opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the security traffic encryption status

   \param[in]  orc_ServerId                 Server id for communication
   \param[out] orq_SecurityOn               read flag if security is on or off on the node
   \param[out] oru8_SecurityAlgorithm       read used security algorithm of the node
   \param[out] opu8_NrCode                  if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadSecurityTrafficEncryptionActivation(
   const C_OscProtocolDriverOsyNode & orc_ServerId, bool & orq_SecurityOn, uint8_t & oru8_SecurityAlgorithm,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadSecurityTrafficEncryptionActivation(orq_SecurityOn,
                                                                                   oru8_SecurityAlgorithm,
                                                                                   opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Writes the security traffic encryption status

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  oq_SecurityOn         new flag if security is on or off on the node
   \param[in]  ou8_SecurityAlgorithm new used security algorithm of the node
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was write successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyWriteSecurityTrafficEncryptionActivation(
   const C_OscProtocolDriverOsyNode & orc_ServerId, const bool oq_SecurityOn, const uint8_t ou8_SecurityAlgorithm,
   uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyWriteSecurityTrafficEncryptionActivation(oq_SecurityOn,
                                                                                    ou8_SecurityAlgorithm,
                                                                                    opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the debugger enabled flag

   \param[in]  orc_ServerId                 Server id for communication
   \param[out] orq_DebuggerEnabled          read flag if debugger is on or off on the node
   \param[out] opu8_NrCode                  if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was read successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyReadDebuggerEnabled(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                        bool & orq_DebuggerEnabled, uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyReadDebuggerEnabled(orq_DebuggerEnabled, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Writes the debugger enabled flag

   \param[in]  orc_ServerId          Server id for communication
   \param[in]  oq_DebuggerEnabled    new flag if debugger is on or off on the node
   \param[out] opu8_NrCode           if != NULL and error response: negative response code

   \return
   C_NO_ERR   Security activation was write successfully
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response (negative response code placed in *opu8_NrCode)
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyWriteDebuggerEnabled(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                         const bool oq_DebuggerEnabled,
                                                         uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyWriteDebuggerEnabled(oq_DebuggerEnabled, opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Call factory mode master reset on an openSYDE server

   \param[in]   orc_ServerId  Server id for communication
   \param[out]  opu8_NrCode   if != NULL and error response: negative response code

   \return
   C_NO_ERR   factory mode master reset was successful
   C_RANGE    openSYDE protocol not found
   C_TIMEOUT  expected response not received within timeout
   C_NOACT    could not put request in Tx queue ...
   C_CONFIG   no transport protocol installed
   C_WARN     error response
   C_RD_WR    unexpected content in response (here: wrong data identifier ID)
   C_COM      communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::SendOsyFactoryModeMasterReset(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                           uint8_t * const opu8_NrCode) const
{
   int32_t s32_Return = C_RANGE;
   C_OscProtocolDriverOsy * const pc_ExistingProtocol = this->m_GetOsyProtocol(orc_ServerId);

   if (pc_ExistingProtocol != NULL)
   {
      s32_Return = pc_ExistingProtocol->OsyFactoryMode(C_OscProtocolDriverOsy::hu8_OSY_FACTORY_MODE_MASTER_RESET,
                                                       opu8_NrCode);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Prepare for shutting down class

   To be called by child classes on shutdown, before they destroy all owned class instances
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComDriverFlash::PrepareForDestructionFlash(void)
{
   this->PrepareForDestruction();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the information about the routing configuration

   \param[out]    ore_Mode       Needed routing mode

   \return
   true     Routing is necessary
   false    Routing is not necessary
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComDriverFlash::m_GetRoutingMode(C_OscRoutingCalculation::E_Mode & ore_Mode) const
{
   if (this->mq_UpdateRoutingMode == true)
   {
      ore_Mode = C_OscRoutingCalculation::eUPDATE;
   }
   else
   {
      ore_Mode = C_OscRoutingCalculation::eROUTING_CHECK;
   }

   return this->mq_RoutingActive;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns needed session ID for the current routing mode

   \return
   Session ID for flashloader
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscComDriverFlash::m_GetRoutingSessionId(void) const
{
   return C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PREPROGRAMMING;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks if the routing for a not openSYDE server is necessary

   \param[in]     orc_Node                             Current node

   \return
   true    Specific server and legacy routing necessary
   false   No specific server and legacy routing necessary
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComDriverFlash::m_IsRoutingSpecificNecessary(const C_OscNode & orc_Node) const
{
   (void)orc_Node;
   return false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Prepares the routing for STW flashloader protocol

   long description of function within several lines

   \param[in]     ou32_ActiveNode                      Active node index of vector mc_ActiveNodes
   \param[in]     opc_Node                             Pointer to current node
   \param[in]     orc_LastNodeOfRouting                The last node in the routing chain before the final target server
   \param[in]     opc_ProtocolOsyOfLastNodeOfRouting   The protocol of the last node
   \param[out]    oppc_RoutingDispatcher               The legacy routing dispatcher

   \return
   C_NO_ERR    Specific server necessary
   C_NOACT     No specific server necessary
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::m_StartRoutingSpecific(const uint32_t ou32_ActiveNode, const C_OscNode * const opc_Node,
                                                    const C_OscRoutingRoutePoint & orc_LastNodeOfRouting,
                                                    C_OscProtocolDriverOsy * const opc_ProtocolOsyOfLastNodeOfRouting,
                                                    C_OscCanDispatcherOsyRouter ** const oppc_RoutingDispatcher)
{
   (void)ou32_ActiveNode;
   (void)opc_Node;
   (void)orc_LastNodeOfRouting;
   (void)opc_ProtocolOsyOfLastNodeOfRouting;
   (void)oppc_RoutingDispatcher;
   return C_NOACT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Stops the specific routing configuration for one specific node

   \param[in]     ou32_ActiveNode                      Active node index of vector mc_ActiveNodes
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComDriverFlash::m_StopRoutingSpecific(const uint32_t ou32_ActiveNode)
{
   C_OscComDriverProtocol::m_StopRoutingSpecific(ou32_ActiveNode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks if the interface has relevant functions activated

   In this case routing and in case of update specific routing the update functionality

   \param[in]     orc_ComItfSettings         Interface configuration

   \return
   true     Interface has relevant functions activated and is connected
   false    Interface has no relevant functions activated or is not connected
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComDriverFlash::m_CheckInterfaceForFunctions(const C_OscNodeComInterfaceSettings & orc_ComItfSettings) const
{
   bool q_Return = false;

   if ((orc_ComItfSettings.GetBusConnected() == true) &&
       ((orc_ComItfSettings.q_IsRoutingEnabled == true) ||
        ((orc_ComItfSettings.q_IsUpdateEnabled == true) || (this->mq_UpdateRoutingMode == false))))
   {
      q_Return = true;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize flash protocols

   The functions fills the vector mc_OsyProtocols of the base class too.

   \return
   C_NO_ERR   Operation success
   C_CONFIG   Invalid initialization
   C_OVERFLOW Unknown diagnostic server for at least one node or invalid node identifier was set in diagnostic protocol
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::m_InitFlashProtocol(void)
{
   int32_t s32_Return = C_NO_ERR;
   const uint32_t u32_ActiveNodeCount = this->m_GetActiveNodeCount();

   if ((this->mc_TransportProtocols.size() >= u32_ActiveNodeCount) &&
       (this->mc_ServerIds.size() == u32_ActiveNodeCount))
   {
      uint32_t u32_ActiveNodeCounter;

      //Init protocol driver
      // The last protocol is for broadcasts
      this->mc_OsyProtocols.resize(u32_ActiveNodeCount, NULL);

      for (u32_ActiveNodeCounter = 0U; u32_ActiveNodeCounter < this->mc_ActiveNodesIndexes.size();
           ++u32_ActiveNodeCounter)
      {
         if (this->mc_ActiveNodesIndexes[u32_ActiveNodeCounter] < this->mpc_SysDef->c_Nodes.size())
         {
            const C_OscNode * const pc_Node =
               &this->mpc_SysDef->c_Nodes[this->mc_ActiveNodesIndexes[u32_ActiveNodeCounter]];
            C_OscProtocolDriverOsy * pc_ProtocolOsy;

            switch (pc_Node->c_Properties.e_FlashLoader)
            {
            case C_OscNodeProperties::eFL_OPEN_SYDE:
               pc_ProtocolOsy = new C_OscProtocolDriverOsy();
               pc_ProtocolOsy->InitializeHandleWaitTime(&C_OscComDriverFlash::mh_HandleWaitTime, this);
               s32_Return = pc_ProtocolOsy->SetTransportProtocol(this->mc_TransportProtocols[u32_ActiveNodeCounter]);
               if (s32_Return == C_NO_ERR)
               {
                  s32_Return = pc_ProtocolOsy->SetNodeIdentifiers(this->GetClientId(),
                                                                  this->mc_ServerIds[u32_ActiveNodeCounter]);
                  if (s32_Return != C_NO_ERR)
                  {
                     //Invalid configuration = programming error
                     s32_Return = C_OVERFLOW;
                  }
               }
               else
               {
                  //Invalid configuration = programming error
                  s32_Return = C_OVERFLOW;
               }
               this->mc_OsyProtocols[u32_ActiveNodeCounter] = pc_ProtocolOsy;
               break;
            case C_OscNodeProperties::eFL_NONE:
            default:
               s32_Return = C_OVERFLOW;
               break;
            }
         }
         else
         {
            s32_Return = C_CONFIG;
         }

         if (s32_Return != C_NO_ERR)
         {
            break;
         }
      }
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::m_PrepareTemporaryOsyProtocol(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                           C_OscProtocolDriverOsy & orc_OsyProtocol,
                                                           C_OscProtocolDriverOsyTpCan & orc_CanTransportProtocol)
{
   int32_t s32_Return;

   s32_Return = orc_CanTransportProtocol.SetNodeIdentifiers(this->GetClientId(), orc_ServerId);
   if (s32_Return == C_NO_ERR)
   {
      s32_Return = orc_CanTransportProtocol.SetDispatcher(this->m_GetCanDispatcher());

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = orc_OsyProtocol.SetTransportProtocol(&orc_CanTransportProtocol);
         if (s32_Return == C_NO_ERR)
         {
            s32_Return = orc_OsyProtocol.SetNodeIdentifiers(this->GetClientId(), orc_ServerId);
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the minimum Flashloader reset wait time in ms for a specific node

   Default minimum value is 500 ms independent of the type.

   \param[in]   oe_Type             Type of minimum flashloader reset wait time
   \param[in]   ou32_NodeIndex      Node index to get the configured wait time
   \param[out]  oru32_TimeValue     Time in ms the node need at least to get from application to the Flashloader or
                                    from Flashloader to Flashloader

   \retval   C_NO_ERR   Time returned
   \retval   C_RANGE    Node with orc_ServerId does not exist or is not active
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComDriverFlash::m_GetMinimumFlashloaderResetWaitTime(
   const C_OscComDriverFlash::E_MinimumFlashloaderResetWaitTimeType oe_Type, const uint32_t ou32_NodeIndex,
   uint32_t & oru32_TimeValue) const
{
   int32_t s32_Return = C_RANGE;

   if (ou32_NodeIndex < this->mpc_SysDef->c_Nodes.size())
   {
      const C_OscNode & rc_Node = this->mpc_SysDef->c_Nodes[ou32_NodeIndex];

      tgl_assert(rc_Node.pc_DeviceDefinition != NULL);
      if (rc_Node.pc_DeviceDefinition != NULL)
      {
         s32_Return = C_NO_ERR;

         tgl_assert(rc_Node.u32_SubDeviceIndex < rc_Node.pc_DeviceDefinition->c_SubDevices.size());
         switch (oe_Type)
         {
         case C_OscComDriverFlash::eNO_CHANGES_CAN:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeNoChangesCan;
            break;
         case C_OscComDriverFlash::eNO_CHANGES_ETHERNET:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeNoChangesEthernet;
            break;
         case C_OscComDriverFlash::eNO_FUNDAMENTAL_COM_CHANGES_CAN:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeNoFundamentalChangesCan;
            break;
         case C_OscComDriverFlash::eNO_FUNDAMENTAL_COM_CHANGES_ETHERNET:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet;
            break;
         case C_OscComDriverFlash::eFUNDAMENTAL_COM_CHANGES_CAN:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeFundamentalChangesCan;
            break;
         case C_OscComDriverFlash::eFUNDAMENTAL_COM_CHANGES_ETHERNET:
            oru32_TimeValue =
               rc_Node.pc_DeviceDefinition->c_SubDevices[rc_Node.u32_SubDeviceIndex].
               u32_FlashloaderResetWaitTimeFundamentalChangesEthernet;
            break;
         default:
            tgl_assert(false);
            break;
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  In C_OscProtocolDriverOsy registered function for handling long waiting times

   See m_HandleWaitTime for detailed description

   \param[in]     opv_Instance     Pointer to the instance of C_OscComDriverFlash
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComDriverFlash::mh_HandleWaitTime(void * const opv_Instance)
{
   //lint -e{9079}  This class is the only one which registers itself at the caller of this function. It must match.
   C_OscComDriverFlash * const pc_ComDriver = reinterpret_cast<C_OscComDriverFlash *>(opv_Instance);

   tgl_assert(pc_ComDriver != NULL);
   if (pc_ComDriver != NULL)
   {
      pc_ComDriver->m_HandleWaitTime();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  In C_OscProtocolDriverOsy registered function for handling long waiting times

   For example erasing of flash can have a long timeout time. It can be longer than five seconds.
   An active CAN routing would be stopped of the server by the session timeout. This function sends
   the tester present to the last CAN router node on the route to keep the route alive

   \param[in]     opv_Instance     Pointer to the instance of C_OscComDriverFlash
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComDriverFlash::m_HandleWaitTime(void)
{
   if (this->mq_Initialized == true)
   {
      uint32_t u32_Counter;

      for (u32_Counter = 0U; u32_Counter < this->mc_ActiveNodesLastCanRouters.size(); ++u32_Counter)
      {
         const uint32_t u32_ActiveNodeRouter = this->mc_ActiveNodesLastCanRouters[u32_Counter];

         tgl_assert(u32_ActiveNodeRouter < this->mc_OsyProtocols.size());
         if (u32_ActiveNodeRouter < this->mc_OsyProtocols.size())
         {
            C_OscProtocolDriverOsy * const pc_ProtocolOsy = this->mc_OsyProtocols[u32_ActiveNodeRouter];
            if (pc_ProtocolOsy != NULL)
            {
               // Send tester present message without expecting a response
               const int32_t s32_Return = pc_ProtocolOsy->OsyTesterPresent(1U);

               if (s32_Return != C_NO_ERR)
               {
                  osc_write_log_error("Sending Tester Present", "Sending Tester Present failed with error code: " +
                                      C_OscLoggingHandler::h_StwError(s32_Return));
               }
               else
               {
                  osc_write_log_info("Sending Tester Present", "Tester Present sent successfully.");
               }
            }
         }
      }
   }
}
