//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core zip/unzip utilities

   \class       stw::opensyde_core::C_OscZipFile
   \brief       openSYDE Core zip/unzip file utilities

   Wrapper for the miniz library.
   Provides functions to zip/unzip data to/from file

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCZIPFILE_HPP
#define C_OSCZIPFILE_HPP

#include <QString>
#include <QStringList>
#include <set>

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

class C_OscZipFile {
public:
  static int32_t h_CreateZipFile(const QString &orc_SourcePath,
                                 const QSet<QString> &orc_SupFiles,
                                 const QString &orc_ZipArchivePath,
                                 QString *const opc_ErrorText = NULL);

  static int32_t h_UnpackZipFile(const QString &orc_SourcePath,
                                 const QString &orc_TargetUnzipPath,
                                 QString *const opc_ErrorText = NULL);

  static void h_AppendFilesRelative(QSet<QString> &orc_Set,
                                    const QStringList &orc_Files,
                                    const QString &orc_BasePath);

  static int32_t h_IsZipFile(const QString &orc_FilePath);

private:
  static int32_t mh_AddContentToZipFile(const QString &orc_ZipArchivePath,
                                        const QString &orc_ItemName,
                                        const char_t *const opcn_Content,
                                        const uint32_t ou32_ContentSize,
                                        const QString &orc_ItemType,
                                        QString *const opc_ErrorText = NULL);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
