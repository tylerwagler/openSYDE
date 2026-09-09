//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load/save system definition data from/to file (V3)

   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMDEFINITIONFILER_HPP
#define C_OSCSYSTEMDEFINITIONFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>
#include <system_error>
#include "C_OscNode.hpp"
#include "C_OscNodeFiler.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscSystemBusFiler.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSystemDefinitionFiler
{
public:
   static std::error_code h_LoadSystemDefinitionFile(
      C_OscSystemDefinition & orc_SystemDefinition, const std::string & orc_PathSystemDefinition,
      const std::string & orc_PathDeviceDefinitions, const bool oq_UseDeviceDefinitions = true,
      uint16_t * const opu16_ReadFileVersion = nullptr, const std::vector<uint8_t> * const opc_NodesToLoad = nullptr,
      const bool oq_SkipContent = false, const std::string * const opc_ExpectedNodeName = nullptr,
      std::vector<std::string> * const opc_ErrorDetailsMissingDevices = nullptr);
   static std::error_code h_SaveSystemDefinitionFile(const C_OscSystemDefinition & orc_SystemDefinition,
                                                     const std::string & orc_Path,
                                                     std::vector<std::string> * const opc_CreatedFiles = nullptr);
   static std::error_code h_LoadNodes(std::vector<C_OscNode> & orc_Nodes, C_OscXmlParserBase & orc_XmlParser,
                                      const C_OscDeviceManager & orc_DeviceDefinitions,
                                      const std::string & orc_BasePath, const bool oq_UseDeviceDefinitions = true,
                                      const bool oq_UseFileInterface = true,
                                      const std::vector<uint8_t> * const opc_NodesToLoad = nullptr,
                                      const bool oq_SkipContent = false,
                                      const std::string * const opc_ExpectedNodeName = nullptr,
                                      std::vector<std::string> * const opc_ErrorDetailsMissingDevices = nullptr);
   static std::error_code h_LoadBuses(std::vector<C_OscSystemBus> & orc_Buses, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_SaveNodes(const std::vector<C_OscNode> & orc_Nodes, C_OscXmlParserBase & orc_XmlParser,
                                      const std::string & orc_BasePath,
                                      std::vector<std::string> * const opc_CreatedFiles);
   static void h_SaveBuses(const std::vector<C_OscSystemBus> & orc_Buses, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_LoadSystemDefinition(
      C_OscSystemDefinition & orc_SystemDefinition, C_OscXmlParserBase & orc_XmlParser,
      const std::string & orc_PathDeviceDefinitions, const std::string & orc_BasePath,
      const bool oq_UseDeviceDefinitions = true, uint16_t * const opu16_ReadFileVersion = nullptr,
      const std::vector<uint8_t> * const opc_NodesToLoad = nullptr, const bool oq_SkipContent = false,
      const std::string * const opc_ExpectedNodeName = nullptr,
      std::vector<std::string> * const opc_ErrorDetailsMissingDevices = nullptr);
   static std::error_code h_SaveSystemDefinition(const C_OscSystemDefinition & orc_SystemDefinition,
                                                 C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath,
                                                 std::vector<std::string> * const opc_CreatedFiles);

   static void h_SplitDeviceType(const std::string & orc_CompleteType, std::string & orc_MainType,
                                 std::string & orc_SubType);

   ///known file versions
   static const uint16_t hu16_FILE_VERSION_1 = 1U;
   static const uint16_t hu16_FILE_VERSION_2 = 2U;
   static const uint16_t hu16_FILE_VERSION_3 = 3U;
   static const uint16_t hu16_FILE_VERSION_LATEST = hu16_FILE_VERSION_3;

private:
   static std::map<uint32_t, std::string> mh_MapNodeIndicesToName(const std::vector<C_OscNode> & orc_Nodes);
   static std::error_code mh_LoadSystemDefinitionProperties(C_OscSystemDefinition & orc_SystemDefinition,
                                                            C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveSystemDefinitionProperties(const C_OscSystemDefinition & orc_SystemDefinition,
                                                 C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
