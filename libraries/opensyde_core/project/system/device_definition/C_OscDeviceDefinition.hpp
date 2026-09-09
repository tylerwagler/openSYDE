//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Device definition data container

   See .cpp file for full description

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEDEFINITION_HPP
#define C_OSCDEVICEDEFINITION_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include <cstdint>
#include <string>

#include "C_OscSubDeviceDefinition.hpp"
#include "C_OscSupportedCanInterfaceFeatures.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///container for definition of one openSYDE device
class C_OscDeviceDefinition
{
public:
   C_OscDeviceDefinition(void);
   //lint -sem(stw::opensyde_core::C_OscDeviceDefinition::Clear,initializer)
   void Clear(void);

   std::string GetDisplayName(void) const;

   std::string c_DeviceName;        ///< full device name used for checks
   std::string c_DeviceNameAlias;   ///< full displayed name of device
   std::string c_DeviceDescription; ///< text describing device
   std::string c_ImagePath;         ///< path to file with image of device (e.g. JPG)
   std::string c_FilePath;          ///< used for service update package (#24474)
   uint8_t u8_NumCanBusses;                   ///< number of CAN buses present on device
   uint8_t u8_NumEthernetBusses;              ///< number of ethernet interfaces present on device

   std::vector<uint16_t> c_SupportedBitrates;                              ///< supported CAN bitrates in kbit/s
   std::vector<uint16_t> c_SupportedCanFdDataBitrates;                     ///< supported CAN-FD data bitrates in kbit/s
   std::vector<C_OscSupportedCanInterfaceFeatures> c_SupportedCanFeatures; ///< supported CAN features per interface

   uint8_t u8_ManufacturerId;                        ///< manufacturer id used by device configuration
   std::string c_ManufacturerDisplayValue; ///< manufacturer string, displayed in device configuration, also
                                                     // used for Toolbox Eco-Partners
   std::string c_CompanyLogoLink;          ///< link to company logo (Eco-Partners)
   std::string c_ProductPageLink;          ///< link to product page (Eco-Partner devices)
   std::string c_ToolboxIcon;              ///< link to Toolbox Icon (Eco-Partners)

   std::vector<C_OscSubDeviceDefinition> c_SubDevices;

   static const std::string hc_DEFAULT_COMPANY_NAME;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
