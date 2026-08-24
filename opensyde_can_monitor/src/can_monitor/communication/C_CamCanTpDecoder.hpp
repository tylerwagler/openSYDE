//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       ISO 15765-2 CAN-TP (Transport Protocol) decoder (header)

   Decodes CAN-TP frames (SF, FF, CF, FC) and performs
   multi-frame reassembly for received messages.

   This is a receive-only decoder — it does not transmit
   Flow Control frames. It tracks multiple concurrent TP
   sessions keyed by source CAN ID.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMCANTPDECODER_HPP
#define C_CAMCANTPDECODER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <map>

#include "stwtypes.hpp"
#include "stw_can.hpp"
#include <string>
#include "C_CamCanTpData.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamCanTpDecoder
{
public:
   C_CamCanTpDecoder();
   ~C_CamCanTpDecoder();

   /// Addressing mode for session key computation
   enum E_AddressingMode
   {
      eSTANDARD_11BIT, ///< Use 11-bit CAN ID as session key
      eEXTENDED_29BIT  ///< Use 29-bit CAN ID as session key
   };

   /// Maximum number of concurrent TP sessions
   static const uint32_t mhu32_MAX_SESSIONS = 32U;

   /// Minimum DLC for a valid CAN-TP frame (must have at least 1 byte for PCI)
   static const uint8_t mhu8_MIN_TP_DLC = 1U;

   /**
    * Process a single CAN frame through the TP decoder.
    *
    * Classifies the frame (SF/FF/CF/FC), updates session state,
    * and indicates whether a multi-frame reassembly completed.
    *
    * \param[in]  orc_Msg     Incoming CAN message
    * \param[out] orc_Result  Decoded TP frame information
    */
   void ProcessFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                     C_CamCanTpResult & orc_Result);

   /**
    * Get the reassembled payload data for a completed session.
    * Only valid when orc_Result.q_ReassemblyComplete is true.
    *
    * \param[in]  ou32_SessionKey  Session identifier
    * \param[out] orc_Data         Reassembled payload bytes
    *
    * \return
    *   true if reassembled data is available, false otherwise
    */
   bool GetReassembledData(const uint32_t ou32_SessionKey,
                           std::vector<uint8_t> & orc_Data) const;

   /**
    * Set the addressing mode for session key computation.
    *
    * \param[in]  oe_Mode  Addressing mode
    */
   void SetAddressingMode(const E_AddressingMode oe_Mode);

   /**
    * Get current addressing mode.
    *
    * \return  Current addressing mode
    */
   E_AddressingMode GetAddressingMode(void) const;

   /**
    * Enable or disable CAN-TP decoding.
    *
    * \param[in]  oq_Enabled  true to enable, false to disable
    */
   void SetEnabled(const bool oq_Enabled);

   /**
    * Check if CAN-TP decoding is enabled.
    *
    * \return  true if enabled
    */
   bool GetEnabled(void) const;

   /**
    * Reset all sessions and clear decoder state.
    */
   void Reset(void);

   /**
    * Get the number of currently active sessions.
    *
    * \return  Active session count
    */
   uint32_t GetActiveSessionCount(void) const;

private:
   // Prevent copying
   C_CamCanTpDecoder(const C_CamCanTpDecoder &);
   C_CamCanTpDecoder & operator =(const C_CamCanTpDecoder &);

   bool mq_Enabled;
   E_AddressingMode me_AddressingMode;
   std::map<uint32_t, C_CamCanTpSession> mc_Sessions;

   uint32_t m_GetSessionKey(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   C_CamCanTpSession * m_FindOrCreateSession(const uint32_t ou32_Key);
   void m_CleanupStaleSessions(void);
   void m_ClassifyFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                        C_CamCanTpResult & orc_Result);
   void m_HandleSingleFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                            C_CamCanTpSession & orc_Session,
                            C_CamCanTpResult & orc_Result);
   void m_HandleFirstFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                           C_CamCanTpSession & orc_Session,
                           C_CamCanTpResult & orc_Result);
   void m_HandleConsecutiveFrame(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                                 C_CamCanTpSession & orc_Session,
                                 C_CamCanTpResult & orc_Result);
   void m_HandleFlowControl(const stw::can::T_STWCAN_Msg_RX & orc_Msg,
                            C_CamCanTpSession & orc_Session,
                            C_CamCanTpResult & orc_Result);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
