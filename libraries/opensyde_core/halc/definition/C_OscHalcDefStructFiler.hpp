//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC definition struct section
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFSTRUCTFILER_HPP
#define C_OSCHALCDEFSTRUCTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscHalcDefStruct.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscHalcDefChannelUseCase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefStructFiler
{
public:
   static std::error_code h_LoadStructs(std::vector<C_OscHalcDefStruct> & orc_Structs,
                                        C_OscXmlParserBase & orc_XmlParser,
                                        const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                        const std::string & orc_CurrentNodeName,
                                        const std::string & orc_SectionNodeName, const std::string & orc_GroupNodeName,
                                        const std::string & orc_SingleNodeName, const bool oq_RequireId,
                                        const bool oq_RequireSection, const uint32_t ou32_DomainNameLength);
   static std::error_code h_SaveStructs(const std::vector<C_OscHalcDefStruct> & orc_Structs,
                                        const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                        C_OscXmlParserBase & orc_XmlParser, const std::string & orc_CurrentNodeName,
                                        const std::string & orc_SectionNodeName,
                                        const std::string & orc_GroupNodeName,
                                        const std::string & orc_SingleNodeName);
   static std::error_code h_ParseSimplestTypeValue(const std::string & orc_TypeStr,
                                                   C_OscNodeDataPoolContent & orc_Content,
                                                   const C_OscXmlParserBase & orc_XmlParser,
                                                   const std::string & orc_AttributeName);
   static std::error_code h_GetTypeForSimplestTypeString(const std::string & orc_TypeStr,
                                                         C_OscNodeDataPoolContent::E_Type & ore_Type,
                                                         const C_OscXmlParserBase & orc_XmlParser);
   static std::string h_GetTypeString(const C_OscNodeDataPoolContent::E_Type oe_Type);
   static std::error_code h_SaveSimpleValueAsAttribute(const std::string & orc_Node,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const C_OscNodeDataPoolContent & orc_Content);
   static std::error_code h_SetType(C_OscXmlParserBase & orc_XmlParser, C_OscHalcDefContent & orc_Content,
                                    std::string & orc_TypeStr, std::string & orc_BaseTypeStr,
                                    const std::string & orc_CurrentNodeName);
   static std::error_code h_ParseAttributeIntoContent(C_OscHalcDefContent & orc_Content,
                                                      const C_OscXmlParserBase & orc_XmlParser,
                                                      const std::string & orc_AttributeName,
                                                      const std::string & orc_Type,
                                                      const std::string & orc_BaseType,
                                                      const bool oq_RequireAttribute);

   static const uint32_t hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH;

private:
   static const std::string mhc_FALSE;
   static const std::string mhc_TRUE;

   C_OscHalcDefStructFiler(void);

   static std::error_code mh_LoadStruct(C_OscHalcDefStruct & orc_Struct, C_OscXmlParserBase & orc_XmlParser,
                                        const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                        const std::string & orc_GroupNodeName,
                                        const std::string & orc_SingleNodeName, const bool oq_RequireId,
                                        const uint32_t ou32_DomainNameLength);
   static std::error_code mh_SaveStruct(const C_OscHalcDefStruct & orc_Struct,
                                        const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                        C_OscXmlParserBase & orc_XmlParser, const std::string & orc_GroupNodeName,
                                        const std::string & orc_SingleNodeName);
   static std::error_code mh_LoadDataElement(C_OscHalcDefElement & orc_Element, C_OscXmlParserBase & orc_XmlParser,
                                             const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                             const std::vector<uint32_t> & orc_BaseAvailability,
                                             const std::string & orc_SingleNodeName, const bool oq_RequireId,
                                             const uint32_t ou32_DomainNameLength);
   static std::error_code mh_SaveDataElement(const C_OscHalcDefElement & orc_Element,
                                             const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                             C_OscXmlParserBase & orc_XmlParser,
                                             const std::string & orc_SingleNodeName);
   static std::error_code mh_ParseAttributeAvailability(std::vector<uint32_t> & orc_Availability,
                                                        const std::string & orc_AttributeContent,
                                                        const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                        const C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_GetAvailabilityString(const std::vector<uint32_t> & orc_Availability,
                                                   const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                   std::string & orc_Output);
   static void mh_SetMaxValForType(const std::string & orc_TypeStr, C_OscNodeDataPoolContent & orc_Content);
   static void mh_SetMinValForType(const std::string & orc_TypeStr, C_OscNodeDataPoolContent & orc_Content);
   static bool mh_CheckInitialBitmaskContentValid(const C_OscHalcDefContent & orc_Content,
                                                  const C_OscXmlParserBase & orc_XmlParser);
   static std::string mh_ConvertToHex(const uint64_t ou64_Value);
   static void mh_HandleEnumMinMax(C_OscHalcDefElement & orc_Element);
   static std::error_code mh_CheckValidUint(const std::string & orc_Item);
   static std::error_code mh_CheckValidSint(const std::string & orc_Item);
   static std::error_code mh_CheckValidDouble(const std::string & orc_Item);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
