//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Adapter wrapping can::ICanBackend (libcan) as a stw::can::C_CanDispatcher (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCLIBCANBACKENDADAPTERHPP
#define C_OSCLIBCANBACKENDADAPTERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <memory>

#include "stwtypes.hpp"
#include "C_CanDispatcher.hpp"
#include "can/i_can_backend.h"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_OscLibcanBackendAdapter :
   public stw::can::C_CanDispatcher
{
private:
   C_OscLibcanBackendAdapter(const C_OscLibcanBackendAdapter & orc_Source);
   C_OscLibcanBackendAdapter & operator = (const C_OscLibcanBackendAdapter & orc_Source);

   std::unique_ptr< ::can::ICanBackend> mpc_Backend;
   ::can::ChannelConfig mc_Config;
   bool mq_Open;

protected:
   virtual int32_t m_CAN_Read_Msg(stw::can::T_STWCAN_Msg_RX & orc_Message);

public:
   C_OscLibcanBackendAdapter(std::unique_ptr< ::can::ICanBackend> opc_Backend, const ::can::ChannelConfig & orc_Config);
   virtual ~C_OscLibcanBackendAdapter(void);

   virtual int32_t CAN_Init(void);
   virtual int32_t CAN_Init(const int32_t os32_BitrateKBitS);
   virtual int32_t CAN_Exit(void);
   virtual int32_t CAN_Reset(void);
   virtual int32_t CAN_Send_Msg(const stw::can::T_STWCAN_Msg_TX & orc_Message);
   virtual int32_t CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const;

   ::can::ICanBackend * GetBackend(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
