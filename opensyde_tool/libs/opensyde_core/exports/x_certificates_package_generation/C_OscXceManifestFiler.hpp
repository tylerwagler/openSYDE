//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XCE manifest files (Multi-Format - Framework)

   Load / save XCE manifest from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSXXCEMANIFESTFILER_NEW_HPP
#define C_OSXXCEMANIFESTFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXceManifest.hpp"
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

class C_OscXceManifestFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXceManifestFile(C_OscXceManifest &orc_Manifest,
                                        const QString &orc_FilePath,
                                        const QString &orc_BasePath);
   static int32_t h_SaveXceManifestFile(const C_OscXceManifest &orc_Manifest,
                                        const QString &orc_FilePath,
                                        const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscXceManifest &orc_Manifest,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const C_OscXceManifest &orc_Manifest,
                               const QString &orc_FilePath);

    // --------------------------------------------------------------------------
    // JSON Format
    // --------------------------------------------------------------------------
    static int32_t h_LoadJson(C_OscXceManifest &orc_Manifest,
                              const QString &orc_FilePath);
    static int32_t h_SaveJson(const C_OscXceManifest &orc_Manifest,
                              const QString &orc_FilePath);

    // --------------------------------------------------------------------------
    // XML Format
    // --------------------------------------------------------------------------
    static int32_t h_LoadXml(C_OscXceManifest &orc_Manifest,
                             const QString &orc_FilePath);
    static int32_t h_SaveXml(const C_OscXceManifest &orc_Manifest,
                             const QString &orc_FilePath);

    // --------------------------------------------------------------------------
    // Legacy Compatibility (deprecated)
    // --------------------------------------------------------------------------
    [[deprecated("Use format-specific methods")]]
    static int32_t h_LoadFile(C_OscXceManifest &orc_Manifest,
                              const QString &orc_Path);
    [[deprecated("Use format-specific methods")]]
    static int32_t h_SaveFile(const C_OscXceManifest &orc_Manifest,
                              const QString &orc_Path);

    // File name constant
    static const QString hc_FILE_NAME;

private:
   static int32_t mh_DetectAndLoad(C_OscXceManifest &orc_Manifest,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
