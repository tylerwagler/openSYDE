//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for logging CAN messages (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCOMMESSAGELOGGERDATA_HPP
#define C_OSCCOMMESSAGELOGGERDATA_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <algorithm>

#include <cstdint>
#include "stw_can.hpp"
#include <string>
#include "C_OscCanSignal.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///CAN-TP (ISO 15765-2) frame type
enum E_OscCanTpFrameType
{
   eCTFT_NONE,       ///< Not a CAN-TP frame (or TP decoding disabled)
   eCTFT_SINGLE,     ///< Single Frame (SF) — complete message ≤7 bytes payload
   eCTFT_FIRST,      ///< First Frame (FF) — start of multi-frame message
   eCTFT_CONSECUTIVE,///< Consecutive Frame (CF) — continuation of multi-frame message
   eCTFT_FLOW_CONTROL///< Flow Control (FC) — peer throttling
};

class C_OscComMessageLoggerDataSignal
{
public:
   C_OscComMessageLoggerDataSignal(void);

   bool operator <(const C_OscComMessageLoggerDataSignal & orc_Cmp) const;

   std::string c_Name;
   std::string c_Value;
   std::string c_Unit;
   std::string c_RawValueDec;
   std::string c_RawValueHex;
   std::string c_Comment;
   bool q_DlcError;

   stw::opensyde_core::C_OscCanSignal c_OscSignal;
};

class C_OscComMessageLoggerData
{
public:
   C_OscComMessageLoggerData(void);
   virtual ~C_OscComMessageLoggerData(void);

   void SortSignals(void);
   static std::string h_GetTimestampAsString(const uint64_t ou64_TimeStamp);

   uint64_t u64_TimeStampRelative;
   uint64_t u64_TimeStampAbsoluteStart;
   uint64_t u64_TimeStampAbsoluteTimeOfDay;
   std::string c_TimeStampRelative;
   std::string c_TimeStampAbsoluteStart;
   std::string c_TimeStampAbsoluteTimeOfDay;
   std::string c_CanIdDec;
   std::string c_CanIdHex;
   std::string c_CanDlc;
   bool q_CanDlcError;
   std::string c_CanDataDec;
   std::string c_CanDataHex;
   std::string c_Name;
   std::string c_ProtocolTextDec;
   std::string c_ProtocolTextHex;
   std::string c_Counter;
   std::string c_Status;

   std::vector<C_OscComMessageLoggerDataSignal> c_Signals;

   stw::can::T_STWCAN_Msg_RX c_CanMsg;
   bool q_IsTx;

   // CAN-TP (ISO 15765-2) metadata
   E_OscCanTpFrameType e_TpFrameType;     ///< Detected CAN-TP frame type (eCTFT_NONE if not TP)
   uint8_t u8_TpSequenceNumber;           ///< Sequence number (SN) for CF frames (0-15)
   uint16_t u16_TpTotalMessageLength;     ///< Total message length from FF (0 for non-FF)
   uint8_t u8_TpBlockSize;                ///< Block Size (BS) from FC frame
   uint8_t u8_TpSeparationTime;           ///< Separation Time (STmin) from FC frame (ms)
   uint32_t u32_TpSessionKey;             ///< Session identifier (typically source CAN ID)
   bool q_TpReassembled;                  ///< true if this frame completes a TP reassembly
   bool q_TpError;                        ///< true if TP protocol error detected
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
