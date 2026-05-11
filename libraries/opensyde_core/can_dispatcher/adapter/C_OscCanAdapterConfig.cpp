//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Configuration data for opening a CAN adapter (implementation)

   Plain-data carrier for opening a CAN adapter. The active discriminator is e_Type; only the fields
   corresponding to that type are read by the factory. The default constructor leaves all fields
   zero/empty — call h_GetPlatformDefault() to obtain a usable starting configuration for the host OS.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscCanAdapterConfig.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig::C_OscCanAdapterConfig(void) :
   e_Type(eCAN_ADAPTER_SOCKET_CAN),
   c_SocketCanInterface(""),
   u16_PeakChannel(0U),
   u32_PeakBitrateKbits(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig C_OscCanAdapterConfig::h_GetPlatformDefault(void)
{
   C_OscCanAdapterConfig c_Config;

#ifdef _WIN32
   c_Config.e_Type = eCAN_ADAPTER_PEAK;
   c_Config.u16_PeakChannel = 1U;
   c_Config.u32_PeakBitrateKbits = 500U;
#else
   c_Config.e_Type = eCAN_ADAPTER_SOCKET_CAN;
   c_Config.c_SocketCanInterface = "can0";
#endif

   return c_Config;
}
