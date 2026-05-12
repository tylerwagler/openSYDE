//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Common CAN message types used by the dispatcher interface

   Defines the TX/RX frame structures exchanged across C_CanDispatcher implementations.
   The legacy STW CAN DLL structures (status, DLL info, 11-bit messages) were removed
   alongside the STW DLL shim; libcan-backed adapters now provide that capability metadata.

   \copyright   Copyright 2010 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef STWCAN_H
#define STWCAN_H

#include "stwtypes.hpp"

namespace stw
{
namespace can
{
///11bit or 29bit CAN message structure for received messages
struct T_STWCAN_Msg_RX
{
   uint32_t u32_ID;        ///< CAN message ID
   uint8_t u8_XTD;         ///< 0 -> 11bit ID; 1 -> 29bit ID
   uint8_t u8_DLC;         ///< data length code (0..8)
   uint8_t u8_RTR;         ///< 0 -> regular frame; 1 -> remote frame
   uint8_t u8_Align;       ///< alignment dummy
   uint8_t au8_Data[8];    ///< message data
   uint64_t u64_TimeStamp; ///< message reception time in micro-seconds since system start; 0 -> not available
};

///11bit or 29bit CAN message structure for sending messages
struct T_STWCAN_Msg_TX
{
   uint32_t u32_ID;     ///< CAN message ID
   uint8_t u8_XTD;      ///< 0 -> 11bit ID; 1 -> 29bit ID
   uint8_t u8_DLC;      ///< data length code (0..8)
   uint8_t u8_RTR;      ///< 0 -> regular frame; 1 -> remote frame
   uint8_t u8_Align;    ///< alignment dummy
   uint8_t au8_Data[8]; ///< message data
};
}
}
#endif
