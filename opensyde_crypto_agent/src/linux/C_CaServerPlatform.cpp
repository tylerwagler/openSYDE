//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief        openSYDE Crypto Agent server class Linux driver implementation

   For details see documentation in .hpp file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_SclString.hpp"
#include "C_CaServerPlatform.hpp"
#include "C_OscLoggingHandler.hpp"

//A few module-wide PC-lint definitions.
//The macros from the socket API have a few non-compliant issues.
//No quality issues expected as we use them in the intended ways.
//lint -emacro(717 953 970 8001 8002 8020 8047 8058 8080 9093 9012 9113 9177, FD_ZERO)
//lint -emacro(970 1924 9084 9123 9124 9126 9130, FD_SET)
//lint -emacro(970 1924 9123 9126 9130, FD_ISSET)
//lint -emacro(1924, INADDR_ANY)
//lint -emacro(641, SOCK_STREAM)

//Similar for signal API macro:
//lint -emacro(1924 9010, SIG_ERR)

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::osy_crypto_agent;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
int C_CaServerPlatform::mhx_ListeningSocket = -1; //lint !e970 !e8080 //using API type
C_CaServerPlatform * C_CaServerPlatform::mhpc_TheInstance = NULL;
volatile sig_atomic_t C_CaServerPlatform::mhx_ShutdownRequested = 0; //lint !e8080 //using API type

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
uint32_t C_CaServerPlatform::mh_Receive(const int ox_Socket, //lint !e970 !e8080 //using API type
                                        uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes)
{
   //lint -e{9176} //Side-effect of the "char"-based API
   char_t * const pcn_Data = reinterpret_cast<char_t *>(opu8_Buffer);
   uint32_t u32_TotalRead = 0U;
   bool q_Cancel = false;

   while ((u32_TotalRead < ou32_NumBytes) && (q_Cancel == false))
   {
      const ssize_t x_ReadNow = //lint !e8080 //using API type
                                recv(ox_Socket, &pcn_Data[u32_TotalRead],
                                     static_cast<size_t>(ou32_NumBytes) - u32_TotalRead, 0);
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
   \param[in]    opu8_Buffer    transmission buffer
   \param[in]    ou32_NumBytes  number of bytes to send

   \retval   total number of sent bytes
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_CaServerPlatform::mh_Send(const int ox_Socket, //lint !e970 !e8080 //using API type
                                     const uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes)
{
   //lint -e{9176} //Side-effect of the "char"-based API
   const char_t * const pcn_Data = reinterpret_cast<const char_t *>(opu8_Buffer);
   bool q_Cancel = false;
   uint32_t u32_TotalSent = 0;

   while ((u32_TotalSent < ou32_NumBytes) && (q_Cancel == false))
   {
      const ssize_t x_SentNow = //lint !e8080 //using API type
                                send(ox_Socket, &pcn_Data[u32_TotalSent],
                                     static_cast<size_t>(ou32_NumBytes) - u32_TotalSent, 0);
      if (x_SentNow <= 0)
      {
         // error
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
   0    shutdown requested via TCP or signal handler without any unexpected errors
   1    shutdown requested; at least one unexpected error happened; see log for details
*/
//----------------------------------------------------------------------------------------------------------------------
void * C_CaServerPlatform::mh_ServerThread(void * const opv_Arg) //lint !e818 //API defined by library
{
   (void)opv_Arg;
   uint32_t u32_ThreadResult = 0U;

   while (mhx_ShutdownRequested == 0)
   {
      int x_Result; //lint !e970 !e8080 //using API type
      timeval c_TimeoutValue;
      fd_set c_Rfds;
      FD_ZERO(&c_Rfds);
      FD_SET(mhx_ListeningSocket, &c_Rfds);

      c_TimeoutValue.tv_sec = 1; // 1-second timeout; poll for shutdown request in this interval
      c_TimeoutValue.tv_usec = 0;

      x_Result = select(mhx_ListeningSocket + 1, &c_Rfds, NULL, NULL, &c_TimeoutValue);
      if ((x_Result > 0) && (FD_ISSET(mhx_ListeningSocket, &c_Rfds) != 0))
      {
         sockaddr_in c_ClientAddress;
         socklen_t x_ClientAddressSize = //lint !e8080 //using API type
                                         static_cast<socklen_t>(sizeof(c_ClientAddress));
         //lint -e{9176}  Side-effect of the API. Match is guaranteed by the API.
         const int x_ClientSocket = //lint !e970 !e8080 //using API type
                                    accept(mhx_ListeningSocket, reinterpret_cast<sockaddr *>(&c_ClientAddress),
                                           &x_ClientAddressSize);
         if (x_ClientSocket < 0)
         {
            // This should not happen in normal operation. Log and try to continue accepting new connections.
            osc_write_log_warning("TCP handler", "Could not accept incoming connection. Retrying.");
            u32_ThreadResult = 1U;
         }
         else
         {
            timeval c_ReceiveTimeout;
            C_SclString c_LogText;
            c_LogText.PrintFormatted("Incoming TCP connection from %u.%u.%u.%u.",
                                     static_cast<uint32_t>(c_ClientAddress.sin_addr.s_addr) & 0xFFU,
                                     (static_cast<uint32_t>(c_ClientAddress.sin_addr.s_addr) >> 8U) & 0xFFU,
                                     (static_cast<uint32_t>(c_ClientAddress.sin_addr.s_addr) >> 16U) & 0xFFU,
                                     (static_cast<uint32_t>(c_ClientAddress.sin_addr.s_addr) >> 24U) & 0xFFU);
            osc_write_log_info("TCP handler", c_LogText.c_str());

            // set socket options to not block forever if the client does not provide enough data:
            c_ReceiveTimeout.tv_sec = 1; // 1 second timeout
            c_ReceiveTimeout.tv_usec = 0;
            if (setsockopt(x_ClientSocket, SOL_SOCKET, SO_RCVTIMEO, &c_ReceiveTimeout,
                           static_cast<socklen_t>(sizeof(c_ReceiveTimeout))) != 0)
            {
               osc_write_log_warning("TCP handler", "Failed to set socket timeout. Unexpected blocking might happen.");
               u32_ThreadResult = 1U;
            }

            //TCP data can arrive in multiple chunks as it is stream based.
            //So a service request can arrive in multiple calls of "recv".
            //We use a simple size header to communicate the expected size of a service.
            //We first check for this uint32 header value and then try to read the rest of the service:

            // Read header (length prefix)
            uint8_t au8_ServiceSize[4];

            if (mh_Receive(x_ClientSocket, &au8_ServiceSize[0], 4U) != 4U)
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
                           u32_ThreadResult = 1U;
                        }
                        else
                        {
                           osc_write_log_info("TCP handler", "Service response sent.");
                        }

                        // tell TCP stack we will close that socket again so it will send out the response in any case:
                        (void)shutdown(x_ClientSocket, SHUT_WR);
                     }
                  }
               }
            }

            (void)close(x_ClientSocket);
         }
      }
   }

   if (mhx_ListeningSocket >= 0)
   {
      (void)close(mhx_ListeningSocket);
      mhx_ListeningSocket = -1;
   }

   return reinterpret_cast<void *>(static_cast<intptr_t>(u32_ThreadResult));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handler for process termination signals

   Called by system as signal handler. e.g. ctrl+c in console.
   Will request the listener thread to shut down.

   \param[in]    os32_Signal   Type of signal detected
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaServerPlatform::mh_SignalHandler(const int32_t os32_Signal)
{
   switch (os32_Signal)
   {
   case SIGINT:
   case SIGTERM:
      //lint -e{2761} //no signal safety issues expected here
      osc_write_log_info("Platform handler", "Application termination requested");
      C_CaServerPlatform::mhx_ShutdownRequested = 1;
      break;
   default:
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Attach to parent console

   For Linux there is nothing to do here.
   Whether there is a console or not is determined by how the application is started, we do not have to care about it.

   If the crypto agent is started from a terminal, the output will automatically go to the terminal.
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{9175} intentionally no functionality in this implementation
void C_CaServerPlatform::h_AttachToParentConsole()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set up and execute listener thread

   Technically as long as the application's only job is to react to incoming TCP requests,
    we could also run the listener in the main thread and not use a separate thread at all.
   Using this approach will improve flexibility for future expansion.

   Steps:
   * initialize handler to handle process termination and allow graceful shutdown
   * initialize TCP listener
   * set up and start listener thread
   * wait for termination by close request or local application shutdown

   \param[in]    ou16_ServerPort     TCP server port to listen on
   \param[in]    orau8_BindAddress   bind address for server

   \retval   C_NO_ERR   All set up, thread was run and was terminated again.
   \retval   C_WARN     All set up, thread was run and was terminated again. Thread reported unexpected issues.
   \retval   C_COM      Could not initialize TCP server
   \retval   C_NOACT    Could not initialize listener thread
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServerPlatform::m_ExecuteListener(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4])
{
   int32_t s32_Result = C_COM;

   mhx_ShutdownRequested = 0;
   mhpc_TheInstance = this;

   //lint -e{586} //signal handling API is the proven approach for Linux
   if (signal(SIGINT, &mh_SignalHandler) == SIG_ERR)
   {
      //just issue a warning, this is not critical
      osc_write_log_warning("Platform handler startup", "Could not initialize SIGINT handler");
   }
   //lint -e{586} //signal handling API is the proven approach for Linux
   if (signal(SIGTERM, &mh_SignalHandler) == SIG_ERR)
   {
      //just issue a warning, this is not critical
      osc_write_log_warning("Platform handler startup", "Could not initialize SIGTERM handler");
   }

   mhx_ListeningSocket = socket(AF_INET, SOCK_STREAM, 0);

   if (mhx_ListeningSocket >= 0)
   {
      const int32_t s32_OPTION = 1;
      if (setsockopt(mhx_ListeningSocket, SOL_SOCKET, SO_REUSEADDR, &s32_OPTION,
                     static_cast<socklen_t>(sizeof(s32_OPTION))) == 0)
      {
         sockaddr_in c_Address;
         c_Address.sin_family = AF_INET;
         c_Address.sin_addr.s_addr = (static_cast<uint32_t>(orau8_BindAddress[3]) << 24U) +
                                     (static_cast<uint32_t>(orau8_BindAddress[2]) << 16U) +
                                     (static_cast<uint32_t>(orau8_BindAddress[1]) << 8U) +
                                     (static_cast<uint32_t>(orau8_BindAddress[0]));
         c_Address.sin_port = htons(ou16_ServerPort);
         (void)memset(&c_Address.sin_zero[0], 0, sizeof(c_Address.sin_zero));

         //lint -e{9176}  //Side-effect of the API. Using correct type.
         if (bind(mhx_ListeningSocket, reinterpret_cast<sockaddr *>(&c_Address),
                  static_cast<socklen_t>(sizeof(c_Address))) == 0)
         {
            if (listen(mhx_ListeningSocket, 16) == 0)
            {
               pthread_t x_Thread = 0U; //lint !e8080 //using API type
               if (pthread_create(&x_Thread, NULL, &mh_ServerThread, NULL) != 0)
               {
                  osc_write_log_error("Platform handler startup", "Failed to create listener thread");
                  s32_Result = C_NOACT;
               }
               else
               {
                  void * pv_ThreadResult = NULL;
                  osc_write_log_info("Platform handler startup", "Service listener up and running");

                  if (pthread_join(x_Thread, &pv_ThreadResult) != 0)
                  {
                     s32_Result = C_WARN;
                  }
                  else
                  {
                     //lint -e{9091}  //Side-effect of the API. Match is guaranteed by the API.
                     const intptr_t x_ThreadResult = //lint !e8080 //using API type
                                                     reinterpret_cast<intptr_t>(pv_ThreadResult);
                     if (x_ThreadResult != 0)
                     {
                        s32_Result = C_WARN;
                     }
                     else
                     {
                        s32_Result = C_NO_ERR;
                     }
                  }
               }
            }
            else
            {
               osc_write_log_error("Platform handler startup", "Failed to listen");
            }
         }
         else
         {
            const int32_t s32_Errno = errno;
            osc_write_log_error("Platform handler startup",
                                "Failed to bind (errno " +
                                C_SclString::IntToStr(s32_Errno) + ": " +
                                strerror(s32_Errno) + "). Port already in use?");
         }
      }
      else
      {
         osc_write_log_error("Platform handler startup", "Failed to set listener socket options");
      }
   }
   else
   {
      C_SclString c_LogText;
      c_LogText.PrintFormatted("Failed to initialize listener socket (errno: %d)", errno);
      osc_write_log_error("Platform handler startup", c_LogText.c_str());
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
   mhx_ShutdownRequested = 1;
}
