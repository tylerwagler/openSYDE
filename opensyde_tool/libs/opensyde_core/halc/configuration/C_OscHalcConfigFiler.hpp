//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for HALC configuration files (Multi-Format - Framework)

   Load / save HALC configuration data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \note        This is a framework implementation. Full serialization requires
                Qt methods to be added to all nested HALC data classes first.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGFILER_HPP
#define C_OSCHALCCONFIGFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfig.hpp"
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

class C_OscHalcConfigFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadHalcConfigFile(C_OscHalcConfig &orc_Config,
                                       const QString &orc_FilePath,
                                       const QString &orc_BasePath);
   static int32_t h_SaveHalcConfigFile(const C_OscHalcConfig &orc_Config,
                                       const QString &orc_FilePath,
                                       const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscHalcConfig &orc_Config,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const C_OscHalcConfig &orc_Config,
                               const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // JSON Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscHalcConfig &orc_Config,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const C_OscHalcConfig &orc_Config,
                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // XML Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscHalcConfig &orc_Config,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const C_OscHalcConfig &orc_Config,
                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use format-specific methods")]]
   static int32_t h_LoadFile(C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath);
   [[deprecated("Use format-specific methods")]]
   static int32_t h_SaveFile(const C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath,
                             QStringList *const opc_CreatedFiles);

private:
   static int32_t mh_DetectAndLoad(C_OscHalcConfig &orc_Config,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
