//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Core communication driver for flashloader protocols (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSYCOMDRIVERFLASH_HPP
#define C_OSYCOMDRIVERFLASH_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "stwtypes.hpp"

#include "C_OscProtocolSerialNumber.hpp"
#include "C_OscComDriverProtocol.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscComFlashloaderInformation.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscComDriverFlash :
   public C_OscComDriverProtocol
{
public:
   //Description of types see C_OscDeviceDefinition::u32_FlashloaderResetWaitTimeXXX
   enum E_MinimumFlashloaderResetWaitTimeType
   {
      eNO_CHANGES_CAN = 0,
      eNO_CHANGES_ETHERNET,
      eNO_FUNDAMENTAL_COM_CHANGES_CAN,
      eNO_FUNDAMENTAL_COM_CHANGES_ETHERNET,
      eFUNDAMENTAL_COM_CHANGES_CAN,
      eFUNDAMENTAL_COM_CHANGES_ETHERNET
   };

   C_OscComDriverFlash(const bool oq_RoutingActive, const bool oq_UpdateRoutingMode);
   virtual ~C_OscComDriverFlash(void);

   virtual std::error_code Init(const C_OscSystemDefinition & orc_SystemDefinition, const uint32_t ou32_ActiveBusIndex,
                                const std::vector<uint8_t> & orc_ActiveNodes,
                                stw::can::C_CanDispatcher * const opc_CanDispatcher,
                                C_OscIpDispatcher * const opc_IpDispatcher,
                                C_OscSecurityPemDatabase * const opc_SecurityPemDb);

   std::error_code InitCanAndSetCanBitrate(const uint32_t ou32_Bitrate);

   // openSYDE protocol configuration
   std::error_code OsySetPollingTimeout(const C_OscProtocolDriverOsyNode & orc_ServerId, const uint32_t ou32_TimeoutMs
                                        ) const;
   std::error_code OsyResetPollingTimeout(const C_OscProtocolDriverOsyNode & orc_ServerId) const;

   uint32_t GetMinimumFlashloaderResetWaitTime(const E_MinimumFlashloaderResetWaitTimeType oe_Type) const;
   std::error_code GetMinimumFlashloaderResetWaitTime(const E_MinimumFlashloaderResetWaitTimeType oe_Type,
                                                      const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                      uint32_t & oru32_TimeValue) const;

   // ETH specific functions for connect when no protocol and/or IP address is initialized
   std::error_code EthConnectNode(const C_OscProtocolDriverOsyNode & orc_ServerId, const uint8_t (&orau8_IpAddress)[4],
                                  C_OscProtocolDriverOsy & orc_TemporaryProtocol, C_OscProtocolDriverOsyTpIp & orc_TpIp
                                  );
   static std::error_code h_EthDisconnectNode(C_OscProtocolDriverOsy & orc_TemporaryProtocol);

   // openSYDE Services
   std::error_code SendOsyBroadcastRequestProgramming(bool & orq_NotAccepted) const;
   std::error_code SendOsyBroadcastEcuReset(const uint8_t ou8_ResetType) const;
   std::error_code SendOsyCanBroadcastEnterPreProgrammingSession(void) const;
   std::error_code SendOsyCanBroadcastEnterDefaultSession(void) const;
   std::error_code SendOsyCanBroadcastReadSerialNumber(
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> & orc_Responses,
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> & orc_ExtendedResponses)
   const;
   std::error_code SendOsyCanBroadcastSetNodeIdBySerialNumber(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                              const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                              uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyCanBroadcastSetNodeIdBySerialNumberExtended(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                                      const uint8_t ou8_SubNodeId,
                                                                      const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                                      uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyEthBroadcastGetDeviceInformation(
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults> & orc_ReadDeviceInfoResults,
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults> & orc_ReadDeviceInfoExtendedResults)
   const;
   std::error_code SendOsyEthBroadcastSetIpAddress(const C_OscProtocolSerialNumber &orc_SerialNumber,
                                                   const uint8_t(&orau8_NewIpAddress)[4],
                                                   const uint8_t(&orau8_NetMask)[4],
                                                   const uint8_t(&orau8_DefaultGateway)[4],
                                                   const C_OscProtocolDriverOsyNode &orc_NewNodeId,
                                                   uint8_t(&orau8_ResponseIp)[4],
                                                   uint8_t * const opu8_ErrorResult = nullptr) const;
   std::error_code SendOsyEthBroadcastSetIpAddressExtended(const C_OscProtocolSerialNumber &orc_SerialNumber,
                                                           const uint8_t(&orau8_NewIpAddress)[4],
                                                           const uint8_t(&orau8_NetMask)[4],
                                                           const uint8_t(&orau8_DefaultGateway)[4],
                                                           const C_OscProtocolDriverOsyNode &orc_NewNodeId,
                                                           const uint8_t ou8_SubNodeId,
                                                           uint8_t(&orau8_ResponseIp)[4],
                                                           uint8_t * const opu8_ErrorResult = nullptr) const;

   std::error_code SendOsyReadDeviceName(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                         std::string & orc_DeviceName, uint8_t * const opu8_NrCode = nullptr);
   std::error_code SendOsyReadSerialNumber(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                           C_OscProtocolSerialNumber & orc_SerialNumberExt,
                                           uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadSerialNumberExt(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                              C_OscProtocolSerialNumber & orc_SerialNumberExt,
                                              uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadActiveDiagnosticSession(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                      uint8_t & oru8_SessionId, uint8_t * const opu8_NrCode = nullptr
                                                      ) const;
   std::error_code SendOsyReadAllFlashBlockData(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                std::vector<C_OscProtocolDriverOsy::C_FlashBlockInfo> & orc_BlockInfo,
                                                uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadInformationFromFlashloader(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                         C_OscComFlashloaderInformation & orc_Information,
                                                         uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyCheckFlashMemoryAvailable(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                    const uint32_t ou32_StartAddress, const uint32_t ou32_Size,
                                                    uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyWriteApplicationSoftwareFingerprint(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                              const uint8_t (&orau8_Date)[3],
                                                              const uint8_t (&orau8_Time)[3],
                                                              const std::string & orc_Username,
                                                              uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyRequestDownload(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                          const uint32_t ou32_StartAddress, const uint32_t ou32_Size,
                                          uint32_t & oru32_MaxBlockLength, uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyRequestFileTransfer(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                              const std::string & orc_FilePath, const uint32_t ou32_FileSize,
                                              uint32_t & oru32_MaxBlockLength, uint8_t * const opu8_NrCode = nullptr
                                              ) const;

   std::error_code SendOsyTransferData(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                       const uint8_t ou8_BlockSequenceCounter, const std::vector<uint8_t> & orc_Data,
                                       uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyRequestTransferExitAddressBased(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                          const bool oq_SendSignatureBlockAddress,
                                                          const uint32_t ou32_SignatureBlockAddress,
                                                          uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyRequestTransferExitFileBased(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                       const uint32_t ou32_CrcOverData,
                                                       uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyRequestFileBasedTransferExitResult(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                             std::string & orc_Result,
                                                             uint8_t * const opu8_NrCode = nullptr) const;

   std::error_code SendOsyRequestProgramming(const C_OscProtocolDriverOsyNode & orc_ServerId) const;
   std::error_code SendOsyEcuReset(const C_OscProtocolDriverOsyNode & orc_ServerId, const uint8_t ou8_ResetType) const;
   std::error_code SendOsySetPreProgrammingMode(C_OscProtocolDriverOsy & orc_Protocol, const bool oq_SessionOnly,
                                                uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsySetPreProgrammingMode(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                const bool oq_SessionOnly, uint8_t * const opu8_NrCode = nullptr,
                                                bool * const opq_SecureAuthenticationActive = nullptr,
                                                bool * const opq_TrafficEncryptionActive = nullptr);
   std::error_code SendOsySetProgrammingMode(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                             const uint8_t * const opu8_SecurityLevel = nullptr,
                                             uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsySetSecurityLevel(const C_OscProtocolDriverOsyNode & orc_ServerId, const uint8_t ou8_Level,
                                           uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsySetBitrate(const C_OscProtocolDriverOsyNode & orc_ServerId, const uint8_t ou8_ChannelIndex,
                                     const uint32_t ou32_Bitrate, uint8_t * const opu8_NrCode = nullptr) const;
   static std::error_code h_SendOsySetIpAddressForChannel(C_OscProtocolDriverOsy & orc_Protocol,
                                                          const uint8_t ou8_ChannelIndex,
                                                          const uint8_t (&orau8_IpAddress)[4],
                                                          const uint8_t (&orau8_NetMask)[4],
                                                          const uint8_t (&orau8_DefaultGateway)[4],
                                                          uint8_t * const opu8_NrCode = nullptr);
   std::error_code SendOsySetIpAddressForChannel(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                 const uint8_t ou8_ChannelIndex, const uint8_t (&orau8_IpAddress)[4],
                                                 const uint8_t (&orau8_NetMask)[4],
                                                 const uint8_t (&orau8_DefaultGateway)[4],
                                                 uint8_t * const opu8_NrCode = nullptr) const;
   static std::error_code h_SendOsySetNodeIdForChannel(C_OscProtocolDriverOsy & orc_Protocol,
                                                       const uint8_t ou8_ChannelType, const uint8_t ou8_ChannelIndex,
                                                       const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                                       uint8_t * const opu8_NrCode = nullptr);
   std::error_code SendOsySetNodeIdForChannel(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                              const uint8_t ou8_ChannelType, const uint8_t ou8_ChannelIndex,
                                              const C_OscProtocolDriverOsyNode & orc_NewNodeId,
                                              uint8_t * const opu8_NrCode = nullptr);

   std::error_code SendOsyReadListOfFeatures(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                             C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ListOfFeatures,
                                             uint8_t * const opu8_NrCode = nullptr) const;

   std::error_code SendOsyReadAuthenticationCertificateSerialNumber(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                    std::vector<uint8_t> & orc_SerialNumber,
                                                                    uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadAuthenticationCertificateSerialNumberL7(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                      std::vector<uint8_t> & orc_SerialNumber,
                                                                      uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyWriteSecurityAuthenticationKey(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                         const std::vector<uint8_t> & orc_PublicKeyModulus,
                                                         const std::vector<uint8_t> & orc_PublicKeyExponent,
                                                         const std::vector<uint8_t> & orc_CertificateSerialNumber,
                                                         uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadSecurityAuthenticationActivation(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                               bool & orq_SecurityOn, uint8_t & oru8_SecurityAlgorithm,
                                                               uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyWriteSecurityAuthenticationActivation(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                const bool oq_SecurityOn,
                                                                const uint8_t ou8_SecurityAlgorithm,
                                                                uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadSecurityTrafficEncryptionActivation(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                  bool & orq_SecurityOn,
                                                                  uint8_t & oru8_SecurityAlgorithm,
                                                                  uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyWriteSecurityTrafficEncryptionActivation(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                                   const bool oq_SecurityOn,
                                                                   const uint8_t ou8_SecurityAlgorithm,
                                                                   uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyReadDebuggerEnabled(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                              bool & orq_DebuggerEnabled, uint8_t * const opu8_NrCode = nullptr) const;
   std::error_code SendOsyWriteDebuggerEnabled(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                               const bool oq_DebuggerEnabled, uint8_t * const opu8_NrCode = nullptr
                                               ) const;
   std::error_code SendOsyFactoryModeMasterReset(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                 uint8_t * const opu8_NrCode = nullptr) const;

   void PrepareForDestructionFlash(void);

protected:
   virtual bool m_GetRoutingMode(C_OscRoutingCalculation::E_Mode & ore_Mode) const;
   virtual uint8_t m_GetRoutingSessionId(void) const;

   virtual bool m_IsRoutingSpecificNecessary(const C_OscNode & orc_Node) const;
   virtual std::error_code m_StartRoutingSpecific(const uint32_t ou32_ActiveNode, const C_OscNode * const opc_Node,
                                                  const C_OscRoutingRoutePoint & orc_LastNodeOfRouting,
                                                  C_OscProtocolDriverOsy * const opc_ProtocolOsyOfLastNodeOfRouting,
                                                  C_OscCanDispatcherOsyRouter ** const oppc_RoutingDispatcher);
   virtual void m_StopRoutingSpecific(const uint32_t ou32_ActiveNode);
   virtual bool m_CheckInterfaceForFunctions(const C_OscNodeComInterfaceSettings & orc_ComItfSettings) const;

private:
   //Avoid call
   C_OscComDriverFlash(const C_OscComDriverFlash &);
   C_OscComDriverFlash & operator =(const C_OscComDriverFlash &); //lint !e1511 //we want to hide the base function

   std::error_code m_InitFlashProtocol(void);

   std::error_code m_PrepareTemporaryOsyProtocol(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                                 C_OscProtocolDriverOsy & orc_OsyProtocol,
                                                 C_OscProtocolDriverOsyTpCan & orc_CanTransportProtocol);
   std::error_code m_GetMinimumFlashloaderResetWaitTime(const E_MinimumFlashloaderResetWaitTimeType oe_Type,
                                                        const uint32_t ou32_NodeIndex, uint32_t & oru32_TimeValue
                                                        ) const;

   static void mh_HandleWaitTime(void * const opv_Instance);
   void m_HandleWaitTime(void);

   const bool mq_RoutingActive;
   const bool mq_UpdateRoutingMode;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
