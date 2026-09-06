//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Certificates Package loading
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCELOAD_HPP
#define C_OSCXCELOAD_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscXceBase.hpp"
#include "C_OscXceManifest.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXceLoad :
   public C_OscXceBase
{
public:
   static std::error_code h_ProcessPackage(const std::string & orc_PackagePath,
                                           const std::string & orc_TargetUnzipPath, C_OscXceManifest & orc_Manifest,
                                           stw::scl::C_SclStringList & orc_WarningMessages,
                                           std::string & orc_ErrorMessage);

private:
   static const std::string mhc_USE_CASE;

   static std::error_code mh_CheckParamsToProcessPackage(const std::string & orc_PackagePath,
                                                         const std::string & orc_TargetUnzipPath);
   static std::error_code mh_CheckXcertFiles(const std::string & orc_PackagePath);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
