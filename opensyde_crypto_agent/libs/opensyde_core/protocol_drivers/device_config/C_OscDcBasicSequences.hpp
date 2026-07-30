//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for simple device configuration sequence
   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDCBASICSEQUENCES_HPP
#define C_OSCDCBASICSEQUENCES_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_SclString.hpp"
#include "C_CanDispatcher.hpp"
#include "C_OscIpDispatcher.hpp"
#include "C_OscProtocolDriverOsyTpCan.hpp"
#include "C_OscProtocolDriverOsyTpIp.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscDcDeviceInformation.hpp"
#include "C_OscProtocolSerialNumber.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDcBasicSequences
{
public:
   C_OscDcBasicSequences(void);
   virtual ~C_OscDcBasicSequences(void);

   int32_t Init(stw::can::C_CanDispatcher * const opc_CanDispatcher, C_OscIpDispatcher * const opc_IpDispatcher = NULL);
   int32_t ScanEnterFlashloader(const uint32_t ou32_FlashloaderResetWaitTime);
   int32_t ScanGetInfo(void);
   int32_t ResetSystem(void);
   int32_t ConfigureDevice(const uint8_t ou8_CurrentNodeId, const uint8_t ou8_NewNodeId, const uint32_t ou32_Bitrate,
                           const uint8_t ou8_InterfaceIndex);
   int32_t ConfigureDeviceBySerialNumber(const stw::opensyde_core::C_OscProtocolSerialNumber & orc_SerialNumber,
                                         const uint8_t ou8_NewNodeId);

   static stw::scl::C_SclString h_DevicesInfoToString(
      const std::vector<C_OscDcDeviceInformation> & orc_DeviceInfoResult, const bool oq_SecurityFeatureUsed,
      const bool oq_PrintDetailedSnInfo = false);

   void PrepareForDestruction(void);

protected:
   virtual void m_ReportProgress(const int32_t os32_Result, const stw::scl::C_SclString & orc_Information);
   virtual void m_ReportDevicesInfoRead(const std::vector<C_OscDcDeviceInformation> & orc_DeviceInfoResult,
                                        const bool oq_SecurityFeatureUsed);

private:
   // driver instances:
   stw::can::C_CanDispatcher * mpc_CanDispatcher;
   C_OscIpDispatcher * mpc_IpDispatcher;
   C_OscProtocolDriverOsyTpCan * mpc_TpCan;
   C_OscProtocolDriverOsyTpIp * mpc_TpIp;
   C_OscProtocolDriverOsy mc_OsyProtocol;

   // Transport protocol broadcast wrappers (handle CAN/IP distinction internally):
   int32_t m_BroadcastRequestProgramming(
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastRequestProgrammingResults> & orc_Results) const;
   int32_t m_BroadcastEcuReset(const uint8_t ou8_ResetType) const;
   int32_t m_BroadcastSendEnterDefaultSession(void) const;
   int32_t m_BroadcastSendEnterPreProgrammingSession(void) const;
   void m_ClearTpDispatcherQueue(void);
   int32_t m_BroadcastReadSerialNumber(
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> & orc_Responses,
      std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> & orc_ExtendedResponses,
      std::vector<stw::scl::C_SclString> & orc_DeviceNames)
   const;
   int32_t m_BroadcastSetNodeIdBySerialNumber(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                              const C_OscProtocolDriverOsyNode & orc_NewNodeId) const;
   int32_t m_BroadcastSetNodeIdBySerialNumberExtended(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                      const uint8_t ou8_SubNodeId,
                                                      const C_OscProtocolDriverOsyNode & orc_NewNodeId) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
