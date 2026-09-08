//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node reader/writer (V3)

   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEFILERV3_HPP
#define C_OSCNODEFILERV3_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>
#include "C_OscNode.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///Node reader/writer
class C_OscNodeFiler
{
public:
   static std::error_code h_LoadNodeFile(C_OscNode & orc_Node, const std::string & orc_FilePath,
                                         const bool oq_SkipContent = false);
   static std::error_code h_LoadNode(C_OscNode & orc_Node, C_OscXmlParserBase & orc_XmlParser,
                                     const std::string & orc_BasePath, const bool oq_SkipContent = false);
   static std::error_code h_SaveNodeFile(const C_OscNode & orc_Node, const std::string & orc_FilePath,
                                         std::vector<std::string> * const opc_CreatedFiles, const std::map<uint32_t,
                                         std::string> & orc_NodeIndicesToNameMap);
   static std::error_code h_SaveNode(const C_OscNode & orc_Node, C_OscXmlParserBase & orc_XmlParser,
                                     const std::string & orc_BasePath,
                                     std::vector<std::string> * const opc_CreatedFiles, const std::map<uint32_t,
                                     std::string> & orc_NodeIndicesToNameMap);
   static std::error_code h_LoadNodeComProtocols(std::vector<C_OscCanProtocol> & orc_NodeComProtocols,
                                                 const std::vector<C_OscNodeDataPool> & orc_NodeDataPools,
                                                 C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath);
   static std::error_code h_SaveNodeComProtocols(const std::vector<C_OscCanProtocol> & orc_NodeComProtocols,
                                                 const std::vector<C_OscNodeDataPool> & orc_NodeDataPools,
                                                 C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath,
                                                 std::vector<std::string> * const opc_CreatedFiles);
   static std::string h_GetFolderName(const std::string & orc_NodeName);
   static std::string h_GetFileName(void);

private:
   static std::error_code mh_LoadProperties(C_OscNodeProperties & orc_NodeProperties,
                                            C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveProperties(const C_OscNodeProperties & orc_NodeProperties, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadComInterface(std::vector<C_OscNodeComInterfaceSettings> & orc_ComInterfaces,
                                              C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveComInterface(const std::vector<C_OscNodeComInterfaceSettings> & orc_ComInterfaces,
                                   C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadApplications(std::vector<C_OscNodeApplication> & orc_NodeApplications,
                                              C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveApplications(const std::vector<C_OscNodeApplication> & orc_NodeApplications,
                                   C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadDataPools(C_OscNode & orc_Node, C_OscXmlParserBase & orc_XmlParser,
                                           const std::string & orc_BasePath);
   static std::error_code mh_SaveDataPools(const std::vector<C_OscNodeDataPool> & orc_NodeDataPools,
                                           C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath,
                                           std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code mh_LoadHalc(C_OscHalcConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser,
                                      const std::string & orc_BasePath);
   static std::error_code mh_SaveHalc(const C_OscHalcConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser,
                                      const std::string & orc_BasePath,
                                      std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code mh_LoadCanOpenManagers(std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                 C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath);
   static std::error_code mh_SaveCanOpenManagers(const std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                 C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath,
                                                 std::vector<std::string> * const opc_CreatedFiles,
                                                 const std::map<uint32_t, std::string> & orc_NodeIndicesToNameMap);
   static std::error_code mh_LoadDataLoggers(std::vector<C_OscDataLoggerJob> & orc_Config,
                                             C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath);
   static std::error_code mh_SaveDataLoggers(const std::vector<C_OscDataLoggerJob> & orc_Config,
                                             C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BasePath,
                                             std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code mh_LoadXappProperties(C_OscXappProperties & orc_Config, C_OscXmlParserBase & orc_XmlParser,
                                                const std::string & orc_BasePath);
   static std::error_code mh_SaveXappProperties(const C_OscNode & orc_Config, C_OscXmlParserBase & orc_XmlParser,
                                                const std::string & orc_BasePath,
                                                std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code mh_LoadOsyServerSettings(C_OscNodeOpenSydeServerSettings & orc_Config,
                                                   C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveOsyServerSettings(const C_OscNodeOpenSydeServerSettings & orc_Config,
                                        C_OscXmlParserBase & orc_XmlParser);
   static std::string mh_DiagnosticServerToString(const C_OscNodeProperties::E_DiagnosticServerProtocol &
                                                            ore_DiagnosticProtocol);
   static std::error_code mh_StringToDiagnosticServer(const std::string & orc_String,
                                                      C_OscNodeProperties::E_DiagnosticServerProtocol & ore_Type);
   static std::string mh_FlashLoaderToString(const C_OscNodeProperties::E_FlashLoaderProtocol &
                                                       ore_FlashLoader);
   static std::error_code mh_StringToFlashLoader(const std::string & orc_String,
                                                 C_OscNodeProperties::E_FlashLoaderProtocol & ore_Type);
   static std::string mh_MaxServiceSizeModeTypeToString(const C_OscNodeOpenSydeServerSettings::E_MaxServiceSizeModeType &
                                                                  ore_MaxServiceSizeModeType);
   static std::error_code mh_StringToMaxServiceSizeModeType(
      const std::string & orc_String, C_OscNodeOpenSydeServerSettings::E_MaxServiceSizeModeType & ore_Type);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
