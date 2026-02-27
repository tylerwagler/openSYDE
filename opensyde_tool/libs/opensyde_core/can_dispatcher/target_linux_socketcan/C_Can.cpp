//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       C_CAN class implementation (Linux SocketCAN).

   CAN driver class providing an API interface for CAN access under
   Linux using the SocketCAN subsystem. Peak CAN devices appear as
   standard SocketCAN interfaces (can0, can1, ...) via the kernel
   peak_usb driver, so no proprietary library is needed.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include "C_Can.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include <QString>
#include <QDir>
#include <QProcess>
#include <QElapsedTimer>

#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <poll.h>
#include <time.h>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::can;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_Can::C_Can(const uint8_t ou8_CommChannel)
    : C_CanDispatcher(ou8_CommChannel), mc_InterfaceName("can0"),
      ms32_Socket(-1), mq_Opened(false), mu32_RxId(CAN_RX_ID_INVALID),
      ms32_BitrateKBitS(0) {}

//----------------------------------------------------------------------------------------------------------------------

C_Can::C_Can(void)
    : C_CanDispatcher(), mc_InterfaceName("can0"), ms32_Socket(-1),
      mq_Opened(false), mu32_RxId(CAN_RX_ID_INVALID), ms32_BitrateKBitS(0) {}

//----------------------------------------------------------------------------------------------------------------------

C_Can::~C_Can(void) {
  if (mu8_CANOpened != 0U) {
    (void)this->CAN_Exit();
  }

  if (mq_Opened == true) {
    (void)this->DLL_Close();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set bitrate on SocketCAN interface

   Uses "ip link" commands to configure the CAN interface bitrate.
   Requires appropriate permissions (root, sudo, or CAP_NET_ADMIN capability).

   \param[in] os32_BitrateKBitS   bitrate in kBit/s

   \return
   C_NO_ERR     bitrate set successfully
   -1           failed to set bitrate (permissions or interface issue)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::m_SetBitrate(const int32_t os32_BitrateKBitS) {
  const int32_t s32_BitrateHz = os32_BitrateKBitS * 1000;
  const QByteArray c_Iface = mc_InterfaceName.toUtf8();
  int32_t s32_Return = C_NO_ERR;

  // Bring interface down
  QProcess c_Process;
  c_Process.start("ip", QStringList() << "link"
                                      << "set" << c_Iface << "down");
  if (c_Process.waitForFinished(5000) == false) {
    return -1;
  }

  // Set bitrate
  c_Process.start("ip", QStringList() << "link"
                                      << "set" << c_Iface << "type"
                                      << "can"
                                      << "bitrate"
                                      << QString::number(s32_BitrateHz));
  if (c_Process.waitForFinished(5000) == false) {
    s32_Return = -1;
  } else if (c_Process.exitCode() != 0) {
    s32_Return = -1;
  } else {
    // success
  }

  // Bring interface back up regardless of bitrate result
  c_Process.start("ip", QStringList() << "link"
                                      << "set" << c_Iface << "up");
  if (c_Process.waitForFinished(5000) == false) {
    s32_Return = -1;
  } else if (c_Process.exitCode() != 0) {
    s32_Return = -1;
  } else {
    // success
  }

  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open SocketCAN raw socket and bind to interface

   \return
   C_NO_ERR     socket opened and bound successfully
   -1           error opening socket or binding
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::m_OpenSocket(void) {
  struct sockaddr_can t_Addr;
  struct ifreq t_Ifr;
  int s32_Fd;
  int s32_Flags;

  // Create raw CAN socket
  s32_Fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s32_Fd < 0) {
    return -1;
  }

  // Set non-blocking
  s32_Flags = fcntl(s32_Fd, F_GETFL, 0);
  if (s32_Flags < 0) {
    close(s32_Fd);
    return -1;
  }
  if (fcntl(s32_Fd, F_SETFL, s32_Flags | O_NONBLOCK) < 0) {
    close(s32_Fd);
    return -1;
  }

  // Enable kernel-level timestamps
  {
    const int s32_TimestampOn = 1;
    (void)setsockopt(s32_Fd, SOL_SOCKET, SO_TIMESTAMPNS, &s32_TimestampOn,
                     sizeof(s32_TimestampOn));
  }

  // Set up RX filter if a specific ID is configured
  if (mu32_RxId != CAN_RX_ID_INVALID) {
    struct can_filter t_Filter;
    t_Filter.can_id = mu32_RxId;
    t_Filter.can_mask = CAN_EFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;
    (void)setsockopt(s32_Fd, SOL_CAN_RAW, CAN_RAW_FILTER, &t_Filter,
                     sizeof(t_Filter));
  }

  // Look up interface index
  (void)std::memset(&t_Ifr, 0, sizeof(t_Ifr));
  std::strncpy(t_Ifr.ifr_name, mc_InterfaceName.toUtf8().constData(),
               sizeof(t_Ifr.ifr_name) - 1U);
  if (ioctl(s32_Fd, SIOCGIFINDEX, &t_Ifr) < 0) {
    close(s32_Fd);
    return -1;
  }

  // Bind to interface
  (void)std::memset(&t_Addr, 0, sizeof(t_Addr));
  t_Addr.can_family = AF_CAN;
  t_Addr.can_ifindex = t_Ifr.ifr_ifindex;

  if (bind(s32_Fd, reinterpret_cast<struct sockaddr *>(&t_Addr),
           sizeof(t_Addr)) < 0) {
    close(s32_Fd);
    return -1;
  }

  ms32_Socket = s32_Fd;
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize CAN bus using interface's current configuration

   Opens the SocketCAN socket and binds to the configured interface.
   The interface must already be up and configured.

   \return
   C_NO_ERR                          socket opened; bus initialized
   -1                                initialization error
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened via DLL_Open
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Init(void) {
  int32_t s32_Return;

  if (mq_Opened == false) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  s32_Return = m_OpenSocket();
  if (s32_Return == C_NO_ERR) {
    mu8_CANOpened = 1U;
    // SocketCAN always supports extended frames
    mq_XTDAvailable = true;
  }
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize CAN bus to specific bitrate

   Sets the bitrate on the SocketCAN interface, then opens the socket.

   \param[in] os32_BitrateKBitS   bitrate to set (in kBit/s)

   \return
   C_NO_ERR                          bus initialized at requested bitrate
   -1                                initialization error
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened via DLL_Open
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Init(const int32_t os32_BitrateKBitS) {
  int32_t s32_Return;

  if (mq_Opened == false) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  ms32_BitrateKBitS = os32_BitrateKBitS;

  // Try to set bitrate; if it fails (e.g., vcan or no permissions), still try
  // to open
  (void)m_SetBitrate(os32_BitrateKBitS);

  s32_Return = m_OpenSocket();
  if (s32_Return == C_NO_ERR) {
    mu8_CANOpened = 1U;
    mq_XTDAvailable = true;
  }
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close CAN bus

   Closes the SocketCAN socket.

   \return
   C_NO_ERR                          socket closed
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Exit(void) {
  if (mq_Opened == false) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  mu8_CANOpened = 0U;

  if (ms32_Socket >= 0) {
    close(ms32_Socket);
    ms32_Socket = -1;
  }

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reset CAN bus

   Closes and reopens the SocketCAN socket.

   \return
   C_NO_ERR                          bus reset
   -1                                error during reset
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Reset(void) {
  if (mq_Opened == false) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  (void)CAN_Exit();

  if (ms32_BitrateKBitS > 0) {
    return CAN_Init(ms32_BitrateKBitS);
  }
  return CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read one CAN message from SocketCAN

   Reads a single CAN frame from the socket and converts it to the
   T_STWCAN_Msg_RX format.

   \param[out] orc_Message   read message

   \return
   C_NO_ERR                          message read
   -1                                no message available or error
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::m_CAN_Read_Msg(T_STWCAN_Msg_RX &orc_Message) {
  struct can_frame t_Frame;
  struct msghdr t_MsgHdr;
  struct iovec t_Iov;
  char acn_CtrlBuf[CMSG_SPACE(sizeof(struct timespec))];
  ssize_t s_NBytes;

  if ((mq_Opened == false) || (ms32_Socket < 0)) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  // Set up recvmsg to get kernel timestamp
  (void)std::memset(&t_MsgHdr, 0, sizeof(t_MsgHdr));
  t_Iov.iov_base = &t_Frame;
  t_Iov.iov_len = sizeof(t_Frame);
  t_MsgHdr.msg_iov = &t_Iov;
  t_MsgHdr.msg_iovlen = 1;
  t_MsgHdr.msg_control = acn_CtrlBuf;
  t_MsgHdr.msg_controllen = sizeof(acn_CtrlBuf);

  s_NBytes = recvmsg(ms32_Socket, &t_MsgHdr, 0);
  if (s_NBytes <= 0) {
    return -1; // no message available
  }

  if (static_cast<size_t>(s_NBytes) < sizeof(struct can_frame)) {
    return -1; // incomplete frame
  }

  // Convert SocketCAN frame to T_STWCAN_Msg_RX
  if ((t_Frame.can_id & CAN_EFF_FLAG) != 0U) {
    orc_Message.u32_ID = t_Frame.can_id & CAN_EFF_MASK;
    orc_Message.u8_XTD = 1U;
  } else {
    orc_Message.u32_ID = t_Frame.can_id & CAN_SFF_MASK;
    orc_Message.u8_XTD = 0U;
  }

  orc_Message.u8_RTR =
      ((t_Frame.can_id & CAN_RTR_FLAG) != 0U) ? 1U : 0U;
  orc_Message.u8_DLC = t_Frame.can_dlc;
  orc_Message.u8_Align = 0U;
  (void)std::memcpy(orc_Message.au8_Data, t_Frame.data, 8U);

  // Try to get kernel timestamp
  orc_Message.u64_TimeStamp = 0U;
  {
    struct cmsghdr *pc_CMsg;
    for (pc_CMsg = CMSG_FIRSTHDR(&t_MsgHdr); pc_CMsg != NULL;
         pc_CMsg = CMSG_NXTHDR(&t_MsgHdr, pc_CMsg)) {
      if ((pc_CMsg->cmsg_level == SOL_SOCKET) &&
          (pc_CMsg->cmsg_type == SO_TIMESTAMPNS)) {
        const struct timespec *const pc_Ts =
            reinterpret_cast<const struct timespec *>(CMSG_DATA(pc_CMsg));
        orc_Message.u64_TimeStamp =
            (static_cast<uint64_t>(pc_Ts->tv_sec) * 1000000ULL) +
            (static_cast<uint64_t>(pc_Ts->tv_nsec) / 1000ULL);
        break;
      }
    }
  }

  // Fallback: use monotonic clock if kernel timestamp not available
  if (orc_Message.u64_TimeStamp == 0U) {
    struct timespec t_Ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &t_Ts);
    orc_Message.u64_TimeStamp =
        (static_cast<uint64_t>(t_Ts.tv_sec) * 1000000ULL) +
        (static_cast<uint64_t>(t_Ts.tv_nsec) / 1000ULL);
  }

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send CAN message via SocketCAN

   Converts T_STWCAN_Msg_TX to a SocketCAN can_frame and writes it to the socket.

   \param[in] orc_Message   CAN message to send

   \return
   C_NO_ERR                          message sent
   -1                                error sending
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Send_Msg(const T_STWCAN_Msg_TX &orc_Message) {
  struct can_frame t_Frame;
  ssize_t s_NBytes;

  if ((mq_Opened == false) || (ms32_Socket < 0)) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  (void)std::memset(&t_Frame, 0, sizeof(t_Frame));

  // Build CAN ID with flags
  t_Frame.can_id = orc_Message.u32_ID;
  if (orc_Message.u8_XTD != 0U) {
    t_Frame.can_id |= CAN_EFF_FLAG;
  }
  if (orc_Message.u8_RTR != 0U) {
    t_Frame.can_id |= CAN_RTR_FLAG;
  }

  t_Frame.can_dlc = orc_Message.u8_DLC;
  (void)std::memcpy(t_Frame.data, orc_Message.au8_Data, 8U);

  s_NBytes = write(ms32_Socket, &t_Frame, sizeof(t_Frame));
  if (s_NBytes != static_cast<ssize_t>(sizeof(t_Frame))) {
    return -1;
  }

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read CAN message with timeout

   Polls the socket for incoming frames up to the specified timeout.

   \param[in]  ou32_MaxWaitTimeMs   maximum time to wait in ms
   \param[out] orc_Message          read message

   \return
   C_NO_ERR                          message read
   -1                                no message received within timeout
   CAN_COMP_ERR_DLL_NOT_OPENED       interface was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Read_Msg_Timeout(const uint32_t ou32_MaxWaitTimeMs,
                                    T_STWCAN_Msg_RX &orc_Message) const {
  if ((mq_Opened == false) || (ms32_Socket < 0)) {
    return CAN_COMP_ERR_DLL_NOT_OPENED;
  }

  struct pollfd t_Pfd;
  t_Pfd.fd = ms32_Socket;
  t_Pfd.events = POLLIN;

  const int s32_Ret =
      poll(&t_Pfd, 1, static_cast<int>(ou32_MaxWaitTimeMs));

  if (s32_Ret <= 0) {
    return -1; // timeout or error
  }

  // Data is available - read it
  // Note: we need to use const_cast here because m_CAN_Read_Msg is not const
  // but we need a const method. Use raw read instead.
  struct can_frame t_Frame;
  const ssize_t s_NBytes = read(ms32_Socket, &t_Frame, sizeof(t_Frame));
  if (s_NBytes <= 0) {
    return -1;
  }

  if (static_cast<size_t>(s_NBytes) < sizeof(struct can_frame)) {
    return -1;
  }

  // Convert frame
  if ((t_Frame.can_id & CAN_EFF_FLAG) != 0U) {
    orc_Message.u32_ID = t_Frame.can_id & CAN_EFF_MASK;
    orc_Message.u8_XTD = 1U;
  } else {
    orc_Message.u32_ID = t_Frame.can_id & CAN_SFF_MASK;
    orc_Message.u8_XTD = 0U;
  }

  orc_Message.u8_RTR =
      ((t_Frame.can_id & CAN_RTR_FLAG) != 0U) ? 1U : 0U;
  orc_Message.u8_DLC = t_Frame.can_dlc;
  orc_Message.u8_Align = 0U;
  (void)std::memcpy(orc_Message.au8_Data, t_Frame.data, 8U);

  // Use monotonic clock for timestamp
  struct timespec t_Ts;
  (void)clock_gettime(CLOCK_MONOTONIC, &t_Ts);
  orc_Message.u64_TimeStamp =
      (static_cast<uint64_t>(t_Ts.tv_sec) * 1000000ULL) +
      (static_cast<uint64_t>(t_Ts.tv_nsec) / 1000ULL);

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get system time in microseconds

   Uses CLOCK_MONOTONIC for a consistent time base.

   \param[out]   oru64_SystemTimeUs    system time in micro-seconds

   \return
   C_NO_ERR     always
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Get_System_Time(uint64_t &oru64_SystemTimeUs) const {
  struct timespec t_Ts;

  (void)clock_gettime(CLOCK_MONOTONIC, &t_Ts);
  oru64_SystemTimeUs =
      (static_cast<uint64_t>(t_Ts.tv_sec) * 1000000ULL) +
      (static_cast<uint64_t>(t_Ts.tv_nsec) / 1000ULL);

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Wait efficiently for incoming CAN frame using poll()

   Uses the poll() system call to wait for data on the socket without
   busy-waiting.

   \param[in]   ou32_MaxWaitTimeMs   maximum number of ms to wait
*/
//----------------------------------------------------------------------------------------------------------------------
void C_Can::WaitForRxFrame(const uint32_t ou32_MaxWaitTimeMs) {
  if (ms32_Socket >= 0) {
    struct pollfd t_Pfd;
    t_Pfd.fd = ms32_Socket;
    t_Pfd.events = POLLIN;
    (void)poll(&t_Pfd, 1, static_cast<int>(ou32_MaxWaitTimeMs));
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open interface (validate that SocketCAN interface exists)

   Checks that the configured interface name exists in /sys/class/net/.
   Analogous to DLL_Open on Windows.

   \return
   C_NO_ERR                          interface exists and is ready
   CAN_COMP_ERR_DLL_ALREADY_OPENED   interface already opened
   CAN_COMP_ERR_DLL_FORMAT           interface not found in system
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Open(void) {
  if (mq_Opened == true) {
    return CAN_COMP_ERR_DLL_ALREADY_OPENED;
  }

  // Verify interface exists
  if (QDir("/sys/class/net/" + mc_InterfaceName).exists() == false) {
    return CAN_COMP_ERR_DLL_FORMAT;
  }

  mq_Opened = true;
  mq_XTDAvailable = true; // SocketCAN always supports extended frames
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open interface by name

   \param[in] orc_InterfaceName   SocketCAN interface name (e.g., "can0")

   \return
   C_NO_ERR                          interface opened
   CAN_COMP_ERR_DLL_ALREADY_OPENED   interface already opened
   CAN_COMP_ERR_DLL_FORMAT           interface not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Open(const QString &orc_InterfaceName) {
  mc_InterfaceName = orc_InterfaceName;
  return DLL_Open();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close interface

   Closes the CAN socket if still open and marks the interface as closed.

   \return
   C_NO_ERR                          interface closed
   CAN_COMP_ERR_DLL_ALREADY_CLOSED   interface was already closed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Close(void) {
  if (mq_Opened == true) {
    if (mu8_CANOpened != 0U) {
      mu8_CANOpened = 0U;
      (void)this->CAN_Exit();
    }
    mq_Opened = false;
    return C_NO_ERR;
  }
  return CAN_COMP_ERR_DLL_ALREADY_CLOSED;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Interactive setup - not applicable on Linux

   \return
   C_NOACT   function not applicable
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_InteractiveSetup(void) const { return C_NOACT; }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get CAN status - not directly applicable on Linux

   \param[out] orc_Status   status structure (zeroed)

   \return
   C_NOACT   function not applicable
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Status(T_STWCAN_Status &orc_Status) const {
  (void)std::memset(&orc_Status, 0, sizeof(orc_Status));
  return C_NOACT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get DLL info - provides SocketCAN driver info

   \param[out] orc_Info   info structure

   \return
   C_NO_ERR   always (on Linux, reports SocketCAN as driver)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_DLL_Info(T_STWCAN_DLLInfo &orc_Info) const {
  (void)std::memset(&orc_Info, 0, sizeof(orc_Info));
  orc_Info.bXTDSupported = 1U; // SocketCAN always supports 29bit
  // Fill in some basic info
  std::strncpy(reinterpret_cast<char *>(orc_Info.abDeviceName), "SocketCAN",
               sizeof(orc_Info.abDeviceName) - 1U);
  std::strncpy(reinterpret_cast<char *>(orc_Info.abManufacturerName), "Linux",
               sizeof(orc_Info.abManufacturerName) - 1U);
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get supported bitrates - provides standard CAN bitrates

   \param[out] orc_Unit                   unit string
   \param[out] orc_Bitrates               list of supported bitrates
   \param[out] oru32_MultiplicationFactor  multiplication factor

   \return
   C_NO_ERR   always
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Get_Supported_Bitrates(
    QString &orc_Unit, QList<uint32_t> &orc_Bitrates,
    uint32_t &oru32_MultiplicationFactor) const {
  oru32_MultiplicationFactor = 1000U;
  orc_Unit = "kBit/s";

  // Standard CAN bitrates
  orc_Bitrates.clear();
  orc_Bitrates << 10U << 20U << 50U << 100U << 125U << 250U << 500U << 800U
               << 1000U;

  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------

void C_Can::SetDLLName(const QString &orc_Name) {
  mc_InterfaceName = orc_Name;
}

//----------------------------------------------------------------------------------------------------------------------

QString C_Can::GetDLLName(void) const { return mc_InterfaceName; }

//----------------------------------------------------------------------------------------------------------------------

void C_Can::SetLimitRXID(const uint32_t ou32_LimitRXID) {
  mu32_RxId = ou32_LimitRXID;
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t C_Can::GetLimitRXID(void) const { return mu32_RxId; }

//----------------------------------------------------------------------------------------------------------------------
