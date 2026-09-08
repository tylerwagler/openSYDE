//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Factory for creating CAN adapter instances (implementation)

   Thin wrapper over libcan's ICanBackend::create() that surfaces a fresh C_OscLibcanBackendAdapter
   to openSYDE's C_CanDispatcher consumers. The persisted C_OscCanAdapterConfig is translated into
   a can::ChannelConfig at call time; unused libcan ChannelConfig fields (sample_point,
   listen_only, loopback, …) stay at their defaults until the openSYDE UI grows controls for them.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "C_OscCanAdapterFactory.hpp"
#include "C_OscLibcanBackendAdapter.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
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

   \param[in]   orc_Config             persisted adapter selection (backend + channel + bitrate)
   \param[out]  orc_ErrorDescription   human-readable description on failure; empty on success

   \return
   non-NULL  owning pointer to a fresh dispatcher (not yet opened — caller must CAN_Init())
   NULL      backend not compiled into libcan, or invalid configuration
*/
//----------------------------------------------------------------------------------------------------------------------
C_CanDispatcher * C_OscCanAdapterFactory::h_CreateAdapter(const C_OscCanAdapterConfig & orc_Config,
                                                          std::string & orc_ErrorDescription)
{
   orc_ErrorDescription = "";

   std::unique_ptr< ::can::ICanBackend> c_Backend = ::can::ICanBackend::create(orc_Config.e_BackendKind);
   if (c_Backend == nullptr)
   {
      orc_ErrorDescription = static_cast<std::string>("Backend not compiled in: ") +
                             ::can::backendKindToString(orc_Config.e_BackendKind).c_str();
      return nullptr;
   }

   ::can::ChannelConfig c_LibConfig;
   c_LibConfig.channel_id = orc_Config.c_ChannelId;
   c_LibConfig.bitrate = orc_Config.u32_BitrateBps;

   return new C_OscLibcanBackendAdapter(std::move(c_Backend), c_LibConfig);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Is the given backend compiled into libcan on this build?

   \param[in]  oe_Kind  backend to query

   \return
   true   backend available at runtime
   false  not compiled in
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanAdapterFactory::h_IsBackendAvailable(const ::can::BackendKind oe_Kind)
{
   bool q_Available = false;
   const std::vector< ::can::BackendKind> c_Kinds = ::can::ICanBackend::availableBackends();
   for (size_t s_Idx = 0U; s_Idx < c_Kinds.size(); ++s_Idx)
   {
      if (c_Kinds[s_Idx] == oe_Kind)
      {
         q_Available = true;
         break;
      }
   }
   return q_Available;
}

//----------------------------------------------------------------------------------------------------------------------
std::string C_OscCanAdapterFactory::h_GetBackendDisplayName(const ::can::BackendKind oe_Kind)
{
   return static_cast<std::string>(::can::backendKindToString(oe_Kind).c_str());
}
