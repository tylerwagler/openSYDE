//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Helper class to check trigger expressions (stub)

   The data-logger trigger-expression validator was built on the prebuilt
   osy_git_data_model_monitor library, which has no source in this tree (only a
   committed .a for a single platform) and so could never be part of a
   cross-platform build. It is stubbed on every platform until the validator is
   reimplemented from scratch -- h_Check accepts any expression; the device
   validates triggers regardless. See docs/TODO.md.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDALTRIGGERCHECKHELPER_HPP
#define C_SDNDEDALTRIGGERCHECKHELPER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>
#include <vector>

#include "C_OscNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDalTriggerCheckHelper
{
public:
   static bool h_Check(const uint32_t ou32_NodeIndex, const std::string & orc_Expression,
                       std::string * const opc_ErrorDetails, bool * const opq_AreVariablesValid,
                       bool * const opq_IsSyntaxValid);
   static std::vector<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId> h_ParseDataElements(
      const std::string & orc_Condition);

private:
   C_SdNdeDalTriggerCheckHelper();
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
