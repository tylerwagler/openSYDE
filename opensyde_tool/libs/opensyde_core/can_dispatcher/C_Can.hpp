//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Header for C_CAN class (Qt QCanBus cross-platform implementation).

   Header to class implementing a high level interface to CAN communication
   using Qt's QCanBusDevice abstraction. Supports SocketCAN on Linux and
   PeakCAN/VectorCAN on Windows via Qt's plugin architecture, providing
   the same public interface as the previous platform-specific implementations.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCANHPP
#define CCANHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_CanDispatcher.hpp"
#include "stwtypes.hpp"
#include <QCanBusDevice>
#include <QList>
#include <QString>

namespace stw {
namespace can {
/* -- Defines ------------------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
/// High level interface to CAN communication via Qt QCanBusDevice
class C_Can : public C_CanDispatcher {
private:
   C_Can(const C_Can & orc_Source);             // not implemented -> prevent copying
   C_Can & operator=(const C_Can & orc_Source); // not implemented -> prevent assignment

   QCanBusDevice * mpc_Device;  ///< Qt CAN bus device (NULL when not connected)
   QString mc_PluginName;       ///< Qt CAN bus plugin name (e.g. "peakcan", "socketcan", "vectorcan")
   QString mc_InterfaceName;    ///< Interface identifier (e.g. "usb0", "can0")
   QString mc_OriginalPath;     ///< Original caller-provided path (for GetDLLName)
   bool mq_Opened;              ///< DLL_Open called flag
   uint32_t mu32_RxId;          ///< RX ID filter (CAN_RX_ID_INVALID = no filter)
   int32_t ms32_BitrateKBitS;   ///< Stored bitrate for CAN_Reset

   int32_t m_ResolvePluginAndInterface(const QString & orc_Path);
   static void mh_QFrameToStwRx(const QCanBusFrame & orc_Frame, T_STWCAN_Msg_RX & orc_Msg);
   static QCanBusFrame mh_StwTxToQFrame(const T_STWCAN_Msg_TX & orc_Msg);

protected:
   // function from Dispatcher (mandatory to implement)
   // actual reading from CAN driver
   virtual int32_t m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message);

public:
   C_Can(void);
   C_Can(const uint8_t ou8_CommChannel);
   virtual ~C_Can(void);

   // functions from base (mandatory to implement):
   virtual int32_t CAN_Init(void);
   virtual int32_t CAN_Init(const int32_t os32_BitrateKBitS);
   virtual int32_t CAN_Exit(void);
   virtual int32_t CAN_Reset(void);
   virtual int32_t CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message);
   virtual int32_t CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const;

   // Override base class to use QCanBusDevice::waitForFramesReceived
   virtual void WaitForRxFrame(const uint32_t ou32_MaxWaitTimeMs);

   // DLL_Open/DLL_Close - reused interface; resolves path to Qt plugin + interface
   int32_t DLL_Open(void);
   int32_t DLL_Open(const QString & orc_FileName);
   int32_t DLL_Close(void);

   int32_t CAN_Read_Msg_Timeout(const uint32_t ou32_MaxWaitTimeMs, T_STWCAN_Msg_RX & orc_Message) const;

   int32_t CAN_InteractiveSetup(void) const;
   int32_t CAN_Status(T_STWCAN_Status & orc_Status) const;
   int32_t CAN_DLL_Info(T_STWCAN_DLLInfo & orc_Info) const;
   int32_t CAN_Get_Supported_Bitrates(QString & orc_Unit, QList<uint32_t> & orc_Bitrates,
                                      uint32_t & oru32_MultiplicationFactor) const;

   // setter/getter functions
   void SetDLLName(const QString & orc_DLLName);
   QString GetDLLName(void) const;

   void SetLimitRXID(const uint32_t ou32_LimitRXID);
   uint32_t GetLimitRXID(void) const;
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */
// error codes (same values as previous implementations for compatibility):
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
