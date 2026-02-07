//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package creation util
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSPASERVICEPACKAGECREATEUTIL_HPP
#define C_OSCSPASERVICEPACKAGECREATEUTIL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscSystemDefinition.hpp"
#include <QString>
#include <QStringList>
#include <set>


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

class C_OscSpaServicePackageCreateUtil {
public:
  static int32_t h_CheckPackagePathParam(const QString &orc_PackagePath,
                                         const QString &orc_UseCase,
                                         const QString &orc_PackageExtension,
                                         const QString &orc_PackageExtensionTmp,
                                         QString &orc_ErrorMessage,
                                         const bool oq_CheckFileExist = true);
  static void h_GetTempFolderName(const QString &orc_PackagePath,
                                  const QString &orc_TemporaryDirectory,
                                  const QString &orc_UseCase,
                                  const QString &orc_PackageExtension,
                                  const QString &orc_TemporaryPackageExtension,
                                  QString &orc_UsedTempPath);
  static int32_t h_CreateTempFolderAndSubFolders(
      const QString &orc_PackagePath, const QString &orc_TemporaryDirectory,
      const QString &orc_UseCase, const QString &orc_PackageExtension,
      const QString &orc_TemporaryPackageExtension,
      const QStringList &orc_AllStaticSubFolders,
      QString &orc_UsedTempPath, QString &orc_ErrorMessage);
  static int32_t h_SaveSystemDefinition(
      const C_OscSystemDefinition &orc_SystemDefinition,
      const QString &orc_SystemDefinitionFileName, const QString &orc_UseCase,
      const QString &orc_UsedTempPath, const QString &orc_OutFilePrefix,
      std::set<QString> &orc_AllCreatedFiles, QString &orc_ErrorMessage);
  static int32_t h_SaveDeviceDefinitionsAndIni(
      const C_OscSystemDefinition &orc_SystemDefinition,
      const QString &orc_UseCase, const QString &orc_UsedTempPath,
      const QString &orc_OutFilePrefix, std::set<QString> &orc_AllCreatedFiles,
      QString &orc_ErrorMessage);
  static int32_t h_CreateZip(const QString &orc_UseCase,
                             const QString &orc_UsedTempPath,
                             const QString &orc_ZipFilePath,
                             const std::set<QString> &orc_AllCreatedFiles,
                             QString &orc_ErrorMessage);
  static void h_CleanUpTempFolder(const QString &orc_UseCase,
                                  const QString &orc_UsedTempPath,
                                  int32_t &ors32_ErrVal,
                                  QString &orc_ErrorMessage);

private:
  static const QString mhc_INI_DEV;

  static int32_t
  mh_CreateDeviceIniFile(const QString &orc_Path,
                         const std::set<QString> &orc_DeviceDefinitionPaths,
                         QString &orc_ErrorMessage);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
