//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Adapter wrapping can::ICanBackend (libcan) as a stw::can::C_CanDispatcher (implementation)

   Bridges the openSYDE protocol stack's synchronous C_CanDispatcher interface to libcan's
   can::ICanBackend. Each opened backend (SocketCAN, PCANBasic, Kvaser, Vector XL) flows through
   the same shim, so adding adapters is a libcan concern — openSYDE stays unaware.

   Frame translation maps stw::can::T_STWCAN_Msg_TX/RX (Pascal-style 8-byte payload, dedicated
   XTD/RTR flags, microsecond timestamp) to can::Frame (SocketCAN-shaped, extended/remote/error
   booleans, microsecond timestamp). DLC is clamped to 8; CAN-FD payloads larger than 8 bytes
   are silently truncated until openSYDE's protocol stack grows FD support.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>

#include "C_OscLibcanBackendAdapter.hpp"
#include "stwerrors.hpp"
#include "TglTime.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
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
C_OscLibcanBackendAdapter::C_OscLibcanBackendAdapter(std::unique_ptr< ::can::ICanBackend> opc_Backend,
                                                     const ::can::ChannelConfig & orc_Config) :
   C_CanDispatcher(),
   mpc_Backend(std::move(opc_Backend)),
   mc_Config(orc_Config),
   mq_Open(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscLibcanBackendAdapter::~C_OscLibcanBackendAdapter(void)
{
   if (mq_Open == true)
   {
      (void)this->C_OscLibcanBackendAdapter::CAN_Exit();
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Init(void)
{
   int32_t s32_Return = C_NO_ERR;

   if ((mpc_Backend == NULL) || (mpc_Backend->open(mc_Config) == false))
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      mq_Open = true;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Init(const int32_t os32_BitrateKBitS)
{
   if (os32_BitrateKBitS > 0)
   {
      mc_Config.bitrate = static_cast<uint32_t>(os32_BitrateKBitS) * 1000U;
   }
   return this->CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Exit(void)
{
   int32_t s32_Return = C_NO_ERR;

   if (mq_Open == true)
   {
      if (mpc_Backend != NULL)
      {
         mpc_Backend->close();
      }
      mq_Open = false;
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Reset(void)
{
   (void)this->CAN_Exit();
   return this->CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message)
{
   int32_t s32_Return;

   if ((mq_Open == false) || (mpc_Backend == NULL))
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      ::can::Frame c_Frame;
      c_Frame.id = orc_Message.u32_ID;
      c_Frame.dlc = (orc_Message.u8_DLC > 8U) ? 8U : orc_Message.u8_DLC;
      c_Frame.is_extended_id = (orc_Message.u8_XTD != 0U);
      c_Frame.is_remote_frame = (orc_Message.u8_RTR != 0U);
      for (uint8_t u8_Byte = 0U; u8_Byte < c_Frame.dlc; ++u8_Byte)
      {
         c_Frame.data[u8_Byte] = orc_Message.au8_Data[u8_Byte];
      }

      s32_Return = mpc_Backend->send(c_Frame) ? C_NO_ERR : C_COM;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message)
{
   int32_t s32_Return;

   if ((mq_Open == false) || (mpc_Backend == NULL))
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      ::can::Frame c_Frame;
      const bool q_Got = mpc_Backend->receive(c_Frame, std::chrono::milliseconds(0));
      if (q_Got == true)
      {
         orc_Message.u32_ID = c_Frame.id;
         orc_Message.u8_DLC = (c_Frame.dlc > 8U) ? 8U : c_Frame.dlc;
         orc_Message.u8_XTD = c_Frame.is_extended_id ? 1U : 0U;
         orc_Message.u8_RTR = c_Frame.is_remote_frame ? 1U : 0U;
         std::memset(orc_Message.au8_Data, 0, sizeof(orc_Message.au8_Data));
         for (uint8_t u8_Byte = 0U; u8_Byte < orc_Message.u8_DLC; ++u8_Byte)
         {
            orc_Message.au8_Data[u8_Byte] = c_Frame.data[u8_Byte];
         }
         orc_Message.u64_TimeStamp = (c_Frame.timestamp_us != 0ULL) ? c_Frame.timestamp_us : TglGetTickCountUs();
         s32_Return = C_NO_ERR;
      }
      else
      {
         s32_Return = C_NOACT;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscLibcanBackendAdapter::CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const
{
   oru64_SystemTimeUs = TglGetTickCountUs();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
::can::ICanBackend * C_OscLibcanBackendAdapter::GetBackend(void) const
{
   return mpc_Backend.get();
}
