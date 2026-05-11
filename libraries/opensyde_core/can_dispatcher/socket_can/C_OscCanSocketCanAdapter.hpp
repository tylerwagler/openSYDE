//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       SocketCAN adapter — Linux native CAN interface (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANSOCKETCANADAPTERHPP
#define C_OSCCANSOCKETCANADAPTERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <sys/time.h>

#include "stwtypes.hpp"
#include "C_CanDispatcher.hpp"
#include "C_SclString.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */
const uint32_t mu32_CAN_RX_ID_INVALID = 0xFFFFFFFFUL;

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_OscCanSocketCanAdapter :
   public stw::can::C_CanDispatcher
{
private:
   C_OscCanSocketCanAdapter(const C_OscCanSocketCanAdapter & orc_Source);               //not implemented
   C_OscCanSocketCanAdapter & operator = (const C_OscCanSocketCanAdapter & orc_Source); //not implemented

   int32_t ms32_Socket;
   int32_t ms32_RxTimeout;
   uint32_t mu32_RxId;
   stw::scl::C_SclString mc_InterfaceName;

protected:
   virtual int32_t m_CAN_Read_Msg(stw::can::T_STWCAN_Msg_RX & orc_Message);
   int32_t m_CanReadMsgTimeout(const uint32_t ou32_MaxWaitTimeMs, stw::can::T_STWCAN_Msg_RX & orc_Message) const;

public:
   explicit C_OscCanSocketCanAdapter(const stw::scl::C_SclString & orc_InterfaceName);
   virtual ~C_OscCanSocketCanAdapter(void);

   //C_CanBase interface
   virtual int32_t CAN_Init(void);
   virtual int32_t CAN_Init(const int32_t os32_BitrateKBitS);
   virtual int32_t CAN_Exit(void);
   virtual int32_t CAN_Reset(void);
   virtual int32_t CAN_Send_Msg(const stw::can::T_STWCAN_Msg_TX & orc_Message);
   virtual int32_t CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const;

   //SocketCAN-specific
   int32_t CAN_Init(const stw::scl::C_SclString & orc_InterfaceName, const int32_t os32_RxTimeout = 0);
   stw::scl::C_SclString GetInterfaceName(void) const;

   void SetLimitRxId(const uint32_t ou32_LimitRxId);
   uint32_t GetLimitRxId(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
