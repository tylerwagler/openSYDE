//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       PEAK adapter — Windows-native PCANBasic interface (implementation)

   Windows-only adapter that talks directly to PEAK's PCANBasic API. Replaces the legacy
   target_windows_stw_dlls path (which wrapped the STW shim DLL around PCANBasic). The channel
   number (1..16) and bitrate (kbit/s) are passed in the constructor; CAN_Init() then calls
   PCANBasic's CAN_Initialize with the resolved TPCANHandle and TPCANBaudrate.

   On Linux this file is excluded from the build entirely — Linux uses the kernel's PEAK driver
   which exposes the device through SocketCAN; C_OscCanSocketCanAdapter handles that path.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <windows.h>
#include "PCANBasic.h"

#include "C_OscCanPeakAdapter.hpp"
#include "stwerrors.hpp"
#include "TglTime.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::can;
using namespace stw::tgl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscCanPeakAdapter::C_OscCanPeakAdapter(const uint16_t ou16_Channel, const uint32_t ou32_BitrateKbits) :
   C_CanDispatcher(),
   mu16_Channel(ou16_Channel),
   mu32_BitrateKbits(ou32_BitrateKbits),
   mu16_PcanHandle(mh_ChannelToPcanHandle(ou16_Channel)),
   mq_Initialized(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanPeakAdapter::~C_OscCanPeakAdapter(void)
{
   if (mq_Initialized == true)
   {
      (void)this->C_OscCanPeakAdapter::CAN_Exit();
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Init(void)
{
   int32_t s32_Return = C_NO_ERR;
   const TPCANBaudrate t_Baud = mh_BitrateKbitsToPcanBaud(mu32_BitrateKbits);
   const TPCANStatus t_Status = CAN_Initialize(mu16_PcanHandle, t_Baud, 0U, 0U, 0U);

   if (t_Status != PCAN_ERROR_OK)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      mq_Initialized = true;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Init(const int32_t os32_BitrateKBitS)
{
   if (os32_BitrateKBitS > 0)
   {
      mu32_BitrateKbits = static_cast<uint32_t>(os32_BitrateKBitS);
   }
   return this->CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Exit(void)
{
   int32_t s32_Return = C_NO_ERR;

   if (mq_Initialized == true)
   {
      const TPCANStatus t_Status = CAN_Uninitialize(mu16_PcanHandle);
      if (t_Status != PCAN_ERROR_OK)
      {
         s32_Return = C_COM;
      }
      mq_Initialized = false;
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Reset(void)
{
   int32_t s32_Return = C_NO_ERR;
   const TPCANStatus t_Status = CAN_Reset(mu16_PcanHandle);

   if (t_Status != PCAN_ERROR_OK)
   {
      s32_Return = C_COM;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message)
{
   int32_t s32_Return = C_NO_ERR;

   if (mq_Initialized == false)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      TPCANMsg t_Msg;
      TPCANStatus t_Status;

      t_Msg.ID = orc_Message.u32_ID;
      t_Msg.LEN = orc_Message.u8_DLC;
      t_Msg.MSGTYPE = (orc_Message.u8_XTD != 0U) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
      if (orc_Message.u8_RTR != 0U)
      {
         t_Msg.MSGTYPE = static_cast<TPCANMessageType>(t_Msg.MSGTYPE | PCAN_MESSAGE_RTR);
      }
      for (uint8_t u8_Byte = 0U; u8_Byte < 8U; ++u8_Byte)
      {
         t_Msg.DATA[u8_Byte] = orc_Message.au8_Data[u8_Byte];
      }

      t_Status = CAN_Write(mu16_PcanHandle, &t_Msg);
      if (t_Status != PCAN_ERROR_OK)
      {
         s32_Return = ((t_Status & PCAN_ERROR_QXMTFULL) != 0U) ? C_BUSY : C_COM;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message)
{
   int32_t s32_Return = C_NOACT;

   if (mq_Initialized == false)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      TPCANMsg t_Msg;
      TPCANTimestamp t_TimeStamp;
      const TPCANStatus t_Status = CAN_Read(mu16_PcanHandle, &t_Msg, &t_TimeStamp);

      if (t_Status == PCAN_ERROR_OK)
      {
         orc_Message.u32_ID = t_Msg.ID;
         orc_Message.u8_DLC = t_Msg.LEN;
         orc_Message.u8_XTD = ((t_Msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0U) ? 1U : 0U;
         orc_Message.u8_RTR = ((t_Msg.MSGTYPE & PCAN_MESSAGE_RTR) != 0U) ? 1U : 0U;
         for (uint8_t u8_Byte = 0U; u8_Byte < 8U; ++u8_Byte)
         {
            orc_Message.au8_Data[u8_Byte] = t_Msg.DATA[u8_Byte];
         }
         // PCANBasic timestamp is microseconds since channel init; we report wall-clock-ish time
         // for cross-adapter consistency.
         orc_Message.u64_TimeStamp =
            (static_cast<uint64_t>(t_TimeStamp.micros)) +
            (static_cast<uint64_t>(t_TimeStamp.millis) * 1000ULL) +
            (static_cast<uint64_t>(t_TimeStamp.millis_overflow) * 1000ULL * 0x100000000ULL);

         s32_Return = C_NO_ERR;
      }
      else if ((t_Status & PCAN_ERROR_QRCVEMPTY) != 0U)
      {
         s32_Return = C_NOACT;
      }
      else
      {
         s32_Return = C_COM;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanPeakAdapter::CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const
{
   oru64_SystemTimeUs = TglGetTickCountUs();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanPeakAdapter::GetChannel(void) const
{
   return mu16_Channel;
}

//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscCanPeakAdapter::GetBitrateKbits(void) const
{
   return mu32_BitrateKbits;
}

//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanPeakAdapter::mh_ChannelToPcanHandle(const uint16_t ou16_Channel)
{
   // PCANBasic defines PCAN_USBBUS1..PCAN_USBBUS16 as 0x51..0x60. Default to PCAN_NONEBUS (0x00)
   // for out-of-range input so init returns a clean error.
   uint16_t u16_Handle;

   switch (ou16_Channel)
   {
   case 1U:  u16_Handle = PCAN_USBBUS1;  break;
   case 2U:  u16_Handle = PCAN_USBBUS2;  break;
   case 3U:  u16_Handle = PCAN_USBBUS3;  break;
   case 4U:  u16_Handle = PCAN_USBBUS4;  break;
   case 5U:  u16_Handle = PCAN_USBBUS5;  break;
   case 6U:  u16_Handle = PCAN_USBBUS6;  break;
   case 7U:  u16_Handle = PCAN_USBBUS7;  break;
   case 8U:  u16_Handle = PCAN_USBBUS8;  break;
   case 9U:  u16_Handle = PCAN_USBBUS9;  break;
   case 10U: u16_Handle = PCAN_USBBUS10; break;
   case 11U: u16_Handle = PCAN_USBBUS11; break;
   case 12U: u16_Handle = PCAN_USBBUS12; break;
   case 13U: u16_Handle = PCAN_USBBUS13; break;
   case 14U: u16_Handle = PCAN_USBBUS14; break;
   case 15U: u16_Handle = PCAN_USBBUS15; break;
   case 16U: u16_Handle = PCAN_USBBUS16; break;
   default:  u16_Handle = PCAN_NONEBUS;  break;
   }

   return u16_Handle;
}

//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanPeakAdapter::mh_BitrateKbitsToPcanBaud(const uint32_t ou32_BitrateKbits)
{
   // PCANBasic defines fixed bitrate constants. Anything unrecognized falls back to 500K for safety.
   TPCANBaudrate t_Baud;

   switch (ou32_BitrateKbits)
   {
   case 1000U: t_Baud = PCAN_BAUD_1M;   break;
   case 800U:  t_Baud = PCAN_BAUD_800K; break;
   case 500U:  t_Baud = PCAN_BAUD_500K; break;
   case 250U:  t_Baud = PCAN_BAUD_250K; break;
   case 125U:  t_Baud = PCAN_BAUD_125K; break;
   case 100U:  t_Baud = PCAN_BAUD_100K; break;
   case 95U:   t_Baud = PCAN_BAUD_95K;  break;
   case 83U:   t_Baud = PCAN_BAUD_83K;  break;
   case 50U:   t_Baud = PCAN_BAUD_50K;  break;
   case 47U:   t_Baud = PCAN_BAUD_47K;  break;
   case 33U:   t_Baud = PCAN_BAUD_33K;  break;
   case 20U:   t_Baud = PCAN_BAUD_20K;  break;
   case 10U:   t_Baud = PCAN_BAUD_10K;  break;
   case 5U:    t_Baud = PCAN_BAUD_5K;   break;
   default:    t_Baud = PCAN_BAUD_500K; break;
   }

   return static_cast<uint16_t>(t_Baud);
}
