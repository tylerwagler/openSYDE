//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class to group a unique node ID and CAN interface
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANINTERFACEID_HPP
#define C_OSCCANINTERFACEID_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

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

class C_OscCanInterfaceId {
public:
  C_OscCanInterfaceId();
  C_OscCanInterfaceId(const uint32_t ou32_NodeIndex,
                      const uint8_t ou8_InterfaceNumber);

  bool operator<(const C_OscCanInterfaceId &orc_Cmp) const;
  bool operator==(const C_OscCanInterfaceId &orc_Cmp) const;
  bool operator!=(const C_OscCanInterfaceId &orc_Cmp) const;

  void CalcHash(uint32_t &oru32_HashValue) const;

  uint32_t u32_NodeIndex;
  uint8_t u8_InterfaceNumber;
};

// Hash specialization for QHash
inline uint qHash(const C_OscCanInterfaceId &key) {
  // Use Qt's built-in qHash for basic types
  uint hash = ::qHash(key.u32_NodeIndex);
  hash ^= ::qHash(static_cast<uint>(key.u8_InterfaceNumber)) << 1;
  return hash;
}

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif