//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Config Package loading
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCOLOAD_HPP
#define C_OSCXCOLOAD_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscXcoBase.hpp"
#include "C_OscXcoManifest.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXcoLoad :
   public C_OscXcoBase
{
public:
   static std::error_code h_ProcessPackage(const std::string & orc_PackagePath,
                                           const std::string & orc_TargetUnzipPath, C_OscXcoManifest & orc_Manifest,
                                           std::string & orc_SystemDefinitionPath,
                                           std::string & orc_DeviceDefinitionPath,
                                           std::vector<std::string> & orc_WarningMessages,
                                           std::string & orc_ErrorMessage);

private:
   static const std::string mhc_USE_CASE;

   static std::error_code mh_CheckParamsToProcessPackage(const std::string & orc_PackagePath,
                                                         const std::string & orc_TargetUnzipPath);
   static std::error_code mh_CheckXcfgFiles(const std::string & orc_PackagePath);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
