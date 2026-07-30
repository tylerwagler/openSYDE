//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       DiagLib Target Glue Layer: Task system related functionality

   cf. header for details

   Here: Implementation for Windows.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <cstdlib>
#include <windows.h>
#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "TglTasks.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::tgl;
using namespace stw::scl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_TglCriticalSection::C_TglCriticalSection(void)
{
   InitializeCriticalSection(&mc_CriticalSection);
}

//----------------------------------------------------------------------------------------------------------------------

C_TglCriticalSection::~C_TglCriticalSection(void)
{
   DeleteCriticalSection(&mc_CriticalSection);
}

//----------------------------------------------------------------------------------------------------------------------
//block until the section is released, then claim it and return
void C_TglCriticalSection::Acquire(void)
{
   EnterCriticalSection(&mc_CriticalSection);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   claim the section if it is free; do not if it is not

   \return
   true      section was claimed
   false     section was not claimed is it is already occupied
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_TglCriticalSection::TryAcquire(void)
{
   bool q_Return;

   q_Return = (TryEnterCriticalSection(&mc_CriticalSection) == 0) ? false : true;
   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
//release claimed section
void C_TglCriticalSection::Release(void)
{
   LeaveCriticalSection(&mc_CriticalSection);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Start process with call orc_CompleteCall and return after start

   \param[in]  orc_BinaryPath    Path to binary to start
   \param[in]  orc_Parameters    Command line parameters to pass to the binary ("" for no parameters)

   \return
   STW error codes

   \retval   C_NO_ERR   Process started
   \retval   C_NOACT    Could not start process
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t stw::tgl::TglStartProcessDetached(const stw::scl::C_SclString & orc_BinaryPath,
                                          const stw::scl::C_SclString & orc_Parameters)
{
   int32_t s32_Retval = C_NO_ERR;

   if (TglFileExists(orc_BinaryPath) == false)
   {
      s32_Retval = C_NOACT;
   }
   else
   {
      STARTUPINFOA c_Info;
      PROCESS_INFORMATION c_ProcessInfo;
      char_t * pcn_CommandLine;
      C_SclString c_Path;
      const int x_START_MODE = CREATE_NEW_CONSOLE; //lint !e970 !e8080 //using type expected by API

      (void)memset(&c_Info, 0, sizeof(c_Info));
      c_Info.cb = sizeof(c_Info);

      c_Path = TglExtractFilePath(orc_BinaryPath);
      if (c_Path.Trim() == "")
      {
         c_Path = ".\\";
      }

      pcn_CommandLine = new char_t[orc_Parameters.Length() + 1];
      strncpy(pcn_CommandLine, orc_Parameters.c_str(), static_cast<size_t>(orc_Parameters.Length()) + 1U);
      {
         const int32_t s32_Return = CreateProcessA(
            orc_BinaryPath.c_str(), pcn_CommandLine, NULL, NULL, FALSE, x_START_MODE,
            NULL, c_Path.c_str(),
            &c_Info, &c_ProcessInfo);

         delete[] pcn_CommandLine;

         if (s32_Return == 0)
         {
            s32_Retval = C_NOACT;
         }
      }
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
