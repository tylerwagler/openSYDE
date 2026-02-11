//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for simple device configuration sequence
   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDCBASICSEQUENCES_HPP
#define C_OSCDCBASICSEQUENCES_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include "C_CanDispatcher.hpp"
#include "C_OscDcDeviceInformation.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscProtocolDriverOsyTpCan.hpp"
#include <QString>


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

class C_OscDcBasicSequences {
public:
  C_OscDcBasicSequences(void);
  virtual ~C_OscDcBasicSequences(void);

  int32_t Init(stw::can::C_CanDispatcher *const opc_CanDispatcher);
  int32_t ScanEnterFlashloader(const uint32_t ou32_FlashloaderResetWaitTime);
  int32_t ScanGetInfo(void);
  int32_t ResetSystem(void);
  int32_t ConfigureDevice(const uint8_t ou8_CurrentNodeId,
                          const uint8_t ou8_NewNodeId,
                          const uint32_t ou32_Bitrate,
                          const uint8_t ou8_InterfaceIndex);

  static QString h_DevicesInfoToString(
      const QList<C_OscDcDeviceInformation> &orc_DeviceInfoResult,
      const bool oq_SecurityFeatureUsed);

  void PrepareForDestruction(void);

protected:
  virtual void m_ReportProgress(const int32_t os32_Result,
                                const QString &orc_Information);
  virtual void m_ReportDevicesInfoRead(
      const QList<C_OscDcDeviceInformation> &orc_DeviceInfoResult,
      const bool oq_SecurityFeatureUsed);

private:
  // driver instances:
  stw::can::C_CanDispatcher *mpc_CanDispatcher;
  stw::opensyde_core::C_OscProtocolDriverOsyTpCan mc_TpCan;
  stw::opensyde_core::C_OscProtocolDriverOsy mc_OsyProtocol;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
