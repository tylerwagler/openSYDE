//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Factory for creating CAN adapter instances (implementation)

   Single entry point used by all openSYDE applications to instantiate a CAN dispatcher. The returned
   pointer owns a freshly-constructed adapter; the caller is responsible for calling CAN_Init() (or
   CAN_Init(bitrate)) to open the bus and for deleting the dispatcher when done.

   Adapters that are not compiled in for the current platform (e.g. PEAK on Linux, SocketCAN on
   Windows) return NULL with a populated error description.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscCanAdapterFactory.hpp"
#include "stwerrors.hpp"

#ifndef _WIN32
#include "C_OscCanSocketCanAdapter.hpp"
#endif

#if defined(_WIN32) && defined(OPENSYDE_HAVE_PEAK_ADAPTER)
#include "C_OscCanPeakAdapter.hpp"
#endif

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::can;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Construct a CAN dispatcher for the given configuration.

   \param[in]   orc_Config             requested adapter type + adapter-specific parameters
   \param[out]  orc_ErrorDescription   human-readable description on failure; empty on success

   \return
   non-NULL  owning pointer to a fresh dispatcher (not yet opened — caller must CAN_Init())
   NULL      adapter type not available on this platform or invalid configuration
*/
//----------------------------------------------------------------------------------------------------------------------
C_CanDispatcher * C_OscCanAdapterFactory::h_CreateAdapter(const C_OscCanAdapterConfig & orc_Config,
                                                          C_SclString & orc_ErrorDescription)
{
   C_CanDispatcher * pc_Adapter = NULL;

   orc_ErrorDescription = "";

   switch (orc_Config.e_Type)
   {
   case eCAN_ADAPTER_SOCKET_CAN:
#ifdef _WIN32
      orc_ErrorDescription = "SocketCAN adapter is not available on Windows.";
#else
      pc_Adapter = new C_OscCanSocketCanAdapter(orc_Config.c_SocketCanInterface);
#endif
      break;
   case eCAN_ADAPTER_PEAK:
#if defined(_WIN32) && defined(OPENSYDE_HAVE_PEAK_ADAPTER)
      pc_Adapter = new C_OscCanPeakAdapter(orc_Config.u16_PeakChannel, orc_Config.u32_PeakBitrateKbits);
#elif defined(_WIN32)
      orc_ErrorDescription = "PEAK adapter not compiled in (OPENSYDE_ENABLE_PEAK_ADAPTER=OFF).";
#else
      orc_ErrorDescription = "PEAK native adapter is not available on Linux. "
                             "Use SocketCAN with the PEAK kernel driver instead.";
#endif
      break;
   default:
      orc_ErrorDescription = "Unknown CAN adapter type.";
      break;
   }

   return pc_Adapter;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Is the given adapter type compiled in for the current platform?

   \param[in]  oe_Type  adapter type to query

   \return
   true   adapter is available
   false  not compiled in for this platform
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanAdapterFactory::h_IsAdapterTypeAvailable(const E_CanAdapterType oe_Type)
{
   bool q_Available = false;

   switch (oe_Type)
   {
   case eCAN_ADAPTER_SOCKET_CAN:
#ifndef _WIN32
      q_Available = true;
#endif
      break;
   case eCAN_ADAPTER_PEAK:
#if defined(_WIN32) && defined(OPENSYDE_HAVE_PEAK_ADAPTER)
      q_Available = true;
#endif
      break;
   default:
      break;
   }

   return q_Available;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Display name for the given adapter type.

   \param[in]  oe_Type  adapter type

   \return
   human-readable name (e.g. "SocketCAN", "PEAK")
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscCanAdapterFactory::h_GetAdapterTypeDisplayName(const E_CanAdapterType oe_Type)
{
   C_SclString c_Name;

   switch (oe_Type)
   {
   case eCAN_ADAPTER_SOCKET_CAN:
      c_Name = "SocketCAN";
      break;
   case eCAN_ADAPTER_PEAK:
      c_Name = "PEAK";
      break;
   default:
      c_Name = "Unknown";
      break;
   }

   return c_Name;
}
