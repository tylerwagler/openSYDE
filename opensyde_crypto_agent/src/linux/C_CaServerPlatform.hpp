//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class Linux driver implementation

   Provides Linux specific implementations for operating the server.
   Either this implementation of C_CaServerPlatform or the Windows one needs to be linked with the application.
   Both implementations use the same class and public interface names so the selection can fully be made
   via makefile configuration with no #ifdefs needed in the code.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CASERVERPLATFORMHPP
#define C_CASERVERPLATFORMHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <pthread.h>
#include <csignal>
#include <sys/socket.h>
#include <cstring>
#include <vector>

#include "stwtypes.hpp"
#include "C_CaServer.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw::osy_crypto_agent
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CaServerPlatform :
   public C_CaServer
{
private:
   static C_CaServerPlatform * mhpc_TheInstance;       //we only support one instance, so we can use this for the static
                                                       // callback functions
   static int mhx_ListeningSocket;                     //lint !e970 !e8080 //using API type
   static volatile sig_atomic_t mhx_ShutdownRequested; //lint !e970 !e8080 //using API type

   static uint32_t mh_Send(const int ox_Socket, //lint !e970 !e8080 //using API type
                           const uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes);
   static uint32_t mh_Receive(const int ox_Socket, //lint !e970 !e8080 //using API type
                              uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes);

   static void * mh_ServerThread(void * const opv_Arg);
   static void mh_SignalHandler(const int32_t os32_Signal);

protected:
   virtual int32_t m_ExecuteListener(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4]);
   virtual void m_Shutdown();

public:
   static void h_AttachToParentConsole();

   C_CaServerPlatform();
   virtual ~C_CaServerPlatform();
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}

#endif
