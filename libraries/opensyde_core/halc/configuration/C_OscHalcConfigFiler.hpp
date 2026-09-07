//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC configuration data
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGFILER_HPP
#define C_OSCHALCCONFIGFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "stwtypes.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscHalcConfig.hpp"
#include "C_OscHalcDefStruct.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcConfigFiler
{
public:
   static std::error_code h_LoadFile(C_OscHalcConfig & orc_IoData, const std::string & orc_Path,
                                     const std::string & orc_BasePath);
   static std::error_code h_SaveFile(const C_OscHalcConfig & orc_IoData, const std::string & orc_Path,
                                     const std::string & orc_BasePath,
                                     std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code h_LoadData(C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                     const std::string & orc_BasePath);
   static std::error_code h_SaveData(const C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                     const std::string & orc_BasePath,
                                     std::vector<std::string> * const opc_CreatedFiles);

   static std::error_code h_PrepareForFile(const std::string & orc_Path);

   static std::error_code h_SaveIoDomain(const C_OscHalcConfigDomain & orc_IoDomain,
                                         C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_LoadIoDomain(C_OscHalcConfigDomain & orc_IoDomain, C_OscXmlParserBase & orc_XmlParser);

private:
   static const uint16_t mhu16_FILE_VERSION_1 = 1U;

   C_OscHalcConfigFiler();

   static std::error_code mh_SaveIoDataBase(const C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                            const std::string & orc_BasePath,
                                            std::vector<std::string> * const opc_CreatedFiles);
   static std::error_code mh_SaveIoDomains(const C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_SaveIoChannels(const std::vector<C_OscHalcConfigChannel> & orc_Channels,
                                            C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_SaveIoChannel(const C_OscHalcConfigChannel & orc_Channel,
                                           C_OscXmlParserBase & orc_XmlParser, const std::string & orc_NodeName,
                                           const std::string & orc_NodeParentName);
   static std::error_code mh_SaveIoParameterStructs(
      const std::vector<C_OscHalcConfigParameterStruct> & orc_ParameterStructs, C_OscXmlParserBase & orc_XmlParser,
      const std::string & orc_NodeName);
   static std::error_code mh_SaveIoParameterStruct(const C_OscHalcConfigParameterStruct & orc_ParameterStruct,
                                                   C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_SaveIoParameters(const std::vector<C_OscHalcConfigParameter> & orc_Parameters,
                                              C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_SaveIoParameter(const C_OscHalcConfigParameter & orc_Parameter,
                                             C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BaseNode);
   static std::error_code mh_LoadIoDataBase(C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                            const std::string & orc_BasePath);
   static std::error_code mh_LoadIoDomains(C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadIoChannel(C_OscHalcConfigChannel & orc_IoChannel, C_OscXmlParserBase & orc_XmlParser,
                                           const std::string & orc_NodeName);
   static std::error_code mh_LoadIoParameterStructs(std::vector<C_OscHalcConfigParameterStruct> & orc_ParameterStructs,
                                                    C_OscXmlParserBase & orc_XmlParser,
                                                    const std::string & orc_NodeName);
   static std::error_code mh_LoadIoParameterStruct(C_OscHalcConfigParameterStruct & orc_ParameterStruct,
                                                   C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadIoParameters(std::vector<C_OscHalcConfigParameter> & orc_Parameters,
                                              C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadIoParameter(C_OscHalcConfigParameter & orc_Parameter,
                                             C_OscXmlParserBase & orc_XmlParser, const std::string & orc_BaseName);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
