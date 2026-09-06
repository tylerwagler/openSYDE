//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       ISO 15765-2 CAN-TP Transmitter (implementation)

   Segments outgoing payloads into FF + CF frames and manages
   the Tx state machine including Flow Control handling.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "TglTime.hpp"

#include "C_CamCanTpTransmitter.hpp"
#include "C_CamCanTpData.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::tgl;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpTransmitter::C_CamCanTpTransmitter() :
   mq_Enabled(true),
   me_State(eIDLE),
   mu32_CanId(0U),
   mq_IsExtended(false),
   mu16_TotalLength(0U),
   mu16_SentBytes(0U),
   mu8_SequenceNumber(0U),
   mu8_BlockSize(0U),
   mu8_BlocksSent(0U),
   mu8_SeparationTime(0U),
   mu32_LastSendTimeMs(0U),
   mu32_StartTimeMs(0U),
   mpf_SendFrame(nullptr),
   mpv_SendContext(nullptr)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpTransmitter::~C_CamCanTpTransmitter()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Start transmitting a payload as CAN-TP multi-frame message

   Segments the payload and sends the FF immediately.
   Subsequent CFs are sent on each call to Cycle().

   \param[in]  ou32_CanId        CAN identifier
   \param[in]  oq_IsExtended     true for 29-bit ID
   \param[in]  orc_Payload       Payload bytes
   \param[in]  opf_SendFrame     Callback to send a frame
   \param[in]  opv_Context       Opaque context

   \return
   C_NO_ERR   Transmission started
   C_BUSY     Another Tx in progress
   C_RANGE    Invalid payload
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CamCanTpTransmitter::StartTransmit(const uint32_t ou32_CanId, const bool oq_IsExtended,
                                              const std::vector<uint8_t> & orc_Payload,
                                              T_FpSendCanFrame opf_SendFrame, void * const opv_Context)
{
   if (!this->mq_Enabled)
   {
      return C_NOACT;
   }

   if (this->me_State != eIDLE)
   {
      return C_BUSY;
   }

   if ((orc_Payload.size() == 0U) || (orc_Payload.size() > mhu16_MAX_PAYLOAD))
   {
      return C_RANGE;
   }

   // Store transmission parameters
   this->mu32_CanId = ou32_CanId;
   this->mq_IsExtended = oq_IsExtended;
   this->mc_Payload = orc_Payload;
   this->mu16_TotalLength = static_cast<uint16_t>(orc_Payload.size());
   this->mpf_SendFrame = opf_SendFrame;
   this->mpv_SendContext = opv_Context;

   // Initialize state
   this->mu16_SentBytes = 0U;
   this->mu8_SequenceNumber = 1U;
   this->mu8_BlockSize = 0U;  // Default: send all (BS=0)
   this->mu8_BlocksSent = 0U;
   this->mu8_SeparationTime = 0U;
   this->mu32_LastSendTimeMs = 0U;
   this->mu32_StartTimeMs = TglGetTickCount();

   // For single-frame payload (≤7 bytes), send as SF directly
   if (orc_Payload.size() <= 7U)
   {
      stw::can::T_STWCAN_Msg_TX c_SfMsg;
      c_SfMsg.u32_ID = ou32_CanId;
      c_SfMsg.u8_XTD = oq_IsExtended ? 1U : 0U;
      c_SfMsg.u8_RTR = 0U;
      c_SfMsg.u8_DLC = static_cast<uint8_t>(orc_Payload.size() + 1U); // PCI byte + payload
      c_SfMsg.au8_Data[0] = mhu8_CAN_TP_PCI_SF | static_cast<uint8_t>(orc_Payload.size());
      (void)std::memcpy(&c_SfMsg.au8_Data[1], &orc_Payload[0], orc_Payload.size());

      if (this->mpf_SendFrame != nullptr)
      {
         this->mpf_SendFrame(c_SfMsg, this->mpv_SendContext);
      }

      this->me_State = eIDLE;
      return C_NO_ERR;
   }

   // Multi-frame: send FF, transition to WAITING_FOR_FLOW_CONTROL
   this->me_State = eWAITING_FOR_FLOW_CONTROL;
   this->m_SendFirstFrame();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Send the First Frame

   FF: byte0 = 0x10 | (totalLength >> 8), byte1 = totalLength & 0xFF,
   bytes 2-7 = first 6 bytes of payload
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::m_SendFirstFrame(void)
{
   stw::can::T_STWCAN_Msg_TX c_FfMsg;
   c_FfMsg.u32_ID = this->mu32_CanId;
   c_FfMsg.u8_XTD = this->mq_IsExtended ? 1U : 0U;
   c_FfMsg.u8_RTR = 0U;
   c_FfMsg.u8_DLC = 8U; // FF always uses full 8-byte frame

   // PCI bytes: total length (12 bits)
   c_FfMsg.au8_Data[0] = mhu8_CAN_TP_PCI_FF |
                          static_cast<uint8_t>((this->mu16_TotalLength >> 8U) & 0x0FU);
   c_FfMsg.au8_Data[1] = static_cast<uint8_t>(this->mu16_TotalLength & 0xFFU);

   // Payload bytes 0-5
   const uint16_t u16_PayloadBytes = (this->mu16_TotalLength < mhu8_FF_PAYLOAD_BYTES) ?
                                      this->mu16_TotalLength : mhu8_FF_PAYLOAD_BYTES;
   for (uint16_t u16_i = 0U; u16_i < u16_PayloadBytes; ++u16_i)
   {
      c_FfMsg.au8_Data[2U + u16_i] = this->mc_Payload[u16_i];
   }

   this->mu16_SentBytes = u16_PayloadBytes;
   this->mu32_LastSendTimeMs = TglGetTickCount();

   if (this->mpf_SendFrame != nullptr)
   {
      this->mpf_SendFrame(c_FfMsg, this->mpv_SendContext);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Send the next Consecutive Frame

   CF: byte0 = 0x20 | sequenceNumber, bytes 1-7 = next payload bytes
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::m_SendConsecutiveFrame(void)
{
   stw::can::T_STWCAN_Msg_TX c_CfMsg;
   c_CfMsg.u32_ID = this->mu32_CanId;
   c_CfMsg.u8_XTD = this->mq_IsExtended ? 1U : 0U;
   c_CfMsg.u8_RTR = 0U;

   // PCI byte with sequence number
   c_CfMsg.au8_Data[0] = mhu8_CAN_TP_PCI_CF | (this->mu8_SequenceNumber & mhu8_CAN_TP_SN_MASK);

   // Payload bytes
   const uint16_t u16_Remaining = this->mu16_TotalLength - this->mu16_SentBytes;
   const uint16_t u16_PayloadBytes = (u16_Remaining < mhu8_CF_PAYLOAD_BYTES) ?
                                      u16_Remaining : mhu8_CF_PAYLOAD_BYTES;
   for (uint16_t u16_i = 0U; u16_i < u16_PayloadBytes; ++u16_i)
   {
      c_CfMsg.au8_Data[1U + u16_i] = this->mc_Payload[this->mu16_SentBytes + u16_i];
   }

   c_CfMsg.u8_DLC = static_cast<uint8_t>(u16_PayloadBytes + 1U); // PCI byte + payload

   this->mu16_SentBytes += u16_PayloadBytes;
   this->mu32_LastSendTimeMs = TglGetTickCount();

   // Advance sequence number (1-15, wraps to 1)
   this->mu8_SequenceNumber = (this->mu8_SequenceNumber == 15U) ? 1U : (this->mu8_SequenceNumber + 1U);
   this->mu8_BlocksSent++;

   if (this->mpf_SendFrame != nullptr)
   {
      this->mpf_SendFrame(c_CfMsg, this->mpv_SendContext);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if all payload bytes have been sent

   \return  true if transmission is complete
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamCanTpTransmitter::m_IsTransmissionComplete(void) const
{
   return (this->mu16_SentBytes >= this->mu16_TotalLength);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Called periodically from DistributeMessages cycle

   Sends pending CFs respecting BS and STmin.
   Checks for N_Bs timeout.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::Cycle(void)
{
   if ((this->me_State == eIDLE) || (!this->mq_Enabled))
   {
      return;
   }

   const uint32_t u32_NowMs = TglGetTickCount();

   // Check for N_Bs timeout (waiting for FC)
   if (this->me_State == eWAITING_FOR_FLOW_CONTROL)
   {
      if ((u32_NowMs - this->mu32_StartTimeMs) > mhu32_N_BS_TIMEOUT_MS)
      {
         // Timeout — abort transmission
         this->me_State = eIDLE;
      }
      return;
   }

   // Sending CFs
   if (this->me_State == eSENDING_CONSECUTIVE)
   {
      // Respect STmin separation time
      const uint32_t u32_Elapsed = u32_NowMs - this->mu32_LastSendTimeMs;
      if (u32_Elapsed < static_cast<uint32_t>(this->mu8_SeparationTime))
      {
         return;
      }

      // Check block size limit
      if ((this->mu8_BlockSize > 0U) && (this->mu8_BlocksSent >= this->mu8_BlockSize))
      {
         // Block complete — wait for next FC
         this->me_State = eWAITING_FOR_FLOW_CONTROL;
         this->mu32_StartTimeMs = u32_NowMs;
         return;
      }

      // Send next CF
      this->m_SendConsecutiveFrame();

      // Check if done
      if (this->m_IsTransmissionComplete())
      {
         this->me_State = eIDLE;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle an incoming Flow Control frame

   Updates BS and STmin, transitions to SENDING_CONSECUTIVE.

   \param[in]  orc_Msg  Received FC frame
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::HandleFlowControl(const stw::can::T_STWCAN_Msg_RX & orc_Msg)
{
   if ((this->me_State != eWAITING_FOR_FLOW_CONTROL) || (!this->mq_Enabled))
   {
      return;
   }

   // Verify PCI type
   if ((orc_Msg.au8_Data[0] & 0xF0U) != mhu8_CAN_TP_PCI_FC)
   {
      return;
   }

   // Extract BS and STmin
   this->mu8_BlockSize = (orc_Msg.u8_DLC >= 2U) ? orc_Msg.au8_Data[1] : 0U;
   this->mu8_SeparationTime = (orc_Msg.u8_DLC >= 3U) ? orc_Msg.au8_Data[2] : 0U;
   this->mu8_BlocksSent = 0U;

   // BS=0 means "send all remaining CFs"
   // BS>0 means "send BS CFs then wait for next FC"

   // Transition to sending CFs
   this->me_State = eSENDING_CONSECUTIVE;
   this->mu32_LastSendTimeMs = 0U; // Allow immediate first CF

   // If BS=0, send first CF immediately (Cycle will handle the rest)
   if (this->mu8_BlockSize == 0U)
   {
      this->m_SendConsecutiveFrame();
      if (this->m_IsTransmissionComplete())
      {
         this->me_State = eIDLE;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Abort any active transmission
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::Reset(void)
{
   this->me_State = eIDLE;
   this->mc_Payload.clear();
   this->mu16_SentBytes = 0U;
   this->mu8_SequenceNumber = 0U;
   this->mu8_BlockSize = 0U;
   this->mu8_BlocksSent = 0U;
   this->mu8_SeparationTime = 0U;
   this->mu32_LastSendTimeMs = 0U;
   this->mu32_StartTimeMs = 0U;
   this->mpf_SendFrame = nullptr;
   this->mpv_SendContext = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if a Tx session is active

   \return  true if actively transmitting
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamCanTpTransmitter::IsActive(void) const
{
   return (this->me_State != eIDLE);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Enable or disable CAN-TP Tx segmentation

   \param[in]  oq_Enabled  true to enable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpTransmitter::SetEnabled(const bool oq_Enabled)
{
   this->mq_Enabled = oq_Enabled;
   if (!oq_Enabled)
   {
      this->Reset();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if CAN-TP Tx is enabled

   \return  true if enabled
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamCanTpTransmitter::GetEnabled(void) const
{
   return this->mq_Enabled;
}
