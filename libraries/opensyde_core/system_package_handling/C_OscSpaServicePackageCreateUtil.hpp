//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package creation util
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSPASERVICEPACKAGECREATEUTIL_HPP
#define C_OSCSPASERVICEPACKAGECREATEUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include "C_OscSystemDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSpaServicePackageCreateUtil
{
public:
   static int32_t h_CheckPackagePathParam(const std::string & orc_PackagePath,
                                          const std::string & orc_UseCase,
                                          const std::string & orc_PackageExtension,
                                          const std::string & orc_PackageExtensionTmp,
                                          std::string & orc_ErrorMessage,
                                          const bool oq_CheckFileExist = true);
   static void h_GetTempFolderName(const std::string & orc_PackagePath,
                                   const std::string & orc_TemporaryDirectory,
                                   const std::string & orc_UseCase,
                                   const std::string & orc_PackageExtension,
                                   const std::string & orc_TemporaryPackageExtension,
                                   std::string & orc_UsedTempPath);
   static int32_t h_CreateTempFolderAndSubFolders(const std::string & orc_PackagePath,
                                                  const std::string & orc_TemporaryDirectory,
                                                  const std::string & orc_UseCase,
                                                  const std::string & orc_PackageExtension,
                                                  const std::string & orc_TemporaryPackageExtension,
                                                  const std::vector<std::string> & orc_AllStaticSubFolders,
                                                  std::string & orc_UsedTempPath,
                                                  std::string & orc_ErrorMessage);
   static int32_t h_SaveSystemDefinition(const C_OscSystemDefinition & orc_SystemDefinition,
                                         const std::string & orc_SystemDefinitionFileName,
                                         const std::string & orc_UseCase,
                                         const std::string & orc_UsedTempPath,
                                         const std::string & orc_OutFilePrefix,
                                         std::set<std::string> & orc_AllCreatedFiles,
                                         std::string & orc_ErrorMessage);
   static int32_t h_SaveDeviceDefinitionsAndIni(const C_OscSystemDefinition & orc_SystemDefinition,
                                                const std::string & orc_UseCase,
                                                const std::string & orc_UsedTempPath,
                                                const std::string & orc_OutFilePrefix,
                                                std::set<std::string> & orc_AllCreatedFiles,
                                                std::string & orc_ErrorMessage);
   static int32_t h_CreateZip(const std::string & orc_UseCase, const std::string & orc_UsedTempPath,
                              const std::string & orc_ZipFilePath,
                              const std::set<std::string> & orc_AllCreatedFiles,
                              std::string & orc_ErrorMessage);
   static void h_CleanUpTempFolder(const std::string & orc_UseCase,
                                   const std::string & orc_UsedTempPath, int32_t & ors32_ErrVal,
                                   std::string & orc_ErrorMessage);

private:
   C_OscSpaServicePackageCreateUtil(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
