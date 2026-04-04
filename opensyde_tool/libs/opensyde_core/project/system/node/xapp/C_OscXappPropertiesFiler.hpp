//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XAPP properties files (Multi-Format - Framework)

   Load / save XAPP properties from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSXXAPPPROPERTIESFILER_HPP
#define C_OSXXAPPPROPERTIESFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXappProperties.hpp"
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

class C_OscXappPropertiesFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXappPropertiesFile(QList<C_OscXappProperties> &orc_Properties,
                                           const QString &orc_FilePath,
                                           const QString &orc_BasePath);
   static int32_t h_SaveXappPropertiesFile(const QList<C_OscXappProperties> &orc_Properties,
                                           const QString &orc_FilePath,
                                           const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(QList<C_OscXappProperties> &orc_Properties,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const QList<C_OscXappProperties> &orc_Properties,
                               const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // JSON Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(QList<C_OscXappProperties> &orc_Properties,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const QList<C_OscXappProperties> &orc_Properties,
                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // XML Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(QList<C_OscXappProperties> &orc_Properties,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const QList<C_OscXappProperties> &orc_Properties,
                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use format-specific methods")]]
   static int32_t h_LoadFile(QList<C_OscXappProperties> &orc_Properties,
                             const QString &orc_Path);
   [[deprecated("Use format-specific methods")]]
   static int32_t h_SaveFile(const QList<C_OscXappProperties> &orc_Properties,
                             const QString &orc_Path);

private:
   static int32_t mh_DetectAndLoad(QList<C_OscXappProperties> &orc_Properties,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
