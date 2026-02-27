//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for one raw entry in a parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETRAWENTRY_HPP
#define C_OSCPARAMSETRAWENTRY_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <vector>

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
/// Describes one raw (black box) entry in a parameter set file
class C_OscParamSetRawEntry {
public:
  C_OscParamSetRawEntry(void);

  uint32_t u32_StartAddress; ///< Start address to write to
  QByteArray c_Bytes;        ///< Bytes to write at address
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
