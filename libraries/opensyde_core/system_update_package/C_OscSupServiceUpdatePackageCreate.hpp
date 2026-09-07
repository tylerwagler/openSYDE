//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package creation

   Provides functions to create and unpack service update packages
   in openSYDE

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSERVICEUPDATEPACKAGECREATE_H
#define C_OSCSUPSERVICEUPDATEPACKAGECREATE_H

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <map>

#include "stwtypes.hpp"
#include <string>
#include <system_error>
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

class C_OscSupServiceUpdatePackageCreate :
   public C_OscSupServiceUpdatePackageBase
{
public:
   static std::error_code h_CreatePackageUsingPemFiles(const std::string & orc_PackagePath,
                                               const C_OscSystemDefinition & orc_SystemDefinition,
                                               const uint32_t ou32_ActiveBusIndex,
                                               const std::vector<uint8_t> & orc_ActiveNodes,
                                               const std::vector<uint32_t> & orc_NodesUpdateOrder,
                                               const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                               stw::scl::C_SclStringList & orc_WarningMessages,
                                               std::string & orc_ErrorMessage,
                                               const std::string & orc_TemporaryDirectory = "",
                                               const std::vector<uint8_t> & orc_EncryptNodes = std::vector<uint8_t>(),
                                               const std::vector<std::string> & orc_EncryptNodesPassword = std::vector<std::string>(), const std::vector<uint8_t> & orc_AddSignatureNodes = std::vector<uint8_t>(), const std::vector<std::string> & orc_NodeSignaturePemFiles = std::vector<std::string>());

   static std::error_code h_CreatePackage(const std::string & orc_PackagePath,
                                  const C_OscSystemDefinition & orc_SystemDefinition,
                                  const uint32_t ou32_ActiveBusIndex, const std::vector<uint8_t> & orc_ActiveNodes,
                                  const std::vector<uint32_t> & orc_NodesUpdateOrder,
                                  const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                  stw::scl::C_SclStringList & orc_WarningMessages,
                                  std::string & orc_ErrorMessage,
                                  const std::string & orc_TemporaryDirectory = "",
                                  const std::vector<uint8_t> & orc_EncryptNodes = std::vector<uint8_t>(),
                                  const std::vector<std::string> & orc_EncryptNodesPassword = std::vector<std::string>(), const std::vector<uint8_t> & orc_AddSignatureNodes = std::vector<uint8_t>(), const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys = std::vector<std::vector<uint8_t> >());

private:
   static std::error_code mh_CheckSecurityParameters(const std::vector<uint8_t> & orc_EncryptNodes,
                                                     const std::vector<std::string> & orc_EncryptNodesPassword,
                                                     const std::vector<uint8_t> & orc_SignatureNodes,
                                                     const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
                                                     const uint32_t ou32_NumNodes, const std::string & orc_Mode,
                                                     const std::string & orc_Function);
   static std::error_code mh_CheckPemFileParameters(const std::vector<std::string> & orc_NodeSignaturePemFiles,
                                                    const std::vector<uint8_t> & orc_SignatureNodes,
                                                    const uint32_t ou32_NumNodes);
   static void mh_AdaptSignatureParameters(const std::vector<uint8_t> & orc_InAddSignatureNodes,
                                           const std::vector<std::vector<uint8_t> > & orc_InNodeSignatureKeys,
                                           const uint32_t ou32_NodeCount,
                                           std::vector<uint8_t> & orc_OutAddSignatureNodes,
                                           std::vector<std::vector<uint8_t> > & orc_OutNodeSignatureKeys);
   static void mh_AdaptPemFileParameters(const std::vector<uint8_t> & orc_InAddSignatureNodes,
                                         const std::vector<std::string> & orc_InNodeSignaturePemFiles,
                                         const uint32_t ou32_NodeCount,
                                         std::vector<std::string> & orc_OutNodeSignaturePemFiles,
                                         std::vector<uint8_t> & orc_OutAddSignatureNodes);
   static std::error_code mh_CheckParamsToCreatePackage(const std::string & orc_PackagePath,
                                                        const C_OscSystemDefinition & orc_SystemDefinition,
                                                        const uint32_t ou32_ActiveBusIndex,
                                                        const std::vector<uint8_t> & orc_ActiveNodes,
                                                        const std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                        const std::vector<stw::opensyde_core::C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, const std::vector<uint8_t> & orc_EncryptNodes, const std::vector<std::string> & orc_EncryptNodesPassword, const std::vector<uint8_t> & orc_AddSignatureNodes, const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);

   static std::error_code mh_SupDefParamAdapter(const uint32_t ou32_ActiveBusIndex,
                                                const std::vector<uint8_t> & orc_ActiveNodes,
                                                const std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                                C_OscSupDefinition & orc_SupDefContent);

   static std::error_code mh_GetUpdatePositionOfNode(const std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                     const uint32_t ou32_NodeForUpdate,
                                                     uint32_t & oru32_UpdatePosition);

   static void mh_GetSydeSecureFileNames(const C_OscSystemDefinition & orc_SystemDefinition,
                                         const std::string & orc_TargetPath,
                                         std::vector<std::string> & orc_AbsPath,
                                         std::vector<std::string> & orc_RelPath);
   static void mh_AppendFlashFilesToSecureFileSections(
      const std::vector<stw::opensyde_core::C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
      const std::vector<std::string> & orc_NodeFoldersAbs,
      std::vector<std::set<std::string> > & orc_SecureFiles);
   static std::error_code mh_CreateDefFilesAndZipSecureFiles(const C_OscSystemDefinition & orc_SystemDefinition,
                                                             C_OscSupDefinition & orc_SupDefContent,
                                                             const std::string & orc_PackagePathTmp,
                                                             const std::vector<uint8_t> & orc_ActiveNodes,
                                                             const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::set<std::string> & orc_SupFiles, const std::vector<uint8_t> & orc_EncryptNodes, const std::vector<std::string> & orc_EncryptNodesPassword, const std::vector<uint8_t> & orc_AddSignatureNodes, const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);
   static std::error_code mh_CreateNodesZip(const std::vector<std::set<std::string> > & orc_SecFiles,
                                            const std::vector<std::string> & orc_SecPackageFilesRel,
                                            const std::vector<std::string> & orc_SecPackageFilesAbs,
                                            const std::vector<std::string> & orc_NodeFoldersAbs,
                                            const std::vector<uint8_t> & orc_ActiveNodes,
                                            const std::vector<uint8_t> & orc_EncryptNodes,
                                            const std::vector<std::string> & orc_EncryptNodesPassword,
                                            const uint32_t ou32_NodeCount, std::set<std::string> & orc_SupFiles);
   static std::error_code mh_HandleNodeDefCreation(const std::vector<uint8_t> & orc_ActiveNodes,
                                                   const std::vector<std::string> & orc_SecDefFilesAbs,
                                                   const std::vector<std::string> & orc_SecDefFilesRel,
                                                   const std::vector<uint8_t> & orc_AddSignatureNodes,
                                                   const uint32_t ou32_NodeCount,
                                                   std::vector<C_OscSupNodeDefinition> & orc_SupDefNodes,
                                                   std::vector<std::set<std::string> > & orc_SecFiles,
                                                   const bool oq_UseMinorVersion1);
   static std::error_code mh_HandleSignatureCreation(const std::vector<std::string> & orc_NodeFoldersAbs,
                                                     const std::vector<uint8_t> & orc_ActiveNodes,
                                                     const std::vector<uint8_t> & orc_AddSignatureNodes,
                                                     const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
                                                     const uint32_t ou32_NodeCount,
                                                     std::vector<std::set<std::string> > & orc_SecFiles);
   static std::error_code mh_CalcSig(const std::string & orc_SourcePath,
                                     const std::set<std::string> & orc_SupFiles, const std::vector<uint8_t> & orc_Key,
                                     std::string & orc_Signature);
   static std::error_code mh_GetPemFileContent(const std::vector<uint8_t> & orc_ActiveNodes,
                                               const std::vector<uint8_t> & orc_SignatureNodes,
                                               const std::vector<std::string> & orc_NodeSignaturePemFiles,
                                               const uint32_t ou32_NumNodes,
                                               std::vector<uint8_t> & orc_PreparedSignatureNodes,
                                               std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);
   static bool mh_CheckMinorVersion1Required(const std::vector<uint8_t> & orc_ActiveNodes,
                                             const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
