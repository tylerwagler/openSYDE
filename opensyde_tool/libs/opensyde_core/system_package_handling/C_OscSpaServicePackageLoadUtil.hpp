//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package loading util
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSPASERVICEPACKAGELOADUTIL_HPP
#define C_OSCSPASERVICEPACKAGELOADUTIL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QString>
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

class C_OscSpaServicePackageLoadUtil {
public:
  static QString h_GetUnzipPath(const QString &orc_TargetUnzipPath);
  static int32_t h_CheckParamsToProcessZipPackage(
      const QString &orc_PackagePath, const QString &orc_TargetUnzipPath,
      const QString &orc_UseCase, QString &orc_ErrorMessage);
  static int32_t
  h_SearchFilesInPath(const QString &orc_PackagePath,
                      const QStringList &orc_NecessaryFiles);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
