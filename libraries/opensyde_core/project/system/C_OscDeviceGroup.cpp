//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group of device descriptions (implementation)

   Group of device descriptions

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_OscDeviceGroup.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDeviceGroup::C_OscDeviceGroup(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Search for device with specified name

   The returned pointer points to one of the device instances owned by this class.
   So the caller has to consider the lifetime of the used instance of this class when using
    the returned pointer.

   \param[in]   orc_Name               Searched device name
   \param[in]   orc_MainDeviceName     Main device name (empty if none exists)
   \param[out]  oru32_SubDeviceIndex   Sub device index

   \return
   != NULL:   pointer to found device
   NULL:      define not found
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscDeviceDefinition * C_OscDeviceGroup::LookForDevice(const std::string & orc_Name,
                                                              const std::string & orc_MainDeviceName,
                                                              uint32_t & oru32_SubDeviceIndex) const
{
   const C_OscDeviceDefinition * pc_Device = nullptr;

   for (uint32_t u32_ItDevice = 0U; (u32_ItDevice < this->mc_Devices.size()) && (pc_Device == nullptr); ++u32_ItDevice)
   {
      if (orc_MainDeviceName.empty())
      {
         oru32_SubDeviceIndex = 0UL;
         if (this->mc_Devices[u32_ItDevice].c_DeviceName == orc_Name)
         {
            pc_Device = &(this->mc_Devices[u32_ItDevice]);
         }
      }
      else
      {
         if (this->mc_Devices[u32_ItDevice].c_DeviceName == orc_MainDeviceName)
         {
            for (uint32_t u32_ItSubDevice = 0U; u32_ItSubDevice < this->mc_Devices[u32_ItDevice].c_SubDevices.size();
                 ++u32_ItSubDevice)
            {
               const C_OscSubDeviceDefinition & rc_SubDeviceDefinition =
                  this->mc_Devices[u32_ItDevice].c_SubDevices[u32_ItSubDevice];
               if (rc_SubDeviceDefinition.c_SubDeviceName == orc_Name)
               {
                  pc_Device = &(this->mc_Devices[u32_ItDevice]);
                  oru32_SubDeviceIndex = u32_ItSubDevice;
               }
            }
         }
      }
   }
   return pc_Device;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set group name

   \param[in]  orc_GroupName  New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceGroup::SetGroupName(const std::string & orc_GroupName)
{
   this->mc_GroupName = orc_GroupName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get group name

   \return
   group name
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscDeviceGroup::GetGroupName(void) const
{
   return mc_GroupName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Append a device definition to this group
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceGroup::AddDevice(const C_OscDeviceDefinition & orc_Device)
{
   this->mc_Devices.push_back(orc_Device);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get all devices

   \return
   copy of device definitions owned by this class
*/
//----------------------------------------------------------------------------------------------------------------------
const std::vector<stw::opensyde_core::C_OscDeviceDefinition> & C_OscDeviceGroup::GetDevices(void) const
{
   return mc_Devices;
}
