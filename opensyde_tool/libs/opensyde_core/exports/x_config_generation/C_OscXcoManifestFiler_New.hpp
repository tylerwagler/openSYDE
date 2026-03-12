//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XCO manifest files (Multi-Format - Framework)

   Load / save XCO manifest from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSXXCOMANIFESTFILER_NEW_HPP
#define C_OSXXCOMANIFESTFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXcoManifest.hpp"
#include "C_OscFilerUtil.hpp"
#include "stwtypes.hpp"
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

class C_OscXcoManifestFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXcoManifestFile(QList<C_OscXcoManifest> &orc_Manifests,
                                        const QString &orc_FilePath,
                                        const QString &orc_BasePath);
   static int32_t h_SaveXcoManifestFile(const QList<C_OscXcoManifest> &orc_Manifests,
                                        const QString &orc_FilePath,
                                        const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(QList<C_OscXcoManifest> &orc_Manifests,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const QList<C_OscXcoManifest> &orc_Manifests,
                               const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // JSON Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(QList<C_OscXcoManifest> &orc_Manifests,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const QList<C_OscXcoManifest> &orc_Manifests,
                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // XML Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(QList<C_OscXcoManifest> &orc_Manifests,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const QList<C_OscXcoManifest> &orc_Manifests,
                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use format-specific methods")]]
   static int32_t h_LoadFile(QList<C_OscXcoManifest> &orc_Manifests,
                             const QString &orc_Path);
   [[deprecated("Use format-specific methods")]]
   static int32_t h_SaveFile(const QList<C_OscXcoManifest> &orc_Manifests,
                             const QString &orc_Path);

private:
   static int32_t mh_DetectAndLoad(QList<C_OscXcoManifest> &orc_Manifests,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
