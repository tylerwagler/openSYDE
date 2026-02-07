//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package loading

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGELOAD_H
#define C_OSCSUPSERVICEUPDATEPACKAGELOAD_H

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <map>

#include "stwtypes.hpp"
#include <QString>
#include <QStringList>
#include "C_OscXmlParser.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSecurityEcdsa.hpp"
#include "C_OscSupDefinition.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSupServiceUpdatePackageBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------

class C_OscSupServiceUpdatePackageLoad :
   public C_OscSupServiceUpdatePackageBase
{
public:
   static int32_t h_ProcessPackageUsingPemFiles(const QString & orc_PackagePath,
                                                const QString & orc_TargetUnzipPath,
                                                C_OscSystemDefinition & orc_SystemDefinition,
                                                uint32_t & oru32_ActiveBusIndex, std::vector<uint8_t> & orc_ActiveNodes,
                                                std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                                QStringList & orc_WarningMessages,
                                                QString & orc_ErrorMessage, const bool oq_IsZip,
                                                const std::vector<uint8_t> & orc_DecryptNodes = std::vector<uint8_t>(),
                                                const QStringList & orc_DecryptNodesPassword = QStringList(), const QStringList & orc_NodeSignaturePemFiles = QStringList());

   static int32_t h_ProcessPackage(const QString & orc_PackagePath,
                                   const QString & orc_TargetUnzipPath,
                                   C_OscSystemDefinition & orc_SystemDefinition, uint32_t & oru32_ActiveBusIndex,
                                   std::vector<uint8_t> & orc_ActiveNodes, std::vector<uint32_t> & orc_NodesUpdateOrder,
                                   std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                   QStringList & orc_WarningMessages,
                                   QString & orc_ErrorMessage, const bool oq_IsZip,
                                   const std::vector<uint8_t> & orc_DecryptNodes = std::vector<uint8_t>(),
                                   const QStringList & orc_DecryptNodesPassword = QStringList(), const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys = std::vector<std::vector<uint8_t> >());

private:
   static int32_t mh_CheckSupFiles(const QString & orc_PackagePath);

   static int32_t mh_CheckParamsToProcessPackage(const QString & orc_PackagePath,
                                                 QString & orc_TargetUnzipPath, const bool oq_IsZip);

   static int32_t mh_SetNodesUpdateOrder(const std::map<uint32_t,
                                                        uint32_t> & orc_UpdateOrderByNodes,
                                         std::vector<uint32_t> & orc_NodesUpdateOrder);
   static int32_t mh_UnpackAndLoadNodes(const C_OscSystemDefinition & orc_SystemDefinition,
                                        const QStringList & orc_PackageFiles,
                                        const QString & orc_TargetUnzipPath,
                                        const std::vector<uint8_t> & orc_ActiveNodes,
                                        const std::vector<uint32_t> & orc_UpdatePosition,
                                        std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                        std::vector<uint32_t> & orc_NodesUpdateOrder,
                                        const std::vector<uint8_t> & orc_DecryptNodes,
                                        const QStringList & orc_DecryptNodesPassword,
                                        const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);
   static int32_t mh_UnpackNodes(const std::vector<uint8_t> & orc_DecryptNodes,
                                 const QStringList & orc_DecryptNodesPassword,
                                 const uint32_t ou32_NodeCount,
                                 const QStringList & orc_PackageFiles,
                                 const QString & orc_TargetUnzipPath,
                                 const QStringList & orc_NodeFoldersAbs);
   static int32_t mh_VerifySignatures(const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                      const std::vector<uint8_t> & orc_ActiveNodes,
                                      const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
                                      const QStringList & orc_Signatures,
                                      const QStringList & orc_AbsSydeSecureDefFileNames);
   static int32_t mh_VerifySignature(const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
                                     const std::vector<uint8_t> & orc_NodeSignatureKeys,
                                     const QString & orc_Signature,
                                     const QString & orc_AbsSydeSecureDefFileName);
   static void mh_GetDigestFiles(const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
                                 const QString & orc_AbsSydeSecureDefFileName,
                                 std::set<QString> & orc_Files);
   static void mh_GetPemFileContent(const QStringList & orc_NodeSignaturePemFiles,
                                    std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
