//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target support package V2 reader/writer (Multi-Format)

   Load / save target support package V2 data from / to binary, JSON, or XML files
   using the Qt-native serialization framework.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGEV2FILER_NEW_HPP
#define C_OSCTARGETSUPPORTPACKAGEV2FILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscTargetSupportPackageV2.hpp"
#include "C_OscFilerUtil.hpp"
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

class C_OscTargetSupportPackageV2Filer_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadFile(C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryBinary(C_OscTargetSupportPackageV2 &orc_Package, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscTargetSupportPackageV2 &orc_Package);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryJson(C_OscTargetSupportPackageV2 &orc_Package, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscTargetSupportPackageV2 &orc_Package);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
   static int32_t h_LoadFromMemoryXml(C_OscTargetSupportPackageV2 &orc_Package, const QDomElement &orc_Element);
   static QDomElement h_SaveToMemoryXml(const C_OscTargetSupportPackageV2 &orc_Package, QDomDocument &orc_Doc);

private:
   static int32_t mh_DetectAndLoad(C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
