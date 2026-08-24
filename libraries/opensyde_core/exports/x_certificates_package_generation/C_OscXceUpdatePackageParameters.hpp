//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp update package parameters data
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEUPDATEPACKAGEPARAMETERS_HPP
#define C_OSCXCEUPDATEPACKAGEPARAMETERS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXceUpdatePackageParameters
{
public:
   C_OscXceUpdatePackageParameters();

   std::string c_Password;
   std::string c_AuthenticationKeyPath;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
