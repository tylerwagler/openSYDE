//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group domain parameter config data for HALC configuration
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGPARAMETER_HPP
#define C_OSCHALCCONFIGPARAMETER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefContent.hpp"
#include "stwtypes.hpp"
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

class C_OscHalcConfigParameter {
public:
  C_OscHalcConfigParameter();
  virtual ~C_OscHalcConfigParameter();

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  C_OscHalcDefContent c_Value; ///< Value configured by user for this parameter
  QString c_Comment;           ///< User comment for this parameter
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
