//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class to store all indices to identify a data element
   stored in a node (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLLISTELEMENTID_HPP
#define C_OSCNODEDATAPOOLLISTELEMENTID_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QHashFunctions>

#include "stwtypes.hpp"

#include "C_OscNodeDataPoolListId.hpp"

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

class C_OscNodeDataPoolListElementId : public C_OscNodeDataPoolListId {
public:
  C_OscNodeDataPoolListElementId(void);
  C_OscNodeDataPoolListElementId(const uint32_t ou32_NodeIndex,
                                 const uint32_t ou32_DataPoolIndex,
                                 const uint32_t ou32_ListIndex,
                                 const uint32_t ou32_ElementIndex);

  virtual bool operator<(const C_OscNodeDataPoolId &orc_Cmp) const;
  virtual bool operator==(const C_OscNodeDataPoolId &orc_Cmp) const;

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  uint32_t u32_ElementIndex;
};

inline size_t qHash(const C_OscNodeDataPoolListElementId &orc_Id,
                    size_t ou_Seed = 0) {
  QtPrivate::QHashCombine c_Hash;
  ou_Seed = c_Hash(ou_Seed, orc_Id.u32_NodeIndex);
  ou_Seed = c_Hash(ou_Seed, orc_Id.u32_DataPoolIndex);
  ou_Seed = c_Hash(ou_Seed, orc_Id.u32_ListIndex);
  ou_Seed = c_Hash(ou_Seed, orc_Id.u32_ElementIndex);
  return ou_Seed;
}

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
