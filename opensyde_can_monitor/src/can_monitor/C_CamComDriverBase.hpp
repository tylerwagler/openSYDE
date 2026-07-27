//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Thread safe core communication driver base class (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMCOMDRIVERBASE_HPP
#define C_CAMCOMDRIVERBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <deque>
#include <QRecursiveMutex>

#include "stw_can.hpp"
#include "C_OscCanProtocol.hpp"

#include "C_OscComDriverBase.hpp"
#include "C_CamCanTpTransmitter.hpp"
/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamComDriverBase :
   public stw::opensyde_core::C_OscComDriverBase
{
public:
   C_CamComDriverBase(void);
   ~C_CamComDriverBase(void) override;

   int32_t StartLogging(const int32_t os32_Bitrate) override;
   void StopLogging(void) override;

   void ContinueLogging(void) override;
   void PauseLogging(void) override;

   void UpdateBitrate(const int32_t os32_Bitrate) override;

   void ClearRxMessages(void) override;
   void DistributeMessages(void) override;
   void SendCanMessageQueued(const stw::can::T_STWCAN_Msg_TX & orc_Msg) override;

   // CAN-TP Tx access
   C_CamCanTpTransmitter & GetCanTpTransmitter(void);

   // Thread-safe CAN-TP transmit: queue a request for the CAN thread
   void QueueTpRequest(const uint32_t ou32_CanId, const bool oq_IsExtended,
                       const std::vector<uint8_t> & orc_Payload);

   // Cyclic CAN-TP: register/cancel periodic TP requests
   void AddCyclicTpRequest(const uint32_t ou32_CanId, const bool oq_IsExtended,
                           const std::vector<uint8_t> & orc_Payload, const uint32_t ou32_IntervalMs);
   void RemoveCyclicTpRequest(const uint32_t ou32_CanId);
   void RemoveAllCyclicTpRequests(void);

   // Callback for CAN-TP transmitter to send a single CAN frame
   static void mh_SendTpFrame(const stw::can::T_STWCAN_Msg_TX & orc_Msg, void * const opv_Context);

   void SendCanMessage(stw::opensyde_core::C_OscComDriverBaseCanMessage & orc_MsgCfg, const bool oq_SetAutoSupportMode,
                       const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType) override;
   void AddCyclicCanMessage(const stw::opensyde_core::C_OscComDriverBaseCanMessage & orc_MsgCfg,
                            const bool oq_SetAutoSupportMode,
                            const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType) override;
   void RemoveCyclicCanMessage(const stw::opensyde_core::C_OscComDriverBaseCanMessage & orc_MsgCfg,
                               const bool oq_SetAutoSupportMode,
                               const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType) override;
   void RemoveAllCyclicCanMessages(void) override;
   void ClearData(void) override;
   void UpdateAutoSupportProtocol(const int32_t os32_CanId, const bool oq_SetAutoSupportMode,
                                  const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType) override;

private:
   // It is mutable because of the constness of the getter functions. Without the keyword mutable the getter functions
   // must be non const and that is not wanted.
   mutable QRecursiveMutex mc_CriticalSectionMsg;

   C_CamCanTpTransmitter mc_CanTpTransmitter;

   struct T_TpRequest
   {
      uint32_t u32_CanId;
      bool q_IsExtended;
      std::vector<uint8_t> c_Payload;
   };
   std::deque<T_TpRequest> mc_TpRequests;

   struct T_CyclicTpConfig
   {
      uint32_t u32_CanId;
      bool q_IsExtended;
      std::vector<uint8_t> c_Payload;
      uint32_t u32_IntervalMs;
      uint32_t u32_NextSendMs;
   };
   std::list<T_CyclicTpConfig> mc_CyclicTpConfigs;

   void m_ProcessTpRequests(void);
   void m_ProcessCyclicTpRequests(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
