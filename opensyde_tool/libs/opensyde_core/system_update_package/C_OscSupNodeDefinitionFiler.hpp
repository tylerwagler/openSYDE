//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update package definition filer
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPNODEDEFINITIONFILER_HPP
#define C_OSCSUPNODEDEFINITIONFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <vector>

#include "C_OscSuSequences.hpp"
#include "C_OscSupNodeDefinition.hpp"
#include "C_OscXmlParser.hpp"
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

class C_OscSupNodeDefinitionFiler {
public:
  static int32_t h_SaveNodes(const QStringList &orc_Files,
                             const QList<C_OscSupNodeDefinition> &orc_Nodes);
  static int32_t h_LoadNodes(
      const QStringList &orc_Files, const QStringList &orc_NodeFoldersAbs,
      const QByteArray &orc_ActiveNodes,
      QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QMap<uint32_t, uint32_t> &orc_UpdateOrderByNodes,
      const QList<uint32_t> &orc_UpdatePosition, QStringList &orc_Signatures);

  static const uint8_t hu8_ACTIVE_NODE;

private:
  static int32_t mh_SaveNode(const QString &orc_File,
                             const C_OscSupNodeDefinition &orc_Node);
  static void mh_LoadFilesSection(QStringList &orc_Files,
                                  const uint32_t ou32_NodeCounter,
                                  const uint32_t ou32_UpdatePos,
                                  QMap<uint32_t, uint32_t> &orc_PositionMap,
                                  const QString &orc_NodeFolderAbs,
                                  C_OscXmlParserBase &orc_XmlParser,
                                  const QString &orc_BaseNodeName,
                                  const QString &orc_ElementNodeName);
  static void mh_LoadPemConfigSection(
      C_OscSuSequences::C_DoFlash &orc_DoFlash, const uint32_t ou32_NodeCounter,
      const uint32_t ou32_UpdatePos,
      QMap<uint32_t, uint32_t> &orc_PositionMap,
      const QString &orc_NodeFolderAbs, C_OscXmlParserBase &orc_XmlParser);
  static void mh_SaveFiles(const QStringList &orc_Files,
                           C_OscXmlParserBase &orc_XmlParser,
                           const QString &orc_BaseNodeName,
                           const QString &orc_ElementNodeName);
  static void mh_SavePemConfig(const C_OscSupNodeDefinition &orc_CurrentNode,
                               C_OscXmlParserBase &orc_XmlParser);
  static void mh_SaveSignatureFile(const C_OscSupNodeDefinition &orc_Node,
                                   C_OscXmlParserBase &orc_XmlParser);
  static int32_t mh_LoadSignatureFile(const QString &orc_NodeFolderAbs,
                                      QString &orc_Signature,
                                      C_OscXmlParserBase &orc_XmlParser);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
