//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Certificates Package creation
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCECREATE_HPP
#define C_OSCXCECREATE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <set>
#include <QStringList>

#include "stwtypes.hpp"
#include "C_OscXceBase.hpp"
#include "C_OscXceManifest.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXceCreate :
   public C_OscXceBase
{
public:
   static int32_t h_CreatePackage(const QString & orc_PackagePath,
                                  const QStringList & orc_CertificatesPath,
                                  const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters,
                                  QStringList & orc_WarningMessages,
                                  QString & orc_ErrorMessage,
                                  const QString & orc_TemporaryDirectory = "");

private:
   static const QString mhc_USE_CASE;

   static int32_t mh_CheckParamsToCreatePackage(const QString & orc_PackagePath,
                                                const QStringList & orc_CertificatesPath,
                                                const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters);
   static int32_t mh_CheckFileExists(const QString & orc_Path);
   static C_OscXceManifest mh_CreateManifest(
      const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters);
   static int32_t mh_PrepareCertFiles(const QString & orc_TmpPath,
                                      const QStringList & orc_CertificatesPath,
                                      std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters,
                                      std::set<QString> & orc_XcertFiles);
   static QString mh_GenOutFilePathPart(const QString & orc_InPath,
                                                      const QString & orc_TargetFolder);
   static int32_t mh_CopyFile(const QString & orc_InPath, const QString & orc_OutBasePath,
                              const QString & orc_OutFolder, std::map<QString,
                                                                                    bool> & orc_ExistingFiles,
                              std::set<QString> & orc_XcertFiles,
                              QString * const opc_OutFilePath = NULL);
   static QString mh_GetUniqueFileName(const QString & orc_InPath,
                                                     const QString & orc_OutFolder,
                                                     std::map<QString,
                                                              bool> & orc_ExistingFiles);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
