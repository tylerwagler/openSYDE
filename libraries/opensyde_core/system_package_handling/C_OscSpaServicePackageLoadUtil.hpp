//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package loading util
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSPASERVICEPACKAGELOADUTIL_HPP
#define C_OSCSPASERVICEPACKAGELOADUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSpaServicePackageLoadUtil
{
public:
   static std::string h_GetUnzipPath(const std::string & orc_TargetUnzipPath);
   static std::error_code h_CheckParamsToProcessZipPackage(const std::string & orc_PackagePath,
                                                   const std::string & orc_TargetUnzipPath,
                                                   const std::string & orc_UseCase,
                                                   std::string & orc_ErrorMessage);
   static std::error_code h_SearchFilesInPath(const std::string & orc_PackagePath,
                                      const std::vector<std::string> & orc_NecessaryFiles);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
