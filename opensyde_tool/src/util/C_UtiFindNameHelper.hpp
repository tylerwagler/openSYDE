//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class to find bus or node by string (header)

   Class to find bus or node by string (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_UTIFINDNAMEHELPE_HPP
#define C_UTIFINDNAMEHELPE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_gui_logic {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_UtiFindNameHelper {
public:
  C_UtiFindNameHelper(const QString &orc_Cmp = "");
  bool operator()(const QString *const opc_Cur) const;

private:
  const QString mc_Cmp; ///< String to compare all items to
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_gui_logic
} // namespace stw

#endif
