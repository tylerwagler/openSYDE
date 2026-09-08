//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Config Package creation
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCOCREATE_HPP
#define C_OSCXCOCREATE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "stwtypes.hpp"
#include "C_OscXcoBase.hpp"
#include "C_OscXcoManifest.hpp"
#include "C_OscSystemDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXcoCreate :
   public C_OscXcoBase
{
public:
   static std::error_code h_CreatePackage(const std::string & orc_PackagePath,
                                          const C_OscSystemDefinition & orc_SystemDefinition,
                                          const C_OscXcoManifest & orc_Manifest,
                                          std::vector<std::string> & orc_WarningMessages,
                                          std::string & orc_ErrorMessage,
                                          const std::string & orc_TemporaryDirectory = "");

private:
   static const std::string mhc_USE_CASE;

   static std::error_code mh_CheckParamsToCreatePackage(const std::string & orc_PackagePath,
                                                        const C_OscSystemDefinition & orc_SystemDefinition,
                                                        const C_OscXcoManifest & orc_Manifest);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
