//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node reader/writer (Multi-Format)

   Load / save node data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEFILER_NEW_HPP
#define C_OSCNODEFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscNode.hpp"
#include "C_OscFilerUtil.hpp"
#include <QList>
#include <QStringList>
#include <QHash>

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

class C_OscNodeFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadNodeFile(C_OscNode &orc_Node, const QString &orc_FilePath,
                                 const bool oq_SkipContent = false);
   static int32_t h_SaveNodeFile(const C_OscNode &orc_Node, const QString &orc_FilePath,
                                 QStringList *const opc_CreatedFiles,
                                 const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscNode &orc_Node, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscNode &orc_Node, const QString &orc_Path,
                               QStringList *const opc_CreatedFiles,
                               const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t h_LoadFromMemoryBinary(C_OscNode &orc_Node, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscNode &orc_Node);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscNode &orc_Node, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscNode &orc_Node, const QString &orc_Path,
                             QStringList *const opc_CreatedFiles,
                             const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t h_LoadFromMemoryJson(C_OscNode &orc_Node, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscNode &orc_Node);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscNode &orc_Node, const QString &orc_Path,
                            const bool oq_SkipContent = false);
   static int32_t h_SaveXml(const C_OscNode &orc_Node, const QString &orc_Path,
                            QStringList *const opc_CreatedFiles,
                            const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t h_LoadFromMemoryXml(C_OscNode &orc_Node, const QDomElement &orc_Element,
                                      const bool oq_SkipContent = false);
   static QDomElement h_SaveToMemoryXml(const C_OscNode &orc_Node, QDomDocument &orc_Doc);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadNodeFile/h_SaveNodeFile with format detection")]]
   static int32_t h_LoadNodeFile_Legacy(C_OscNode &orc_Node, const QString &orc_FilePath,
                                        const bool oq_SkipContent);
   [[deprecated("Use h_LoadNodeFile/h_SaveNodeFile with format detection")]]
   static int32_t h_SaveNodeFile_Legacy(const C_OscNode &orc_Node, const QString &orc_FilePath,
                                        QStringList *const opc_CreatedFiles,
                                        const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   [[deprecated("Use h_LoadNode/h_SaveNode with format detection")]]
   static int32_t h_LoadNode_Legacy(C_OscNode &orc_Node, C_OscXmlParserBase &orc_XmlParser,
                                    const QString &orc_BasePath, const bool oq_SkipContent);
   [[deprecated("Use h_LoadNode/h_SaveNode with format detection")]]
   static int32_t h_SaveNode_Legacy(const C_OscNode &orc_Node, C_OscXmlParserBase &orc_XmlParser,
                                    const QString &orc_BasePath, QStringList *const opc_CreatedFiles,
                                    const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);

   // --------------------------------------------------------------------------
   // Helper Methods (for complex nested structures)
   // --------------------------------------------------------------------------
   static QString h_GetFolderName(const QString &orc_NodeName);
   static QString h_GetFileName(void);

private:
   // Internal helpers
   static int32_t mh_DetectAndLoad(C_OscNode &orc_Node, const QString &orc_Path,
                                   const bool oq_SkipContent);
   static int32_t mh_SaveNodeInternal(const C_OscNode &orc_Node, const QString &orc_Path,
                                      QStringList *const opc_CreatedFiles,
                                      const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap,
                                      const QString &orc_Format);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
