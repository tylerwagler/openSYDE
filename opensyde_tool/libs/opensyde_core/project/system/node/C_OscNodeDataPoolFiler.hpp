//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node data pool reader/writer (V3) (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLFILERV3_HPP
#define C_OSCNODEDATAPOOLFILERV3_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

#include "C_OscNodeDataPool.hpp"
#include "C_OscXmlParser.hpp"
#include <QList>

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

class C_OscNodeDataPoolFiler {
public:
  C_OscNodeDataPoolFiler();

  /// load functions
  static int32_t h_LoadDataPoolFile(C_OscNodeDataPool &orc_NodeDataPool,
                                    const QString &orc_FilePath);
  static int32_t h_LoadDataPool(C_OscNodeDataPool &orc_NodeDataPool,
                                C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolList(C_OscNodeDataPoolList &orc_NodeDataPoolList,
                                    C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolElement(
      C_OscNodeDataPoolListElement &orc_NodeDataPoolListElement,
      C_OscXmlParserBase &orc_XmlParser);
  static int32_t
  h_LoadDataPoolLists(QList<C_OscNodeDataPoolList> &orc_NodeDataPoolLists,
                      C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolListElements(
      QList<C_OscNodeDataPoolListElement> &orc_NodeDataPoolListElements,
      C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolListElementDataSetValues(
      const C_OscNodeDataPoolContent &orc_ContentType,
      QList<C_OscNodeDataPoolContent> &orc_NodeDataPoolListElementDataSetValues,
      C_OscXmlParserBase &orc_XmlParser);
  static int32_t
  h_LoadDataPoolElementType(C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
                            C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolElementValue(
      C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
      C_OscXmlParserBase &orc_XmlParser, const bool oq_CheckDataType,
      QString *const opc_CheckDataTypeErrorDetails = NULL);
  static int32_t
  h_LoadDataPoolContentV1(C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
                          C_OscXmlParserBase &orc_XmlParser);

  /// save functions
  static int32_t h_SaveDataPoolFile(const C_OscNodeDataPool &orc_NodeDataPool,
                                    const QString &orc_FilePath);
  static void h_SaveDataPool(const C_OscNodeDataPool &orc_NodeDataPool,
                             C_OscXmlParserBase &orc_XmlParser);
  static void
  h_SaveDataPoolList(const C_OscNodeDataPoolList &orc_NodeDataPoolList,
                     C_OscXmlParserBase &orc_XmlParser,
                     const C_OscNodeDataPool::E_Type oe_DatapoolType);
  static void h_SaveDataPoolElement(
      const C_OscNodeDataPoolListElement &orc_NodeDataPoolListElement,
      C_OscXmlParserBase &orc_XmlParser,
      const C_OscNodeDataPool::E_Type oe_DatapoolType);
  static void
  h_SaveDataPoolLists(const QList<C_OscNodeDataPoolList> &orc_NodeDataPoolLists,
                      C_OscXmlParserBase &orc_XmlParser,
                      const C_OscNodeDataPool::E_Type oe_DatapoolType);
  static void h_SaveDataPoolListElements(
      const QList<C_OscNodeDataPoolListElement> &orc_NodeDataPoolListElements,
      C_OscXmlParserBase &orc_XmlParser,
      const C_OscNodeDataPool::E_Type oe_DatapoolType);
  static void h_SaveDataPoolListElementDataSetValues(
      const QList<C_OscNodeDataPoolContent>
          &orc_NodeDataPoolListElementDataSetValues,
      C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadDataPoolListDataSets(
      QList<C_OscNodeDataPoolDataSet> &orc_NodeDataPoolListDataSets,
      C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveDataPoolListDataSets(
      const QList<C_OscNodeDataPoolDataSet> &orc_NodeDataPoolListDataSets,
      C_OscXmlParserBase &orc_XmlParser);
  static QString
  h_DataPoolToString(const C_OscNodeDataPool::E_Type &ore_DataPool);
  static int32_t h_StringToDataPool(const QString &orc_String,
                                    C_OscNodeDataPool::E_Type &ore_Type);

  static void h_SaveDataPoolElementType(
      const C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
      C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveDataPoolElementValue(
      const QString &orc_NodeName,
      const C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
      C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveDataPoolContentV1(
      const C_OscNodeDataPoolContent &orc_NodeDataPoolContent,
      C_OscXmlParserBase &orc_XmlParser);

  // Misc
  static QString h_GetFileName(const QString &orc_DatapoolName);
  static int32_t h_CheckDataPoolElementValueType(
      const C_OscNodeDataPoolContent::E_Type oe_ContentType,
      const C_OscXmlParserBase &orc_XmlParser,
      QString *const opc_CheckDataTypeErrorDetails = NULL);

private:
  static QString mh_NodeDataPoolContentToString(
      const C_OscNodeDataPoolContent::E_Type &ore_NodeDataPoolContent);
  static int32_t
  mh_StringToNodeDataPoolContent(const QString &orc_String,
                                 C_OscNodeDataPoolContent::E_Type &ore_Type);
  static QString mh_NodeDataPoolElementAccessToString(
      const C_OscNodeDataPoolListElement::E_Access
          &ore_NodeDataPoolElementAccess);
  static int32_t mh_StringToNodeDataPoolElementAccess(
      const QString &orc_String,
      C_OscNodeDataPoolListElement::E_Access &ore_Type);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
