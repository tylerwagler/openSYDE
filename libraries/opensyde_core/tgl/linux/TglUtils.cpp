//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: Utilities

   Target Glue Layer module containing utilities
   Implementation for Linux.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <err.h>
#include <pwd.h>
#include <climits>
#include <cstdint>
#include "TglUtils.hpp"

// HOST_NAME_MAX is a Linux extension. POSIX only guarantees _POSIX_HOST_NAME_MAX (255), which
// is what macOS provides, and 255 is also what Linux resolves HOST_NAME_MAX to on glibc.
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif


/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report an assertion.

   Show a error message.

   \param[in]     opcn_Module    Module name where the problem turned up
   \param[in]     opcn_Func      Function name where the problem turned up
   \param[in]     os32_Line      Line number where the problem turned up
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglReportAssertion(const char * const opcn_Module, const char * const opcn_Func,
                                  const int32_t os32_Line)
{
   std::string c_Text;

   c_Text = std::string("Extremely nasty error\n (assertion in module ") + opcn_Module + ", function " +
            opcn_Func + ", line " + std::to_string(os32_Line) + ") !";
   warnx("%s", c_Text.c_str());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report assertion with detail

   Show a messagebox with an detailed error message.

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

   c_Text = std::string(opcn_DetailInfo) +
            "\nfunction  " + opcn_Func +
            "\nassertion in module  " + opcn_Module +
            ", line  " + std::to_string(os32_Line) +
            "!";
   warnx("%s", c_Text.c_str());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get system user name

   Reports the name of the currently logged in system user (i.e. the user running the active process)

   Known issue: in the following scenario this function will return false:
      - compiling this with a 32-bit compiler on a 64-bit machine
      - that machine doesn't have the 32-bit (i386) verion of libnss-sss installed
      - user is no local user on the machine but a network user (LDAP)

   \param[out]    orc_UserName     name of logged in user

   \return
   true      user name detected and places in oc_UserName  \n
   false     error -> oc_UserName not valid
*/
//----------------------------------------------------------------------------------------------------------------------
bool stw::tgl::TglGetSystemUserName(std::string & orc_UserName)
{
   struct passwd * pc_PassWord;
   bool q_Return = false;

   pc_PassWord = getpwuid(geteuid());
   if (pc_PassWord != nullptr)
   {
      orc_UserName = pc_PassWord->pw_name;
      q_Return    = true;
   }
   else
   {
      orc_UserName = "\?\?\?\?\?";
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
   char acn_HostName[HOST_NAME_MAX + 1];
   const int x_Result = //lint !e8080 !e970  type defined by API we use
                        gethostname(&acn_HostName[0], sizeof(acn_HostName));

   if (x_Result == 0)
   {
      orc_MachineName = acn_HostName;
   }
   else
   {
      orc_MachineName = "?\?\?\?\?";
   }
   return (x_Result == 0) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Poll system message queue

   Polls the system's message queue and posts detected messages for handling.
   This function can be used to proceed processing system messages while actively waiting for an event.
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglHandleSystemMessages(void)
{
   /*
      MSG t_Msg;
      bool q_Return;
      q_Return = PeekMessage(&t_Msg, 0, 0, 0, PM_REMOVE) == 0 ? false : true;
      if (q_Return == true)
      {
         TranslateMessage(&t_Msg);
         DispatchMessage(&t_Msg);
      }
   */
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set environment variable for calling process

   Sets an environment variable to a fixed value.
   Implemented with setenv() on Linux, which copies both strings. The historical
   putenv()-based body was unsafe: putenv retains a pointer to the passed storage
   (POS34-C), and the previous implementation passed a stack buffer, leaving the
   environment pointing at dead memory once the function returned.

   \param[in]    orc_Name    name of environment variable
   \param[in]    orc_Value   value of environment variable

   \return
   0   variable set
   -1  could not set variable
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t stw::tgl::TglSetEnvironmentVariable(const std::string & orc_Name, const std::string & orc_Value)
{
   const int32_t s32_Return = setenv(orc_Name.c_str(), orc_Value.c_str(), 1);
   return (s32_Return == 0) ? 0 : -1;
}
