//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC definition struct section
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFSTRUCTFILER_HPP
#define C_OSCHALCDEFSTRUCTFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefChannelUseCase.hpp"
#include "C_OscHalcDefStruct.hpp"
#include "C_OscXmlParser.hpp"
#include <QList>
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscHalcDefStructFiler {
public:
  static int32_t h_LoadStructs(
      QList<C_OscHalcDefStruct> &orc_Structs, C_OscXmlParserBase &orc_XmlParser,
      const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
      const QString &orc_CurrentNodeName, const QString &orc_SectionNodeName,
      const QString &orc_GroupNodeName, const QString &orc_SingleNodeName,
      const bool oq_RequireId, const bool oq_RequireSection,
      const uint32_t ou32_DomainNameLength);
  static int32_t h_SaveStructs(
      const QList<C_OscHalcDefStruct> &orc_Structs,
      const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
      C_OscXmlParserBase &orc_XmlParser, const QString &orc_CurrentNodeName,
      const QString &orc_SectionNodeName, const QString &orc_GroupNodeName,
      const QString &orc_SingleNodeName);
  static int32_t
  h_ParseSimplestTypeValue(const QString &orc_TypeStr,
                           C_OscNodeDataPoolContent &orc_Content,
                           const C_OscXmlParserBase &orc_XmlParser,
                           const QString &orc_AttributeName);
  static int32_t
  h_GetTypeForSimplestTypeString(const QString &orc_TypeStr,
                                 C_OscNodeDataPoolContent::E_Type &ore_Type,
                                 const C_OscXmlParserBase &orc_XmlParser);
  static QString
  h_GetTypeString(const C_OscNodeDataPoolContent::E_Type oe_Type);
  static int32_t
  h_SaveSimpleValueAsAttribute(const QString &orc_Node,
                               C_OscXmlParserBase &orc_XmlParser,
                               const C_OscNodeDataPoolContent &orc_Content);
  static int32_t h_SetType(C_OscXmlParserBase &orc_XmlParser,
                           C_OscHalcDefContent &orc_Content,
                           QString &orc_TypeStr, QString &orc_BaseTypeStr,
                           const QString &orc_CurrentNodeName);
  static int32_t h_ParseAttributeIntoContent(
      C_OscHalcDefContent &orc_Content, const C_OscXmlParserBase &orc_XmlParser,
      const QString &orc_AttributeName, const QString &orc_Type,
      const QString &orc_BaseType, const bool oq_RequireAttribute);

  static const uint32_t hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH;

private:
  static const QString mhc_FALSE;
  static const QString mhc_TRUE;

  C_OscHalcDefStructFiler(void);

  static int32_t mh_LoadStruct(
      C_OscHalcDefStruct &orc_Struct, C_OscXmlParserBase &orc_XmlParser,
      const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
      const QString &orc_GroupNodeName, const QString &orc_SingleNodeName,
      const bool oq_RequireId, const uint32_t ou32_DomainNameLength);
  static int32_t
  mh_SaveStruct(const C_OscHalcDefStruct &orc_Struct,
                const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
                C_OscXmlParserBase &orc_XmlParser,
                const QString &orc_GroupNodeName,
                const QString &orc_SingleNodeName);
  static int32_t
  mh_LoadDataElement(C_OscHalcDefElement &orc_Element,
                     C_OscXmlParserBase &orc_XmlParser,
                     const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
                     const QList<uint32_t> &orc_BaseAvailability,
                     const QString &orc_SingleNodeName, const bool oq_RequireId,
                     const uint32_t ou32_DomainNameLength);
  static int32_t
  mh_SaveDataElement(const C_OscHalcDefElement &orc_Element,
                     const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
                     C_OscXmlParserBase &orc_XmlParser,
                     const QString &orc_SingleNodeName);
  static int32_t mh_ParseAttributeAvailability(
      QList<uint32_t> &orc_Availability, const QString &orc_AttributeContent,
      const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
      const C_OscXmlParserBase &orc_XmlParser);
  static int32_t mh_GetAvailabilityString(
      const QList<uint32_t> &orc_Availability,
      const QList<C_OscHalcDefChannelUseCase> &orc_UseCases,
      QString &orc_Output);
  static void mh_SetMaxValForType(const QString &orc_TypeStr,
                                  C_OscNodeDataPoolContent &orc_Content);
  static void mh_SetMinValForType(const QString &orc_TypeStr,
                                  C_OscNodeDataPoolContent &orc_Content);
  static bool
  mh_CheckInitialBitmaskContentValid(const C_OscHalcDefContent &orc_Content,
                                     const C_OscXmlParserBase &orc_XmlParser);
  static QString mh_ConvertToHex(const uint64_t ou64_Value);
  static void mh_HandleEnumMinMax(C_OscHalcDefElement &orc_Element);
  static int32_t mh_CheckValidUint(const QString &orc_Item);
  static int32_t mh_CheckValidSint(const QString &orc_Item);
  static int32_t mh_CheckValidDouble(const QString &orc_Item);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
