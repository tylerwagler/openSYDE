//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for view data files (Multi-Format - Framework)

   Load / save view data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWDATAFILER_HPP
#define C_OSCVIEWDATAFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscViewData.hpp"
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

class C_OscViewDataFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadViewDataFile(QList<C_OscViewData> &orc_Views,
                                     const QString &orc_FilePath,
                                     const QString &orc_BasePath);
   static int32_t h_SaveViewDataFile(const QList<C_OscViewData> &orc_Views,
                                     const QString &orc_FilePath,
                                     const QString &orc_BasePath);

   // --------------------------------------------------------------------------
   // Binary Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(QList<C_OscViewData> &orc_Views,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const QList<C_OscViewData> &orc_Views,
                               const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // JSON Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(QList<C_OscViewData> &orc_Views,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const QList<C_OscViewData> &orc_Views,
                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // XML Format
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(QList<C_OscViewData> &orc_Views,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const QList<C_OscViewData> &orc_Views,
                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use format-specific methods")]]
   static int32_t h_LoadFile(QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);
   [[deprecated("Use format-specific methods")]]
   static int32_t h_SaveFile(const QList<C_OscViewData> &orc_Views,
                             const QString &orc_Path);

private:
   static int32_t mh_DetectAndLoad(QList<C_OscViewData> &orc_Views,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
