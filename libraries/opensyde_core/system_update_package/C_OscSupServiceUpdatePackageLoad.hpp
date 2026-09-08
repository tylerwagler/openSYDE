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

class C_OscSupServiceUpdatePackageLoad :
   public C_OscSupServiceUpdatePackageBase
{
public:
   static std::error_code h_ProcessPackageUsingPemFiles(const std::string & orc_PackagePath,
                                                const std::string & orc_TargetUnzipPath,
                                                C_OscSystemDefinition & orc_SystemDefinition,
                                                uint32_t & oru32_ActiveBusIndex, std::vector<uint8_t> & orc_ActiveNodes,
                                                std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                                std::vector<std::string> & orc_WarningMessages,
                                                std::string & orc_ErrorMessage, const bool oq_IsZip,
                                                const std::vector<uint8_t> & orc_DecryptNodes = std::vector<uint8_t>(),
                                                const std::vector<std::string> & orc_DecryptNodesPassword = std::vector<std::string>(), const std::vector<std::string> & orc_NodeSignaturePemFiles = std::vector<std::string>());

   static std::error_code h_ProcessPackage(const std::string & orc_PackagePath,
                                   const std::string & orc_TargetUnzipPath,
                                   C_OscSystemDefinition & orc_SystemDefinition, uint32_t & oru32_ActiveBusIndex,
                                   std::vector<uint8_t> & orc_ActiveNodes, std::vector<uint32_t> & orc_NodesUpdateOrder,
                                   std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                   std::vector<std::string> & orc_WarningMessages,
                                   std::string & orc_ErrorMessage, const bool oq_IsZip,
                                   const std::vector<uint8_t> & orc_DecryptNodes = std::vector<uint8_t>(),
                                   const std::vector<std::string> & orc_DecryptNodesPassword = std::vector<std::string>(), const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys = std::vector<std::vector<uint8_t> >());

private:
   static std::error_code mh_CheckSupFiles(const std::string & orc_PackagePath);

   static std::error_code mh_CheckParamsToProcessPackage(const std::string & orc_PackagePath,
                                                         std::string & orc_TargetUnzipPath, const bool oq_IsZip);

   static std::error_code mh_SetNodesUpdateOrder(const std::map<uint32_t,
                                                                uint32_t> & orc_UpdateOrderByNodes,
                                                 std::vector<uint32_t> & orc_NodesUpdateOrder);
   static std::error_code mh_UnpackAndLoadNodes(const C_OscSystemDefinition & orc_SystemDefinition,
                                                const std::vector<std::string> & orc_PackageFiles,
                                                const std::string & orc_TargetUnzipPath,
                                                const std::vector<uint8_t> & orc_ActiveNodes,
                                                const std::vector<uint32_t> & orc_UpdatePosition,
                                                std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                                std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                const std::vector<uint8_t> & orc_DecryptNodes,
                                                const std::vector<std::string> & orc_DecryptNodesPassword,
                                                const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
                                                const bool oq_UseMinorVersion1);
   static std::error_code mh_UnpackNodes(const std::vector<uint8_t> & orc_DecryptNodes,
                                         const std::vector<std::string> & orc_DecryptNodesPassword,
                                         const uint32_t ou32_NodeCount,
                                         const std::vector<std::string> & orc_PackageFiles,
                                         const std::string & orc_TargetUnzipPath,
                                         const std::vector<std::string> & orc_NodeFoldersAbs);
   static std::error_code mh_VerifySignatures(const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                              const std::vector<uint8_t> & orc_ActiveNodes,
                                              const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
                                              const std::vector<std::string> & orc_Signatures,
                                              const std::vector<std::string> & orc_AbsSydeSecureDefFileNames);
   static std::error_code mh_VerifySignature(const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
                                             const std::vector<uint8_t> & orc_NodeSignatureKeys,
                                             const std::string & orc_Signature,
                                             const std::string & orc_AbsSydeSecureDefFileName);
   static void mh_GetDigestFiles(const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
                                 const std::string & orc_AbsSydeSecureDefFileName,
                                 std::set<std::string> & orc_Files);
   static void mh_GetPemFileContent(const std::vector<std::string> & orc_NodeSignaturePemFiles,
                                    std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
