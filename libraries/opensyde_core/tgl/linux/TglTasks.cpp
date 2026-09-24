//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: Task system related functionality

   Target Glue Layer module containing task system related functionality.
   Implementation for Linux/POSIX.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>

#include <cstdint>
#include "stwerrors.hpp"
#include "TglTasks.hpp"
#include "TglFile.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */
static void m_SplitCommandLineParameters(const std::string & orc_Parameters, std::vector<std::string> & orc_Arguments);

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_TglCriticalSection::C_TglCriticalSection(void)
{
   pthread_mutex_init(&mu_Mutex, NULL);
}

//----------------------------------------------------------------------------------------------------------------------

C_TglCriticalSection::~C_TglCriticalSection(void)
{
   pthread_mutex_destroy(&mu_Mutex);
}

//----------------------------------------------------------------------------------------------------------------------
//block until the section is released, then claim it and return
void C_TglCriticalSection::Acquire(void)
{
   pthread_mutex_lock(&mu_Mutex);
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
   const int x_Ret = pthread_mutex_trylock(&mu_Mutex);

   return (x_Ret == 0) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------
//release claimed section
void C_TglCriticalSection::Release(void)
{
   pthread_mutex_unlock(&mu_Mutex);
}

//----------------------------------------------------------------------------------------------------------------------

static void m_SplitCommandLineParameters(const std::string & orc_Parameters, std::vector<std::string> & orc_Arguments)
{
   std::string c_Current;
   bool q_InSingleQuotes = false;
   bool q_InDoubleQuotes = false;
   bool q_EscapeNext = false;

   for (uint32_t u32_It = 0U; u32_It < orc_Parameters.size(); ++u32_It)
   {
      const char cn_Char = orc_Parameters[u32_It];

      if (q_EscapeNext == true)
      {
         c_Current += cn_Char;
         q_EscapeNext = false;
      }
      else if ((cn_Char == '\\') && (q_InSingleQuotes == false))
      {
         q_EscapeNext = true;
      }
      else if ((cn_Char == '"') && (q_InSingleQuotes == false))
      {
         q_InDoubleQuotes = (q_InDoubleQuotes == false);
      }
      else if ((cn_Char == '\'') && (q_InDoubleQuotes == false))
      {
         q_InSingleQuotes = (q_InSingleQuotes == false);
      }
      else if (((cn_Char == ' ') || (cn_Char == '\t')) && (q_InSingleQuotes == false) && (q_InDoubleQuotes == false))
      {
         if (c_Current.empty() == false)
         {
            orc_Arguments.push_back(c_Current);
            c_Current.clear();
         }
      }
      else
      {
         c_Current += cn_Char;
      }
   }

   if (q_EscapeNext == true)
   {
      c_Current += '\\';
   }

   if (c_Current.empty() == false)
   {
      orc_Arguments.push_back(c_Current);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Start process and return after start

   Function shall set execution folder to the folder containing the binary.
   This ensures that relative paths in the executable and its dependencies work as expected.

   \param[in]  orc_BinaryPath     Path to binary to start
   \param[in]  orc_Parameters     Command line parameters to pass to the binary ("" for no parameters)

   \return
   STW error codes

   \retval   C_NO_ERR   Process started
   \retval   C_NOACT    Could not start process
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t stw::tgl::TglStartProcessDetached(const std::string & orc_BinaryPath, const std::string & orc_Parameters)
{
   std::vector<std::string> c_ArgumentStorage;
   int32_t as32_Pipes[2] = {-1, -1}; //initialize to invalid file descriptors
   int32_t s32_Retval = C_NOACT;
   bool q_Continue = true;

   // Set up argument vector for execv
   // Path to binary is first argument by convention:
   c_ArgumentStorage.push_back(orc_BinaryPath);
   // Parse and add additional parameters:
   m_SplitCommandLineParameters(orc_Parameters, c_ArgumentStorage);

   // Create a parent/child status channel used to detect exec success.
   if (pipe(&as32_Pipes[0]) != 0)
   {
      q_Continue = false;
   }

   // Ensure the write end closes automatically on successful exec.
   if ((q_Continue == true) && (fcntl(as32_Pipes[1], F_SETFD, FD_CLOEXEC) != 0))
   {
      (void)close(as32_Pipes[0]);
      (void)close(as32_Pipes[1]);
      q_Continue = false;
   }

   if (q_Continue == true)
   {
      const pid_t x_ForkResult = fork();
      if (x_ForkResult == 0)
      {
         int32_t s32_ExecErrno;
         std::string c_BinaryDir;
         std::vector<char *> c_Argv;
         ssize_t x_WriteResult;

         // Child does not read from the synchronization pipe.
         (void)close(as32_Pipes[0]);
         // Start a new session so the child is detached from the parent context.
         (void)setsid();

         // Run the executable in its own directory. So relative paths in the executable and its dependencies work as
         // expected.
         c_BinaryDir = TglExtractFilePath(orc_BinaryPath);
         if (chdir(c_BinaryDir.c_str()) != 0)
         {
            s32_ExecErrno = errno;
            // Report startup failure back to the parent.
            // assign and then ignore the result; nothing serious can happen
            x_WriteResult = write(as32_Pipes[1], &s32_ExecErrno, sizeof(s32_ExecErrno));
            (void)x_WriteResult;
            // Terminate child immediately without running parent-process cleanup.
            _exit(EXIT_FAILURE);
         }

         // Convert argument storage to the form required by execv:
         c_Argv.reserve(c_ArgumentStorage.size() + 1U); //leave +1 for the final NULL
         for (uint32_t u32_It = 0U; u32_It < c_ArgumentStorage.size(); ++u32_It)
         {
            // Let it point to the already existing string data:
            c_Argv.push_back(const_cast<char *>(c_ArgumentStorage[u32_It].c_str()));
         }
         c_Argv.push_back(NULL);

         // Replace child image with target executable and arguments.
         (void)execv(orc_BinaryPath.c_str(), &c_Argv[0]);
         s32_ExecErrno = errno;
         // If exec returns, it failed; communicate the error to parent.
         // assign and then ignore the result; nothing serious can happen
         x_WriteResult = write(as32_Pipes[1], &s32_ExecErrno, sizeof(s32_ExecErrno));
         (void)x_WriteResult;
         // Terminate child immediately without running parent-process cleanup.
         _exit(EXIT_FAILURE);
      }
      else if (x_ForkResult > 0)
      {
         int32_t s32_ChildErrno;
         ssize_t x_ReadResult;

         // Parent does not write to the synchronization pipe.
         (void)close(as32_Pipes[1]);
         // Read child error status
         x_ReadResult = read(as32_Pipes[0], &s32_ChildErrno, sizeof(s32_ChildErrno));
         (void)close(as32_Pipes[0]);

         if (x_ReadResult == 0)
         {
            //0 == EOF means exec succeeded and pipe was closed by CLOEXEC.
            s32_Retval = C_NO_ERR;
         }
         else
         {
            // Reap failed child process to avoid leaving a zombie.
            (void)waitpid(x_ForkResult, NULL, 0);
         }
      }
      else
      {
         // Fork failed; close pipe resources in the parent.
         (void)close(as32_Pipes[0]);
         (void)close(as32_Pipes[1]);
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
