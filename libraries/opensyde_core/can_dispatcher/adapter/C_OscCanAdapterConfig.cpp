//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Persisted CAN adapter configuration (implementation)

   Plain-data carrier for the user's CAN adapter selection. The factory consumes this directly:
   e_BackendKind picks the libcan backend, c_ChannelId + u32_BitrateBps populate the
   can::ChannelConfig.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscCanAdapterConfig.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig::C_OscCanAdapterConfig(void) :
   e_BackendKind(::can::BackendKind::SocketCan),
   c_ChannelId(),
   u32_BitrateBps(500000U)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig C_OscCanAdapterConfig::h_GetPlatformDefault(void)
{
   C_OscCanAdapterConfig c_Config;

#ifdef _WIN32
   c_Config.e_BackendKind = ::can::BackendKind::PcanBasic;
   c_Config.c_ChannelId = "PCAN_USBBUS1";
#else
   c_Config.e_BackendKind = ::can::BackendKind::SocketCan;
   c_Config.c_ChannelId = "can0";
#endif
   c_Config.u32_BitrateBps = 500000U;

   return c_Config;
}
