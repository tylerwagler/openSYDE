//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: Utilities

   cf. header for details

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <windows.h>
#include <lmcons.h> //for UNLEN
#include <cstdint>
#include "TglUtils.hpp"
#include <string>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Report assertion

   In this implementation: Display a messagebox with an error message containing all specified details.

   \param[in]   opcn_Module  Module name where the problem turned up
   \param[in]   opcn_Func    Function name where the problem turned up
   \param[in]   os32_Line    Line number where the problem turned up
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglReportAssertion(const char * const opcn_Module, const char * const opcn_Func,
                                  const int32_t os32_Line)
{
   std::string c_Text;

   c_Text = static_cast<std::string>("Extremely nasty error\n (assertion in module ") + opcn_Module + ", function " +
            opcn_Func + ", line " + std::to_string(os32_Line) + ") !";
   MessageBoxA(GetFocus(), c_Text.c_str(), "Error", MB_OK); //lint !e1960
                                                            //provided by system headers; no problems expected
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   report assertion with detail

   Show a messagebox with a detailed error message.

   \param[in]     opcn_DetailInfo        More information
   \param[in]     opcn_Module            Module name where the problem turned up
   \param[in]     opcn_Func              Function name where the problem turned up
   \param[in]     os32_Line              Line number where the problem turned up
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglReportAssertionDetail(const char * const opcn_DetailInfo, const char * const opcn_Module,
                                        const char * const opcn_Func, const int32_t os32_Line)
{
   std::string c_Text;

   c_Text = static_cast<std::string>(opcn_DetailInfo) + "\nfunction  " + opcn_Func + "\nassertion in module  " +
            opcn_Module + ", line  " + std::to_string(os32_Line) + "!";
   MessageBoxA(GetFocus(), c_Text.c_str(), "Detailed-Error", MB_OK); //lint !e1960
                                                                     //provided by system headers; no problems expected
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get system user name

   Reports the name of the currently logged in system user (i.e. the user running the active process)

   \param[out]    orc_UserName     name of logged in user

   \return
   true      user name detected and placed in orc_UserName  \n
   false     error -> orc_UserName not valid
*/
//----------------------------------------------------------------------------------------------------------------------
bool stw::tgl::TglGetSystemUserName(std::string & orc_UserName)
{
   char acn_WinUserName[UNLEN + 1];
   DWORD u32_Size = sizeof(acn_WinUserName);
   bool q_Return;

   q_Return = (GetUserNameA(acn_WinUserName, &u32_Size) == 0) ? false : true;
   if (q_Return == true)
   {
      orc_UserName = acn_WinUserName;
   }
   else
   {
      orc_UserName = "?\?\?\?\?";
   }
   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get system machine name

   Reports the name of the system

   \param[out]    orc_MachineName     name of system

   \return
   true      system name detected and placed in orc_MachineName  \n
   false     error -> orc_MachineName not valid
*/
//----------------------------------------------------------------------------------------------------------------------
bool stw::tgl::TglGetSystemMachineName(std::string & orc_MachineName)
{
   char acn_WinSystemName[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD u32_Size = sizeof(acn_WinSystemName);
   const bool q_Return = (GetComputerNameA(acn_WinSystemName, &u32_Size) == 0) ? false : true;

   if (q_Return == true)
   {
      orc_MachineName = acn_WinSystemName;
   }
   else
   {
      orc_MachineName = "?\?\?\?\?";
   }
   return q_Return;
}
