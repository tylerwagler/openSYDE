//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package loading

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGELOAD_H
#define C_OSCSUPSERVICEUPDATEPACKAGELOAD_H

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QMap>

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

class C_OscSupServiceUpdatePackageLoad
    : public C_OscSupServiceUpdatePackageBase {
public:
  static int32_t h_ProcessPackageUsingPemFiles(
      const QString &orc_PackagePath, const QString &orc_TargetUnzipPath,
      C_OscSystemDefinition &orc_SystemDefinition,
      uint32_t &oru32_ActiveBusIndex, QByteArray &orc_ActiveNodes,
      QList<uint32_t> &orc_NodesUpdateOrder,
      QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
      const bool oq_IsZip, const QByteArray &orc_DecryptNodes = QByteArray(),
      const QStringList &orc_DecryptNodesPassword = QStringList(),
      const QStringList &orc_NodeSignaturePemFiles = QStringList());

  static int32_t h_ProcessPackage(
      const QString &orc_PackagePath, const QString &orc_TargetUnzipPath,
      C_OscSystemDefinition &orc_SystemDefinition,
      uint32_t &oru32_ActiveBusIndex, QByteArray &orc_ActiveNodes,
      QList<uint32_t> &orc_NodesUpdateOrder,
      QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QStringList &orc_WarningMessages, QString &orc_ErrorMessage,
      const bool oq_IsZip, const QByteArray &orc_DecryptNodes = QByteArray(),
      const QStringList &orc_DecryptNodesPassword = QStringList(),
      const QList<QByteArray> &orc_NodeSignatureKeys = QList<QByteArray>());

private:
  static int32_t mh_CheckSupFiles(const QString &orc_PackagePath);

  static int32_t mh_CheckParamsToProcessPackage(const QString &orc_PackagePath,
                                                QString &orc_TargetUnzipPath,
                                                const bool oq_IsZip);

  static int32_t mh_SetNodesUpdateOrder(
      const QMap<uint32_t, uint32_t> &orc_UpdateOrderByNodes,
      QList<uint32_t> &orc_NodesUpdateOrder);
  static int32_t mh_UnpackAndLoadNodes(
      const C_OscSystemDefinition &orc_SystemDefinition,
      const QStringList &orc_PackageFiles, const QString &orc_TargetUnzipPath,
      const QByteArray &orc_ActiveNodes,
      const QList<uint32_t> &orc_UpdatePosition,
      QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      QList<uint32_t> &orc_NodesUpdateOrder, const QByteArray &orc_DecryptNodes,
      const QStringList &orc_DecryptNodesPassword,
      const QList<QByteArray> &orc_NodeSignatureKeys);
  static int32_t mh_UnpackNodes(const QByteArray &orc_DecryptNodes,
                                const QStringList &orc_DecryptNodesPassword,
                                const uint32_t ou32_NodeCount,
                                const QStringList &orc_PackageFiles,
                                const QString &orc_TargetUnzipPath,
                                const QStringList &orc_NodeFoldersAbs);
  static int32_t mh_VerifySignatures(
      const QList<C_OscSuSequences::C_DoFlash> &orc_ApplicationsToWrite,
      const QByteArray &orc_ActiveNodes,
      const QList<QByteArray> &orc_NodeSignatureKeys,
      const QStringList &orc_Signatures,
      const QStringList &orc_AbsSydeSecureDefFileNames);
  static int32_t
  mh_VerifySignature(const C_OscSuSequences::C_DoFlash &orc_ApplicationsToWrite,
                     const QByteArray &orc_NodeSignatureKeys,
                     const QString &orc_Signature,
                     const QString &orc_AbsSydeSecureDefFileName);
  static void
  mh_GetDigestFiles(const C_OscSuSequences::C_DoFlash &orc_ApplicationsToWrite,
                    const QString &orc_AbsSydeSecureDefFileName,
                    QSet<QString> &orc_Files);
  static void mh_GetPemFileContent(const QStringList &orc_NodeSignaturePemFiles,
                                   QList<QByteArray> &orc_NodeSignatureKeys);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
