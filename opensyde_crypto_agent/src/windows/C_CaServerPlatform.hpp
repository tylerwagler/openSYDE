//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class Windows driver implementation

   Provides Windows specific implementations for operating the server.
   Either this implementation of C_CaServerPlatform or the Linux one needs to be linked with the application.
   Both implementations use the same class and public interface names so the selection can fully be made
   via makefile configuration with no #ifdefs needed in the code.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CASERVERPLATFORMHPP
#define C_CASERVERPLATFORMHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <winsock2.h>
#include <windows.h>
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
   static C_CaServerPlatform * mhpc_TheInstance; //we only support one instance, so we can use this for the static
                                                 // callback functions
   //lint -e{8080} //using type to match used library interface
   static SOCKET mhx_ListeningSocket;
   static volatile bool mhq_ShutdownRequested; //no mutex needed for simple bool flag

   //lint -e{8080} //using type to match used library interface
   static uint32_t mh_Send(const SOCKET ox_Socket, const uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes);
   //lint -e{8080} //using type to match used library interface
   static uint32_t mh_Receive(const SOCKET ox_Socket, uint8_t * const opu8_Buffer, const uint32_t ou32_NumBytes);

   static DWORD WINAPI mh_ServerThread(LPVOID opv_Arg);
   //lint -e{8080} //using type to match used library interface
   static BOOL WINAPI mh_ConsoleCtrlHandler(const DWORD ox_CtrlType);

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
