//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job additional trigger expert mode properties
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERJOBADDITIONALTRIGGEREXPERTMODE_H
#define C_OSCDATALOGGERJOBADDITIONALTRIGGEREXPERTMODE_H

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include "stwtypes.hpp"
#include <string>
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDataLoggerJobAdditionalTriggerExpertMode
{
public:
   C_OscDataLoggerJobAdditionalTriggerExpertMode();

   void CalcHash(uint32_t & oru32_HashValue) const;
   static void h_ReplaceUiVarNames(std::string & orc_Expression);

   bool q_Enable;                                                               ///< flag to enable expert mode
   std::string c_TriggerConfiguration;                                ///< trigger configuration, only
                                                                                // valid if enabled flag is set
   std::vector<C_OscNodeDataPoolListElementOptArrayId> c_TriggerDataElementIds; ///< data element IDs configured in
                                                                                // trigger string
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
