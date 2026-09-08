//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       ISO 15765-2 CAN-TP (Transport Protocol) decoder (implementation)

   Implements receive-side CAN-TP frame classification and
   multi-frame reassembly per ISO 15765-2.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "TglTime.hpp"

#include "C_CamCanTpDecoder.hpp"

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
C_CamCanTpDecoder::C_CamCanTpDecoder() :
   mq_Enabled(true),
   me_AddressingMode(eSTANDARD_11BIT)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpDecoder::~C_CamCanTpDecoder()
{
   this->mc_Sessions.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compute session key from CAN message

   For 11-bit mode, the raw CAN ID is the session key.
   For 29-bit mode, the full 29-bit ID is used.

   \param[in]  orc_Msg  CAN message

   \return  Session key (CAN ID)
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CamCanTpDecoder::m_GetSessionKey(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const
{
   return orc_Msg.u32_ID;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Find existing session or create a new one

   \param[in]  ou32_Key  Session key (CAN ID)

   \return  Pointer to session (never NULL)
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpSession * C_CamCanTpDecoder::m_FindOrCreateSession(const uint32_t ou32_Key)
{
   std::map<uint32_t, C_CamCanTpSession>::iterator c_It = this->mc_Sessions.find(ou32_Key);

   if (c_It != this->mc_Sessions.end())
   {
      return &(c_It->second);
   }

   // Enforce max sessions — evict oldest if necessary
   if (this->mc_Sessions.size() >= mhu32_MAX_SESSIONS)
   {
      // Find and remove the session with the oldest last-frame time
      std::map<uint32_t, C_CamCanTpSession>::iterator c_Evict = this->mc_Sessions.begin();
      for (std::map<uint32_t, C_CamCanTpSession>::iterator c_It2 = this->mc_Sessions.begin();
           c_It2 != this->mc_Sessions.end(); ++c_It2)
      {
         if (c_It2->second.u64_LastFrameTimeUs < c_Evict->second.u64_LastFrameTimeUs)
         {
            c_Evict = c_It2;
         }
      }
      this->mc_Sessions.erase(c_Evict);
   }

   C_CamCanTpSession & rc_New = this->mc_Sessions[ou32_Key];
   rc_New.u32_SourceCanId = ou32_Key;
   return &rc_New;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Remove sessions that have timed out

   A session is stale if no frame has been received within
   the N_Bs timeout period (1000ms).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_CleanupStaleSessions(void)
{
   const uint64_t u64_NowUs = TglGetTickCountUs();
   std::map<uint32_t, C_CamCanTpSession>::iterator c_It = this->mc_Sessions.begin();

   while (c_It != this->mc_Sessions.end())
   {
      const uint64_t u64_ElapsedUs = u64_NowUs - c_It->second.u64_LastFrameTimeUs;
      if ((c_It->second.e_Status != C_CamCanTpSession::eIDLE) &&
          (u64_ElapsedUs > (static_cast<uint64_t>(mhu16_CAN_TP_TIMEOUT_MS) * 1000ULL)))
      {
         c_It = this->mc_Sessions.erase(c_It);
      }
      else
      {
         ++c_It;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Classify a CAN frame and extract PCI byte info

   Examines the first byte of CAN data to determine the
   CAN-TP frame type. A frame is considered a TP frame if
   DLC >= 1 and the upper nibble of the first byte matches
   a known PCI type.

   \param[in]   orc_Msg     CAN message
   \param[out]  orc_Result  Result structure to populate
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_ClassifyFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                         C_CamCanTpResult & orc_Result)
{
   orc_Result.clear();

   // Minimum DLC check
   if (orc_Msg.u8_DLC < mhu8_MIN_TP_DLC)
   {
      orc_Result.q_IsTpFrame = false;
      return;
   }

   const uint8_t u8_PciByte = orc_Msg.au8_Data[0];
   const uint8_t u8_PciType = u8_PciByte & 0xF0U;

   // Check if upper nibble matches a known PCI type
   switch (u8_PciType)
   {
   case mhu8_CAN_TP_PCI_SF:
   case mhu8_CAN_TP_PCI_FF:
   case mhu8_CAN_TP_PCI_CF:
   case mhu8_CAN_TP_PCI_FC:
      orc_Result.q_IsTpFrame = true;
      orc_Result.u8_PciType = u8_PciType;
      break;
   default:
      orc_Result.q_IsTpFrame = false;
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle a Single Frame (SF)

   SF carries up to 7 bytes of payload in a single frame.
   The lower nibble of PCI byte 1 indicates the data length
   (0-7). For DLC=8, the payload is 7 bytes (PCI consumes 1 byte).

   \param[in]   orc_Msg      CAN message
   \param[out]  orc_Session  Session (reset to idle)
   \param[out]  orc_Result   Result structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_HandleSingleFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                             C_CamCanTpSession & orc_Session,
                                             C_CamCanTpResult & orc_Result)
{
   const uint8_t u8_Length = orc_Msg.au8_Data[0] & 0x0FU; // SF_DL = lower nibble
   const uint8_t u8_DataBytes = (orc_Msg.u8_DLC > 1U) ? (orc_Msg.u8_DLC - 1U) : 0U;

   // Validate: SF_DL should match available data bytes (excluding PCI)
   if (u8_Length > u8_DataBytes)
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = "SF length exceeds available data bytes";
      return;
   }

   orc_Result.u16_TotalMessageLength = u8_Length;
   orc_Result.q_ReassemblyComplete = true;

   // Reset any ongoing session — SF is self-contained
   orc_Session.Reset();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle a First Frame (FF)

   FF starts a multi-frame transmission. The lower 12 bits
   of the first 2 bytes encode the total message length
   (8-4095 bytes). Subsequent data follows.

   \param[in]   orc_Msg      CAN message
   \param[out]  orc_Session  Session (transitioned to WAITING_CONSECUTIVE)
   \param[out]  orc_Result   Result structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_HandleFirstFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                            C_CamCanTpSession & orc_Session,
                                            C_CamCanTpResult & orc_Result)
{
   // Extract total length from first 2 bytes
   const uint16_t u16_TotalLength =
      ((static_cast<uint16_t>(orc_Msg.au8_Data[0] & 0x0FU)) << 8U) |
       static_cast<uint16_t>(orc_Msg.au8_Data[1]);

   // Validate length
   if ((u16_TotalLength < 8U) || (u16_TotalLength > mhu16_CAN_TP_MAX_FF_LENGTH))
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = "FF invalid total length";
      return;
   }

   // Payload starts at byte 2; compute how many bytes in this frame
   const uint16_t u16_PayloadInFrame = (orc_Msg.u8_DLC > 2U) ?
                                       (static_cast<uint16_t>(orc_Msg.u8_DLC) - 2U) : 0U;

   // Initialize session for reassembly
   orc_Session.Reset();
   orc_Session.e_Status = C_CamCanTpSession::eWAITING_CONSECUTIVE;
   orc_Session.u16_TotalLength = u16_TotalLength;
   orc_Session.u8_ExpectedSn = 1U; // First CF has SN=1
   orc_Session.u16_BytesReceived = u16_PayloadInFrame;
   orc_Session.u32_FirstFrameTimeMs = static_cast<uint32_t>(TglGetTickCountUs() / 1000ULL);
   orc_Session.u64_LastFrameTimeUs = TglGetTickCountUs();

   // Buffer the FF payload
   orc_Session.c_Buffer.resize(u16_TotalLength);
   if (u16_PayloadInFrame > 0U)
   {
      for (uint16_t u16_i = 0U; u16_i < u16_PayloadInFrame; ++u16_i)
      {
         orc_Session.c_Buffer[u16_i] = orc_Msg.au8_Data[2U + u16_i];
      }
   }

   orc_Result.u16_TotalMessageLength = u16_TotalLength;
   orc_Result.u8_SequenceNumber = 0U; // FF has implicit SN=0

   // If total length fits entirely in FF payload (unusual but valid), complete immediately
   if (u16_PayloadInFrame >= u16_TotalLength)
   {
      orc_Session.e_Status = C_CamCanTpSession::eIDLE;
      orc_Result.q_ReassemblyComplete = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle a Consecutive Frame (CF)

   CF carries a sequence number (lower nibble) and up to 7
   bytes of continuation data. Sequence numbers wrap from
   15 to 1 (0 is reserved for FF).

   \param[in]   orc_Msg      CAN message
   \param[out]  orc_Session  Session (updated with new data)
   \param[out]  orc_Result   Result structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_HandleConsecutiveFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                                   C_CamCanTpSession & orc_Session,
                                                   C_CamCanTpResult & orc_Result)
{
   const uint8_t u8_Sn = orc_Msg.au8_Data[0] & mhu8_CAN_TP_SN_MASK;
   const uint8_t u8_DataBytes = (orc_Msg.u8_DLC > 1U) ? (orc_Msg.u8_DLC - 1U) : 0U;

   orc_Result.u8_SequenceNumber = u8_Sn;

   // Check if we have an active reassembly
   if (orc_Session.e_Status != C_CamCanTpSession::eWAITING_CONSECUTIVE)
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = "CF without active FF session";
      orc_Session.Reset();
      return;
   }

   // Validate sequence number
   if (u8_Sn != orc_Session.u8_ExpectedSn)
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = "CF sequence number mismatch";
      orc_Session.Reset();
      return;
   }

   // Advance expected SN (wrap 15 -> 1, skipping 0)
   orc_Session.u8_ExpectedSn = (u8_Sn == 15U) ? 1U : (u8_Sn + 1U);
   orc_Session.u64_LastFrameTimeUs = TglGetTickCountUs();

   // Copy payload data into buffer
   const uint16_t u16_Remaining = orc_Session.u16_TotalLength - orc_Session.u16_BytesReceived;
   const uint16_t u16_ToCopy = (static_cast<uint16_t>(u8_DataBytes) < u16_Remaining) ?
                                static_cast<uint16_t>(u8_DataBytes) : u16_Remaining;

   for (uint16_t u16_i = 0U; u16_i < u16_ToCopy; ++u16_i)
   {
      orc_Session.c_Buffer[orc_Session.u16_BytesReceived + u16_i] = orc_Msg.au8_Data[1U + u16_i];
   }
   orc_Session.u16_BytesReceived += u16_ToCopy;

   // Check if reassembly is complete
   if (orc_Session.u16_BytesReceived >= orc_Session.u16_TotalLength)
   {
      orc_Session.e_Status = C_CamCanTpSession::eIDLE;
      orc_Result.q_ReassemblyComplete = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle a Flow Control (FC) frame

   FC carries Block Size (BS) and Separation Time (STmin).
   As a receive-only decoder, we don't send FC, but we
   decode and report incoming FC frames for visibility.

   \param[in]   orc_Msg      CAN message
   \param[out]  orc_Session  Session (not modified for Rx-only)
   \param[out]  orc_Result   Result structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::m_HandleFlowControl(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                             C_CamCanTpSession & orc_Session,
                                             C_CamCanTpResult & orc_Result)
{
   // FC: byte0 = PCI (0x30), byte1 = BS, byte2 = STmin
   orc_Result.u8_BlockSize = (orc_Msg.u8_DLC >= 2U) ? orc_Msg.au8_Data[1] : 0U;
   orc_Result.u8_SeparationTime = (orc_Msg.u8_DLC >= 3U) ? orc_Msg.au8_Data[2] : 0U;

   // FC is a control frame, not a data message — leave ReassemblyComplete
   // as false so the trace model can distinguish it from reassembled payloads.
   (void)orc_Session; // Not modifying session state for Rx-only
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Process a single CAN frame

   Main entry point. Classifies the frame, routes to the
   appropriate handler, and updates session state.

   \param[in]   orc_Msg     CAN message
   \param[out]  orc_Result  Decoded TP frame information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::ProcessFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                      C_CamCanTpResult & orc_Result)
{
   orc_Result.clear();

   if (!this->mq_Enabled)
   {
      return;
   }

   // Periodic cleanup of stale sessions
   this->m_CleanupStaleSessions();

   // Classify the frame
   this->m_ClassifyFrame(orc_Msg, orc_Result);
   if (!orc_Result.q_IsTpFrame)
   {
      return;
   }

   // Get or create session
   const uint32_t u32_Key = this->m_GetSessionKey(orc_Msg);
   orc_Result.u32_SessionKey = u32_Key;
   orc_Result.u8_SessionCount = static_cast<uint8_t>(this->mc_Sessions.size());

   C_CamCanTpSession * const pc_Session = this->m_FindOrCreateSession(u32_Key);
   if (pc_Session == nullptr)
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = "Failed to create TP session";
      return;
   }

   // Route to appropriate handler based on PCI type
   switch (orc_Result.u8_PciType)
   {
   case mhu8_CAN_TP_PCI_SF:
      this->m_HandleSingleFrame(orc_Msg, *pc_Session, orc_Result);
      break;
   case mhu8_CAN_TP_PCI_FF:
      this->m_HandleFirstFrame(orc_Msg, *pc_Session, orc_Result);
      break;
   case mhu8_CAN_TP_PCI_CF:
      this->m_HandleConsecutiveFrame(orc_Msg, *pc_Session, orc_Result);
      break;
   case mhu8_CAN_TP_PCI_FC:
      this->m_HandleFlowControl(orc_Msg, *pc_Session, orc_Result);
      break;
   default:
      // Should not reach here — m_ClassifyFrame already validated
      break;
   }

   // Propagate error to result
   if (pc_Session->q_Error)
   {
      orc_Result.q_Error = true;
      orc_Result.c_ErrorDescription = pc_Session->c_ErrorDescription;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get reassembled data for a completed session

   \param[in]   ou32_SessionKey  Session key
   \param[out]  orc_Data         Reassembled payload

   \return
   true if data available, false otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamCanTpDecoder::GetReassembledData(const uint32_t ou32_SessionKey,
                                            std::vector<uint8_t> & orc_Data) const
{
   std::map<uint32_t, C_CamCanTpSession>::const_iterator c_It =
      this->mc_Sessions.find(ou32_SessionKey);

   if ((c_It != this->mc_Sessions.end()) &&
       (c_It->second.e_Status == C_CamCanTpSession::eIDLE) &&
       (c_It->second.c_Buffer.size() > 0U))
   {
      orc_Data = c_It->second.c_Buffer;
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set addressing mode

   \param[in]  oe_Mode  Addressing mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::SetAddressingMode(const E_AddressingMode oe_Mode)
{
   this->me_AddressingMode = oe_Mode;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get current addressing mode

   \return  Current addressing mode
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpDecoder::E_AddressingMode C_CamCanTpDecoder::GetAddressingMode(void) const
{
   return this->me_AddressingMode;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Enable or disable CAN-TP decoding

   \param[in]  oq_Enabled  true to enable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::SetEnabled(const bool oq_Enabled)
{
   this->mq_Enabled = oq_Enabled;
   if (!oq_Enabled)
   {
      this->Reset();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if CAN-TP decoding is enabled

   \return  true if enabled
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CamCanTpDecoder::GetEnabled(void) const
{
   return this->mq_Enabled;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Reset all sessions and clear decoder state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpDecoder::Reset(void)
{
   this->mc_Sessions.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get number of currently active sessions

   \return  Active session count
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CamCanTpDecoder::GetActiveSessionCount(void) const
{
   return static_cast<uint32_t>(this->mc_Sessions.size());
}
