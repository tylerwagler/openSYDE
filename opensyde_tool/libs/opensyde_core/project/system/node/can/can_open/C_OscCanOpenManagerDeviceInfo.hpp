//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for CANopen manager device specific data information
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENMANAGERDEVICEINFO_HPP
#define C_OSCCANOPENMANAGERDEVICEINFO_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscCanOpenManagerMappableSignal.hpp"
#include "C_OscCanOpenObjectDictionary.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>

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

class C_OscCanOpenManagerDeviceInfo {
private:
  // Use mutable to keep "dynamic loading" from having a const-correctness
  // impact for the application. From an API perspective access via
  // GetEdsFileContent is still logically const
  mutable C_OscCanOpenObjectDictionary
      mc_EdsFileContent; ///< Content of loaded EDS file
  mutable bool mq_EdsFileContentLoaded;

public:
  C_OscCanOpenManagerDeviceInfo();

  void CalcHash(uint32_t &oru32_HashValue) const;

  const C_OscCanOpenObjectDictionary &GetEdsFileContent() const;
  void SetEdsFileContent(const C_OscCanOpenObjectDictionary &orc_NewContent);

  QString c_ProjectEdsFilePath; ///< Absolute file path to EDS file for delayed
                                 ///< loading; not part of the
                                 // file's content

  QString c_OriginalEdsFileName; ///< File name of original EDS file

  QList<C_OscCanOpenManagerMappableSignal>
      c_EdsFileMappableSignals; ///< All parsed and valid mappable signals
  bool q_DeviceOptional;        ///< Flag for CANopen manager to consider
   ///< this device
   ///< optional
  bool q_NoInitialization; ///< Flag for CANopen manager to skip initialization
                           ///< for this device
  bool q_FactorySettingsActive; ///< Reset device to default communication
                                ///< settings before starting
                                // configuration
  uint8_t u8_ResetNodeObjectDictionarySubIndex; ///< Selected object dictionary
                                                ///< sub index for reset node
  bool q_EnableHeartbeatProducing;      ///< Enable CANopen heartbeat producing
  uint16_t u16_HeartbeatProducerTimeMs; ///< Heartbeat producing frequency in
                                        ///< milliseconds
  bool q_UseOpenSydeNodeId; ///< Use openSYDE node ID instead of CANopen node ID
  uint8_t u8_NodeIdValue;   ///< Value of CANopen node ID, valid range 1-127
  bool q_EnableHeartbeatConsuming;      ///< Enable CANopen heartbeat consuming
  uint16_t u16_HeartbeatConsumerTimeMs; ///< Heartbeat consumer frequency in
                                        ///< milliseconds
  bool q_EnableHeartbeatConsumingAutoCalculation; ///< Enable CANopen heartbeat
                                                  ///< consuming automatic
                                                  ///< calculation

  // --------------------------------------------------------------------------
  // Qt Native Serialization
  // --------------------------------------------------------------------------
  void ToQDataStream(QDataStream &ro_DataStream) const;
  void FromQDataStream(QDataStream &ro_DataStream);
  QJsonObject ToJsonObject() const;
  void FromJsonObject(const QJsonObject &ro_Json);
  QDomElement ToQDomElement(QDomDocument &ro_Doc,
                            const QString &orc_ElementName) const;
  void FromQDomElement(const QDomElement &ro_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
