//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       C_CAN class implementation (Qt QCanBus cross-platform).

   CAN driver class providing an API interface for CAN access using
   Qt's QCanBusDevice. Supports SocketCAN on Linux and PeakCAN/VectorCAN
   on Windows via Qt's serial bus plugin architecture.

   The DLL path / interface name passed to DLL_Open() is resolved to a
   Qt CAN bus plugin and interface name:
   - No path separators and no ".dll" suffix -> socketcan plugin (e.g. "can0" -> socketcan:can0)
   - Contains "stwpeak" (case-insensitive)   -> peakcan plugin (usb{CommChannel})
   - Contains "stwvec" (case-insensitive)     -> vectorcan plugin (can0)
   - Otherwise                                -> CAN_COMP_ERR_DLL_FORMAT

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include "C_Can.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <chrono>
#include <cstring>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::can;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_Can::C_Can(const uint8_t ou8_CommChannel) :
   C_CanDispatcher(ou8_CommChannel),
   mpc_Device(NULL),
   mc_PluginName(""),
   mc_InterfaceName(""),
   mc_OriginalPath(""),
   mq_Opened(false),
   mu32_RxId(CAN_RX_ID_INVALID),
   ms32_BitrateKBitS(0)
{
}

//----------------------------------------------------------------------------------------------------------------------

C_Can::C_Can(void) :
   C_CanDispatcher(),
   mpc_Device(NULL),
   mc_PluginName(""),
   mc_InterfaceName(""),
   mc_OriginalPath(""),
   mq_Opened(false),
   mu32_RxId(CAN_RX_ID_INVALID),
   ms32_BitrateKBitS(0)
{
}

//----------------------------------------------------------------------------------------------------------------------

C_Can::~C_Can(void)
{
   if (mu8_CANOpened != 0U)
   {
      (void)this->CAN_Exit();
   }

   if (mq_Opened == true)
   {
      (void)this->DLL_Close();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resolve a DLL path / interface name to a Qt CAN bus plugin and interface

   Mapping rules:
   - No path separators ('/', '\\') and no ".dll" suffix -> SocketCAN interface name
   - Contains "stwpeak" (case-insensitive) -> peakcan plugin, interface "usb{CommChannel}"
   - Contains "stwvec" (case-insensitive) -> vectorcan plugin, interface "can0"
   - Otherwise -> CAN_COMP_ERR_DLL_FORMAT

   \param[in]   orc_Path   DLL path or interface name as provided by the caller

   \return
   C_NO_ERR                     plugin and interface resolved successfully
   CAN_COMP_ERR_DLL_FORMAT      path does not match any known pattern
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::m_ResolvePluginAndInterface(const QString & orc_Path)
{
   const bool q_HasPathSeparator = orc_Path.contains('/') || orc_Path.contains('\\');
   const bool q_HasDllSuffix = orc_Path.endsWith(".dll", Qt::CaseInsensitive);

   if ((q_HasPathSeparator == false) && (q_HasDllSuffix == false))
   {
      // Plain interface name like "can0", "vcan0" -> SocketCAN
      mc_PluginName = "socketcan";
      mc_InterfaceName = orc_Path;
      return C_NO_ERR;
   }

   const QString c_Lower = orc_Path.toLower();

   if (c_Lower.contains("stwpeak"))
   {
      mc_PluginName = "peakcan";
      mc_InterfaceName = "usb" + QString::number(mu8_CommChannel);
      return C_NO_ERR;
   }

   if (c_Lower.contains("stwvec"))
   {
      mc_PluginName = "vectorcan";
      mc_InterfaceName = "can" + QString::number(mu8_CommChannel);
      return C_NO_ERR;
   }

   return CAN_COMP_ERR_DLL_FORMAT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert a QCanBusFrame to a T_STWCAN_Msg_RX

   \param[in]   orc_Frame   Qt CAN bus frame
   \param[out]  orc_Msg     STW CAN RX message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_Can::mh_QFrameToStwRx(const QCanBusFrame & orc_Frame, T_STWCAN_Msg_RX & orc_Msg)
{
   orc_Msg.u32_ID = orc_Frame.frameId();
   orc_Msg.u8_XTD = orc_Frame.hasExtendedFrameFormat() ? 1U : 0U;
   orc_Msg.u8_RTR = (orc_Frame.frameType() == QCanBusFrame::RemoteRequestFrame) ? 1U : 0U;

   const QByteArray c_Payload = orc_Frame.payload();
   orc_Msg.u8_DLC = static_cast<uint8_t>(c_Payload.size());
   if (orc_Msg.u8_DLC > 8U)
   {
      orc_Msg.u8_DLC = 8U;
   }
   orc_Msg.u8_Align = 0U;
   std::memset(orc_Msg.au8_Data, 0, sizeof(orc_Msg.au8_Data));
   std::memcpy(orc_Msg.au8_Data, c_Payload.constData(), orc_Msg.u8_DLC);

   // Timestamp: QCanBusFrame provides microseconds
   const QCanBusFrame::TimeStamp c_Ts = orc_Frame.timeStamp();
   orc_Msg.u64_TimeStamp = (static_cast<uint64_t>(c_Ts.seconds()) * 1000000ULL) +
                            static_cast<uint64_t>(c_Ts.microSeconds());

   // If no timestamp from driver, use steady_clock
   if (orc_Msg.u64_TimeStamp == 0U)
   {
      auto now = std::chrono::steady_clock::now();
      auto duration = now.time_since_epoch();
      orc_Msg.u64_TimeStamp =
         std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert a T_STWCAN_Msg_TX to a QCanBusFrame

   \param[in]   orc_Msg   STW CAN TX message

   \return  QCanBusFrame ready to send
*/
//----------------------------------------------------------------------------------------------------------------------
QCanBusFrame C_Can::mh_StwTxToQFrame(const T_STWCAN_Msg_TX & orc_Msg)
{
   QCanBusFrame c_Frame;

   c_Frame.setFrameId(orc_Msg.u32_ID);
   c_Frame.setExtendedFrameFormat(orc_Msg.u8_XTD != 0U);
   if (orc_Msg.u8_RTR != 0U)
   {
      c_Frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
   }
   else
   {
      c_Frame.setFrameType(QCanBusFrame::DataFrame);
   }

   const uint8_t u8_Dlc = (orc_Msg.u8_DLC > 8U) ? 8U : orc_Msg.u8_DLC;
   c_Frame.setPayload(QByteArray(reinterpret_cast<const char *>(orc_Msg.au8_Data), u8_Dlc));

   return c_Frame;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open the CAN driver (resolve path and verify plugin availability)

   Uses the previously set DLL name (mc_OriginalPath).

   \return
   C_NO_ERR                          plugin available
   CAN_COMP_ERR_DLL_ALREADY_OPENED   already opened
   CAN_COMP_ERR_DLL_FORMAT           path does not match any known plugin pattern
   CAN_COMP_ERR_DLL_INIT             Qt CAN bus plugin not available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Open(void)
{
   if (mq_Opened == true)
   {
      return CAN_COMP_ERR_DLL_ALREADY_OPENED;
   }

   const int32_t s32_Resolve = m_ResolvePluginAndInterface(mc_OriginalPath);
   if (s32_Resolve != C_NO_ERR)
   {
      return s32_Resolve;
   }

   // Verify that the plugin is available
   const QStringList c_Plugins = QCanBus::instance()->plugins();
   if (c_Plugins.contains(mc_PluginName) == false)
   {
      return CAN_COMP_ERR_DLL_INIT;
   }

   mq_Opened = true;
   // XTD is always supported via QCanBusDevice
   mq_XTDAvailable = true;

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open the CAN driver with a specific path / interface name

   \param[in]   orc_FileName   DLL path or SocketCAN interface name

   \return
   C_NO_ERR                          plugin available
   CAN_COMP_ERR_DLL_ALREADY_OPENED   already opened
   CAN_COMP_ERR_DLL_FORMAT           path does not match any known plugin pattern
   CAN_COMP_ERR_DLL_INIT             Qt CAN bus plugin not available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Open(const QString & orc_FileName)
{
   mc_OriginalPath = orc_FileName;
   return DLL_Open();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close the CAN driver

   \return
   C_NO_ERR                          closed OK
   CAN_COMP_ERR_DLL_ALREADY_CLOSED   was not opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::DLL_Close(void)
{
   if (mq_Opened == true)
   {
      if (mu8_CANOpened != 0U)
      {
         mu8_CANOpened = 0U;
         (void)this->CAN_Exit();
      }
      mq_Opened = false;

      return C_NO_ERR;
   }
   return CAN_COMP_ERR_DLL_ALREADY_CLOSED;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize CAN bus (use device default bitrate)

   Creates the QCanBusDevice and connects to the bus.

   \return
   C_NO_ERR                          bus initialized
   -1                                initialization error
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Init(void)
{
   if (mq_Opened == false)
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   // Clean up any existing device
   if (mpc_Device != NULL)
   {
      mpc_Device->disconnectDevice();
      delete mpc_Device;
      mpc_Device = NULL;
   }

   QString c_ErrorString;
   mpc_Device = QCanBus::instance()->createDevice(mc_PluginName, mc_InterfaceName, &c_ErrorString);
   if (mpc_Device == NULL)
   {
      return CAN_COMP_ERR_DLL_INIT;
   }

   // Apply RX filter if configured
   if (mu32_RxId != CAN_RX_ID_INVALID)
   {
      QList<QCanBusDevice::Filter> c_Filters;
      QCanBusDevice::Filter c_Filter;
      c_Filter.frameId = mu32_RxId;
      c_Filter.frameIdMask = 0x1FFFFFFFU;
      c_Filter.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
      c_Filter.type = QCanBusFrame::DataFrame;
      c_Filters.append(c_Filter);
      mpc_Device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(c_Filters));
   }

   if (mpc_Device->connectDevice() == false)
   {
      delete mpc_Device;
      mpc_Device = NULL;
      return -1;
   }

   mu8_CANOpened = 1U;
   ms32_BitrateKBitS = 0; // no explicit bitrate set
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize CAN bus to specific bitrate

   \param[in]   os32_BitrateKBitS   bitrate in kBit/s

   \return
   C_NO_ERR                          bus initialized
   -1                                initialization error
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Init(const int32_t os32_BitrateKBitS)
{
   if (mq_Opened == false)
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   // Clean up any existing device
   if (mpc_Device != NULL)
   {
      mpc_Device->disconnectDevice();
      delete mpc_Device;
      mpc_Device = NULL;
   }

   QString c_ErrorString;
   mpc_Device = QCanBus::instance()->createDevice(mc_PluginName, mc_InterfaceName, &c_ErrorString);
   if (mpc_Device == NULL)
   {
      return CAN_COMP_ERR_DLL_INIT;
   }

   // Set bitrate (Qt expects bit/s, we get kBit/s)
   mpc_Device->setConfigurationParameter(QCanBusDevice::BitRateKey,
                                          static_cast<int>(os32_BitrateKBitS) * 1000);

   // Apply RX filter if configured
   if (mu32_RxId != CAN_RX_ID_INVALID)
   {
      QList<QCanBusDevice::Filter> c_Filters;
      QCanBusDevice::Filter c_Filter;
      c_Filter.frameId = mu32_RxId;
      c_Filter.frameIdMask = 0x1FFFFFFFU;
      c_Filter.format = QCanBusDevice::Filter::MatchBaseAndExtendedFormat;
      c_Filter.type = QCanBusFrame::DataFrame;
      c_Filters.append(c_Filter);
      mpc_Device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(c_Filters));
   }

   if (mpc_Device->connectDevice() == false)
   {
      delete mpc_Device;
      mpc_Device = NULL;
      return -1;
   }

   mu8_CANOpened = 1U;
   ms32_BitrateKBitS = os32_BitrateKBitS;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close CAN bus

   Disconnects and destroys the QCanBusDevice.

   \return
   C_NO_ERR                          closed OK
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Exit(void)
{
   if (mq_Opened == false)
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   mu8_CANOpened = 0U;

   if (mpc_Device != NULL)
   {
      mpc_Device->disconnectDevice();
      delete mpc_Device;
      mpc_Device = NULL;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reset CAN bus

   Disconnects and reconnects the QCanBusDevice with the same bitrate.

   \return
   C_NO_ERR                          bus reset OK
   -1                                error during re-init
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Reset(void)
{
   if (mq_Opened == false)
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   (void)CAN_Exit();

   if (ms32_BitrateKBitS != 0)
   {
      return CAN_Init(ms32_BitrateKBitS);
   }
   return CAN_Init();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read one CAN message (non-blocking)

   Called by the dispatcher's DispatchIncoming mechanism.

   \param[out]   orc_Message   read message

   \return
   C_NO_ERR                          message read
   -1                                no message available
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message)
{
   if ((mq_Opened == false) || (mpc_Device == NULL))
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   if (mpc_Device->framesAvailable() > 0)
   {
      const QCanBusFrame c_Frame = mpc_Device->readFrame();
      if (c_Frame.isValid() && (c_Frame.frameType() != QCanBusFrame::ErrorFrame))
      {
         mh_QFrameToStwRx(c_Frame, orc_Message);
         return C_NO_ERR;
      }
   }

   return -1;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send one CAN message

   \param[in]   orc_Message   CAN message to send

   \return
   C_NO_ERR                          message sent
   -1                                error sending
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Send_Msg(const T_STWCAN_Msg_TX & orc_Message)
{
   if ((mq_Opened == false) || (mpc_Device == NULL))
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   const QCanBusFrame c_Frame = mh_StwTxToQFrame(orc_Message);
   if (mpc_Device->writeFrame(c_Frame) == true)
   {
      return C_NO_ERR;
   }
   return -1;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read one CAN message with timeout

   Waits for a frame to arrive, then reads it.

   \param[in]   ou32_MaxWaitTimeMs   maximum wait time in milliseconds
   \param[out]  orc_Message          read message

   \return
   C_NO_ERR                          message read
   -1                                no message within timeout
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Read_Msg_Timeout(const uint32_t ou32_MaxWaitTimeMs, T_STWCAN_Msg_RX & orc_Message) const
{
   if ((mq_Opened == false) || (mpc_Device == NULL))
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   // Check if frames are already available
   if (mpc_Device->framesAvailable() > 0)
   {
      const QCanBusFrame c_Frame = mpc_Device->readFrame();
      if (c_Frame.isValid() && (c_Frame.frameType() != QCanBusFrame::ErrorFrame))
      {
         mh_QFrameToStwRx(c_Frame, orc_Message);
         return C_NO_ERR;
      }
   }

   // Wait for frames
   if (mpc_Device->waitForFramesReceived(static_cast<int>(ou32_MaxWaitTimeMs)) == true)
   {
      if (mpc_Device->framesAvailable() > 0)
      {
         const QCanBusFrame c_Frame = mpc_Device->readFrame();
         if (c_Frame.isValid() && (c_Frame.frameType() != QCanBusFrame::ErrorFrame))
         {
            mh_QFrameToStwRx(c_Frame, orc_Message);
            return C_NO_ERR;
         }
      }
   }

   return -1;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get system time in microseconds

   \param[out]   oru64_SystemTimeUs   system time in microseconds

   \return
   C_NO_ERR   always
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const
{
   auto now = std::chrono::steady_clock::now();
   auto duration = now.time_since_epoch();
   oru64_SystemTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Wait for incoming CAN frame

   Uses QCanBusDevice::waitForFramesReceived for efficient OS-level waiting.

   \param[in]   ou32_MaxWaitTimeMs   maximum wait time in milliseconds
*/
//----------------------------------------------------------------------------------------------------------------------
void C_Can::WaitForRxFrame(const uint32_t ou32_MaxWaitTimeMs)
{
   if (mpc_Device != NULL)
   {
      (void)mpc_Device->waitForFramesReceived(static_cast<int>(ou32_MaxWaitTimeMs));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show interactive setup dialog

   Not applicable for QCanBusDevice.

   \return
   C_NOACT   always (not supported)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_InteractiveSetup(void) const
{
   return C_NOACT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get CAN bus status

   Maps QCanBusDevice::state() to T_STWCAN_Status fields.

   \param[out]   orc_Status   status information

   \return
   C_NO_ERR                          status retrieved
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Status(T_STWCAN_Status & orc_Status) const
{
   if ((mq_Opened == false) || (mpc_Device == NULL))
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   std::memset(&orc_Status, 0, sizeof(orc_Status));

   const QCanBusDevice::CanBusStatus e_Status = mpc_Device->busStatus();
   switch (e_Status)
   {
   case QCanBusDevice::CanBusStatus::BusOff:
      orc_Status.iBusOff = 1;
      break;
   case QCanBusDevice::CanBusStatus::Warning:
      orc_Status.iBusWarn = 1;
      break;
   case QCanBusDevice::CanBusStatus::Good: // fall through
   case QCanBusDevice::CanBusStatus::Error: // fall through
   default:
      break;
   }

   orc_Status.iTXFree = 1; // assume always ready
   orc_Status.iTXOK = 1;

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get DLL / driver info

   Returns generic QCanBus information.

   \param[out]   orc_Info   DLL info structure

   \return
   C_NO_ERR                          info retrieved
   CAN_COMP_ERR_DLL_NOT_OPENED       DLL was not yet opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_DLL_Info(T_STWCAN_DLLInfo & orc_Info) const
{
   if (mq_Opened == false)
   {
      return CAN_COMP_ERR_DLL_NOT_OPENED;
   }

   std::memset(&orc_Info, 0, sizeof(orc_Info));

   orc_Info.bXTDSupported = 1U; // QCanBusDevice always supports extended frames

   // Version info
   const char * const pc_Version = "1.0.0";
   std::strncpy(reinterpret_cast<char *>(orc_Info.abDLLVersionNumber), pc_Version,
                sizeof(orc_Info.abDLLVersionNumber));

   // Device name
   const QByteArray c_PluginUtf8 = mc_PluginName.toUtf8();
   std::strncpy(reinterpret_cast<char *>(orc_Info.abDeviceName), c_PluginUtf8.constData(),
                sizeof(orc_Info.abDeviceName));

   // Manufacturer
   const char * const pc_Mfr = "Qt QCanBus";
   std::strncpy(reinterpret_cast<char *>(orc_Info.abManufacturerName), pc_Mfr,
                sizeof(orc_Info.abManufacturerName));

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get supported bitrates

   Returns a hardcoded list of standard CAN bitrates.

   \param[out]   orc_Unit                    unit string ("kBit/s")
   \param[out]   orc_Bitrates                list of supported bitrates
   \param[out]   oru32_MultiplicationFactor   multiplication factor for unit

   \return
   C_NO_ERR   always
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_Can::CAN_Get_Supported_Bitrates(QString & orc_Unit, QList<uint32_t> & orc_Bitrates,
                                           uint32_t & oru32_MultiplicationFactor) const
{
   orc_Unit = "kBit/s";
   oru32_MultiplicationFactor = 1000U;

   orc_Bitrates.clear();
   orc_Bitrates << 10U << 20U << 50U << 100U << 125U << 250U << 500U << 800U << 1000U;

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------

void C_Can::SetDLLName(const QString & orc_DLLName)
{
   mc_OriginalPath = orc_DLLName;
}

//----------------------------------------------------------------------------------------------------------------------

QString C_Can::GetDLLName(void) const
{
   return mc_OriginalPath;
}

//----------------------------------------------------------------------------------------------------------------------

void C_Can::SetLimitRXID(const uint32_t ou32_LimitRXID)
{
   mu32_RxId = ou32_LimitRXID;
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t C_Can::GetLimitRXID(void) const
{
   return mu32_RxId;
}

//----------------------------------------------------------------------------------------------------------------------
