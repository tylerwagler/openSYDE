//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export communication stack settings of an openSYDE node.

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTCOMMUNICATIONSTACK_HPP
#define C_OSCEXPORTCOMMUNICATIONSTACK_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QMap>
#include <vector>

#include "stwtypes.hpp"
#include <QList>

#include "C_OscNode.hpp"
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

class C_OscExportCommunicationStack {
public:
  static QString
  h_GetFileName(const uint8_t ou8_InterfaceIndex,
                const C_OscCanProtocol::E_Type &ore_ProtocolType);
  static QString
  h_GetConfigurationName(const uint8_t ou8_InterfaceIndex,
                         const C_OscCanProtocol::E_Type &ore_ProtocolType);
  static int32_t
  h_CreateSourceCode(const QString &orc_Path, const C_OscNode &orc_Node,
                     const uint16_t ou16_ApplicationIndex,
                     const uint8_t ou8_InterfaceIndex,
                     const uint32_t ou32_DatapoolIndex,
                     const C_OscCanProtocol::E_Type &ore_Protocol,
                     const QString &orc_ExportToolInfo = "");
  static uint16_t
  h_ConvertOverallCodeVersion(const uint16_t ou16_GenCodeVersion);

protected:
  static const bool mhq_IS_HEADER_FILE = false;
  static const bool mhq_IS_IMPLEMENTATION_FILE = true;

  static int32_t mh_CreateHeaderFile(const QString &orc_ExportToolInfo,
                                     const QString &orc_Path,
                                     const C_OscNodeApplication &orc_Applicaton,
                                     const C_OscCanProtocol &orc_ComProtocol,
                                     const uint8_t ou8_InterfaceIndex,
                                     const QString &orc_ProjectId);
  static int32_t mh_CreateImplementationFile(
      const QString &orc_ExportToolInfo, const QString &orc_Path,
      const C_OscNodeApplication &orc_Applicaton,
      const C_OscCanProtocol &orc_ComProtocol,
      const C_OscNodeDataPool &orc_DataPool, const uint8_t ou8_InterfaceIndex,
      const QString &orc_ProjectId);

  static void mh_AddHeader(const QString &orc_ExportToolInfo,
                           QStringList &orc_Data,
                           const uint8_t ou8_InterfaceIndex,
                           const C_OscCanProtocol::E_Type &ore_Protocol,
                           const bool oq_FileType);
  static void mh_AddCeIncludes(QStringList &orc_Data,
                               const C_OscNodeDataPool &orc_DataPool,
                               const uint8_t ou8_InterfaceIndex,
                               const C_OscCanProtocol::E_Type &ore_Protocol,
                               const bool oq_NullRequired);
  static void mh_AddDefines(QStringList &orc_Data,
                            const C_OscCanMessageContainer &orc_ComMessage,
                            const uint8_t ou8_InterfaceIndex,
                            const C_OscCanProtocol::E_Type &ore_Protocol,
                            const QString &orc_ProjectId,
                            const uint16_t ou16_GenCodeVersion,
                            const bool oq_FileType);
  static void
  mh_AddCeModuleGlobal(QStringList &orc_Data, const bool oq_SafeData,
                       const C_OscCanMessageContainer &orc_ComMessage,
                       const uint8_t ou8_InterfaceIndex,
                       const C_OscCanProtocol::E_Type &ore_Protocol,
                       const uint16_t ou16_GenCodeVersion,
                       const uint32_t ou32_TxListIndex,
                       const uint32_t ou32_RxListIndex);
  static void mh_AddCeGlobalVariables(
      QStringList &orc_Data, const QString &orc_DataPoolName,
      const uint8_t ou8_InterfaceIndex,
      const C_OscCanProtocol::E_Type &ore_Protocol,
      const bool oq_TxMessagesPresent, const bool oq_RxMessagesPresent);
  static void
  mh_AddSignalDefinitions(QStringList &orc_Data,
                          const uint32_t ou32_SignalListIndex,
                          const QList<C_OscCanMessage> &orc_Messages,
                          const uint16_t ou16_GenCodeVersion);
  static void
  mh_AddMessageMuxDefinitions(QStringList &orc_Data,
                              const QList<C_OscCanMessage> &orc_Messages,
                              const QString &orc_TxRxString);
  static void mh_AddMessageDefinitions(
      QStringList &orc_Data, const uint8_t ou8_InterfaceIndex,
      const C_OscCanProtocol::E_Type &ore_Protocol,
      const QList<C_OscCanMessage> &orc_Messages,
      const uint16_t ou16_GenCodeVersion, const bool oq_Tx);
  static uint32_t
  mh_CountMuxMessages(const QList<C_OscCanMessage> &orc_Messages);
  static void mh_ConvertSignalsToStrings(
      QStringList &orc_Data, const QList<C_OscCanSignal> &orc_Signals,
      const uint32_t ou32_SignalListIndex, const bool oq_RemoveLastComma);
  static void mh_GroupSignalsByMuxValue(
      const C_OscCanMessage &orc_Message, const uint32_t ou32_MultiplexerIndex,
      QMap<int32_t, QList<C_OscCanSignal>> &orc_MuxedSignalsPerValue,
      QList<C_OscCanSignal> &orc_NonMuxedSignals);
  static QString
  mh_GetProtocolNameByType(const C_OscCanProtocol::E_Type &ore_Protocol);
  static QString mh_GetByteOrderNameByType(
      const C_OscCanSignal::E_ByteOrderType &ore_ByteOrder);
  static QString mh_GetTransmissionTriggerNameByType(
      const C_OscCanMessage::E_TxMethodType &ore_Trigger);
  static QString mh_GetMagicName(const QString &orc_ProjectId,
                                 const uint8_t ou8_InterfaceIndex,
                                 const C_OscCanProtocol::E_Type &ore_Protocol);
};
/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
