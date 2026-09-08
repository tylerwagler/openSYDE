//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Config Package base
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCOBASE_HPP
#define C_OSCXCOBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <string>
#include <vector>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXcoBase
{
public:
   static std::string h_GetPackageExtension();

   static const std::string hc_PACKAGE_EXT;
   static const std::string hc_PACKAGE_EXT_TMP; // intermediate directory before creating zip archive
   static const std::string hc_XCFG_SYSDEF;
   static const std::string hc_XCFG_SYSDEF_FOLDER;
   static const std::string hc_INI_DEV_FOLDER;

protected:
   static stw::scl::std::vector<std::string> mhc_WarningMessages; // global warnings e.g. if update position of active node is
                                                         // not available
   static std::string mhc_ErrorMessage;        // description of error which caused the service update package
                                                         // to fail

   static void mh_Init(void);
   static void mh_GetWarningsAndErrors(stw::scl::std::vector<std::string> & orc_WarningMessages,
                                       std::string & orc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
