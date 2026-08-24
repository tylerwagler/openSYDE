//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standalone HALC channel config with parameter Ids
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONECHANNEL_HPP
#define C_OSCHALCCONFIGSTANDALONECHANNEL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcConfigStandaloneChannel
{
public:
   C_OscHalcConfigStandaloneChannel(void);

   std::vector<std::string> c_ParameterIds;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
