//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Persisted CAN adapter configuration (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANADAPTERCONFIGHPP
#define C_OSCCANADAPTERCONFIGHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

#include "stwtypes.hpp"
#include "can/i_can_backend.h"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Flat record storing the user's CAN adapter selection across openSYDE projects / settings files.
///Mirrors the inputs to C_OscCanAdapterFactory::h_CreateAdapter; one field per persisted attribute.
class C_OscCanAdapterConfig
{
public:
   C_OscCanAdapterConfig(void);

   ::can::BackendKind e_BackendKind; ///< Which libcan backend (SocketCAN / PCAN / Kvaser / VectorXL)
   std::string c_ChannelId;          ///< Backend-specific channel string (e.g. "can0", "PCAN_USBBUS1")
   uint32_t u32_BitrateBps;          ///< Arbitration bitrate in bits per second

   ///Platform-appropriate default: SocketCAN/can0 on Linux, PEAK/PCAN_USBBUS1 on Windows; 500 kbps.
   static C_OscCanAdapterConfig h_GetPlatformDefault(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
