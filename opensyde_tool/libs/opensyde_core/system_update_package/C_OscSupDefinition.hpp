//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System update package definition
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPDEFINITION_HPP
#define C_OSCSUPDEFINITION_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscSupNodeDefinition.hpp"
#include <QList>

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

class C_OscSupDefinition {
public:
  C_OscSupDefinition(void);

  // lint -sem(stw::opensyde_core::C_OscSupDefinition::Initialize,initializer)
  void Initialize(void);

  QList<C_OscSupNodeDefinition> c_Nodes;
  uint32_t u32_ActiveBusIndex;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
