//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent main module

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_CaApplication.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::osy_crypto_agent;

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   crypto agent main function

   \param[in]    os32_Argc      number of command line arguments
   \param[in]    opcn_Argv      command line arguments

   \return
   result code of application; (0 = OK; >0 = error);
   see C_CaApplication::E_ExecutionResult for specific values
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t main(const int32_t os32_Argc, char_t * const opcn_Argv[])
{
   C_CaApplication::E_ExecutionResult e_Result;
   C_CaApplication c_TheApplication;

   //call as early as possible to ensure we are attached to the console before any output is generated
   C_CaServerPlatform::h_AttachToParentConsole();

   e_Result = c_TheApplication.Execute(os32_Argc, opcn_Argv);

   return static_cast<int32_t>(e_Result);
}

//----------------------------------------------------------------------------------------------------------------------
