//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       SocketCAN adapter — Linux native CAN interface (implementation)

   Linux-only adapter that talks directly to the kernel's SocketCAN subsystem via raw PF_CAN sockets.
   Replaces the legacy can_dispatcher/target_linux_socket_can/C_Can class. The interface name (e.g.
   "can0", "vcan0") is passed in the constructor; bitrate is configured outside the application via
   `ip link set canX up type can bitrate N` and is therefore ignored by CAN_Init(int32_t).

   The kernel's PEAK-USB / Kvaser / IXXAT drivers expose their devices as SocketCAN interfaces, so
   this adapter covers every Linux CAN scenario in practice.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <poll.h>

#include "C_OscCanSocketCanAdapter.hpp"
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
C_OscCanSocketCanAdapter::C_OscCanSocketCanAdapter(const C_SclString & orc_InterfaceName) :
   C_CanDispatcher(),
   ms32_Socket(-1),
   ms32_RxTimeout(0),
   mu32_RxId(mu32_CAN_RX_ID_INVALID),
   mc_InterfaceName(orc_InterfaceName)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanSocketCanAdapter::~C_OscCanSocketCanAdapter(void)
{
   if (ms32_Socket >= 0)
   {
      (void)this->C_OscCanSocketCanAdapter::CAN_Exit();
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Init(void)
{
   return this->CAN_Init(mc_InterfaceName, 0);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Init(const int32_t os32_BitrateKBitS)
{
   // SocketCAN bitrate is configured externally (`ip link set canX up type can bitrate N`).
   (void)os32_BitrateKBitS;
   return this->CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Init(const C_SclString & orc_InterfaceName, const int32_t os32_RxTimeout)
{
   int32_t s32_RetVal = C_NO_ERR;
   struct sockaddr_can c_Addr;
   struct ifreq c_IfRequest;

   (void)memcpy(c_IfRequest.ifr_name, orc_InterfaceName.c_str(), sizeof(c_IfRequest.ifr_name));

   ms32_Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
   if (ms32_Socket >= 0)
   {
      if (ioctl(ms32_Socket, SIOCGIFINDEX, &c_IfRequest) >= 0)
      {
         struct can_filter ac_CanFilter[1];

         if (mu32_RxId == mu32_CAN_RX_ID_INVALID)
         {
            ac_CanFilter[0].can_id = 0U;
            ac_CanFilter[0].can_mask = 0U;
         }
         else
         {
            ac_CanFilter[0].can_id = mu32_RxId;
            ac_CanFilter[0].can_mask = ((mu32_RxId & 0x1FFFFFFFU) != 0U) ? CAN_EFF_MASK : CAN_SFF_MASK;
         }
         if (setsockopt(ms32_Socket, SOL_CAN_RAW, CAN_RAW_FILTER, &ac_CanFilter, sizeof(ac_CanFilter)) == 0)
         {
            c_Addr.can_family = AF_CAN;
            c_Addr.can_ifindex = c_IfRequest.ifr_ifindex;
            //lint -e{9176}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
            if (bind(ms32_Socket, reinterpret_cast<struct sockaddr *>(&c_Addr), sizeof(c_Addr)) < 0)
            {
               (void)this->CAN_Exit();
               s32_RetVal = C_CONFIG;
            }
            ms32_RxTimeout = os32_RxTimeout;
            mc_InterfaceName = orc_InterfaceName;
         }
         else
         {
            (void)this->CAN_Exit();
            s32_RetVal = C_CONFIG;
         }
      }
      else
      {
         (void)this->CAN_Exit();
         s32_RetVal = C_CONFIG;
      }
   }
   else
   {
      s32_RetVal = C_CONFIG;
   }

   return s32_RetVal;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Exit(void)
{
   int32_t s32_Return = C_NO_ERR;

   if (ms32_Socket >= 0)
   {
      (void)close(ms32_Socket);
      ms32_Socket = -1;
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Reset(void)
{
   (void)this->CAN_Exit();
   return this->CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message)
{
   int32_t s32_Error;

   if (this->ms32_Socket >= 0)
   {
      struct pollfd c_PollFd;
      struct can_frame c_Frame;
      int32_t s32_Ret;

      c_Frame.can_id = CAN_ERR_MASK & orc_Message.u32_ID;
      if (orc_Message.u8_XTD != 0)
      {
         c_Frame.can_id |= CAN_EFF_FLAG;
      }
      if (orc_Message.u8_RTR != 0)
      {
         c_Frame.can_id |= CAN_RTR_FLAG;
      }
      c_Frame.can_dlc = orc_Message.u8_DLC;
      for (uint8_t u8_Byte = 0U; u8_Byte < 8U; ++u8_Byte)
      {
         c_Frame.data[u8_Byte] = orc_Message.au8_Data[u8_Byte];
      }

      c_PollFd.fd = this->ms32_Socket;
      c_PollFd.events = POLLOUT;
      s32_Ret = poll(&c_PollFd, 1, 50);
      if (s32_Ret > 0)
      {
         if ((c_PollFd.revents & POLLOUT) != 0)
         {
            s32_Ret = write(this->ms32_Socket, &c_Frame, sizeof(c_Frame));
            s32_Error = (s32_Ret == sizeof(c_Frame)) ? C_NO_ERR : C_COM;
         }
         else
         {
            s32_Error = C_COM;
         }
      }
      else if (s32_Ret == 0)
      {
         s32_Error = C_BUSY;
      }
      else
      {
         s32_Error = ((errno == EAGAIN) || (errno == EINTR)) ? C_BUSY : C_COM;
      }
   }
   else
   {
      s32_Error = C_CONFIG;
   }

   return s32_Error;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message)
{
   return this->m_CanReadMsgTimeout(static_cast<uint32_t>(ms32_RxTimeout), orc_Message);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::m_CanReadMsgTimeout(const uint32_t ou32_MaxWaitTimeMs,
                                                      T_STWCAN_Msg_RX & orc_Message) const
{
   int32_t s32_Error;

   if (this->ms32_Socket >= 0)
   {
      struct pollfd c_PollFd;
      int32_t s32_Ret;

      c_PollFd.fd = this->ms32_Socket;
      c_PollFd.events = POLLIN;
      s32_Ret = poll(&c_PollFd, 1, ou32_MaxWaitTimeMs);
      if (s32_Ret > 0)
      {
         s32_Error = C_COM;
         if ((c_PollFd.revents & POLLIN) != 0)
         {
            struct can_frame c_Frame;
            const ssize_t x_Ret = //lint !e8080  matches POSIX read() prototype
                                  read(this->ms32_Socket, &c_Frame, sizeof(c_Frame));
            if (x_Ret == static_cast<ssize_t>(sizeof(c_Frame)))
            {
               orc_Message.u64_TimeStamp = TglGetTickCountUs();
               orc_Message.u32_ID = c_Frame.can_id & CAN_ERR_MASK;
               orc_Message.u8_XTD = ((c_Frame.can_id & CAN_EFF_FLAG) == 0U) ? 0U : 1U;
               orc_Message.u8_RTR = ((c_Frame.can_id & CAN_RTR_FLAG) == 0U) ? 0U : 1U;
               orc_Message.u8_DLC = c_Frame.can_dlc;
               for (uint8_t u8_Byte = 0U; u8_Byte < 8U; ++u8_Byte)
               {
                  orc_Message.au8_Data[u8_Byte] = c_Frame.data[u8_Byte];
               }
               s32_Error = C_NO_ERR;
            }
         }
      }
      else if (s32_Ret == 0)
      {
         s32_Error = C_NOACT;
      }
      else
      {
         s32_Error = ((errno == EAGAIN) || (errno == EINTR)) ? C_NOACT : C_COM;
      }
   }
   else
   {
      s32_Error = C_CONFIG;
   }

   return s32_Error;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSocketCanAdapter::CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const
{
   oru64_SystemTimeUs = TglGetTickCountUs();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscCanSocketCanAdapter::GetInterfaceName(void) const
{
   return mc_InterfaceName;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSocketCanAdapter::SetLimitRxId(const uint32_t ou32_LimitRxId)
{
   mu32_RxId = ou32_LimitRxId;
}

//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscCanSocketCanAdapter::GetLimitRxId(void) const
{
   return mu32_RxId;
}
