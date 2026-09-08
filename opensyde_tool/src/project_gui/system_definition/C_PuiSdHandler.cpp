//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI topology: stores additional UI information (implementation)

   UI topology: stores additional UI information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <mutex>
#include "precomp_headers.hpp"

#include "C_PuiSdHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
C_PuiSdHandler * C_PuiSdHandler::mhpc_Singleton = nullptr;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get singleton (Create if necessary)

   \return
   Pointer to singleton
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdHandler * C_PuiSdHandler::h_GetInstance(void)
{
   //Guard the lazy construction: the previous check-then-new was a data race
   //if two threads reached it at once. Destruction stays explicit via h_Destroy()
   //so shutdown ordering is preserved.
   static std::once_flag hc_OnceFlag;

   std::call_once(hc_OnceFlag, []
   {
      C_PuiSdHandler::mhpc_Singleton = new C_PuiSdHandler();
   });

   return C_PuiSdHandler::mhpc_Singleton;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clean up singleton
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdHandler::h_Destroy(void)
{
   delete C_PuiSdHandler::mhpc_Singleton;
   C_PuiSdHandler::mhpc_Singleton = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdHandler::C_PuiSdHandler(QObject * const opc_Parent) :
   C_PuiSdHandlerDataLoggerLogic(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdHandler::~C_PuiSdHandler(void)
{
}
