//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility functions for system filers (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMFILERUTIL_HPP
#define C_OSCSYSTEMFILERUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscXmlParser.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscNodeCodeExportSettings.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSystemFilerUtil
{
public:
   static std::string h_BusTypeEnumToString(const C_OscSystemBus::E_Type oe_Type);
   static int32_t h_BusTypeStringToEnum(const std::string & orc_Type, C_OscSystemBus::E_Type & ore_Type);
   static int32_t h_GetParserForExistingFile(C_OscXmlParser & orc_FileXmlParser, const std::string & orc_Path,
                                             const std::string & orc_RootNode);
   static int32_t h_GetParserForNewFile(C_OscXmlParser & orc_FileXmlParser, const std::string & orc_Path,
                                        const std::string & orc_RootNode);
   static int32_t h_CreateFolder(const std::string & orc_Path);
   static std::string h_PrepareItemNameForFileName(const std::string & orc_ItemName);
   static std::string h_CombinePaths(const std::string & orc_BasePathName,
                                               const std::string & orc_SubFolderFileName);
   static int32_t h_SaveStringToFile(const std::string & orc_CompleteFileAsString,
                                     const std::string & orc_CompleteFilePath,
                                     const std::string & orc_LogHeading);
   static void h_AdaptProjectPathToSystemDefinition(const std::string & orc_ProjectPath,
                                                    std::string & orc_SystemDefintionPath);
   static void h_AdaptProjectPathToSystemViews(const std::string & orc_ProjectPath,
                                               std::string & orc_SystemViewsPath);
   static std::string h_CodeExportScalingTypeToString(const C_OscNodeCodeExportSettings::E_Scaling &
                                                                ore_Scaling);
   static int32_t h_StringToCodeExportScalingType(const std::string & orc_String,
                                                  C_OscNodeCodeExportSettings::E_Scaling & ore_Scaling);
   static int32_t h_CheckVersion(C_OscXmlParserBase & orc_XmlParser, const uint16_t ou16_ExpectedFileVersion,
                                 const std::string & orc_TagName, const std::string & orc_UseCase);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
