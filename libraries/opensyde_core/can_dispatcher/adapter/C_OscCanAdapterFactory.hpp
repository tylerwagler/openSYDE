//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Factory for creating CAN adapter instances (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANADAPTERFACTORYHPP
#define C_OSCCANADAPTERFACTORYHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_SclString.hpp"
#include "C_CanDispatcher.hpp"
#include "C_OscCanAdapterConfig.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_OscCanAdapterFactory
{
public:
   static stw::can::C_CanDispatcher * h_CreateAdapter(const C_OscCanAdapterConfig & orc_Config,
                                                      stw::scl::C_SclString & orc_ErrorDescription);

   static bool h_IsBackendAvailable(const ::can::BackendKind oe_Kind);
   static stw::scl::C_SclString h_GetBackendDisplayName(const ::can::BackendKind oe_Kind);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
