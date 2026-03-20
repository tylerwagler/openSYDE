//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for data pool files (Multi-Format)

   Load / save data pool data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLFILER_NEW_HPP
#define C_OSCNODEDATAPOOLFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscFilerUtil.hpp"
#include "C_OscNodeDataPool.hpp"
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

class C_OscNodeDataPoolFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadDataPoolFile(C_OscNodeDataPool &orc_DataPool,
                                     const QString &orc_FilePath);
   static int32_t h_SaveDataPoolFile(const C_OscNodeDataPool &orc_DataPool,
                                     const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscNodeDataPool &orc_DataPool,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const C_OscNodeDataPool &orc_DataPool,
                               const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryBinary(C_OscNodeDataPool &orc_DataPool,
                                         const QByteArray &orc_Data);
   QByteArray h_SaveToMemoryBinary(const C_OscNodeDataPool &orc_DataPool) const;

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscNodeDataPool &orc_DataPool,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const C_OscNodeDataPool &orc_DataPool,
                             const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryJson(C_OscNodeDataPool &orc_DataPool,
                                       const QJsonObject &orc_Object);
   QJsonObject h_SaveToMemoryJson(const C_OscNodeDataPool &orc_DataPool) const;

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscNodeDataPool &orc_DataPool,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const C_OscNodeDataPool &orc_DataPool,
                            const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryXml(C_OscNodeDataPool &orc_DataPool,
                                      const QDomElement &orc_Element);
   QDomElement h_SaveToMemoryXml(const C_OscNodeDataPool &orc_DataPool,
                                 QDomDocument &ro_Doc) const;

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadDataPoolFile/h_SaveDataPoolFile with format detection, or format-specific methods")]]
   static int32_t h_LoadDataPool(C_OscNodeDataPool &orc_DataPool,
                                 C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use h_LoadDataPoolFile/h_SaveDataPoolFile with format detection, or format-specific methods")]]
   static void h_SaveDataPool(const C_OscNodeDataPool &orc_DataPool,
                              C_OscXmlParserBase &orc_XmlParser);

private:
   // Helper for format detection
   static int32_t mh_DetectAndLoad(C_OscNodeDataPool &orc_DataPool,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
