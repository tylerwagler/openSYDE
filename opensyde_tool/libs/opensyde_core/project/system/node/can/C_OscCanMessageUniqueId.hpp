//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group for message ID part which identifies a unique CAN message
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANMESSAGEUNIQUEID_HPP
#define C_OSCCANMESSAGEUNIQUEID_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QHashFunctions>

#include "stwtypes.hpp"

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

class C_OscCanMessageUniqueId {
public:
  C_OscCanMessageUniqueId();
  C_OscCanMessageUniqueId(const uint32_t ou32_CanId, const bool oq_IsExtended);

  bool operator<(const C_OscCanMessageUniqueId &orc_Cmp) const;
  bool operator==(const C_OscCanMessageUniqueId &orc_Cmp) const;
  bool operator!=(const C_OscCanMessageUniqueId &orc_Cmp) const;

  uint32_t u32_CanId; ///< CAN message identifier
  bool q_IsExtended;  ///< Flag if message id is using extended format
};

inline size_t qHash(const C_OscCanMessageUniqueId &orc_Id,
                    size_t ou_Seed = 0) {
  QtPrivate::QHashCombine c_Hash;
  ou_Seed = c_Hash(ou_Seed, orc_Id.u32_CanId);
  ou_Seed = c_Hash(ou_Seed, orc_Id.q_IsExtended);
  return ou_Seed;
}

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
