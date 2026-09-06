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
   static int32_t h_CreatePackage(const std::string & orc_PackagePath,
                                  const std::vector<std::string> & orc_CertificatesPath,
                                  const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters,
                                  stw::scl::C_SclStringList & orc_WarningMessages,
                                  std::string & orc_ErrorMessage,
                                  const std::string & orc_TemporaryDirectory = "");

private:
   static const std::string mhc_USE_CASE;

   static int32_t mh_CheckParamsToCreatePackage(const std::string & orc_PackagePath,
                                                const std::vector<std::string> & orc_CertificatesPath,
                                                const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters);
   static int32_t mh_CheckFileExists(const std::string & orc_Path);
   static C_OscXceManifest mh_CreateManifest(
      const std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters);
   static int32_t mh_PrepareCertFiles(const std::string & orc_TmpPath,
                                      const std::vector<std::string> & orc_CertificatesPath,
                                      std::vector<C_OscXceUpdatePackageParameters> & orc_UpdatePackageParameters,
                                      std::set<std::string> & orc_XcertFiles);
   static std::string mh_GenOutFilePathPart(const std::string & orc_InPath,
                                                      const std::string & orc_TargetFolder);
   static int32_t mh_CopyFile(const std::string & orc_InPath, const std::string & orc_OutBasePath,
                              const std::string & orc_OutFolder, std::map<std::string,
                                                                                    bool> & orc_ExistingFiles,
                              std::set<std::string> & orc_XcertFiles,
                              std::string * const opc_OutFilePath = nullptr);
   static std::string mh_GetUniqueFileName(const std::string & orc_InPath,
                                                     const std::string & orc_OutFolder,
                                                     std::map<std::string,
                                                              bool> & orc_ExistingFiles);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
