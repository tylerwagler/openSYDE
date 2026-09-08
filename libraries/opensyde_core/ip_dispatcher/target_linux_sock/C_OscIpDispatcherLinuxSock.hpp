//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol IP driver for Linux

   \class       stw::opensyde_core::C_OSCIpDispatcherLinuxSock
   \brief       openSYDE protocol IP driver for Linux

   Implements target specific IP routines.
   Here: for Linux using BSD Socket API
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCIPDISPATCHERLINUXSOCK_HPP
#define C_OSCIPDISPATCHERLINUXSOCK_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <mutex>
#include <vector>
#include <map>
#include <list>
#include <system_error>
#include "stwtypes.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscIpDispatcher.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscIpDispatcherLinuxSock :
   public C_OscIpDispatcher
{
private:
   class C_TcpConnection
   {
   public:
      int32_t s32_Socket;       ///< client socket
      uint8_t au8_IpAddress[4]; ///< server's IP (remembered for reconnecting)
   };

   class C_BufferIdentifier
   {
   public:
      C_BufferIdentifier(const uint8_t ou8_ClientBusIdentifier, const uint8_t ou8_ClientNodeIdentifier,
                         const uint8_t ou8_ServerBusIdentifier, const uint8_t ou8_ServerNodeIdentifier) :
         u8_ClientBusIdentifier(ou8_ClientBusIdentifier),
         u8_ClientNodeIdentifier(ou8_ClientNodeIdentifier),
         u8_ServerBusIdentifier(ou8_ServerBusIdentifier),
         u8_ServerNodeIdentifier(ou8_ServerNodeIdentifier)
      {
      }

      bool operator < (const C_BufferIdentifier & orc_Cmp) const;

      const uint8_t u8_ClientBusIdentifier;
      const uint8_t u8_ClientNodeIdentifier;
      const uint8_t u8_ServerBusIdentifier;
      const uint8_t u8_ServerNodeIdentifier;
   };

   std::vector<C_TcpConnection> mc_SocketsTcp; ///< one per connection

   std::vector<int32_t> mc_SocketsUdpClient; ///< one socket per local interface (for sending broadcasts)
   int32_t ms32_SocketUdpServer;             ///< one socket for all interfaces (for receiving responses)

   std::vector<uint32_t> mc_LocalInterfaceIps; ///< IPs of local interfaces

   static std::map<C_BufferIdentifier, std::list<std::vector<uint8_t> > > mhc_TcpBuffer; ///< dispatcher buffer
   static std::mutex mhc_LockBuffer;

   std::error_code m_GetAllInstalledInterfaceIps(void);
   std::error_code m_ConnectTcp(C_TcpConnection & orc_Connection) const;
   //ors32_Socket returns a socket descriptor, not an error code -> stays on int32_t
   std::error_code m_ConfigureUdpSocket(const bool oq_ServerPort, const uint32_t ou32_IpToBindTo,
                                        int32_t & ors32_Socket) const;

   static std::string mh_IpToText(const uint8_t (&orau8_Ip)[4]);

public:
   C_OscIpDispatcherLinuxSock(void);
   virtual ~C_OscIpDispatcherLinuxSock(void);

   virtual std::error_code InitTcp(const uint8_t (&orau8_Ip)[4], uint32_t & oru32_Handle);
   virtual std::error_code InitUdp(void);
   virtual std::error_code IsTcpConnected(const uint32_t ou32_Handle);
   virtual std::error_code ReConnectTcp(const uint32_t ou32_Handle);

   virtual std::error_code CloseTcp(const uint32_t ou32_Handle);
   virtual std::error_code CloseUdp(void);
   virtual std::error_code SendTcp(const uint32_t ou32_Handle, const std::vector<uint8_t> & orc_Data);
   virtual std::error_code ReadTcp(const uint32_t ou32_Handle, std::vector<uint8_t> & orc_Data);
   virtual std::error_code ReadTcp(const uint32_t ou32_Handle, const uint8_t ou8_ClientBusIdentifier,
                                   const uint8_t ou8_ClientNodeIdentifier,
                                   const uint8_t ou8_ServerBusIdentifier,
                                   const uint8_t ou8_ServerNodeIdentifier, std::vector<uint8_t> & orc_Data);
   virtual std::error_code ReadTcpBuffer(const uint8_t ou8_ClientBusIdentifier,
                                         const uint8_t ou8_ClientNodeIdentifier,
                                         const uint8_t ou8_ServerBusIdentifier,
                                         const uint8_t ou8_ServerNodeIdentifier,
                                         std::vector<uint8_t> & orc_Data);
   virtual std::error_code SendUdp(const std::vector<uint8_t> & orc_Data);
   virtual std::error_code ReadUdp(std::vector<uint8_t> &orc_Data, uint8_t(&orau8_Ip)[4]);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
