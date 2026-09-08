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
   static int32_t h_ProcessPackage(const std::string & orc_PackagePath,
                                   const std::string & orc_TargetUnzipPath, C_OscXcoManifest & orc_Manifest,
                                   std::string & orc_SystemDefinitionPath,
                                   std::string & orc_DeviceDefinitionPath,
                                   stw::scl::std::vector<std::string> & orc_WarningMessages,
                                   std::string & orc_ErrorMessage);

private:
   static const std::string mhc_USE_CASE;

   static int32_t mh_CheckParamsToProcessPackage(const std::string & orc_PackagePath,
                                                 const std::string & orc_TargetUnzipPath);
   static int32_t mh_CheckXcfgFiles(const std::string & orc_PackagePath);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
