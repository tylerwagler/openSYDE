//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE .conf file handler
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
 */
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCFGFILEHANDLER_HPP
#define C_OSCCFGFILEHANDLER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include <string>
#include "C_SclStringList.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///Generic config class handler
class C_OscConfFileHandler
{
public:
   virtual ~C_OscConfFileHandler();

   std::error_code LoadSettings(const std::string & orc_Path);

protected:
   // Function for loading concrete settings. Must return C_CONFIG in error case
   virtual std::error_code m_LoadSettings(const stw::scl::C_SclStringList & orc_SettingsWithoutComments) = 0;
   static std::error_code mh_ReplaceSettings(const std::string & orc_Path,
                                      const std::vector<std::pair<std::string,
                                                                  std::string> > & orc_Configs);

    std::string mc_ConfigFilePath; //path where the config file was loaded from
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
