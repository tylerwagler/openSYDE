//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Configuration data for opening a CAN adapter (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANADAPTERCONFIGHPP
#define C_OSCCANADAPTERCONFIGHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_SclString.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Identifies which CAN adapter implementation should service the bus.
enum E_CanAdapterType
{
   eCAN_ADAPTER_SOCKET_CAN = 0, ///< Linux SocketCAN (raw). Used for every Linux adapter regardless of vendor.
   eCAN_ADAPTER_PEAK       = 1  ///< PEAK PCANBasic native (Windows only).
};

///Flat configuration carrying every adapter-specific field. Unused fields for the active e_Type are ignored.
class C_OscCanAdapterConfig
{
public:
   C_OscCanAdapterConfig(void);

   E_CanAdapterType e_Type;

   // eCAN_ADAPTER_SOCKET_CAN
   stw::scl::C_SclString c_SocketCanInterface; ///< e.g. "can0", "vcan0"

   // eCAN_ADAPTER_PEAK
   uint16_t u16_PeakChannel;        ///< 1..16 (mapped to PCAN_USBBUS1..16 in the adapter)
   uint32_t u32_PeakBitrateKbits;   ///< 125 / 250 / 500 / 1000 / ...

   ///Platform-appropriate default: SocketCAN/can0 on Linux, PEAK/ch1/500kbit on Windows.
   static C_OscCanAdapterConfig h_GetPlatformDefault(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
