//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package V1

   \class       stw::opensyde_core::C_OscSuServiceUpdatePackage

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGEV1_H
#define C_OSCSUPSERVICEUPDATEPACKAGEV1_H

#include <QList>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <vector>

#include "C_OscSuSequences.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscXmlParser.hpp"
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

//----------------------------------------------------------------------------------------------------------------------

class C_OscSupServiceUpdatePackageV1 {
public:
  static int32_t h_CreatePackage(
      const QString &orc_PackagePath,
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
      const bool oq_SaveInCompatibilityFormat, const bool oq_SaveAsFile,
      const QString &orc_TemporaryDirectory = "");

  static int32_t
  h_ProcessPackage(const QString &orc_PackagePath,
                   const QString &orc_TargetUnzipPath,
                   C_OscSystemDefinition &orc_SystemDefinition,
                   uint32_t &oru32_ActiveBusIndex, QByteArray &orc_ActiveNodes,
                   QList<uint32_t> &orc_NodesUpdateOrder,
                   QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
                   QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
                   const bool oq_IsZip);

  static QString h_GetPackageExtension();

protected:
  /// class for one node in service update package definition file
  ///  is used for easy conversion of input parameters to target xml file
  class C_SupDefNodeContent {
  public:
    uint8_t u8_Active;
    uint32_t u32_Position;
    QStringList c_ApplicationFileNames; // with relative path
    QStringList c_NvmFileNames;         // with relative path
    QString c_PemFile;

    /// Node configuration flags for security state
    bool q_SendSecurityEnabledState;
    bool q_SecurityEnabled;

    /// Node configuration flags for debugger state
    bool q_SendDebuggerEnabledState;
    bool q_DebuggerEnabled;
  };

  /// class to create service update package definition file
  ///  is used for easy conversion of input parameters to target xml file
  class C_SupDefContent {
  public:
    QList<C_SupDefNodeContent> c_Nodes;
    uint32_t u32_ActiveBusIndex;
  };

  static QStringList
      mhc_WarningMessages; // global warnings e.g. if update position of active
                           // node is not available
  static QString mhc_ErrorMessage; // description of error which caused the
                                   // service update package to fail

  static int32_t mh_CheckSupFiles(const QString &orc_PackagePath);

  static int32_t mh_CheckParamsToCreatePackage(
      const QString &orc_PackagePath,
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<stw::opensyde_core::C_OscSuSequences::C_DoFlash>
          &orc_ApplicationsToWrite,
      const bool oq_SaveAsFile);

  static int32_t
  mh_CreateUpdatePackageDefFile(const QString &orc_Path,
                                const C_SupDefContent &orc_SupDefContent);

  static int32_t
  mh_CreateDeviceIniFile(const QString &orc_Path,
                         const QSet<QString> &orc_DeviceDefinitionPaths);

  static int32_t mh_SupDefParamAdapter(
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      C_OscSupServiceUpdatePackageV1::C_SupDefContent &orc_SupDefContent);

  static int32_t
  mh_GetUpdatePositionOfNode(const QList<uint32_t> &orc_NodesUpdateOrder,
                             const uint32_t ou32_NodeForUpdate,
                             uint32_t &oru32_UpdatePosition);

  static int32_t mh_SetNodesUpdateOrder(
      const QMap<uint32_t, uint32_t> &orc_UpdateOrderByNodes,
      QList<uint32_t> &orc_NodesUpdateOrder);
  static void mh_LoadFilesSection(QStringList &orc_Files,
                                  const uint32_t ou32_NodeCounter,
                                  const uint32_t ou32_UpdatePos,
                                  QMap<uint32_t, uint32_t> &orc_PositionMap,
                                  const QString &orc_TargetUnzipPath,
                                  C_OscXmlParserBase &orc_XmlParser,
                                  const QString &orc_BaseNodeName,
                                  const QString &orc_ElementNodeName);
  static void mh_LoadPemConfigSection(
      C_OscSuSequences::C_DoFlash &orc_DoFlash, const uint32_t ou32_NodeCounter,
      const uint32_t ou32_UpdatePos,
      QMap<uint32_t, uint32_t> &orc_PositionMap,
      const QString &orc_TargetUnzipPath, C_OscXmlParserBase &orc_XmlParser);
  static void mh_SaveFiles(const QStringList &orc_Files,
                           C_OscXmlParserBase &orc_XmlParser,
                           const QString &orc_BaseNodeName,
                           const QString &orc_ElementNodeName);
  static void mh_SavePemConfig(const C_SupDefNodeContent &orc_CurrentNode,
                               C_OscXmlParserBase &orc_XmlParser);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
