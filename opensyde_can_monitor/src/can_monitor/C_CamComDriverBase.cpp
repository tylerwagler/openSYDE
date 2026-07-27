//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Thread safe core communication driver base class (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>

#include "C_CamComDriverBase.hpp"
#include "C_CamCanTpData.hpp"
#include "TglTime.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamComDriverBase::C_CamComDriverBase(void) :
   C_OscComDriverBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamComDriverBase::~C_CamComDriverBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Starts the logging

   This function is thread safe

   \param[in]  os32_Bitrate          CAN bitrate in kBit/s. Is used for the bus load calculation not the initialization

   \return
   C_NO_ERR                          CAN initialized and logging started
   C_CONFIG                          CAN dispatcher is not set
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CamComDriverBase::StartLogging(const int32_t os32_Bitrate)
{
   int32_t s32_Return;

   this->mc_CriticalSectionMsg.lock();
   s32_Return = C_OscComDriverBase::StartLogging(os32_Bitrate);
   this->mc_CriticalSectionMsg.unlock();

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Stops the logging

   This function is thread safe
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::StopLogging(void)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::StopLogging();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Continues the paused the logging

   This function is thread safe
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::ContinueLogging(void)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::ContinueLogging();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Pauses the logging.

   This function is thread safe
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::PauseLogging(void)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::PauseLogging();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update the bitrate to adapt the calculation for the bus load

   \param[in]  os32_Bitrate          CAN bitrate in kBit/s. Is used for the bus load calculation not the initialization
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::UpdateBitrate(const int32_t os32_Bitrate)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::UpdateBitrate(os32_Bitrate);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clear messages
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::ClearRxMessages()
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::ClearRxMessages();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Distributes the CAN message to all registered C_OscMessageLogger instances.

   This function is thread safe
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::DistributeMessages(void)
{
   this->mc_CriticalSectionMsg.lock();

   // Process any queued CAN-TP transmit requests (from GUI thread)
   this->m_ProcessTpRequests();

   // Process cyclic CAN-TP requests (periodic re-queue)
   this->m_ProcessCyclicTpRequests();

   // Drive the CAN-TP Tx state machine (sends pending CFs, checks timeouts)
   this->mc_CanTpTransmitter.Cycle();

   // Let the base class handle Rx distribution and normal Tx
   C_OscComDriverBase::DistributeMessages();

   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sending a CAN message by putting it into the queue

   If CAN-TP Tx is enabled and the payload exceeds 7 bytes, the
   message is automatically segmented into FF + CF frames.

   \param[in]  orc_Msg  Tx CAN message

   This function is thread safe
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::SendCanMessageQueued(const stw::can::T_STWCAN_Msg_TX & orc_Msg)
{
   // Check if CAN-TP segmentation is needed
   if ((this->mc_CanTpTransmitter.GetEnabled()) && (orc_Msg.u8_DLC > 7U))
   {
      // Extract payload bytes
      std::vector<uint8_t> c_Payload;
      c_Payload.reserve(orc_Msg.u8_DLC);
      for (uint8_t u8_i = 0U; u8_i < orc_Msg.u8_DLC; ++u8_i)
      {
         c_Payload.push_back(orc_Msg.au8_Data[u8_i]);
      }

      // Start segmented transmission
      this->mc_CanTpTransmitter.StartTransmit(orc_Msg.u32_ID,
                                               (orc_Msg.u8_XTD != 0U),
                                               c_Payload,
                                               &C_CamComDriverBase::mh_SendTpFrame,
                                               this);
      return;
   }

   // Normal path: queue for direct sending
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::SendCanMessageQueued(orc_Msg);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sends a CAN message with a specific configuration

   Interval is ignored in this function.
   Use AddCyclicCanMessage for registration of a cyclic CAN message.

   \param[in]     orc_MsgCfg                    CAN message configuration
   \param[in]     oq_SetAutoSupportMode         Message auto protocol mode
   \param[in]     oe_ProtocolType                    Message protocol type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::SendCanMessage(C_OscComDriverBaseCanMessage & orc_MsgCfg, const bool oq_SetAutoSupportMode,
                                        const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::SendCanMessage(orc_MsgCfg, oq_SetAutoSupportMode, oe_ProtocolType);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Registers a cyclic CAN message with a specific configuration

   \param[in]     orc_MsgCfg                    CAN message configuration
   \param[in]     oq_SetAutoSupportMode         Message auto protocol mode
   \param[in]     oe_ProtocolType               Message protocol type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::AddCyclicCanMessage(const C_OscComDriverBaseCanMessage & orc_MsgCfg,
                                             const bool oq_SetAutoSupportMode,
                                             const C_OscCanProtocol::E_Type oe_ProtocolType)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::AddCyclicCanMessage(orc_MsgCfg, oq_SetAutoSupportMode, oe_ProtocolType);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Removes a cyclic CAN message with a specific configuration

   \param[in]     orc_MsgCfg                    CAN message configuration
   \param[in]     oq_SetAutoSupportMode         Message auto protocol mode
   \param[in]     oe_ProtocolType               Message protocol type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::RemoveCyclicCanMessage(const C_OscComDriverBaseCanMessage & orc_MsgCfg,
                                                const bool oq_SetAutoSupportMode,
                                                const C_OscCanProtocol::E_Type oe_ProtocolType)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::RemoveCyclicCanMessage(orc_MsgCfg, oq_SetAutoSupportMode, oe_ProtocolType);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Removes all cyclic CAN messages
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::RemoveAllCyclicCanMessages(void)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::RemoveAllCyclicCanMessages();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  On Clear trace we send information to base class to clear message counters
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::ClearData()
{
   C_OscComDriverBase::ClearData();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  On Update of AutoSupportProtocol to send information for stop/start Message counter

   \param[in]       os32_CanId                Current message Can Id
   \param[in]      oq_SetAutoSupportMode     Is AutoSupport activated
   \param[in]      oe_ProtocolType           Current message Protocol type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::UpdateAutoSupportProtocol(const int32_t os32_CanId, const bool oq_SetAutoSupportMode,
                                                   const C_OscCanProtocol::E_Type oe_ProtocolType)
{
   this->mc_CriticalSectionMsg.lock();
   C_OscComDriverBase::UpdateAutoSupportProtocol(os32_CanId, oq_SetAutoSupportMode, oe_ProtocolType);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Queue a CAN-TP transmit request from the GUI thread

   The request is processed by the CAN thread in DistributeMessages()
   under the critical section lock, avoiding thread-safety issues with
   the transmitter state machine.

   \param[in]  ou32_CanId     CAN identifier
   \param[in]  oq_IsExtended  true for 29-bit ID
   \param[in]  orc_Payload    Payload bytes to transmit
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::QueueTpRequest(const uint32_t ou32_CanId, const bool oq_IsExtended,
                                        const std::vector<uint8_t> & orc_Payload)
{
   this->mc_CriticalSectionMsg.lock();
   this->mc_TpRequests.push_back({ou32_CanId, oq_IsExtended, orc_Payload});
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Register a cyclic CAN-TP transmit request

   The request is re-queued periodically by the CAN thread based on the
   interval.  Thread-safe (locks the critical section).

   \param[in]  ou32_CanId       CAN identifier
   \param[in]  oq_IsExtended    true for 29-bit ID
   \param[in]  orc_Payload      Payload bytes to transmit
   \param[in]  ou32_IntervalMs  Period in milliseconds
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::AddCyclicTpRequest(const uint32_t ou32_CanId, const bool oq_IsExtended,
                                            const std::vector<uint8_t> & orc_Payload,
                                            const uint32_t ou32_IntervalMs)
{
   this->mc_CriticalSectionMsg.lock();
   for (std::list<T_CyclicTpConfig>::iterator c_It = this->mc_CyclicTpConfigs.begin();
        c_It != this->mc_CyclicTpConfigs.end(); ++c_It)
   {
      if (c_It->u32_CanId == ou32_CanId)
      {
         this->mc_CyclicTpConfigs.erase(c_It);
         break;
      }
   }
   T_CyclicTpConfig c_Cfg;
   c_Cfg.u32_CanId = ou32_CanId;
   c_Cfg.q_IsExtended = oq_IsExtended;
   c_Cfg.c_Payload = orc_Payload;
   c_Cfg.u32_IntervalMs = ou32_IntervalMs;
   c_Cfg.u32_NextSendMs = stw::tgl::TglGetTickCount();
   this->mc_CyclicTpConfigs.push_back(c_Cfg);
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Remove a cyclic CAN-TP transmit request

   \param[in]  ou32_CanId  CAN identifier of the cyclic request to remove
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::RemoveCyclicTpRequest(const uint32_t ou32_CanId)
{
   this->mc_CriticalSectionMsg.lock();
   for (std::list<T_CyclicTpConfig>::iterator c_It = this->mc_CyclicTpConfigs.begin();
        c_It != this->mc_CyclicTpConfigs.end(); ++c_It)
   {
      if (c_It->u32_CanId == ou32_CanId)
      {
         this->mc_CyclicTpConfigs.erase(c_It);
         break;
      }
   }
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Remove all cyclic CAN-TP transmit requests
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::RemoveAllCyclicTpRequests(void)
{
   this->mc_CriticalSectionMsg.lock();
   this->mc_CyclicTpConfigs.clear();
   this->mc_CriticalSectionMsg.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Process queued CAN-TP transmit requests

   Called from DistributeMessages() under the critical section lock.
   Drains the queue and starts each transmission via the transmitter.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::m_ProcessTpRequests(void)
{
   while (this->mc_TpRequests.empty() == false)
   {
      const T_TpRequest c_Req = this->mc_TpRequests.front();
      this->mc_TpRequests.pop_front();
      this->mc_CanTpTransmitter.StartTransmit(c_Req.u32_CanId, c_Req.q_IsExtended,
                                               c_Req.c_Payload,
                                               &C_CamComDriverBase::mh_SendTpFrame,
                                               this);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Process cyclic CAN-TP transmit requests

   Called from DistributeMessages() under the critical section lock.
   Re-queues TP requests whose interval has elapsed.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::m_ProcessCyclicTpRequests(void)
{
   const uint32_t u32_NowMs = TglGetTickCount();
   for (std::list<T_CyclicTpConfig>::iterator c_It = this->mc_CyclicTpConfigs.begin();
        c_It != this->mc_CyclicTpConfigs.end(); )
   {
      if (u32_NowMs >= c_It->u32_NextSendMs)
      {
         this->mc_CanTpTransmitter.StartTransmit(c_It->u32_CanId, c_It->q_IsExtended,
                                                  c_It->c_Payload,
                                                  &C_CamComDriverBase::mh_SendTpFrame,
                                                  this);
         c_It->u32_NextSendMs = u32_NowMs + c_It->u32_IntervalMs;
         ++c_It;
      }
      else
      {
         ++c_It;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Static callback for CAN-TP transmitter to send a single frame

   Forwards the segmented frame to the CAN dispatcher via the base class.

   \param[in]  orc_Msg      CAN frame to send
   \param[in]  opv_Context  Pointer to C_CamComDriverBase instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamComDriverBase::mh_SendTpFrame(const stw::can::T_STWCAN_Msg_TX & orc_Msg, void * const opv_Context)
{
   C_CamComDriverBase * const pc_This = static_cast<C_CamComDriverBase *>(opv_Context);
   if (pc_This != NULL)
   {
      // Queue the TP frame for sending via the normal path.
      // These frames are always ≤8 bytes, so they won't recurse.
      pc_This->mc_CriticalSectionMsg.lock();
      pc_This->C_OscComDriverBase::SendCanMessageQueued(orc_Msg);
      pc_This->mc_CriticalSectionMsg.unlock();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get reference to CAN-TP transmitter

   \return
   Reference to internal CAN-TP transmitter instance
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpTransmitter & C_CamComDriverBase::GetCanTpTransmitter(void)
{
   return this->mc_CanTpTransmitter;
}
