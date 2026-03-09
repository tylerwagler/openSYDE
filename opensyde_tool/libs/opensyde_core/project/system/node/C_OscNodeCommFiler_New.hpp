//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for comm description files (Multi-Format)

   Load / save CAN communication protocol data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODECOMMFILER_NEW_HPP
#define C_OSCNODECOMMFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscCanProtocol.hpp"
#include "C_OscFilerHelper.hpp"
#include "stwtypes.hpp"
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

class C_OscNodeCommFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadProtocolFile(C_OscCanProtocol &orc_Protocol,
                                     const QString &orc_FilePath,
                                     const QList<C_OscNodeDataPool> &orc_NodeDataPools);
   static int32_t h_SaveProtocolFile(const C_OscCanProtocol &orc_Protocol,
                                     const QString &orc_FilePath,
                                     const QString &orc_DatapoolName);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
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
   QByteArray h_SaveToMemoryBinary(const C_OscCanProtocol &orc_Protocol,
                                   const QString &orc_DatapoolName) const;

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
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
   QJsonObject h_SaveToMemoryJson(const C_OscCanProtocol &orc_Protocol,
                                  const QString &orc_DatapoolName) const;

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
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
   QDomElement h_SaveToMemoryXml(const C_OscCanProtocol &orc_Protocol,
                                 QDomDocument &ro_Doc,
                                 const QString &orc_DatapoolName) const;

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadProtocolFile/h_SaveProtocolFile with format detection, or format-specific methods")]]
   static int32_t h_LoadNodeComProtocol(C_OscCanProtocol &orc_NodeComProtocol,
                                        C_OscXmlParserBase &orc_XmlParser,
                                        const QList<C_OscNodeDataPool> &orc_NodeDataPools);
   [[deprecated("Use h_LoadProtocolFile/h_SaveProtocolFile with format detection, or format-specific methods")]]
   static void h_SaveNodeComProtocol(const C_OscCanProtocol &orc_NodeComProtocol,
                                     C_OscXmlParserBase &orc_XmlParser,
                                     const QString &orc_DatapoolName);

private:
   // Helper for format detection
   static int32_t mh_DetectAndLoad(C_OscCanProtocol &orc_Protocol,
                                   const QString &orc_FilePath,
                                   const QList<C_OscNodeDataPool> &orc_NodeDataPools);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
