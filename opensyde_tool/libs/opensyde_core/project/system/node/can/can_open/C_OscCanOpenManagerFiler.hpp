//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for CANopen data classes
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENMANAGERFILER_HPP
#define C_OSCCANOPENMANAGERFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscCanOpenManagerInfo.hpp"
#include "C_OscXmlParser.hpp"
#include <QList>
#include <QStringList>

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

class C_OscCanOpenManagerFiler {
public:
  C_OscCanOpenManagerFiler();

   static int32_t
   h_LoadFile(QHash<uint8_t, C_OscCanOpenManagerInfo> &orc_Config,
              const QString &orc_Path, const QString &orc_BasePath);
   static int32_t
   h_SaveFile(const QHash<uint8_t, C_OscCanOpenManagerInfo> &orc_Config,
              const QString &orc_Path, const QString &orc_BasePath,
              QStringList *const opc_CreatedFiles,
              const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t
   h_LoadData(QHash<uint8_t, C_OscCanOpenManagerInfo> &orc_Config,
              C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath);
   static int32_t
   h_SaveData(const QHash<uint8_t, C_OscCanOpenManagerInfo> &orc_Config,
              C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath,
              QStringList *const opc_CreatedFiles,
              const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);

public:
   static const uint16_t mhu16_FILE_VERSION_1;

   static int32_t mh_LoadManagerData(C_OscCanOpenManagerInfo &orc_Config,
                                   C_OscXmlParserBase &orc_XmlParser,
                                   const QString &orc_BasePath);
    static int32_t mh_SaveManagerData(
        const C_OscCanOpenManagerInfo &orc_Config,
        C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath,
        QStringList *const opc_CreatedFiles,
        const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t mh_LoadManagerProperties(C_OscCanOpenManagerInfo &orc_Config,
                                           C_OscXmlParserBase &orc_XmlParser);
   static void
   mh_SaveManagerProperties(const C_OscCanOpenManagerInfo &orc_Config,
                            C_OscXmlParserBase &orc_XmlParser);
   static int32_t
   mh_LoadManagerSyncProperties(C_OscCanOpenManagerInfo &orc_Config,
                                C_OscXmlParserBase &orc_XmlParser);
   static void
   mh_SaveManagerSyncProperties(const C_OscCanOpenManagerInfo &orc_Config,
                                C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadManagerSubDevices(
       QHash<C_OscCanInterfaceId, C_OscCanOpenManagerDeviceInfo> &orc_Config,
       C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath);
   static int32_t mh_SaveManagerSubDevices(
       const QHash<C_OscCanInterfaceId, C_OscCanOpenManagerDeviceInfo>
           &orc_Config,
       C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath,
       QStringList *const opc_CreatedFiles,
       const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap);
   static int32_t
   mh_LoadManagerSubDevice(C_OscCanOpenManagerDeviceInfo &orc_Config,
                           C_OscXmlParserBase &orc_XmlParser,
                           const QString &orc_BasePath);
    static int32_t mh_SaveManagerSubDevice(
        const C_OscCanOpenManagerDeviceInfo &orc_Config,
        C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath,
        QStringList *const opc_CreatedFiles, const QString &orc_NodeName,
        const uint8_t ou8_InterfaceNumber);
   static int32_t
   mh_LoadManagerSubDeviceEdsPart(C_OscCanOpenManagerDeviceInfo &orc_Config,
                                  C_OscXmlParserBase &orc_XmlParser,
                                  const QString &orc_BasePath);
   static int32_t mh_SaveManagerSubDeviceEdsPart(
       const C_OscCanOpenManagerDeviceInfo &orc_Config,
       C_OscXmlParserBase &orc_XmlParser, const QString &orc_BasePath,
       QStringList *const opc_CreatedFiles, const QString &orc_NodeName,
       const uint8_t ou8_InterfaceNumber);
   static int32_t mh_LoadManagerMappedSignals(
       QList<C_OscCanOpenManagerMappableSignal> &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveManagerMappedSignals(
       const QList<C_OscCanOpenManagerMappableSignal> &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static int32_t
   mh_LoadManagerMappedSignal(C_OscCanOpenManagerMappableSignal &orc_Config,
                              C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveManagerMappedSignal(
       const C_OscCanOpenManagerMappableSignal &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static QString mh_CanOpenManagerInfoTypeToString(
       const C_OscCanOpenManagerInfo::E_NmtErrorBehaviourType &ore_Type);
   static int32_t mh_StringToCanOpenManagerInfoType(
       const QString &orc_String,
       C_OscCanOpenManagerInfo::E_NmtErrorBehaviourType &ore_Type);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
