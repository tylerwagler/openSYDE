//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Communication protocol reader/writer (multi-format)

   Multi-format filer supporting binary, JSON, and XML serialization of CAN
   communication protocol data, plus legacy XML element-level helpers.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODECOMMFILER_HPP
#define C_OSCNODECOMMFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscCanProtocol.hpp"
#include "C_OscXmlParser.hpp"
#include "stwtypes.hpp"
#include <QByteArray>
#include <QDataStream>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
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

class C_OscNodeCommFiler {
public:
  // --------------------------------------------------------------------------
  // Multi-format file I/O (auto-detect format from extension)
  // --------------------------------------------------------------------------
  static int32_t h_LoadFile(C_OscCanProtocol &orc_Protocol,
                            const QString &orc_FilePath,
                            const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t h_SaveFile(const C_OscCanProtocol &orc_Protocol,
                            const QString &orc_FilePath,
                            const QString &orc_DatapoolName);

  // --------------------------------------------------------------------------
  // Binary Format
  // --------------------------------------------------------------------------
  static int32_t h_LoadBinary(C_OscCanProtocol &orc_Protocol,
                              const QString &orc_FilePath,
                              const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t h_SaveBinary(const C_OscCanProtocol &orc_Protocol,
                              const QString &orc_FilePath,
                              const QString &orc_DatapoolName);
  static int32_t h_LoadFromMemoryBinary(C_OscCanProtocol &orc_Protocol,
                                        const QByteArray &orc_Data,
                                        const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static QByteArray h_SaveToMemoryBinary(const C_OscCanProtocol &orc_Protocol,
                                         const QString &orc_DatapoolName);

  // --------------------------------------------------------------------------
  // JSON Format
  // --------------------------------------------------------------------------
  static int32_t h_LoadJson(C_OscCanProtocol &orc_Protocol,
                            const QString &orc_FilePath,
                            const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t h_SaveJson(const C_OscCanProtocol &orc_Protocol,
                            const QString &orc_FilePath,
                            const QString &orc_DatapoolName);
  static int32_t h_LoadFromMemoryJson(C_OscCanProtocol &orc_Protocol,
                                      const QJsonObject &orc_Object,
                                      const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static QJsonObject h_SaveToMemoryJson(const C_OscCanProtocol &orc_Protocol,
                                        const QString &orc_DatapoolName);

  // --------------------------------------------------------------------------
  // XML Format (QDom-based)
  // --------------------------------------------------------------------------
  static int32_t h_LoadXml(C_OscCanProtocol &orc_Protocol,
                           const QString &orc_FilePath,
                           const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t h_SaveXml(const C_OscCanProtocol &orc_Protocol,
                           const QString &orc_FilePath,
                           const QString &orc_DatapoolName);
  static int32_t h_LoadFromMemoryXml(C_OscCanProtocol &orc_Protocol,
                                     const QDomElement &orc_Element,
                                     const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static QDomElement h_SaveToMemoryXml(const C_OscCanProtocol &orc_Protocol,
                                       QDomDocument &ro_Doc,
                                       const QString &orc_DatapoolName);

  // --------------------------------------------------------------------------
  // Legacy XML element-level methods (TinyXML parser-based)
  // --------------------------------------------------------------------------
  static int32_t
  h_LoadNodeComProtocolFile(C_OscCanProtocol &orc_NodeComProtocol,
                            const QString &orc_FilePath,
                            const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t
  h_LoadNodeComProtocol(C_OscCanProtocol &orc_NodeComProtocol,
                        C_OscXmlParserBase &orc_XmlParser,
                        const QList<C_OscNodeDataPool> &orc_NodeDataPools);
  static int32_t
  h_SaveNodeComProtocolFile(const C_OscCanProtocol &orc_NodeComProtocol,
                            const QString &orc_FilePath,
                            const QString &orc_DatapoolName);
  static void h_SaveNodeComProtocol(const C_OscCanProtocol &orc_NodeComProtocol,
                                    C_OscXmlParserBase &orc_XmlParser,
                                    const QString &orc_DatapoolName);
  static int32_t h_LoadNodeComMessageContainers(
      QList<C_OscCanMessageContainer> &orc_NodeComMessageContainers,
      C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveNodeComMessageContainers(
      const QList<C_OscCanMessageContainer> &orc_NodeComMessageContainers,
      C_OscXmlParserBase &orc_XmlParser,
      const C_OscCanProtocol::E_Type oe_ProtocolType);
  static int32_t h_LoadNodeComMessageContainer(
      C_OscCanMessageContainer &orc_NodeComMessageContainer,
      C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveNodeComMessageContainer(
      const C_OscCanMessageContainer &orc_NodeComMessageContainer,
      C_OscXmlParserBase &orc_XmlParser,
      const C_OscCanProtocol::E_Type oe_ProtocolType);
  static int32_t
  h_LoadNodeComMessages(QList<C_OscCanMessage> &orc_NodeComMessages,
                        C_OscXmlParserBase &orc_XmlParser);
  static void
  h_SaveNodeComMessages(const QList<C_OscCanMessage> &orc_NodeComMessages,
                        C_OscXmlParserBase &orc_XmlParser,
                        const C_OscCanProtocol::E_Type oe_ProtocolType);
  static int32_t h_LoadNodeComMessage(C_OscCanMessage &orc_NodeComMessage,
                                      C_OscXmlParserBase &orc_XmlParser);
  static void
  h_SaveNodeComMessage(const C_OscCanMessage &orc_NodeComMessage,
                       C_OscXmlParserBase &orc_XmlParser,
                       const C_OscCanProtocol::E_Type oe_ProtocolType);
  static int32_t h_LoadNodeComSignals(QList<C_OscCanSignal> &orc_NodeComSignals,
                                      C_OscXmlParserBase &orc_XmlParser);
  static void
  h_SaveNodeComSignals(const QList<C_OscCanSignal> &orc_NodeComSignals,
                       C_OscXmlParserBase &orc_XmlParser,
                       const C_OscCanProtocol::E_Type oe_ProtocolType);
  static int32_t h_LoadNodeComSignal(C_OscCanSignal &orc_NodeComSignal,
                                     C_OscXmlParserBase &orc_XmlParser,
                                     const bool oq_CanOpenOnly = false);
  static void
  h_SaveNodeComSignal(const C_OscCanSignal &orc_NodeComSignal,
                      C_OscXmlParserBase &orc_XmlParser,
                      const C_OscCanProtocol::E_Type oe_ProtocolType);
  static QString h_CommunicationProtocolToString(
      const C_OscCanProtocol::E_Type &ore_CommunicationProtocol);
  static int32_t
  h_StringToCommunicationProtocol(const QString &orc_String,
                                  C_OscCanProtocol::E_Type &ore_Type);
  static QString h_GetFileName(const QString &orc_DatapoolName);
  static void
  h_SaveNodeOwnerIndex(const C_OscCanInterfaceId &orc_OwnerNodeIndex,
                       C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadNodeOwnerIndex(C_OscCanInterfaceId &orc_OwnerNodeIndex,
                                      const C_OscXmlParserBase &orc_XmlParser);

private:
  // Multi-format helpers
  static int32_t mh_DetectAndLoad(C_OscCanProtocol &orc_Protocol,
                                  const QString &orc_FilePath,
                                  const QList<C_OscNodeDataPool> &orc_NodeDataPools);

  // Legacy XML helpers
  static void
  mh_SaveMessageCanOpenPart(const C_OscCanMessage &orc_NodeCommMessage,
                            C_OscXmlParserBase &orc_XmlParser);
  static int32_t mh_LoadMessageCanOpenPart(C_OscCanMessage &orc_NodeCommMessage,
                                           C_OscXmlParserBase &orc_XmlParser);
  static void
  mh_SaveNodeOwnerIndices(const C_OscCanInterfaceId &orc_OwnerNodeIndex,
                          C_OscXmlParserBase &orc_XmlParser);
  static int32_t
  mh_LoadNodeOwnerIndices(C_OscCanInterfaceId &orc_OwnerNodeIndex,
                          C_OscXmlParserBase &orc_XmlParser);
  static void mh_SaveSignalCanOpenPart(const C_OscCanSignal &orc_NodeCommSignal,
                                       C_OscXmlParserBase &orc_XmlParser);
  static int32_t mh_LoadSignalCanOpenPart(C_OscCanSignal &orc_NodeCommSignal,
                                          C_OscXmlParserBase &orc_XmlParser);
  static void mh_SaveSignalJ1939Part(const C_OscCanSignal &orc_NodeCommSignal,
                                     C_OscXmlParserBase &orc_XmlParser);
  static int32_t mh_LoadSignalJ1939Part(C_OscCanSignal &orc_NodeCommSignal,
                                        C_OscXmlParserBase &orc_XmlParser);
  static QString mh_CommunicationByteOrderToString(
      const C_OscCanSignal::E_ByteOrderType &ore_CommunicationByteOrder);
  static int32_t
  mh_StringToCommunicationByteOrder(const QString &orc_String,
                                    C_OscCanSignal::E_ByteOrderType &ore_Type);
  static QString mh_CommunicationMuxTypeToString(
      const C_OscCanSignal::E_MultiplexerType &ore_CommunicationByteOrder);
  static int32_t
  mh_StringToCommunicationMuxType(const QString &orc_String,
                                  C_OscCanSignal::E_MultiplexerType &ore_Type);
  static QString mh_NodeComMessageTxMethodToString(
      const C_OscCanMessage::E_TxMethodType &ore_NodeComMessageTxMethod);
  static void
  mh_StringToNodeComMessageTxMethod(const QString &orc_String,
                                    C_OscCanMessage::E_TxMethodType &ore_Type);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
