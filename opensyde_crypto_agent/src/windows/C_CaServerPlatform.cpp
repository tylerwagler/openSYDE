//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief        openSYDE Crypto Agent server class Windows driver implementation

   For details see documentation in .hpp file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdio>
#include <string>

#include <winsock2.h>
#include <windows.h>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_SclString.hpp"
#include "C_CaServerPlatform.hpp"
#include "C_OscLoggingHandler.hpp"

//A few module-wide PC-lint definitions.
//The macros from the socket API have a few non-compliant issues.
//No quality issues expected as we use them in the intended ways.
//lint -emacro(717 953 970 1924 8001 8002 8020 8047 8058 8080 9012 9093 9113 9177, FD_ZERO)
//lint -emacro(661 676 717  970 1924 8001 8002 8058 8080 9084 9093 9177 9123 9126 9130, FD_SET)
//lint -emacro(661 676 970 1924 9119 9123 9126 9130 9133 9177, FD_ISSET)
//lint -emacro(9105, INFINITE)
//lint -emacro(9130, INVALID_SOCKET)

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::osy_crypto_agent;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
SOCKET C_CaServerPlatform::mhx_ListeningSocket = INVALID_SOCKET; //lint !e8080 //using API type
C_CaServerPlatform * C_CaServerPlatform::mhpc_TheInstance = NULL;
volatile bool C_CaServerPlatform::mhq_ShutdownRequested = false;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_CaServerPlatform::C_CaServerPlatform() :
   C_CaServer()
{
}

//----------------------------------------------------------------------------------------------------------------------
C_CaServerPlatform::~C_CaServerPlatform()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Receive TCP data

   Try to receive specified number of bytes from tcp socket.
   Wraps multiple calls of recv as we might receive a service in multiple calls due to the stream-based nature of TCP.

   \param[in]    ox_Socket      socket to receive on
   \param[out]   opu8_Buffer    reception buffer
   \param[in]    ou32_NumBytes  number of bytes to receive

   \retval   total number of received bytes
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{8080} //using type to match used library interface
uint32_t C_CaServerPlatform::mh_Receive(const SOCKET ox_Socket, uint8_t * const opu8_Buffer,
                                        const uint32_t ou32_NumBytes)
{
   //lint -e{9176} //Side-effect of the "char"-based API
   char_t * const pcn_Data = reinterpret_cast<char_t *>(opu8_Buffer);
   uint32_t u32_TotalRead = 0U;
   bool q_Cancel = false;

   while ((u32_TotalRead < ou32_NumBytes) && (q_Cancel == false))
   {
      const int x_ReadNow = //lint !e970 !e8080 //using API type
                            recv(ox_Socket, &pcn_Data[u32_TotalRead],
                                 static_cast<int>(ou32_NumBytes - u32_TotalRead), //lint !e970 //using API type
                                 0);
      if (x_ReadNow <= 0) // peer closed or error
      {
         q_Cancel = true;
         u32_TotalRead = 0;
      }
      else
      {
         u32_TotalRead += static_cast<uint32_t>(x_ReadNow);
      }
   }
   return u32_TotalRead;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send TCP data

   Try to send specified number of bytes via tcp socket.
   In rare cases if could happen that not all bytes are sent in one call, so we use this wrapper to be defensive.

   \param[in]    ox_Socket      socket to send on
   \param[in]    opu8_Buffer     transmission buffer
   \param[in]    ou32_NumBytes  number of bytes to send

   \retval   total number of sent bytes
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{8080} //using type to match used library interface
uint32_t C_CaServerPlatform::mh_Send(const SOCKET ox_Socket, const uint8_t * const opu8_Buffer,
                                     const uint32_t ou32_NumBytes)
{
   //lint -e{9176} //Side-effect of the "char"-based API
   const char_t * const pcn_Data = reinterpret_cast<const char_t *>(opu8_Buffer);
   bool q_Cancel = false;
   uint32_t u32_TotalSent = 0;

   while ((u32_TotalSent < ou32_NumBytes) && (q_Cancel == false))
   {
      const int x_SentNow = //lint !e970 !e8080 //using API type
                            send(ox_Socket, &pcn_Data[u32_TotalSent],
                                 static_cast<int>(ou32_NumBytes - u32_TotalSent), //lint !e970 //using API type
                                 0);
      if (x_SentNow <= 0)
      {
         //error
         q_Cancel = true;
         u32_TotalSent = 0;
      }
      else
      {
         u32_TotalSent += static_cast<uint32_t>(x_SentNow);
      }
   }
   return u32_TotalSent;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Main server thread function

   Loop and wait for TCP connects.
   Abort if the corresponding flag is set.

   \param[in]    opv_Arg     thread handler parameter (unused)

   \retval
   0    shutdown requested via TCP or CTRL handler without any unexepcted errors
   1    shutdown requested; at least one unexpected error happened; see log for details
*/
//----------------------------------------------------------------------------------------------------------------------
DWORD WINAPI C_CaServerPlatform::mh_ServerThread(LPVOID opv_Arg) //lint !e818 !e952 //API defined by library
{
   (void)opv_Arg;
   DWORD x_ThreadResult = 0; //lint !e8080 //using API type

   while (mhq_ShutdownRequested == false)
   {
      int x_Result; //lint !e970 !e8080 //using API type
      timeval c_TimeoutValue;
      fd_set c_Rfds;
      FD_ZERO(&c_Rfds);
      FD_SET(mhx_ListeningSocket, &c_Rfds);

      c_TimeoutValue.tv_sec = 1; // 1-second timeout; poll for shutdown request in this interval
      c_TimeoutValue.tv_usec = 0;

      x_Result = select(static_cast<int>(mhx_ListeningSocket + 1), //lint !e970 //using API type
                        &c_Rfds, NULL, NULL, &c_TimeoutValue);
      if ((x_Result > 0) && (FD_ISSET(mhx_ListeningSocket, &c_Rfds) != 0))
      {
         sockaddr_in c_ClientAddress;
         int x_ClientAddressSize = sizeof(c_ClientAddress); //lint !e970 !e8080 //using API type

         //lint -e{9176}  Side-effect of the API. Match is guaranteed by the API.
         const SOCKET x_ClientSocket = //lint !e8080 //using API type
                                       accept(mhx_ListeningSocket,
                                              reinterpret_cast<sockaddr *>(&c_ClientAddress),
                                              &x_ClientAddressSize);
         if (x_ClientSocket == INVALID_SOCKET)
         {
            // This should not happen in normal operation. Log and try to continue accepting new connections.
            osc_write_log_warning("TCP handler", "Could not accept incoming connection. Retrying.");
            x_ThreadResult = 1;
         }
         else
         {
            C_SclString c_LogText;
            c_LogText.PrintFormatted("Incoming TCP connection from %d.%d.%d.%d.",
                                     c_ClientAddress.sin_addr.S_un.S_un_b.s_b1,
                                     c_ClientAddress.sin_addr.S_un.S_un_b.s_b2,
                                     c_ClientAddress.sin_addr.S_un.S_un_b.s_b3,
                                     c_ClientAddress.sin_addr.S_un.S_un_b.s_b4);
            osc_write_log_info("TCP handler", c_LogText.c_str());

            //set socket options to not block forever if the client does not provide enough data:
            const DWORD x_TIMEOUT_MS = 1000; //lint !e8080 //using API type  // 1 second
            //lint -e{9176}  Side-effect of the API. We use the correct type.
            if (setsockopt(x_ClientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char_t *>(&x_TIMEOUT_MS),
                           sizeof(x_TIMEOUT_MS)) == SOCKET_ERROR)
            {
               // Handle error
               osc_write_log_warning("TCP handler", "Failed to set socket timeout. Unexpected blocking might happen.");
               x_ThreadResult = 1;
            }

            //TCP data can arrive in multiple chunks as it is stream based.
            //So a service request can arrive in multiple calls of "recv".
            //We use a simple size header to communicate the expected size of a service.
            //We first check for this uint32 header value and then try to read the rest of the service:

            // Read header (length prefix)
            uint8_t au8_ServiceSize[4];

            if (mh_Receive(x_ClientSocket, &au8_ServiceSize[0], 4) != 4U)
            {
               osc_write_log_error("TCP handler", "Received incomplete service header, ignoring.");
               //in this case also close socket to get a clean start for the next connection
            }
            else
            {
               uint32_t u32_ServiceSize =
                  (static_cast<uint32_t>(au8_ServiceSize[0]) << 24U) +
                  (static_cast<uint32_t>(au8_ServiceSize[1]) << 16U) +
                  (static_cast<uint32_t>(au8_ServiceSize[2]) << 8U) +
                  (static_cast<uint32_t>(au8_ServiceSize[3]));

               //perform sanity check: no need to try to handle services larger than max
               if (u32_ServiceSize > mhu16_MAX_REQUEST_SIZE)
               {
                  osc_write_log_error("TCP handler", "Received service size too big, ignoring.");
               }
               else
               {
                  // Allocate buffer and read payload
                  std::vector<uint8_t> c_Request;
                  c_Request.resize(u32_ServiceSize);
                  if (mh_Receive(x_ClientSocket, &c_Request[0], u32_ServiceSize) != u32_ServiceSize)
                  {
                     osc_write_log_error("TCP handler", "Received incomplete service, ignoring.");
                  }
                  else
                  {
                     std::vector<uint8_t> c_Response;
                     //call generic routine to handle actual request payload:
                     mhpc_TheInstance->m_HandleRequest(c_Request, c_Response);
                     if (c_Response.size() > 0)
                     {
                        std::vector<uint8_t> c_ServiceSize;
                        u32_ServiceSize = static_cast<uint32_t>(c_Response.size());
                        c_ServiceSize.resize(4);
                        c_ServiceSize[0] = static_cast<uint8_t>(u32_ServiceSize >> 24U);
                        c_ServiceSize[1] = static_cast<uint8_t>(u32_ServiceSize >> 16U);
                        c_ServiceSize[2] = static_cast<uint8_t>(u32_ServiceSize >> 8U);
                        c_ServiceSize[3] = static_cast<uint8_t>(u32_ServiceSize & 0xFFU);

                        c_Response.insert(c_Response.begin(), c_ServiceSize.begin(), c_ServiceSize.end());

                        if (mh_Send(x_ClientSocket, &c_Response[0], static_cast<uint32_t>(c_Response.size())) !=
                            static_cast<uint32_t>(c_Response.size()))
                        {
                           osc_write_log_error("TCP handler", "Failed to send service response.");
                           x_ThreadResult = 1;
                        }
                        else
                        {
                           osc_write_log_info("TCP handler", "Service response sent.");
                        }

                        //tell WinSock we will close that socket again so it will send out the response in any case:
                        shutdown(x_ClientSocket, SD_SEND);
                     }
                  }
               }
            }
         }

         closesocket(x_ClientSocket);
      }
   }

   closesocket(mhx_ListeningSocket);
   mhx_ListeningSocket = INVALID_SOCKET;
   return x_ThreadResult;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handler for control events

   Called by system as control handler. e.g. ctrl+c in console.
   Will request the listener thread to shut down.

   \param[in]    ox_CtrlType   Type of event detected

   \retval   TRUE   event handled
   \retval   FALSE  event not handled (system will try to do its best)
*/
//----------------------------------------------------------------------------------------------------------------------
BOOL WINAPI C_CaServerPlatform::mh_ConsoleCtrlHandler(const DWORD ox_CtrlType) //lint !e8080 //API define by library
{
   BOOL x_Result; //lint !e8080 //using API type

   switch (ox_CtrlType)
   {
   case CTRL_C_EVENT:
   case CTRL_BREAK_EVENT:
   case CTRL_CLOSE_EVENT:
   case CTRL_SHUTDOWN_EVENT:
   case CTRL_LOGOFF_EVENT:
      osc_write_log_info("Platform handler", "Application termination requested");
      C_CaServerPlatform::mhq_ShutdownRequested = true;
      x_Result = TRUE;
      break;
   default:
      x_Result = FALSE;
      break;
   }
   return x_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Attach to parent console

   For Windows the application should be built as a WIN32 application, so we do not get a console window by default.

   Console strategy:
   Goal: For regular operation we do not want a console window,
         but when started from a command line we want to print output to the console.

   Approach:
   * By default run without console (build as WIN32 application)
   ** But try to attach to the parent process's console, if it exists.
   ** So when started from a command line, we can print output to the console.
   ** When started from e.g. explorer or spawned by another tool we will not have any output
      (but we still have the log file).
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaServerPlatform::h_AttachToParentConsole()
{
   // Try to attach to the parent process's console, if it exists.
   // This allows us to print output to the console when started from a command line.
   //lint -e{1924} //plain C cast used in macro; intended by API
   if (AttachConsole(ATTACH_PARENT_PROCESS) != 0)
   {
      // Redirect stdio to the console
      FILE * pc_File;
      freopen_s(&pc_File, "CONOUT$", "w", stdout);
      freopen_s(&pc_File, "CONOUT$", "w", stderr);
      freopen_s(&pc_File, "CONIN$",  "r", stdin);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set up and execute listener thread

   Technically as long as the application's only job is to react to incoming TCP requests,
    we could also run the listener in the main thread and not use a separate thread at all.
   Using this approach will improve flexibility for future expansion.

   Steps:
   * initialize handler to handle process termination (e.g. ctrl+c) and allow graceful shutdown
   * initialize TCP listener
   * set up and start listener thread
   * wait for termination by close request or local applicationm shutdown

   \param[in]    ou16_ServerPort    TCP server port to listen on
   \param[in]    orau8_BindAddress  bind address for server

   \retval   C_NO_ERR   All set up, thread was run and was terminated again.
   \retval   C_WARN     All set up, thread was run and was terminated again. Thread reported unexpected issues.
   \retval   C_COM      Could not initialize TCP server
   \retval   C_NOACT    Could not initialize listener thread
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServerPlatform::m_ExecuteListener(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4])
{
   int32_t s32_Result;
   WSADATA c_WsaData;

   const BOOL x_Result = SetConsoleCtrlHandler(&mh_ConsoleCtrlHandler, TRUE); //lint !e8080 //using API type

   if (x_Result == 0)
   {
      osc_write_log_warning("Platform handler startup", "Could not initialize console ctrl handler");
   }

   mhpc_TheInstance = this;
   s32_Result = C_COM;

   if (WSAStartup(0x0202U, &c_WsaData) == 0)
   {
      mhx_ListeningSocket = socket(AF_INET, SOCK_STREAM, 0);

      if (mhx_ListeningSocket != INVALID_SOCKET)
      {
         const int x_OPTION = 1; //lint !e970 !e8080 //using API type

         //lint -e{9176}  //Side-effect of the used API. Using correct type.
         if (setsockopt(mhx_ListeningSocket, SOL_SOCKET, SO_REUSEADDR,
                        reinterpret_cast<const char_t *>(&x_OPTION), sizeof(x_OPTION)) == 0)
         {
            struct sockaddr_in c_Address;
            c_Address.sin_family = AF_INET;
            c_Address.sin_addr.S_un.S_un_b.s_b1 = orau8_BindAddress[0];
            c_Address.sin_addr.S_un.S_un_b.s_b2 = orau8_BindAddress[1];
            c_Address.sin_addr.S_un.S_un_b.s_b3 = orau8_BindAddress[2];
            c_Address.sin_addr.S_un.S_un_b.s_b4 = orau8_BindAddress[3];
            c_Address.sin_port = htons(ou16_ServerPort);

            //lint -e{9176}  //Side-effect of the API. Using correct type.
            if (bind(mhx_ListeningSocket, reinterpret_cast<sockaddr *>(&c_Address), sizeof(c_Address)) == 0)
            {
               if (listen(mhx_ListeningSocket, 16) == 0)
               {
                  const HANDLE pv_Thread = CreateThread(NULL, 0, &mh_ServerThread, NULL, 0, NULL);
                  if (pv_Thread == NULL)
                  {
                     osc_write_log_error("Platform handler startup", "Failed to create listener thread");
                     s32_Result = C_NOACT;
                  }
                  else
                  {
                     DWORD x_ExitCode; //lint !e8080 //using API type
                     osc_write_log_info("Platform handler startup", "Service listener up and running");
                     WaitForSingleObject(pv_Thread, INFINITE);

                     if ((GetExitCodeThread(pv_Thread, &x_ExitCode) != 0) && (x_ExitCode == 0))
                     {
                        s32_Result = C_NO_ERR;
                     }
                     else
                     {
                        s32_Result = C_WARN;
                     }
                     CloseHandle(pv_Thread);
                  }
               }
               else
               {
                  osc_write_log_error("Platform handler startup", "Failed to listen");
               }
            }
            else
            {
               osc_write_log_error("Platform handler startup",
                                   "Failed to bind (WSA error " +
                                   C_SclString::IntToStr(WSAGetLastError()) +
                                   "). Port already in use?");
            }
         }
         else
         {
            osc_write_log_error("Platform handler startup", "Failed to set listener socket options");
         }
      }
      else
      {
         osc_write_log_error("Platform handler startup", "Failed to initialize listener socket");
      }

      WSACleanup();
   }
   else
   {
      osc_write_log_error("Platform handler startup", "WSAStartup failed");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set flag to shut down

   Remember to shut down running operation.
   The shutdown will not be performed immediately, to make sure ongoing TCP communication will be cleanly shut down.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaServerPlatform::m_Shutdown()
{
   mhq_ShutdownRequested = true;
}
