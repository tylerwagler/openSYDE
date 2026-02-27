//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Config Package loading
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCOLOAD_HPP
#define C_OSCXCOLOAD_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXcoBase.hpp"
#include "C_OscXcoManifest.hpp"

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

class C_OscXcoLoad : public C_OscXcoBase {
public:
  static int32_t h_ProcessPackage(const QString &orc_PackagePath,
                                  const QString &orc_TargetUnzipPath,
                                  C_OscXcoManifest &orc_Manifest,
                                  QString &orc_SystemDefinitionPath,
                                  QString &orc_DeviceDefinitionPath,
                                  QStringList &orc_WarningMessages,
                                  QString &orc_ErrorMessage);

private:
  static const QString mhc_USE_CASE;

  static int32_t
  mh_CheckParamsToProcessPackage(const QString &orc_PackagePath,
                                 const QString &orc_TargetUnzipPath);
  static int32_t mh_CheckXcfgFiles(const QString &orc_PackagePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
