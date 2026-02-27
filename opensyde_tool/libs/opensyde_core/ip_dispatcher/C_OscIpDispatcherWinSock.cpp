//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol IP driver (Qt cross-platform implementation)

   For details cf. documentation in .hpp file.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscIpDispatcherWinSock.hpp"
#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <QFileInfo>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QSettings>
#include <QString>
#include <cstring>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
QMap<C_OscIpDispatcherWinSock::C_BufferIdentifier, QList<QByteArray> >
   C_OscIpDispatcherWinSock::mhc_TcpBuffer;
QRecursiveMutex C_OscIpDispatcherWinSock::mhc_LockBuffer;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize elements
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherWinSock::C_TcpConnection::C_TcpConnection(void) :
   pc_Socket(NULL)
{
   (void)std::memset(&au8_IpAddress[0], 0U, 4U);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Destructor

   Clean up socket if still open
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherWinSock::C_TcpConnection::~C_TcpConnection(void)
{
   // Note: socket cleanup is handled by CloseTcp, not here,
   // because QList copies may share the pointer
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current smaller than orc_Cmp

   \param[in] orc_Cmp Compared instance

   \return
   Current smaller than orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscIpDispatcherWinSock::C_BufferIdentifier::operator<(
   const C_OscIpDispatcherWinSock::C_BufferIdentifier & orc_Cmp) const
{
   bool q_Return;

   if (this->u8_ServerBusIdentifier < orc_Cmp.u8_ServerBusIdentifier)
   {
      q_Return = true;
   }
   else if (this->u8_ServerBusIdentifier == orc_Cmp.u8_ServerBusIdentifier)
   {
      if (this->u8_ServerNodeIdentifier < orc_Cmp.u8_ServerNodeIdentifier)
      {
         q_Return = true;
      }
      else if (this->u8_ServerNodeIdentifier == orc_Cmp.u8_ServerNodeIdentifier)
      {
         if (this->u8_ClientBusIdentifier < orc_Cmp.u8_ClientBusIdentifier)
         {
            q_Return = true;
         }
         else if (this->u8_ClientBusIdentifier == orc_Cmp.u8_ClientBusIdentifier)
         {
            if (this->u8_ClientNodeIdentifier < orc_Cmp.u8_ClientNodeIdentifier)
            {
               q_Return = true;
            }
            else
            {
               q_Return = false;
            }
         }
         else
         {
            q_Return = false;
         }
      }
      else
      {
         q_Return = false;
      }
   }
   else
   {
      q_Return = false;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set up class

   Initialize class elements
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherWinSock::C_OscIpDispatcherWinSock(void) :
   C_OscIpDispatcher()
{
   this->mc_PreferredInterfaceNames.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clean up class

   Release allocated resources
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherWinSock::~C_OscIpDispatcherWinSock(void)
{
   // make sure to release resources in case the user forgot to
   for (int32_t s32_Index = 0; s32_Index < this->mc_SocketsTcp.size(); s32_Index++)
   {
      (void)this->C_OscIpDispatcherWinSock::CloseTcp(static_cast<uint32_t>(s32_Index));
   }
   (void)this->C_OscIpDispatcherWinSock::CloseUdp();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize TCP communication

   Jobs to perform:
   - create one TCP connection entry (socket not yet connected)

   Strategy for "book keeping":
   * new connections will always be added at the end
   * closing connection closes the handle but does not reduce the list of handles
   * so we don't have any trouble with existing handles getting invalid

   \param[in]     orau8_Ip      IP address of server to connect to
   \param[out]    oru32_Handle  handle to new TCP connection

   \return
   C_NO_ERR   Connection created
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::InitTcp(const uint8_t (& orau8_Ip)[4], uint32_t & oru32_Handle)
{
   C_TcpConnection c_NewConnection;

   (void)memcpy(&c_NewConnection.au8_IpAddress[0], &orau8_Ip[0], 4U);
   c_NewConnection.pc_Socket = NULL;

   this->mc_SocketsTcp.push_back(c_NewConnection);
   oru32_Handle = static_cast<uint32_t>(mc_SocketsTcp.size() - 1);

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: get IP of all local interfaces

   Gets list of IP addresses of all local IP interfaces.
   Does not report "localhost".
   Results will be placed in mc_LocalInterfaceIps

   If mc_PreferredInterfaceNames is not empty, only IPs of adapters
   with matching names will be used.

   \return
   C_NO_ERR     IPs listed
   C_NOACT      could not get IP information
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::m_GetAllInstalledInterfaceIps(void)
{
   int32_t s32_Return = C_NOACT;

   mc_LocalInterfaceIps.clear();

   const QList<QNetworkInterface> c_Interfaces = QNetworkInterface::allInterfaces();

   for (const QNetworkInterface & rc_Interface : c_Interfaces)
   {
      // Only active interfaces are relevant
      if ((rc_Interface.flags() & QNetworkInterface::IsUp) == 0)
      {
         continue;
      }
      if ((rc_Interface.flags() & QNetworkInterface::IsRunning) == 0)
      {
         continue;
      }

      // Check preferred interface filter
      if ((this->mc_PreferredInterfaceNames.count() > 0) &&
          (this->mc_PreferredInterfaceNames.indexOf(rc_Interface.humanReadableName()) == -1))
      {
         continue;
      }

      const QList<QNetworkAddressEntry> c_Entries = rc_Interface.addressEntries();

      for (const QNetworkAddressEntry & rc_Entry : c_Entries)
      {
         const QHostAddress c_Ip = rc_Entry.ip();

         // Only IPv4
         if (c_Ip.protocol() != QAbstractSocket::IPv4Protocol)
         {
            continue;
         }

         // Ignore localhost
         if (c_Ip == QHostAddress::LocalHost)
         {
            continue;
         }

         mc_LocalInterfaceIps.push_back(c_Ip);

         const QString c_Info = "Local IP interface used with IP: " +
                                c_Ip.toString() + ", name of adapter: \"" +
                                rc_Interface.humanReadableName() + "\"";
         osc_write_log_info("openSYDE IP-TP", c_Info);

         s32_Return = C_NO_ERR;
      }
   }

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error("openSYDE IP-TP",
                          "UDP init failed. Could not get IP addresses of local interfaces.");
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: connect TCP socket to server

   Jobs to perform:
   - create QTcpSocket and connect to port 13400 of the specified server node

   Function shall return when connected or after timeout.

   \param[in,out]  orc_Connection     TCP connection status and configuration

   \return
   C_NO_ERR   connected
   C_BUSY     connection failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::m_ConnectTcp(C_TcpConnection & orc_Connection) const
{
   bool q_Error = false;

   // Create TCP socket
   orc_Connection.pc_Socket = new QTcpSocket();
   if (orc_Connection.pc_Socket == NULL)
   {
      osc_write_log_error("openSYDE IP-TP",
                          "Error creating TCP socket. IP-Address: " +
                             mh_IpToText(orc_Connection.au8_IpAddress));
      q_Error = true;
   }
   else
   {
      osc_write_log_info("openSYDE IP-TP",
                         "TCP socket OK IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress));
   }

   if (q_Error == false)
   {
      const QHostAddress c_Host(mh_IpToText(orc_Connection.au8_IpAddress));

      orc_Connection.pc_Socket->connectToHost(c_Host, mhu16_UDP_TCP_PORT);

      // Wait for connection with configured timeout
      const uint32_t u32_TimeoutMs = mu32_ConnectionTimeoutSeconds * 1000U;
      if (orc_Connection.pc_Socket->waitForConnected(static_cast<int>(u32_TimeoutMs)))
      {
         osc_write_log_info("openSYDE IP-TP",
                            "TCP connect OK. IP-Address: " +
                               mh_IpToText(orc_Connection.au8_IpAddress) +
                               " on client port: " +
                               QString::number(orc_Connection.pc_Socket->localPort()));
      }
      else
      {
         // Timeout or error - but not necessarily fatal
         // The connection may be established later via ReConnectTcp
         const QAbstractSocket::SocketError e_Error = orc_Connection.pc_Socket->error();
         if (e_Error == QAbstractSocket::SocketTimeoutError)
         {
            osc_write_log_warning("openSYDE IP-TP",
                                  "TCP connect timeout. IP-Address: " +
                                     mh_IpToText(orc_Connection.au8_IpAddress) +
                                     " No connection within timeout");
            // Not an error - connection can be established later
         }
         else
         {
            osc_write_log_error("openSYDE IP-TP",
                                "TCP connect failed. IP-Address: " +
                                   mh_IpToText(orc_Connection.au8_IpAddress) +
                                   " Error: " + orc_Connection.pc_Socket->errorString());
            q_Error = true;
         }
      }
   }

   return (q_Error == true) ? C_BUSY : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: configure UDP socket

   \param[in]   oq_ServerPort     true: set up server socket (bind to port 13400)
                                  false: set up client socket (dynamic port, broadcast enabled)
   \param[in]   orc_IpToBindTo    IP address to bind the socket to
   \param[out]  orpc_Socket       Resulting socket pointer

   \return
   C_NO_ERR   socket set up
   C_NOACT    could not set up socket
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::m_ConfigureUdpSocket(const bool oq_ServerPort,
                                                        const QHostAddress & orc_IpToBindTo,
                                                        QUdpSocket * & orpc_Socket) const
{
   bool q_Error = false;

   // Create UDP socket
   orpc_Socket = new QUdpSocket();
   if (orpc_Socket == NULL)
   {
      osc_write_log_error("openSYDE IP-TP", "Error creating UDP socket");
      q_Error = true;
   }
   else
   {
      osc_write_log_info("openSYDE IP-TP", "UDP socket OK");
   }

   if (q_Error == false)
   {
      bool q_Bound;

      if (oq_ServerPort == true)
      {
         // Server socket: bind to specific IP and port 13400
         // ReuseAddressHint allows multiple sockets to bind to the same address/port
         q_Bound = orpc_Socket->bind(orc_IpToBindTo, mhu16_UDP_TCP_PORT,
                                     QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
      }
      else
      {
         // Client socket: bind to specific IP, dynamic port
         q_Bound = orpc_Socket->bind(orc_IpToBindTo, 0);
      }

      if (q_Bound == false)
      {
         osc_write_log_error("openSYDE IP-TP",
                             "UDP bind() failed. Error: " + orpc_Socket->errorString());
         delete orpc_Socket;
         orpc_Socket = NULL;
         q_Error = true;
      }
      else
      {
         osc_write_log_info("openSYDE IP-TP", "UDP bind() OK");
      }
   }

   return (q_Error == true) ? C_NOACT : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: compose textual representation of IP

   Format: xxx.xxx.xxx.xxx

   \param[in]  orau8_Ip   IP address bytes

   \return
   text representation of IP
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscIpDispatcherWinSock::mh_IpToText(const uint8_t (& orau8_Ip)[4])
{
   return QString::asprintf("%d.%d.%d.%d", orau8_Ip[0], orau8_Ip[1], orau8_Ip[2], orau8_Ip[3]);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize UDP communication

   Jobs to perform:
   * create non-blocking UDP "server" sockets on each local interface
   ** bind them to the IP of the interface and port 13400
   * create non-blocking UDP "client" sockets on each local interface
   ** bind them to the IP of the interface but don't assign a fixed port

   \return
   C_NO_ERR   connected
   C_NOACT    connection failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::InitUdp(void)
{
   bool q_Error = false;
   int32_t s32_Return;

   s32_Return = m_GetAllInstalledInterfaceIps();
   if (s32_Return != C_NO_ERR)
   {
      // we could not find out which interfaces we have: try "any"
      osc_write_log_warning("openSYDE IP-TP",
                            "UDP init: could not find IP(s) of local interfaces. Trying \"any\" ...");
      mc_LocalInterfaceIps.clear();
      mc_LocalInterfaceIps.push_back(QHostAddress::AnyIPv4);
   }

   mc_SocketsUdpClient.resize(mc_LocalInterfaceIps.size());
   mc_SocketsUdpServer.resize(mc_LocalInterfaceIps.size());

   for (int32_t s32_Interface = 0; s32_Interface < mc_LocalInterfaceIps.size(); s32_Interface++)
   {
      mc_SocketsUdpClient[s32_Interface] = NULL;
      mc_SocketsUdpServer[s32_Interface] = NULL;
   }

   for (int32_t s32_Interface = 0; s32_Interface < mc_LocalInterfaceIps.size(); s32_Interface++)
   {
      s32_Return = m_ConfigureUdpSocket(false, mc_LocalInterfaceIps[s32_Interface],
                                        mc_SocketsUdpClient[s32_Interface]);
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = m_ConfigureUdpSocket(true, mc_LocalInterfaceIps[s32_Interface],
                                           mc_SocketsUdpServer[s32_Interface]);
      }
      if (s32_Return != C_NO_ERR)
      {
         q_Error = true;
         break;
      }
   }

   // close sockets in case of error:
   if (q_Error == true)
   {
      this->CloseUdp();
   }

   return (q_Error == true) ? C_NOACT : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks the connection of the TCP socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   is connected
   C_NOACT    is not connected
   C_RANGE    invalid handle
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::IsTcpConnected(const uint32_t ou32_Handle)
{
   int32_t s32_Return = C_NOACT;

   if (ou32_Handle >= static_cast<uint32_t>(this->mc_SocketsTcp.size()))
   {
      osc_write_log_error("openSYDE IP-TP", "IsTcpConnected called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else if (this->mc_SocketsTcp[ou32_Handle].pc_Socket == NULL)
   {
      // Socket not opened yet
      s32_Return = C_NOACT;
   }
   else
   {
      const QAbstractSocket::SocketState e_State =
         this->mc_SocketsTcp[ou32_Handle].pc_Socket->state();

      if (e_State == QAbstractSocket::ConnectedState)
      {
         s32_Return = C_NO_ERR;
      }
      else
      {
         s32_Return = C_NOACT;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reconnect TCP socket

   If there is still an active connection the function shall close it before reconnecting.
   Function shall return when connected or after timeout.

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   reconnected
   C_BUSY     connection failed
   C_RANGE    invalid handle
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::ReConnectTcp(const uint32_t ou32_Handle)
{
   int32_t s32_Return;

   if (ou32_Handle >= static_cast<uint32_t>(this->mc_SocketsTcp.size()))
   {
      osc_write_log_error("openSYDE IP-TP", "ReConnectTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      // still connected?
      if (this->mc_SocketsTcp[ou32_Handle].pc_Socket != NULL)
      {
         // disconnect first
         this->mc_SocketsTcp[ou32_Handle].pc_Socket->abort();
         delete this->mc_SocketsTcp[ou32_Handle].pc_Socket;
         this->mc_SocketsTcp[ou32_Handle].pc_Socket = NULL;
      }
      // connect
      s32_Return = this->m_ConnectTcp(this->mc_SocketsTcp[ou32_Handle]);
      if (s32_Return != C_NO_ERR)
      {
         delete this->mc_SocketsTcp[ou32_Handle].pc_Socket;
         this->mc_SocketsTcp[ou32_Handle].pc_Socket = NULL;
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close TCP communication

   Jobs to perform:
   - close opened TCP client socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   disconnected
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::CloseTcp(const uint32_t ou32_Handle)
{
   int32_t s32_Return;

   if ((ou32_Handle >= static_cast<uint32_t>(this->mc_SocketsTcp.size())) ||
       (this->mc_SocketsTcp[ou32_Handle].pc_Socket == NULL))
   {
      osc_write_log_error("openSYDE IP-TP", "CloseTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      const uint16_t u16_LocalPort = this->mc_SocketsTcp[ou32_Handle].pc_Socket->localPort();

      this->mc_SocketsTcp[ou32_Handle].pc_Socket->disconnectFromHost();

      // Wait briefly for graceful disconnect if still connected
      if (this->mc_SocketsTcp[ou32_Handle].pc_Socket->state() != QAbstractSocket::UnconnectedState)
      {
         this->mc_SocketsTcp[ou32_Handle].pc_Socket->waitForDisconnected(500);
      }

      delete this->mc_SocketsTcp[ou32_Handle].pc_Socket;
      this->mc_SocketsTcp[ou32_Handle].pc_Socket = NULL;

      osc_write_log_info("openSYDE IP-TP",
                         "TCP close OK. IP-Address: " +
                            mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress) +
                            " on client port: " + QString::number(u16_LocalPort));

      s32_Return = C_NO_ERR;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close UDP communication

   Jobs to perform:
   - close opened UDP sockets

   \return
   C_NO_ERR   disconnected
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::CloseUdp(void)
{
   for (int32_t s32_Interface = 0; s32_Interface < mc_SocketsUdpClient.size(); s32_Interface++)
   {
      if (mc_SocketsUdpClient[s32_Interface] != NULL)
      {
         mc_SocketsUdpClient[s32_Interface]->close();
         delete mc_SocketsUdpClient[s32_Interface];
         mc_SocketsUdpClient[s32_Interface] = NULL;
      }
      if (mc_SocketsUdpServer[s32_Interface] != NULL)
      {
         mc_SocketsUdpServer[s32_Interface]->close();
         delete mc_SocketsUdpServer[s32_Interface];
         mc_SocketsUdpServer[s32_Interface] = NULL;
      }
   }
   mc_SocketsUdpClient.clear();
   mc_SocketsUdpServer.clear();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send package on TCP socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()
   \param[in]   orc_Data      data to send

   \return
   C_NO_ERR   data sent successfully
   C_CONFIG   required socket not initialized
   C_RD_WR    error sending data
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::SendTcp(const uint32_t ou32_Handle, const QByteArray & orc_Data)
{
   int32_t s32_Return;

   if (ou32_Handle >= static_cast<uint32_t>(this->mc_SocketsTcp.size()))
   {
      osc_write_log_error("openSYDE IP-TP", "SendTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      s32_Return = C_NO_ERR;

      if (this->mc_SocketsTcp[ou32_Handle].pc_Socket == NULL)
      {
         osc_write_log_error("openSYDE IP-TP", "SendTcp called with invalid socket.");
         s32_Return = C_CONFIG;
      }
      else
      {
         const qint64 s64_BytesToSend = orc_Data.size();
         const qint64 s64_BytesSent =
            this->mc_SocketsTcp[ou32_Handle].pc_Socket->write(orc_Data);

         if (s64_BytesSent != s64_BytesToSend)
         {
            if (s64_BytesSent == -1)
            {
               const QAbstractSocket::SocketError e_Error =
                  this->mc_SocketsTcp[ou32_Handle].pc_Socket->error();

               osc_write_log_error("openSYDE IP-TP",
                                   "SendTcp: Could not send TCP service. Data lost. Error: " +
                                      this->mc_SocketsTcp[ou32_Handle].pc_Socket->errorString() +
                                      " IP-Address: " +
                                      mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));

               if ((e_Error == QAbstractSocket::RemoteHostClosedError) ||
                   (e_Error == QAbstractSocket::NetworkError) ||
                   (e_Error == QAbstractSocket::ConnectionRefusedError))
               {
                  // Connection dropped
                  osc_write_log_warning("openSYDE IP-TP",
                                        "SendTcp: Connection aborted or reset ... IP-Address: " +
                                           mh_IpToText(
                                              this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
                  this->mc_SocketsTcp[ou32_Handle].pc_Socket->abort();
                  delete this->mc_SocketsTcp[ou32_Handle].pc_Socket;
                  this->mc_SocketsTcp[ou32_Handle].pc_Socket = NULL;
                  m_OnTcpConnectionDropped(ou32_Handle);
               }
            }
            else
            {
               osc_write_log_error("openSYDE IP-TP",
                                   "SendTcp: Could not send all data: tried: " +
                                      QString::number(orc_Data.size()) +
                                      " sent: " + QString::number(s64_BytesSent));
            }
            s32_Return = C_RD_WR;
         }
         else
         {
            // Flush to ensure data is sent immediately
            this->mc_SocketsTcp[ou32_Handle].pc_Socket->flush();
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP socket

   Jobs to perform:
   - check whether TCP receive buffer contains required number of data bytes
   - read those bytes

   The function shall not return any data unless it can provide as many data
   bytes as specified.

   \param[in]      ou32_Handle   handle obtained by InitTcp()
   \param[in,out]  orc_Data      in: the expected number of bytes is set by the
                                 caller (orc_Data.size()); out: received data

   \return
   C_NO_ERR   data read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    not enough bytes
   C_RD_WR    error reading data
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::ReadTcp(const uint32_t ou32_Handle, QByteArray & orc_Data)
{
   int32_t s32_Return;

   if (ou32_Handle >= static_cast<uint32_t>(this->mc_SocketsTcp.size()))
   {
      osc_write_log_error("openSYDE IP-TP", "ReadTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      s32_Return = C_NOACT;

      if (this->mc_SocketsTcp[ou32_Handle].pc_Socket == NULL)
      {
         osc_write_log_error("openSYDE IP-TP", "ReadTcp called with invalid socket.");
         s32_Return = C_CONFIG;
      }
      else
      {
         // Check for available data (non-blocking)
         const qint64 s64_Available =
            this->mc_SocketsTcp[ou32_Handle].pc_Socket->bytesAvailable();

         if (s64_Available >= orc_Data.size())
         {
            // Enough bytes available: read exactly the requested amount
            const QByteArray c_ReadData =
               this->mc_SocketsTcp[ou32_Handle].pc_Socket->read(orc_Data.size());

            if (c_ReadData.size() == orc_Data.size())
            {
               orc_Data = c_ReadData;
               s32_Return = C_NO_ERR;
            }
            else
            {
               // comm error: data reported as available but reading failed
               osc_write_log_error("openSYDE IP-TP",
                                   "TCP unexpected error: data reported as available but reading "
                                   "failed. IP-Address: " +
                                      mh_IpToText(
                                         this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
               s32_Return = C_RD_WR;
            }
         }
         else if (s64_Available < 0)
         {
            osc_write_log_error("openSYDE IP-TP",
                                "Could not read TCP: buffer count could not be read. Error: " +
                                   this->mc_SocketsTcp[ou32_Handle].pc_Socket->errorString() +
                                   " IP-Address: " +
                                   mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
         }
         else
         {
            // Not enough data yet - return C_NOACT (default)
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP socket with identifier matching

   Jobs to perform:
   - check whether TCP receive buffer contains required number of data bytes
   - Check if the identifiers are matching (starting at byte 0). The header
     must be already read separately. If the identifiers are not matching,
     the data will be stored in the buffer of the dispatcher.
   - read those bytes

   \param[in]      ou32_Handle               handle obtained by InitTcp()
   \param[in]      ou8_ClientBusIdentifier   client identifier of bus
   \param[in]      ou8_ClientNodeIdentifier  client identifier of node
   \param[in]      ou8_ServerBusIdentifier   server identifier of bus
   \param[in]      ou8_ServerNodeIdentifier  server identifier of node
   \param[in,out]  orc_Data                  in: the expected number of bytes is
                                             set by the caller (orc_Data.size());
                                             out: received data

   \return
   C_NO_ERR   data read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    not enough bytes
   C_RD_WR    error reading data
   C_RANGE    handle invalid
   C_WARN     data is not for the server with the node identifier and bus identifier
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::ReadTcp(const uint32_t ou32_Handle,
                                           const uint8_t ou8_ClientBusIdentifier,
                                           const uint8_t ou8_ClientNodeIdentifier,
                                           const uint8_t ou8_ServerBusIdentifier,
                                           const uint8_t ou8_ServerNodeIdentifier,
                                           QByteArray & orc_Data)
{
   int32_t s32_Return;

   s32_Return = this->ReadTcp(ou32_Handle, orc_Data);

   if (s32_Return == C_NO_ERR)
   {
      if (orc_Data.size() > 4)
      {
         // are source and target address correct?
         const uint16_t u16_SourceAddress =
            (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[0]) << 8U) +
             orc_Data[1]) - 1U;
         const uint16_t u16_TargetAddress =
            (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[2]) << 8U) +
             orc_Data[3]) - 1U;
         const uint8_t u8_SourceNodeId =
            static_cast<uint8_t>(u16_SourceAddress & 0x7FU);
         const uint8_t u8_SourceBusId =
            static_cast<uint8_t>((u16_SourceAddress >> 7U) & 0x0FU);
         const uint8_t u8_TargetNodeId =
            static_cast<uint8_t>(u16_TargetAddress & 0x7FU);
         const uint8_t u8_TargetBusId =
            static_cast<uint8_t>((u16_TargetAddress >> 7U) & 0x0FU);

         if ((u8_SourceNodeId != ou8_ServerNodeIdentifier) ||
             (u8_SourceBusId != ou8_ServerBusIdentifier) ||
             (u8_TargetNodeId != ou8_ClientNodeIdentifier) ||
             (u8_TargetBusId != ou8_ClientBusIdentifier))
         {
            // Save the message for other client server connections over the same IP address
            QMap<C_BufferIdentifier, QList<QByteArray> >::iterator c_ItBuffer;
            const C_BufferIdentifier c_Id(u8_TargetBusId, u8_TargetNodeId,
                                          u8_SourceBusId, u8_SourceNodeId);

            {
               QMutexLocker c_Lock(&mhc_LockBuffer);

               // Search for already existing data of this identifier
               c_ItBuffer = mhc_TcpBuffer.find(c_Id);

               if (c_ItBuffer != mhc_TcpBuffer.end())
               {
                  c_ItBuffer.value().push_back(orc_Data);
               }
               else
               {
                  QList<QByteArray> c_List;
                  c_List.push_back(orc_Data);
                  mhc_TcpBuffer.insert(c_Id, c_List);
               }
            }

            s32_Return = C_WARN;
         }
         else
         {
            s32_Return = C_NO_ERR;
         }
      }
      else
      {
         s32_Return = C_NOACT;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP buffer of dispatcher

   \param[in]      ou8_ClientBusIdentifier   client identifier of bus
   \param[in]      ou8_ClientNodeIdentifier  client identifier of node
   \param[in]      ou8_ServerBusIdentifier   server identifier of bus
   \param[in]      ou8_ServerNodeIdentifier  server identifier of node
   \param[in,out]  orc_Data                  in: the expected number of bytes is
                                             set by the caller (orc_Data.size());
                                             out: received data

   \return
   C_NO_ERR   data read successfully
   C_NOACT    no data for these identifiers
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::ReadTcpBuffer(const uint8_t ou8_ClientBusIdentifier,
                                                 const uint8_t ou8_ClientNodeIdentifier,
                                                 const uint8_t ou8_ServerBusIdentifier,
                                                 const uint8_t ou8_ServerNodeIdentifier,
                                                 QByteArray & orc_Data)
{
   int32_t s32_Return = C_NOACT;

   QMap<C_BufferIdentifier, QList<QByteArray> >::iterator c_ItBuffer;
   const C_BufferIdentifier c_Id(ou8_ClientBusIdentifier, ou8_ClientNodeIdentifier,
                                 ou8_ServerBusIdentifier, ou8_ServerNodeIdentifier);

   {
      QMutexLocker c_Lock(&mhc_LockBuffer);

      // Search for saved data in the buffer
      c_ItBuffer = mhc_TcpBuffer.find(c_Id);

      if (c_ItBuffer != mhc_TcpBuffer.end())
      {
         QList<QByteArray> & rc_List = c_ItBuffer.value();

         if (rc_List.size() > 0)
         {
            // Copy the data
            orc_Data = rc_List.first();
            // Remove the read data package
            rc_List.removeFirst();

            s32_Return = C_NO_ERR;
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast package on UDP request socket

   Go through all set up UDP "client" sockets and send specified data as broadcast.

   \param[in]  orc_Data   data to send

   \return
   C_NO_ERR   data sent successfully
   C_CONFIG   required socket not initialized
   C_RD_WR    error sending data
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::SendUdp(const QByteArray & orc_Data)
{
   int32_t s32_Return = C_NO_ERR;

   if (mc_SocketsUdpClient.size() == 0)
   {
      osc_write_log_error("openSYDE IP-TP", "SendUdp called with no socket.");
      s32_Return = C_CONFIG;
   }
   else
   {
      for (int32_t s32_Interface = 0; s32_Interface < mc_SocketsUdpClient.size(); s32_Interface++)
      {
         if (mc_SocketsUdpClient[s32_Interface] != NULL)
         {
            const qint64 s64_BytesSent =
               mc_SocketsUdpClient[s32_Interface]->writeDatagram(
                  orc_Data, QHostAddress::Broadcast, mhu16_UDP_TCP_PORT);

            if (s64_BytesSent != orc_Data.size())
            {
               osc_write_log_error("openSYDE IP-TP",
                                   "SendUdp sendto error: " +
                                      mc_SocketsUdpClient[s32_Interface]->errorString());
               s32_Return = C_RD_WR;
            }
         }
         else
         {
            // Write error to log, then ignore the error and continue
            osc_write_log_error("openSYDE IP-TP", "SendUdp called with invalid socket.");
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read package from UDP socket

   Go through all set up UDP "server" sockets and try to read incoming datagram.
   Report datagrams sent from:
   * any IP
   * port 13400 (to ignore reception of our own broadcasts and unexpected traffic)

   \param[out]  orc_Data   received data
   \param[out]  orau8_Ip   IP address of sender of data

   \return
   C_NO_ERR   datagram read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    no data received
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherWinSock::ReadUdp(QByteArray & orc_Data, uint8_t (& orau8_Ip)[4])
{
   int32_t s32_Return = C_NOACT;

   if (mc_SocketsUdpServer.size() == 0)
   {
      osc_write_log_error("openSYDE IP-TP", "ReadUdp called with no socket.");
      s32_Return = C_CONFIG;
   }
   else
   {
      for (int32_t s32_Interface = 0; s32_Interface < mc_SocketsUdpServer.size(); s32_Interface++)
      {
         if (mc_SocketsUdpServer[s32_Interface] != NULL)
         {
            // Check if there is a pending datagram
            if (mc_SocketsUdpServer[s32_Interface]->hasPendingDatagrams())
            {
               QHostAddress c_SenderAddress;
               quint16 u16_SenderPort = 0U;

               orc_Data.resize(
                  static_cast<int>(mc_SocketsUdpServer[s32_Interface]->pendingDatagramSize()));

               const qint64 s64_BytesRead =
                  mc_SocketsUdpServer[s32_Interface]->readDatagram(
                     orc_Data.data(), orc_Data.size(), &c_SenderAddress, &u16_SenderPort);

               if (s64_BytesRead > 0)
               {
                  // Extract sender IP address
                  const quint32 u32_Ipv4 = c_SenderAddress.toIPv4Address();
                  orau8_Ip[0] = static_cast<uint8_t>((u32_Ipv4 >> 24U) & 0xFFU);
                  orau8_Ip[1] = static_cast<uint8_t>((u32_Ipv4 >> 16U) & 0xFFU);
                  orau8_Ip[2] = static_cast<uint8_t>((u32_Ipv4 >> 8U) & 0xFFU);
                  orau8_Ip[3] = static_cast<uint8_t>(u32_Ipv4 & 0xFFU);

                  if (s64_BytesRead != orc_Data.size())
                  {
                     orc_Data.resize(static_cast<int>(s64_BytesRead));
                  }

                  // Filter out local reception of broadcasts we sent ourselves
                  // Responses from real nodes are sent from Port 13400
                  // Our broadcasts are sent with dynamically assigned Port != 13400
                  if (u16_SenderPort == mhu16_UDP_TCP_PORT)
                  {
                     s32_Return = C_NO_ERR;
                  }
                  else
                  {
                     s32_Return = C_NOACT;
                  }
                  break; // we have a package
               }
               else
               {
                  // comm error: data reported as available but reading failed
                  osc_write_log_error("openSYDE IP-TP",
                                      "ReadUdp unexpected error: data reported as available but "
                                      "reading failed. Reported size: " +
                                         QString::number(orc_Data.size()) +
                                         " Read size: " + QString::number(s64_BytesRead));
                  s32_Return = C_RD_WR;
               }
            }
         }
         else
         {
            // Write error to log, then ignore the error and continue
            osc_write_log_error("openSYDE IP-TP", "ReadUdp called with invalid socket.");
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Loads optional configuration file

   If the file does not exist, no specific configuration will be used for Ethernet.

   \param[in] orc_FileLocation Log file location path and file name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscIpDispatcherWinSock::LoadConfigFile(const QString & orc_FileLocation)
{
   if ((QFileInfo(orc_FileLocation).exists() &&
        QFileInfo(orc_FileLocation).isFile()) == true)
   {
      QSettings c_Ini(orc_FileLocation, QSettings::IniFormat);
      const QString c_Help =
         c_Ini.value("ETH_CONFIG/ETH_INTERFACE_NAME", "").toString();

      this->mc_PreferredInterfaceNames = c_Help.split(",", Qt::SkipEmptyParts);
   }
   else
   {
      this->mc_PreferredInterfaceNames.clear();
   }
}
