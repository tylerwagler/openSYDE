//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol IP driver for Windows

   \class       stw::opensyde_core::C_OscIpDispatcherWinSock
   \brief       openSYDE protocol IP driver for Windows

   Implements target specific IP routines.
   Here: for MS Windows using WinSock

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCIPDISPATCHERWINSOCK_HPP
#define C_OSCIPDISPATCHERWINSOCK_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscIpDispatcher.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QMap>
#include <QRecursiveMutex>
#include <QString>
#include <vector>
#include <winsock.h>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscIpDispatcherWinSock : public C_OscIpDispatcher {
private:
  class C_TcpConnection {
  public:
    C_TcpConnection(void);

    // lint -save -e8080 //using type expected by the library for compatibility
    SOCKET x_Socket; ///< client socket
    // lint -restore
    uint8_t au8_IpAddress[4]; ///< server's IP (remembered for reconnecting)
  };

  class C_BufferIdentifier {
  public:
    C_BufferIdentifier(const uint8_t ou8_ClientBusIdentifier,
                       const uint8_t ou8_ClientNodeIdentifier,
                       const uint8_t ou8_ServerBusIdentifier,
                       const uint8_t ou8_ServerNodeIdentifier)
        : u8_ClientBusIdentifier(ou8_ClientBusIdentifier),
          u8_ClientNodeIdentifier(ou8_ClientNodeIdentifier),
          u8_ServerBusIdentifier(ou8_ServerBusIdentifier),
          u8_ServerNodeIdentifier(ou8_ServerNodeIdentifier) {}

    bool operator<(const C_BufferIdentifier &orc_Cmp) const;

    const uint8_t u8_ClientBusIdentifier;
    const uint8_t u8_ClientNodeIdentifier;
    const uint8_t u8_ServerBusIdentifier;
    const uint8_t u8_ServerNodeIdentifier;
  };

  QList<C_TcpConnection> mc_SocketsTcp; ///< one per connection

  QList<SOCKET> mc_SocketsUdpClient; ///< one socket per local interface
                                     ///< (for sending broadcasts)
  QList<SOCKET> mc_SocketsUdpServer; ///< one socket per local interface
                                     ///< (for receiving responses)

  QList<uint32_t> mc_LocalInterfaceIps;   ///< IPs of local interfaces
  QStringList mc_PreferredInterfaceNames; ///< Optional preferred interfaces

  static QMap<C_BufferIdentifier, QList<QByteArray>>
      mhc_TcpBuffer; ///< dispatcher buffer
  static QRecursiveMutex mhc_LockBuffer;

  int32_t m_GetAllInstalledInterfaceIps(void);
  int32_t m_ConnectTcp(C_TcpConnection &orc_Connection) const;
  // lint -e{8080} //using type to match library interface
  int32_t m_ConfigureUdpSocket(const bool oq_ServerPort,
                               const uint32_t ou32_IpToBindTo,
                               SOCKET &orx_Socket) const;

  static QString mh_IpToText(const uint8_t (&orau8_Ip)[4]);

public:
  C_OscIpDispatcherWinSock(void);
  virtual ~C_OscIpDispatcherWinSock(void);

  virtual int32_t InitTcp(const uint8_t (&orau8_Ip)[4], uint32_t &oru32_Handle);
  virtual int32_t InitUdp(void);
  virtual int32_t IsTcpConnected(const uint32_t ou32_Handle);
  virtual int32_t ReConnectTcp(const uint32_t ou32_Handle);

  virtual int32_t CloseTcp(const uint32_t ou32_Handle);
  virtual int32_t CloseUdp(void);
  virtual int32_t SendTcp(const uint32_t ou32_Handle,
                          const QByteArray &orc_Data);
  virtual int32_t ReadTcp(const uint32_t ou32_Handle, QByteArray &orc_Data);
  virtual int32_t ReadTcp(const uint32_t ou32_Handle,
                          const uint8_t ou8_ClientBusIdentifier,
                          const uint8_t ou8_ClientNodeIdentifier,
                          const uint8_t ou8_ServerBusIdentifier,
                          const uint8_t ou8_ServerNodeIdentifier,
                          QByteArray &orc_Data);
  virtual int32_t ReadTcpBuffer(const uint8_t ou8_ClientBusIdentifier,
                                const uint8_t ou8_ClientNodeIdentifier,
                                const uint8_t ou8_ServerBusIdentifier,
                                const uint8_t ou8_ServerNodeIdentifier,
                                QByteArray &orc_Data);
  virtual int32_t SendUdp(const QByteArray &orc_Data);
  virtual int32_t ReadUdp(QByteArray &orc_Data, uint8_t (&orau8_Ip)[4]);

  void LoadConfigFile(const QString &orc_FileLocation);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
