//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp update package parameters data
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEUPDATEPACKAGEPARAMETERS_HPP
#define C_OSCXCEUPDATEPACKAGEPARAMETERS_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscXceUpdatePackageParameters {
public:
  C_OscXceUpdatePackageParameters();

  QString c_Password;
  QString c_AuthenticationKeyPath;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
