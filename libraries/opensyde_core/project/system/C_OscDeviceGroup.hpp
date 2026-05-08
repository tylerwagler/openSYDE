//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group of device descriptions (header)

   Group of device descriptions (note: main module description should be in .cpp file)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEGROUP_HPP
#define C_OSCDEVICEGROUP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "C_OscDeviceDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDeviceGroup
{
public:
   C_OscDeviceGroup(void);

   const C_OscDeviceDefinition * LookForDevice(const stw::scl::C_SclString & orc_Name,
                                               const stw::scl::C_SclString & orc_MainDeviceName,
                                               uint32_t & oru32_SubDeviceIndex) const;

   void SetGroupName(const stw::scl::C_SclString & orc_GroupName);
   stw::scl::C_SclString GetGroupName(void) const;

   void AddDevice(const C_OscDeviceDefinition & orc_Device);

   const std::vector<C_OscDeviceDefinition> & GetDevices(void) const;

private:
   stw::scl::C_SclString mc_GroupName;            ///< Group name (folder path from scan root, "/"-separated)
   std::vector<C_OscDeviceDefinition> mc_Devices; ///< All devices belonging to this group
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
