//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Header for C_CAN class (Linux SocketCAN implementation).

   Header to class implementing a high level interface to CAN communication
   using the Linux SocketCAN subsystem. Provides the same public interface as
   the Windows STW-CAN-DLL implementation so that application code can use
   either one transparently.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCANHPP
#define CCANHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_CanDispatcher.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>

namespace stw {
namespace can {
/* -- Defines ------------------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
/// High level interface to Linux SocketCAN
class C_Can : public C_CanDispatcher {
private:
  C_Can(const C_Can &orc_Source);             // not implemented -> prevent copying
  C_Can &operator=(const C_Can &orc_Source);  // not implemented -> prevent assignment

  QString mc_InterfaceName; ///< SocketCAN interface name (e.g., "can0")
  int ms32_Socket;          ///< Socket file descriptor (-1 = closed)
  bool mq_Opened;           ///< Interface opened flag (analogous to "DLL opened" on Windows)
  uint32_t mu32_RxId;       ///< RX ID filter (CAN_RX_ID_INVALID = no filter)
  int32_t ms32_BitrateKBitS; ///< Stored bitrate for reset

  int32_t m_SetBitrate(const int32_t os32_BitrateKBitS);
  int32_t m_OpenSocket(void);

protected:
  // function from Dispatcher (mandatory to implement)
  // actual reading from CAN driver
  virtual int32_t m_CAN_Read_Msg(T_STWCAN_Msg_RX &orc_Message);

public:
  C_Can(void);
  C_Can(const uint8_t ou8_CommChannel);
  virtual ~C_Can(void);

  // functions from base (mandatory to implement):
  virtual int32_t
  CAN_Init(void); // use bitrate currently configured on interface
  virtual int32_t CAN_Init(const int32_t os32_BitrateKBitS);
  virtual int32_t CAN_Exit(void);
  virtual int32_t CAN_Reset(void);
  virtual int32_t CAN_Send_Msg(const T_STWCAN_Msg_TX &orc_Message);
  virtual int32_t CAN_Get_System_Time(uint64_t &oru64_SystemTimeUs) const;

  // Override base class to use poll() for efficient waiting
  virtual void WaitForRxFrame(const uint32_t ou32_MaxWaitTimeMs);

  // DLL_Open/DLL_Close - reused interface; takes interface name on Linux
  int32_t DLL_Open(void);
  int32_t DLL_Open(const QString &orc_InterfaceName);
  int32_t DLL_Close(void);

  int32_t CAN_Read_Msg_Timeout(const uint32_t ou32_MaxWaitTimeMs,
                               T_STWCAN_Msg_RX &orc_Message) const;

  // Not applicable on Linux - return C_NOACT
  int32_t CAN_InteractiveSetup(void) const;
  int32_t CAN_Status(T_STWCAN_Status &orc_Status) const;
  int32_t CAN_DLL_Info(T_STWCAN_DLLInfo &orc_Info) const;
  int32_t
  CAN_Get_Supported_Bitrates(QString &orc_Unit, QList<uint32_t> &orc_Bitrates,
                             uint32_t &oru32_MultiplicationFactor) const;

  // setter/getter functions (interface name on Linux, DLL name on Windows)
  void SetDLLName(const QString &orc_Name);
  QString GetDLLName(void) const;

  void SetLimitRXID(const uint32_t ou32_LimitRXID);
  uint32_t GetLimitRXID(void) const;
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */
// error codes (same values as Windows implementation for compatibility):
const int32_t CAN_COMP_ERR_DLL_NOT_OPENED = -201;
const int32_t CAN_COMP_ERR_DLL_ALREADY_CLOSED = -203;
const int32_t CAN_COMP_ERR_DLL_ALREADY_OPENED = -204;
const int32_t CAN_COMP_ERR_DLL_INIT = -205;
const int32_t CAN_COMP_ERR_DLL_FORMAT = -206;

const uint32_t CAN_RX_ID_INVALID = 0xFFFFFFFFUL;

/* -- Function Prototypes ------------------------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */
} // namespace can
} // namespace stw
#endif
