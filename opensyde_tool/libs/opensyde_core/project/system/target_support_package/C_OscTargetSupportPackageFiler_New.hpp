//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target support package reader/writer (Multi-Format)

   Load / save target support package data from / to binary, JSON, or XML files
   using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGEFILER_NEW_HPP
#define C_OSCTARGETSUPPORTPACKAGEFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscTargetSupportPackage.hpp"
#include "C_OscFilerHelper.hpp"
#include "stwtypes.hpp"

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

class C_OscTargetSupportPackageFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadFile(C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryBinary(C_OscTargetSupportPackage &orc_Package, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscTargetSupportPackage &orc_Package);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryJson(C_OscTargetSupportPackage &orc_Package, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscTargetSupportPackage &orc_Package);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryXml(C_OscTargetSupportPackage &orc_Package, const QDomElement &orc_Element);
   static QDomElement h_SaveToMemoryXml(const C_OscTargetSupportPackage &orc_Package, QDomDocument &orc_Doc);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadFile/h_SaveFile with format detection")]]
   static int32_t h_LoadPackage(C_OscTargetSupportPackage &orc_Package, C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use h_LoadFile/h_SaveFile with format detection")]]
   static void h_SavePackage(const C_OscTargetSupportPackage &orc_Package, C_OscXmlParserBase &orc_XmlParser);

private:
   static int32_t mh_DetectAndLoad(C_OscTargetSupportPackage &orc_Package, const QString &orc_Path);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
