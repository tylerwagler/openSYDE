//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       New HALC configuration filer with Qt-native serialization
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGFILER_NEW_HPP
#define C_OSCHALCCONFIGFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfig.hpp"
#include "stwtypes.hpp"
#include <QJsonDocument>
#include <QDomDocument>
#include <QDataStream>
#include <QStringList>

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

class C_OscHalcConfigFiler_New {
public:
   static int32_t h_LoadFile(C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath);
   static int32_t h_SaveFile(const C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path,
                             const QString &orc_BasePath,
                             QStringList *const opc_CreatedFiles);

   // Format-specific methods
   static int32_t h_LoadBinary(C_OscHalcConfig &orc_IoData,
                               const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscHalcConfig &orc_IoData,
                               const QString &orc_Path);
   static int32_t h_LoadJson(C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscHalcConfig &orc_IoData,
                             const QString &orc_Path);
   static int32_t h_LoadXml(C_OscHalcConfig &orc_IoData,
                            const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscHalcConfig &orc_IoData,
                            const QString &orc_Path);

private:
   C_OscHalcConfigFiler_New();

   // Helper methods for automatic format detection
   static int32_t mh_GetFormatFromExtension(const QString &orc_Path,
                                           QString &or_Format);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif