//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Manager for all device descriptions (header)

   Manager for all device descriptions (note: main module description should be in .cpp file)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEMANAGER_HPP
#define C_OSCDEVICEMANAGER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <string>
#include <system_error>
#include "C_OscDeviceGroup.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDeviceManager
{
public:
   C_OscDeviceManager(void);

   const C_OscDeviceDefinition * LookForDevice(const std::string & orc_Name,
                                               const std::string & orc_MainDeviceName,
                                               uint32_t & oru32_SubDeviceIndex) const;

   std::vector<C_OscDeviceGroup> GetDeviceGroups(void) const;

   bool WasLoaded(void) const;

   std::error_code LoadFromPaths(const std::vector<std::string> & orc_RootPaths);

private:
   bool mq_WasLoaded;
   std::vector<C_OscDeviceGroup> mc_DeviceGroups; ///< Array of all known device groups
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
