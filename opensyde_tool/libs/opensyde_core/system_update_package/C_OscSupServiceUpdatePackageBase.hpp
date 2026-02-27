//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package base

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGEBASE_H
#define C_OSCSUPSERVICEUPDATEPACKAGEBASE_H

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <map>

#include "C_OscSecurityEcdsa.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSupDefinition.hpp"
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

class C_OscSupServiceUpdatePackageBase {
public:
  static QString h_GetPackageExtension();

protected:
  static const QString mhc_PACKAGE_EXT;
  static const QString
      mhc_PACKAGE_EXT_TMP; // intermediate directory before creating zip archive
  static const QString mhc_SUP_SYSDEF;
  static const QString mhc_INI_DEV;

  static QStringList
      mhc_WarningMessages; // global warnings e.g. if update position of active
                           // node is not available
  static QString mhc_ErrorMessage; // description of error which caused the
                                   // service update package to fail

  static int32_t mh_CheckCommonSecurityParameters(
      const QByteArray &orc_EncryptNodes,
      const QStringList &orc_EncryptNodesPassword,
      const QList<QByteArray> &orc_NodeSignatureKeys,
      const uint32_t ou32_NumNodes, const QString &orc_Mode,
      const QString &orc_Function);

  static void mh_GetSydeSecureDefFileNames(
      const C_OscSystemDefinition &orc_SystemDefinition,
      const QString &orc_TargetPath, QStringList &orc_AbsPath,
      QStringList &orc_RelPath);
  static void
  mh_GetNodeFolderNames(const C_OscSystemDefinition &orc_SystemDefinition,
                        const QString &orc_TargetPath, QStringList &orc_AbsPath,
                        QStringList &orc_RelPath);
  static void
  mh_AdaptEncryptionParameters(const QByteArray &orc_InEncryptNodes,
                               const QStringList &orc_InEncryptNodesPassword,
                               const uint32_t ou32_NodeCount,
                               QByteArray &orc_OutEncryptNodes,
                               QStringList &orc_OutEncryptNodesPassword);
  static void mh_AdaptCommonSignatureParameters(
      const QList<QByteArray> &orc_InNodeSignatureKeys,
      const uint32_t ou32_NodeCount,
      QList<QByteArray> &orc_OutNodeSignatureKeys);
  static int32_t mh_CalcDigest(
      const QString &orc_SourcePath, const QSet<QString> &orc_SupFiles,
      uint8_t (&orau8_Digest)[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH],
      const bool oq_PathsAreAbsolute);
  static int32_t mh_AddFileToDigest(const QString &orc_FilePath,
                                    C_OscSecurityEcdsa &orc_Signature);
  static int32_t mh_AddFileSectionToDigest(std::ifstream &orc_File,
                                           C_OscSecurityEcdsa &orc_Signature,
                                           const uint32_t ou32_SectionLength);
  static void mh_Init(void);
  static void mh_GetWarningsAndErrors(QStringList &orc_WarningMessages,
                                      QString &orc_ErrorMessage);
  static void mh_DigestToString(
      const uint8_t (
          &orau8_DigestBin)[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH],
      QString &orc_Digest);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
