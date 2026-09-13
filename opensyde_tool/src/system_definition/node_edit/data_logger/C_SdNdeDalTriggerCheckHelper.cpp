//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Helper class to check trigger expressions (stub)

   Stub implementation. The real validator was built on the prebuilt
   osy_git_data_model_monitor library (lexer + expression AST), which has no
   source in this tree and only ever shipped as a committed .a for one platform.
   Until it is reimplemented, editor-time trigger validation is a no-op: h_Check
   reports the expression as valid and h_ParseDataElements returns nothing. The
   target device still validates trigger conditions at runtime. See docs/TODO.md.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdint>
#include "C_SdNdeDalTriggerCheckHelper.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check a trigger expression (stub: always reports valid)

   \param[in]   ou32_NodeIndex         Node index (unused in the stub)
   \param[in]   orc_Expression         Trigger expression (unused in the stub)
   \param[out]  opc_ErrorDetails       Error details (unused in the stub)
   \param[out]  opq_AreVariablesValid  Set to true
   \param[out]  opq_IsSyntaxValid      Set to true

   \return  true (the device validates trigger conditions at runtime)
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SdNdeDalTriggerCheckHelper::h_Check(const uint32_t ou32_NodeIndex, const std::string & orc_Expression,
                                           std::string * const opc_ErrorDetails, bool * const opq_AreVariablesValid,
                                           bool * const opq_IsSyntaxValid)
{
   static_cast<void>(ou32_NodeIndex);
   static_cast<void>(orc_Expression);
   static_cast<void>(opc_ErrorDetails);

   if (opq_AreVariablesValid != nullptr)
   {
      *opq_AreVariablesValid = true;
   }
   if (opq_IsSyntaxValid != nullptr)
   {
      *opq_IsSyntaxValid = true;
   }
   return true;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Parse the data elements referenced by a trigger condition (stub: none)

   \param[in]  orc_Condition   Trigger condition (unused in the stub)

   \return  empty list
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId> C_SdNdeDalTriggerCheckHelper::h_ParseDataElements(
   const std::string & orc_Condition)
{
   static_cast<void>(orc_Condition);
   return std::vector<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId>();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDalTriggerCheckHelper::C_SdNdeDalTriggerCheckHelper()
{
}
