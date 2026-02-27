//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Core communication driver base class (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCOMDRIVERBASE_HPP
#define C_OSCCOMDRIVERBASE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

#include <QList>

#include "C_CanDispatcher.hpp"
#include "C_OscComAutoSupport.hpp"
#include "C_OscComMessageLogger.hpp"
#include "stw_can.hpp"
#include "stwtypes.hpp"

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

class C_OscComDriverBaseCanMessage {
public:
  C_OscComDriverBaseCanMessage(void);
  bool operator==(const C_OscComDriverBaseCanMessage &orc_Cmp) const;

  stw::can::T_STWCAN_Msg_TX c_Msg;
  uint32_t u32_TimeToSend; // First time to send message in ms. Can be used for
                           // an offset
  uint32_t u32_Interval;   // Cyclic interval time in ms
};

class C_OscComDriverBase {
public:
  C_OscComDriverBase(void);
  virtual ~C_OscComDriverBase(void);

  int32_t InitBase(stw::can::C_CanDispatcher *const opc_CanDispatcher);

  void RegisterLogger(C_OscComMessageLogger *const opc_Logger);

  virtual int32_t StartLogging(const int32_t os32_Bitrate);
  virtual void StopLogging(void);

  virtual void ContinueLogging(void);
  virtual void PauseLogging(void);

  virtual void UpdateBitrate(const int32_t os32_Bitrate);

  virtual void ClearRxMessages(void);
  virtual void DistributeMessages(void);
  virtual void SendCanMessageQueued(const stw::can::T_STWCAN_Msg_TX &orc_Msg);
  int32_t SendCanMessageDirect(stw::can::T_STWCAN_Msg_TX &orc_Msg);

  virtual void SendCanMessage(
      C_OscComDriverBaseCanMessage &orc_MsgCfg,
      const bool oq_SetAutoSupportMode,
      const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType);
  virtual void
  AddCyclicCanMessage(const C_OscComDriverBaseCanMessage &orc_MsgCfg,
                      const bool oq_SetAutoSupportMode,
                      const C_OscCanProtocol::E_Type oe_ProtocolType);
  virtual void
  RemoveCyclicCanMessage(const C_OscComDriverBaseCanMessage &orc_MsgCfg,
                         const bool oq_SetAutoSupportMode,
                         const C_OscCanProtocol::E_Type oe_ProtocolType);
  virtual void RemoveAllCyclicCanMessages(void);

  virtual void PrepareForDestruction(void);
  virtual void ClearData(void);
  virtual void
  UpdateAutoSupportProtocol(const int32_t os32_CanId,
                            const bool oq_SetAutoSupportMode,
                            const C_OscCanProtocol::E_Type oe_ProtocolType);

protected:
  virtual void m_HandleCanMessage(const stw::can::T_STWCAN_Msg_RX &orc_Msg,
                                  const bool oq_IsTx);

  stw::can::C_CanDispatcher
      *mpc_CanDispatcher; ///< Holds created instance of CAN dispatcher

private:
  // Avoid call
  C_OscComDriverBase(const C_OscComDriverBase &);
  C_OscComDriverBase &operator=(const C_OscComDriverBase &);

  void m_HandleCanMessagesForSending(void);

  static uint32_t
  mh_GetCanMessageSizeInBits(const stw::can::T_STWCAN_Msg_RX &orc_Msg);

  // Handling the CAN message logging and monitoring
  QList<C_OscComMessageLogger *> mc_Logger;
  uint16_t mu16_DispatcherClientHandle;
  bool mq_Started;
  bool mq_Paused;

  // Sending of CAN messages
  QList<stw::can::T_STWCAN_Msg_TX> mc_CanMessages;
  // Sending of configured CAN messages
  QList<C_OscComDriverBaseCanMessage> mc_CanMessageConfigs;

  // Bus load information
  uint32_t mu32_CanMessageBits;
  int32_t ms32_CanBitrate;

  uint32_t mu32_CanTxCounter;
  uint32_t mu32_CanTxErrors;

  C_OscComAutoSupport *mpc_AutoSupportProtocol;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
