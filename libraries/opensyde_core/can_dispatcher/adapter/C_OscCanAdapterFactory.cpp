//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Factory for creating CAN adapter instances (implementation)

   Translates a C_OscCanAdapterConfig (openSYDE's persisted CAN setting) into the libcan
   abstraction (can::BackendKind + can::ChannelConfig), constructs the appropriate backend
   through can::ICanBackend::create(), and hands the result back as a stw::can::C_CanDispatcher
   pointer wrapped in C_OscLibcanBackendAdapter.

   All supported backends — SocketCAN, PCANBasic, Kvaser canlib, Vector XL Driver Library —
   live in libcan; this factory only maps openSYDE's two-choice enum (SocketCAN on Linux,
   PEAK on Windows) to libcan's broader BackendKind. Future expansion (Kvaser/Vector picker
   in the UI) only touches this file and C_OscCanAdapterConfig.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdio>
#include <utility>
#include <vector>

#include "C_OscCanAdapterFactory.hpp"
#include "C_OscLibcanBackendAdapter.hpp"
#include "stwerrors.hpp"

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
   orc_ErrorDescription = "";

   ::can::BackendKind e_Kind;
   ::can::ChannelConfig c_LibConfig;

   switch (orc_Config.e_Type)
   {
   case eCAN_ADAPTER_SOCKET_CAN:
      e_Kind = ::can::BackendKind::SocketCan;
      c_LibConfig.channel_id = orc_Config.c_SocketCanInterface.c_str();
      // SocketCAN bitrate is configured externally via `ip link`; libcan ignores ChannelConfig::bitrate
      // for SocketCAN open paths, so any value works here.
      c_LibConfig.bitrate = 500000U;
      break;
   case eCAN_ADAPTER_PEAK:
      {
         e_Kind = ::can::BackendKind::PcanBasic;
         // libcan's PCAN backend resolves channel_id strings of the form "PCAN_USBBUSn".
         // openSYDE currently only models PCAN USB channels; PCI/LAN/ISA buses are a future
         // expansion (no UI surface yet).
         char acn_Channel[24];
         (void)std::snprintf(acn_Channel, sizeof(acn_Channel), "PCAN_USBBUS%u",
                             static_cast<unsigned int>(orc_Config.u16_PeakChannel));
         c_LibConfig.channel_id = acn_Channel;
         c_LibConfig.bitrate = orc_Config.u32_PeakBitrateKbits * 1000U;
      }
      break;
   default:
      orc_ErrorDescription = "Unknown CAN adapter type.";
      return NULL;
   }

   std::unique_ptr< ::can::ICanBackend> c_Backend = ::can::ICanBackend::create(e_Kind);
   if (c_Backend == NULL)
   {
      orc_ErrorDescription = static_cast<C_SclString>("Backend not compiled in: ") +
                             ::can::backendKindToString(e_Kind).c_str();
      return NULL;
   }

   return new C_OscLibcanBackendAdapter(std::move(c_Backend), c_LibConfig);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Is the given adapter type available at runtime?

   Delegates to libcan, which reports backends compiled in at libcan-build time
   (CAN_BACKEND_SOCKETCAN, CAN_BACKEND_PCAN, etc.).

   \param[in]  oe_Type  adapter type to query

   \return
   true   adapter is compiled into libcan on this build
   false  not compiled in
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanAdapterFactory::h_IsAdapterTypeAvailable(const E_CanAdapterType oe_Type)
{
   ::can::BackendKind e_Kind;
   bool q_Mapped = true;

   switch (oe_Type)
   {
   case eCAN_ADAPTER_SOCKET_CAN:
      e_Kind = ::can::BackendKind::SocketCan;
      break;
   case eCAN_ADAPTER_PEAK:
      e_Kind = ::can::BackendKind::PcanBasic;
      break;
   default:
      q_Mapped = false;
      e_Kind = ::can::BackendKind::SocketCan; // unused
      break;
   }

   bool q_Available = false;
   if (q_Mapped == true)
   {
      const std::vector< ::can::BackendKind> c_Kinds = ::can::ICanBackend::availableBackends();
      for (size_t s_Idx = 0U; s_Idx < c_Kinds.size(); ++s_Idx)
      {
         if (c_Kinds[s_Idx] == e_Kind)
         {
            q_Available = true;
            break;
         }
      }
   }
   return q_Available;
}

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
