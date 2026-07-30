//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Simple typedef to allow OS specific IpDispatcher to be created with same include and name
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCIPDISPATCHERIMPL_HPP
#define C_OSCIPDISPATCHERIMPL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscIpDispatcherLinuxSock.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
typedef C_OscIpDispatcherLinuxSock C_OscIpDispatcherImpl;

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
