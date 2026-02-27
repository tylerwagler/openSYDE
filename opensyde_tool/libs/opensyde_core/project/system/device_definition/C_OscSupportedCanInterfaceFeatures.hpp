//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dataclass for supported can interface features
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPPORTEDCANINTERFACEFEATURES_H
#define C_OSCSUPPORTEDCANINTERFACEFEATURES_H

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

class C_OscSupportedCanInterfaceFeatures {
public:
  C_OscSupportedCanInterfaceFeatures(void);

  QString c_Interface;  ///< Interface name
  bool q_SupportsCanFd; ///< Flag if interface supports CAN-FD
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
