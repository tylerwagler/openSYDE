//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN-TP (ISO 15765-2) data structures

   Defines the data types used by the CAN-TP decoder for
   transport-protocol frame classification and multi-frame
   reassembly state tracking.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMCANTPDATA_HPP
#define C_CAMCANTPDATA_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <cstdint>
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

///PCI (Protocol Control Information) type identifiers per ISO 15765-2
static const uint8_t mhu8_CAN_TP_PCI_SF = 0x00U; ///< Single Frame (SF)
static const uint8_t mhu8_CAN_TP_PCI_FF = 0x10U; ///< First Frame (FF)
static const uint8_t mhu8_CAN_TP_PCI_CF = 0x20U; ///< Consecutive Frame (CF)
static const uint8_t mhu8_CAN_TP_PCI_FC = 0x30U; ///< Flow Control (FC)

///CAN-TP protocol constants
static const uint8_t  mhu8_CAN_TP_SN_MASK       = 0x0FU; ///< Sequence number mask (lower nibble)
static const uint16_t mhu8_CAN_TP_FF_DL_MASK    = 0x0FFFU; ///< FF data length mask (12 bits)
static const uint8_t  mhu8_CAN_TP_FC_BS_MASK    = 0xFFU;   ///< FC block size mask
static const uint8_t  mhu8_CAN_TP_FC_STMIN_MASK = 0x0FU;   ///< FC STmin mask (lower nibble)
static const uint8_t  mhu8_CAN_TP_MAX_SN        = 0x0FU;   ///< Maximum sequence number (0-15)
static const uint16_t mhu16_CAN_TP_MAX_FF_LENGTH = 4095U;   ///< Maximum FF data length
static const uint16_t mhu16_CAN_TP_TIMEOUT_MS   = 1000U;   ///< N_Bs timeout per ISO 15765-2

/* -- Types --------------------------------------------------------------------------------------------------------- */

///CAN-TP session state for tracking multi-frame reassembly
class C_CamCanTpSession
{
public:
   ///Session status
   enum E_SessionStatus
   {
      eIDLE,                    ///< No active reassembly
      eWAITING_CONSECUTIVE      ///< Waiting for consecutive frames
   };

   C_CamCanTpSession();

   void Reset(void);

   // Session identification
   uint32_t u32_SourceCanId;    ///< CAN ID identifying this session

   // Reassembly state
   E_SessionStatus e_Status;
   uint8_t u8_ExpectedSn;       ///< Next expected sequence number
   uint16_t u16_TotalLength;    ///< Total message length (from FF)
   uint16_t u16_BytesReceived;  ///< Bytes received so far
   std::vector<uint8_t> c_Buffer; ///< Reassembly buffer

   // Timing
   uint64_t u64_LastFrameTimeUs; ///< Timestamp of last received frame (us)
   uint32_t u32_FirstFrameTimeMs; ///< Timestamp of FF reception (ms)

   // Error tracking
   bool q_Error;
   std::string c_ErrorDescription;
};

///Result of processing one CAN frame through the TP decoder
class C_CamCanTpResult
{
public:
   C_CamCanTpResult();

   void Clear(void);

   // Frame classification
   bool q_IsTpFrame;              ///< true if this frame carries a CAN-TP PCI byte
   uint8_t u8_PciType;            ///< Raw PCI type (upper nibble of first byte)

   // Frame info
   uint8_t u8_SequenceNumber;     ///< Sequence number (valid for CF)
   uint16_t u16_TotalMessageLength; ///< Total message length (valid for FF)

   // FC-specific
   uint8_t u8_BlockSize;          ///< Block Size (valid for FC)
   uint8_t u8_SeparationTime;     ///< STmin (valid for FC)

   // Session tracking
   uint32_t u32_SessionKey;       ///< CAN ID used as session key
   uint8_t u8_SessionCount;       ///< Number of active sessions

   // Reassembly result
   bool q_ReassemblyComplete;     ///< true if a multi-frame message was fully reassembled
   bool q_Error;                  ///< true if a TP protocol error was detected
   std::string c_ErrorDescription;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
