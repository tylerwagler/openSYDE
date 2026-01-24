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
   static QString h_BusTypeEnumToString(const C_OscSystemBus::E_Type oe_Type);
   static int32_t h_BusTypeStringToEnum(const QString & orc_Type, C_OscSystemBus::E_Type & ore_Type);
   static int32_t h_GetParserForExistingFile(C_OscXmlParser & orc_FileXmlParser, const QString & orc_Path,
                                             const QString & orc_RootNode);
   static int32_t h_GetParserForNewFile(C_OscXmlParser & orc_FileXmlParser, const QString & orc_Path,
                                        const QString & orc_RootNode);
   static int32_t h_CreateFolder(const QString & orc_Path);
   static QString h_PrepareItemNameForFileName(const QString & orc_ItemName);
   static QString h_CombinePaths(const QString & orc_BasePathName,
                                               const QString & orc_SubFolderFileName);
   static int32_t h_SaveStringToFile(const QString & orc_CompleteFileAsString,
                                     const QString & orc_CompleteFilePath,
                                     const QString & orc_LogHeading);
   static void h_AdaptProjectPathToSystemDefinition(const QString & orc_ProjectPath,
                                                    QString & orc_SystemDefintionPath);
   static void h_AdaptProjectPathToSystemViews(const QString & orc_ProjectPath,
                                               QString & orc_SystemViewsPath);
   static QString h_CodeExportScalingTypeToString(const C_OscNodeCodeExportSettings::E_Scaling &
                                                                ore_Scaling);
   static int32_t h_StringToCodeExportScalingType(const QString & orc_String,
                                                  C_OscNodeCodeExportSettings::E_Scaling & ore_Scaling);
   static int32_t h_CheckVersion(C_OscXmlParserBase & orc_XmlParser, const uint16_t ou16_ExpectedFileVersion,
                                 const QString & orc_TagName, const QString & orc_UseCase);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
