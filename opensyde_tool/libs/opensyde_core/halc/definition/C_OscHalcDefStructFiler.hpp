//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for HALC definition struct files (Multi-Format - Framework)

   Load / save HALC definition struct data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFSTRUCTFILER_HPP
#define C_OSCHALCDEFSTRUCTFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefStruct.hpp"
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

class C_OscHalcDefStructFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadHalcDefStructFile(C_OscHalcDefStruct &orc_Struct,
                                          const QString &orc_FilePath,
                                          const QString &orc_BasePath);
   static int32_t h_SaveHalcDefStructFile(const C_OscHalcDefStruct &orc_Struct,
                                          const QString &orc_FilePath,
                                          const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscHalcDefStruct &orc_Struct,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const C_OscHalcDefStruct &orc_Struct,
                               const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // JSON Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscHalcDefStruct &orc_Struct,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const C_OscHalcDefStruct &orc_Struct,
                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // XML Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscHalcDefStruct &orc_Struct,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const C_OscHalcDefStruct &orc_Struct,
                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use format-specific methods")]]
   static int32_t h_LoadFile(C_OscHalcDefStruct &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath);
   [[deprecated("Use format-specific methods")]]
   static int32_t h_SaveFile(const C_OscHalcDefStruct &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath,
                             QStringList *const opc_CreatedFiles);

private:
   static int32_t mh_DetectAndLoad(C_OscHalcDefStruct &orc_Struct,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
