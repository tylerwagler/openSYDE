//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for device definition file data

   see .cpp file for main information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEDEFINITIONFILER_HPP
#define C_OSCDEVICEDEFINITIONFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <system_error>
#include "C_OscDeviceDefinition.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDeviceDefinitionFiler
{
private:
   static const uint16_t mhu16_FILE_VERSION; ///< file version this class can read / write

   static void mh_ParseOpenSydeAvailability(const C_OscXmlParser & orc_Parser, bool & orq_ProtocolSupportedCan,
                                            bool & orq_ProtocolSupportedEthernet);
   static void mh_ParseOpenSydeFlashloaderParameter(const C_OscXmlParser & orc_Parser,
                                                    uint32_t & oru32_RequestDownloadTimeout,
                                                    uint32_t & oru32_TransferDataTimeout, bool & orq_IsFileBased);

   static std::error_code mh_Load(C_OscDeviceDefinition & orc_DeviceDefinition, C_OscXmlParser & orc_Parser,
                                  const std::string & orc_Path);
   static std::error_code mh_LoadSubDevice(C_OscSubDeviceDefinition & orc_SubDeviceDefinition,
                                           C_OscXmlParser & orc_Parser,
                                           const C_OscDeviceDefinition & orc_DeviceDefinition,
                                           const std::string & orc_Path);
   static void mh_SaveSubDevice(const C_OscSubDeviceDefinition & orc_SubDeviceDefinition, C_OscXmlParser & orc_Parser);
   static std::error_code mh_HandleConnectedInterfaces(C_OscDeviceDefinition & orc_DeviceDefinition);
   static std::error_code mh_CheckContentErrors(const C_OscDeviceDefinition & orc_DeviceDefinition);
   static std::error_code mh_CheckNotConnectedInterface(const C_OscDeviceDefinition & orc_DeviceDefinition);
   static std::error_code mh_CheckNotConnectedInterfaceByType(const C_OscDeviceDefinition & orc_DeviceDefinition,
                                                              const C_OscSystemBus::E_Type oe_Type,
                                                              const uint8_t ou8_NumAvailableInterfaces);
   static std::error_code mh_CheckNotConnectedDevice(const C_OscDeviceDefinition & orc_DeviceDefinition);
   static bool mh_CheckNotConnectedDeviceByType(const C_OscSubDeviceDefinition & orc_SubDeviceDefinition,
                                                const C_OscSystemBus::E_Type oe_Type,
                                                const uint8_t ou8_NumAvailableInterfaces);
   static std::error_code mh_CheckDeviceName(const C_OscDeviceDefinition & orc_DeviceDefinition);
   static std::error_code mh_LoadCanFdProperties(C_OscDeviceDefinition & orc_DeviceDefinition,
                                                 C_OscXmlParser & orc_Parser);
   static void mh_SaveCanFdProperties(const C_OscDeviceDefinition & orc_DeviceDefinition, C_OscXmlParser & orc_Parser);
   static std::error_code mh_LoadCanFdBitrates(std::vector<uint16_t> & orc_CanFdDataBitrates,
                                               C_OscXmlParser & orc_Parser);
   static void mh_SaveCanFdBitrates(const std::vector<uint16_t> & orc_CanFdDataBitrates, C_OscXmlParser & orc_Parser);
   static std::error_code mh_LoadFeatures(std::vector<C_OscSupportedCanInterfaceFeatures> & orc_SupportedCanFeatures,
                                          C_OscXmlParser & orc_Parser);
   static void mh_SaveFeatures(const std::vector<C_OscSupportedCanInterfaceFeatures> & orc_SupportedCanFeatures,
                               C_OscXmlParser & orc_Parser);

public:
   static std::error_code h_Load(C_OscDeviceDefinition & orc_DeviceDefinition, const std::string & orc_Path);
   static std::error_code h_Save(const C_OscDeviceDefinition & orc_DeviceDefinition, const std::string & orc_Path);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
