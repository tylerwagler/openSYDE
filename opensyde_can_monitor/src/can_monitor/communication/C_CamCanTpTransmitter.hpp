//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       ISO 15765-2 CAN-TP Transmitter (header)

   Segments outgoing payloads into FF + CF frames and manages
   the Tx state machine including Flow Control handling.

   Driven by the periodic DistributeMessages cycle.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMCANTPTRANSMITTER_HPP
#define C_CAMCANTPTRANSMITTER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <deque>
#include <cstdint>
#include "stw_can.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamCanTpTransmitter
{
public:
   C_CamCanTpTransmitter();
   ~C_CamCanTpTransmitter();

   /// Tx session state
   enum E_TxState
   {
      eIDLE,                       ///< No active Tx session
      eWAITING_FOR_FLOW_CONTROL,   ///< Sent FF, awaiting FC
      eSENDING_CONSECUTIVE         ///< Sending CFs (FC received with BS>0 or BS=0)
   };

   /// Callback type for transmitting a raw CAN frame
   typedef void (*T_FpSendCanFrame)(const stw::can::T_STWCAN_Msg_TX & orc_Msg, void * const opv_Context);

   /**
    * Start transmitting a payload as a CAN-TP multi-frame message.
    *
    * Segments the payload and sends the FF immediately via the callback.
    * Subsequent CFs are sent on each call to Cycle().
    *
    * \param[in]  ou32_CanId        CAN identifier for the message
    * \param[in]  oq_IsExtended     true for 29-bit ID
    * \param[in]  orc_Payload       Payload bytes to transmit
    * \param[in]  opf_SendFrame     Callback to send a single CAN frame
    * \param[in]  opv_Context       Opaque context for callback
    *
    * \return
    *   C_NO_ERR   Transmission started
    *   C_BUSY     Another Tx session is already in progress
    *   C_RANGE    Payload empty or too large (>4095 bytes)
    */
   int32_t StartTransmit(const uint32_t ou32_CanId, const bool oq_IsExtended,
                         const std::vector<uint8_t> & orc_Payload,
                         T_FpSendCanFrame opf_SendFrame, void * const opv_Context);

   /**
    * Called periodically from DistributeMessages cycle.
    * Sends pending CFs and checks for timeouts.
    */
   void Cycle(void);

   /**
    * Handle an incoming Flow Control frame.
    *
    * \param[in]  orc_Msg  Received FC frame
    */
   void HandleFlowControl(const stw::can::T_STWCAN_Msg_RX & orc_Msg);

   /**
    * Abort any active transmission and reset to IDLE.
    */
   void Reset(void);

   /**
    * Check if a Tx session is active.
    *
    * \return  true if actively transmitting
    */
   bool IsActive(void) const;

   /**
    * Enable or disable CAN-TP Tx segmentation.
    *
    * \param[in]  oq_Enabled  true to enable
    */
   void SetEnabled(const bool oq_Enabled);

   /**
    * Check if CAN-TP Tx is enabled.
    *
    * \return  true if enabled
    */
   bool GetEnabled(void) const;

private:
   // Prevent copying
   C_CamCanTpTransmitter(const C_CamCanTpTransmitter &);
   C_CamCanTpTransmitter & operator =(const C_CamCanTpTransmitter &);

   bool mq_Enabled;
   E_TxState me_State;

   // Original message info
   uint32_t mu32_CanId;
   bool mq_IsExtended;
   std::vector<uint8_t> mc_Payload;
   uint16_t mu16_TotalLength;

   // Segmentation state
   uint16_t mu16_SentBytes;        ///< Bytes sent so far (across all frames)
   uint8_t mu8_SequenceNumber;     ///< Next CF sequence number (1-15, wraps)
   uint8_t mu8_BlockSize;          ///< BS from FC (0 = send all)
   uint8_t mu8_BlocksSent;         ///< CFs sent in current block
   uint8_t mu8_SeparationTime;     ///< STmin from FC (ms)
   uint32_t mu32_LastSendTimeMs;   ///< Timestamp of last frame sent (ms)
   uint32_t mu32_StartTimeMs;      ///< Timestamp when transmission started

   // Callback
   T_FpSendCanFrame mpf_SendFrame;
   void * mpv_SendContext;

   // Constants
   static const uint8_t mhu8_FF_PAYLOAD_BYTES = 6U;  ///< FF carries 6 bytes of payload (PCI uses 2)
   static const uint8_t mhu8_CF_PAYLOAD_BYTES = 7U;  ///< CF carries 7 bytes of payload (PCI uses 1)
   static const uint16_t mhu16_MAX_PAYLOAD = 4095U;   ///< Max payload per ISO 15765-2
   static const uint32_t mhu32_N_BS_TIMEOUT_MS = 1000U; ///< N_Bs timeout

   void m_SendNextFrame(void);
   void m_SendFirstFrame(void);
   void m_SendConsecutiveFrame(void);
   bool m_IsTransmissionComplete(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
