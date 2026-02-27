//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package creation

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGECREATE_H
#define C_OSCSUPSERVICEUPDATEPACKAGECREATE_H

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <map>

#include "C_OscSecurityEcdsa.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSupDefinition.hpp"
#include "C_OscSupServiceUpdatePackageBase.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscXmlParser.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QSet>
#include <QString>
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

//----------------------------------------------------------------------------------------------------------------------

class C_OscSupServiceUpdatePackageCreate
    : public C_OscSupServiceUpdatePackageBase {
public:
  static int32_t h_CreatePackageUsingPemFiles(
      const QString &orc_PackagePath,
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
      const QString &orc_TemporaryDirectory = "",
      const QByteArray &orc_EncryptNodes = QByteArray(),
      const QStringList &orc_EncryptNodesPassword = QStringList(),
      const QByteArray &orc_AddSignatureNodes = QByteArray(),
      const QStringList &orc_NodeSignaturePemFiles = QStringList());

  static int32_t h_CreatePackage(
      const QString &orc_PackagePath,
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
      const QString &orc_TemporaryDirectory = "",
      const QByteArray &orc_EncryptNodes = QByteArray(),
      const QStringList &orc_EncryptNodesPassword = QStringList(),
      const QByteArray &orc_AddSignatureNodes = QByteArray(),
      const QList<QByteArray> &orc_NodeSignatureKeys = QList<QByteArray>());

private:
  static int32_t
  mh_CheckSecurityParameters(const QByteArray &orc_EncryptNodes,
                             const QStringList &orc_EncryptNodesPassword,
                             const QByteArray &orc_SignatureNodes,
                             const QList<QByteArray> &orc_NodeSignatureKeys,
                             const uint32_t ou32_NumNodes,
                             const QString &orc_Mode,
                             const QString &orc_Function);
  static int32_t
  mh_CheckPemFileParameters(const QStringList &orc_NodeSignaturePemFiles,
                            const QByteArray &orc_SignatureNodes,
                            const uint32_t ou32_NumNodes);
  static void
  mh_AdaptSignatureParameters(const QByteArray &orc_InAddSignatureNodes,
                              const QList<QByteArray> &orc_InNodeSignatureKeys,
                              const uint32_t ou32_NodeCount,
                              QByteArray &orc_OutAddSignatureNodes,
                              QList<QByteArray> &orc_OutNodeSignatureKeys);
  static void
  mh_AdaptPemFileParameters(const QByteArray &orc_InAddSignatureNodes,
                            const QStringList &orc_InNodeSignaturePemFiles,
                            const uint32_t ou32_NodeCount,
                            QStringList &orc_OutNodeSignaturePemFiles,
                            QByteArray &orc_OutAddSignatureNodes);
  static int32_t mh_CheckParamsToCreatePackage(
      const QString &orc_PackagePath,
      const C_OscSystemDefinition &orc_SystemDefinition,
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<stw::opensyde_core::C_OscSuSequences::C_DoFlash>
          &orc_ApplicationsToWrite,
      const QByteArray &orc_EncryptNodes,
      const QStringList &orc_EncryptNodesPassword,
      const QByteArray &orc_AddSignatureNodes,
      const QList<QByteArray> &orc_NodeSignatureKeys);

  static int32_t mh_SupDefParamAdapter(
      const uint32_t ou32_ActiveBusIndex, const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_NodesUpdateOrder,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      C_OscSupDefinition &orc_SupDefContent);

  static int32_t
  mh_GetUpdatePositionOfNode(const QList<uint32_t> &orc_NodesUpdateOrder,
                             const uint32_t ou32_NodeForUpdate,
                             uint32_t &oru32_UpdatePosition);

  static void
  mh_GetSydeSecureFileNames(const C_OscSystemDefinition &orc_SystemDefinition,
                            const QString &orc_TargetPath,
                            QStringList &orc_AbsPath, QStringList &orc_RelPath);
  static void mh_AppendFlashFilesToSecureFileSections(
      const QList<stw::opensyde_core::C_OscSuSequences::C_DoFlash>
          &orc_ApplicationsToWrite,
      const QStringList &orc_NodeFoldersAbs,
      QList<QSet<QString>> &orc_SecureFiles);
  static int32_t mh_CreateDefFilesAndZipSecureFiles(
      const C_OscSystemDefinition &orc_SystemDefinition,
      C_OscSupDefinition &orc_SupDefContent, const QString &orc_PackagePathTmp,
      const QByteArray &orc_ActiveNodes,
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QSet<QString> &orc_SupFiles, const QByteArray &orc_EncryptNodes,
      const QStringList &orc_EncryptNodesPassword,
      const QByteArray &orc_AddSignatureNodes,
      const QList<QByteArray> &orc_NodeSignatureKeys);
  static int32_t mh_CreateNodesZip(const QList<QSet<QString>> &orc_SecFiles,
                                   const QStringList &orc_SecPackageFilesRel,
                                   const QStringList &orc_SecPackageFilesAbs,
                                   const QStringList &orc_NodeFoldersAbs,
                                   const QByteArray &orc_ActiveNodes,
                                   const QByteArray &orc_EncryptNodes,
                                   const QStringList &orc_EncryptNodesPassword,
                                   const uint32_t ou32_NodeCount,
                                   QSet<QString> &orc_SupFiles);
  static int32_t mh_HandleNodeDefCreation(
      const QByteArray &orc_ActiveNodes, const QStringList &orc_SecDefFilesAbs,
      const QStringList &orc_SecDefFilesRel,
      const QByteArray &orc_AddSignatureNodes, const uint32_t ou32_NodeCount,
      QList<C_OscSupNodeDefinition> &orc_SupDefNodes,
      QList<QSet<QString>> &orc_SecFiles);
  static int32_t mh_HandleSignatureCreation(
      const QStringList &orc_NodeFoldersAbs, const QByteArray &orc_ActiveNodes,
      const QByteArray &orc_AddSignatureNodes,
      const QList<QByteArray> &orc_NodeSignatureKeys,
      const uint32_t ou32_NodeCount, QList<QSet<QString>> &orc_SecFiles);
  static int32_t mh_CalcSig(const QString &orc_SourcePath,
                            const QSet<QString> &orc_SupFiles,
                            const QByteArray &orc_Key, QString &orc_Signature);
  static int32_t mh_GetPemFileContent(
      const QByteArray &orc_ActiveNodes, const QByteArray &orc_SignatureNodes,
      const QStringList &orc_NodeSignaturePemFiles,
      const uint32_t ou32_NumNodes, QByteArray &orc_PreparedSignatureNodes,
      QList<QByteArray> &orc_NodeSignatureKeys);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
