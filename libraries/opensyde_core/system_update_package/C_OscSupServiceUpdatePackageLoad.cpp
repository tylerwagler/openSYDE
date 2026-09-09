//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package loading

   For details cf. documentation in .h file.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <fstream>
#include <iterator>
#include <cstdint>
#include "stwerrors.hpp"
#include <string>
#include <system_error>
#include "C_OscErrorCategory.hpp"
#include "C_OscSupServiceUpdatePackageLoad.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSystemDefinitionFiler.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscSuSequences.hpp"
#include "TglFile.hpp"
#include "C_SclIniFile.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscSecurityAesFile.hpp"
#include "C_OscSupSignatureFiler.hpp"
#include "C_OscSupDefinitionFiler.hpp"
#include "C_OscSecurityPemSecUpdate.hpp"
#include "C_OscSupNodeDefinitionFiler.hpp"
#include "C_OscSpaServicePackageLoadUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;
using namespace std;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Unpack an update package

   We have to unzip the package persistent to target folder on disk (orc_TargetUnzipPath) because we need
   the application files for update. If orc_TargetUnzipPath folder exists already it will be deleted.
   Will create the target folder (recursively if it needs to be).

   Result of function after successful execution:
   * unzipped package at given target path with the following contents:
       * system definition as class instance
       * index of bus the client is connected to
       * information which nodes are active or not
       * update order of active nodes
       * path of application files for active nodes

   Assumptions:
   * valid and compatible service update package created by C_OscSuServiceUpdatePackage::h_CreatePackage function
   * write permission of target path for unzipped package

   \param[in]   orc_PackagePath              full path of package
                                             (with constant extension ".syde_sup", e.g. 012345.syde_sup)
   \param[in]   orc_TargetUnzipPath          folder of path to unzip the service update package
                                             (works with or without trailing path delimiter)
   \param[out]  orc_SystemDefinition         system definition (class instance - no file!)
   \param[out]  oru32_ActiveBusIndex         index of bus the client is connected to
   \param[out]  orc_ActiveNodes              list of all nodes contains information which node is available for update
   \param[out]  orc_NodesUpdateOrder         update order of active nodes
   \param[out]  orc_ApplicationsToWrite      files for updating nodes
   \param[out]  orc_WarningMessages          warning messages for imported package (empty list if no warnings)
   \param[out]  orc_ErrorMessage             error message in case of failure (empty string if no error)
   \param[in]   oq_IsZip                     true: given package is ".syde_sup" format
                                             false: given package is just a plain directory
   \param[in]   orc_DecryptNodes             list of all nodes contains information which node is decrypted
                                             e.g. {0} or {0,0,0} means no decryption for all or individual nodes
                                             {1} or {1,1,1} means decryption for all or individual nodes
   \param[in]   orc_DecryptNodesPassword     list of all nodes contains information which node uses which password
                                             e.g. {pwd} or {pwd1, pwd2, pwd3} for all or individual nodes
   \param[in]   orc_NodeSignaturePemFiles    list of all nodes contains information which pem file to use for which node
                                             e.g. {pem} or {pem1, pem2, pem3} for all or individual nodes

   \return
   Errc::success    success
   Errc::config     could not find update package archive or update package directory
   Errc::rd_wr      could not unzip update package from disk to target path
   Errc::busy       could not erase pre-existing target path (note: can result in partially erased target path)
   Errc::range      error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::noact      invalid signature or error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::overflow   error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::default_   error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::checksum   size of orc_EncryptNodes does not match system definition
                    or wrong password
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::h_ProcessPackageUsingPemFiles(const std::string & orc_PackagePath,
                                                                        const std::string & orc_TargetUnzipPath,
                                                                        C_OscSystemDefinition & orc_SystemDefinition,
                                                                        uint32_t & oru32_ActiveBusIndex,
                                                                        std::vector<uint8_t> & orc_ActiveNodes,
                                                                        std::vector<uint32_t> & orc_NodesUpdateOrder,
                                                                        std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::vector<std::string> & orc_WarningMessages, std::string & orc_ErrorMessage, const bool oq_IsZip, const std::vector<uint8_t> & orc_DecryptNodes, const std::vector<std::string> & orc_DecryptNodesPassword,
                                                                        const std::vector<std::string> & orc_NodeSignaturePemFiles)
{
   mh_Init();

   std::vector<std::vector<uint8_t> > c_NodeSignatureKeys;
   mh_GetPemFileContent(orc_NodeSignaturePemFiles,
                        c_NodeSignatureKeys);
   const std::error_code c_Retval = h_ProcessPackage(orc_PackagePath, orc_TargetUnzipPath, orc_SystemDefinition,
                                                     oru32_ActiveBusIndex,
                                                     orc_ActiveNodes,
                                                     orc_NodesUpdateOrder, orc_ApplicationsToWrite,
                                                     orc_WarningMessages,
                                                     orc_ErrorMessage, oq_IsZip, orc_DecryptNodes,
                                                     orc_DecryptNodesPassword, c_NodeSignatureKeys);

   mh_GetWarningsAndErrors(orc_WarningMessages, orc_ErrorMessage);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Unpack an update package

   We have to unzip the package persistent to target folder on disk (orc_TargetUnzipPath) because we need
   the application files for update. If orc_TargetUnzipPath folder exists already it will be deleted.
   Will create the target folder (recursively if it needs to be).

   Result of function after successful execution:
   * unzipped package at given target path with the following contents:
       * system definition as class instance
       * index of bus the client is connected to
       * information which nodes are active or not
       * update order of active nodes
       * path of application files for active nodes

   Assumptions:
   * valid and compatible service update package created by C_OscSuServiceUpdatePackage::h_CreatePackage function
   * write permission of target path for unzipped package

   \param[in]   orc_PackagePath           full path of package
                                          (with constant extension ".syde_sup", e.g. 012345.syde_sup)
   \param[in]   orc_TargetUnzipPath       folder of path to unzip the service update package
                                          (works with or without trailing path delimiter)
   \param[out]  orc_SystemDefinition      system definition (class instance - no file!)
   \param[out]  oru32_ActiveBusIndex      index of bus the client is connected to
   \param[out]  orc_ActiveNodes           list of all nodes contains information which node is available for update
   \param[out]  orc_NodesUpdateOrder      update order of active nodes
   \param[out]  orc_ApplicationsToWrite   files for updating nodes
   \param[out]  orc_WarningMessages       warning messages for imported package (empty list if no warnings)
   \param[out]  orc_ErrorMessage          error message in case of failure (empty string if no error)
   \param[in]   oq_IsZip                  true: given package is ".syde_sup" format
                                          false: given package is just a plain directory
   \param[in]   orc_DecryptNodes          list of all nodes contains information which node is decrypted
   \param[in]   orc_DecryptNodesPassword  list of all nodes contains information which node uses which password
   \param[in]   orc_NodeSignatureKeys     list of all nodes contains information which key to use for which node

   \return
   Errc::success    success
   Errc::config     could not find update package archive or update package directory
                    or package file version unknown
   Errc::rd_wr      could not unzip update package from disk to target path or other file read issues
   Errc::busy       could not erase pre-existing target path (note: can result in partially erased target path)
   Errc::range      error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::noact      invalid signature or error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::overflow   error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::default_   error code of a called core function
                    (should not occur for valid and compatible service update package)
   Errc::checksum   size of orc_EncryptNodes does not match system definition
                    or wrong password
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::h_ProcessPackage(const std::string & orc_PackagePath,
                                                           const std::string & orc_TargetUnzipPath,
                                                           C_OscSystemDefinition & orc_SystemDefinition,
                                                           uint32_t & oru32_ActiveBusIndex,
                                                           vector<uint8_t> & orc_ActiveNodes,
                                                           vector<uint32_t> & orc_NodesUpdateOrder,
                                                           vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::vector<std::string> & orc_WarningMessages, std::string & orc_ErrorMessage, const bool oq_IsZip, const std::vector<uint8_t> & orc_DecryptNodes, const std::vector<std::string> & orc_DecryptNodesPassword,
                                                           const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys)
{
   std::error_code c_Return = Errc::success;

   std::string c_TargetUnzipPath = C_OscSpaServicePackageLoadUtil::h_GetUnzipPath(orc_TargetUnzipPath);

   mh_Init();

   // as a precaution of inconsistent data, clear the input container
   orc_ActiveNodes.clear();
   orc_NodesUpdateOrder.clear();
   orc_ApplicationsToWrite.clear();

   c_Return = mh_CheckParamsToProcessPackage(orc_PackagePath, c_TargetUnzipPath, oq_IsZip);

   // load service update package definition
   if (!c_Return)
   {
      uint32_t u32_FileVersion;
      std::string c_FilePackagePath;
      std::vector<std::string> c_PackageFiles;
      std::vector<uint32_t> c_UpdatePosition;

      c_Return = C_OscSupDefinitionFiler::h_LoadUpdatePackageDefFile(c_TargetUnzipPath, oq_IsZip, orc_PackagePath,
                                                                     u32_FileVersion,
                                                                     c_FilePackagePath,
                                                                     oru32_ActiveBusIndex, orc_ActiveNodes,
                                                                     c_UpdatePosition,
                                                                     c_PackageFiles);
      if (!c_Return)
      {
         if (u32_FileVersion == 1U)
         {
            mhc_ErrorMessage =
               "V1 service update packages are no longer supported. Re-create the package with a current openSYDE.";
            osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
            c_Return = Errc::config;
         }
         else if ((u32_FileVersion == 2U) || (u32_FileVersion == 0x102U))
         {
            if (!oq_IsZip)
            {
               mhc_ErrorMessage = "Unsupported non zip format with v2 file version.";
               osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
               c_Return = Errc::config;
            }
            else
            {
               // load system definition (has constant name) for active nodes; the unzip
               // directory is itself the device-bundle root that the scanner walks.
               const std::string c_SysDefPath = c_TargetUnzipPath + mhc_SUP_SYSDEF;

               c_Return = C_OscSystemDefinitionFiler::h_LoadSystemDefinitionFile(orc_SystemDefinition, c_SysDefPath,
                                                                                 c_TargetUnzipPath, true, nullptr,
                                                                                 &orc_ActiveNodes, true); // skip
                                                                                                          // content
               if (!c_Return)
               {
                  c_Return = mh_UnpackAndLoadNodes(orc_SystemDefinition, c_PackageFiles, c_TargetUnzipPath,
                                                   orc_ActiveNodes, c_UpdatePosition, orc_ApplicationsToWrite,
                                                   orc_NodesUpdateOrder, orc_DecryptNodes, orc_DecryptNodesPassword,
                                                   orc_NodeSignatureKeys, (u32_FileVersion == 0x102U));
               }
            }
         }
         else
         {
            //Unknown version
            mhc_ErrorMessage = "Unknown file version: " + std::to_string(u32_FileVersion) + ".";
            osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
            c_Return = Errc::config;
         }
      }
      else
      {
         mhc_ErrorMessage = "Could not load service update package definition file.";
         // would be better to add path of definition file to this error message, but what path is used is handled
         // by h_LoadUpdatePackageDefFile and therefore not known here
         osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
         c_Return = Errc::rd_wr;
      }
   }

   // get "other accepted names" for active nodes
   if (!c_Return)
   {
      for (uint8_t u8_Node = 0U; u8_Node < orc_SystemDefinition.c_Nodes.size(); u8_Node++)
      {
         if ((orc_ActiveNodes[u8_Node] == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE) &&
             (orc_SystemDefinition.c_Nodes[u8_Node].pc_DeviceDefinition != nullptr))
         {
            const C_OscNode & rc_CurNode = orc_SystemDefinition.c_Nodes[u8_Node];
            tgl_assert(rc_CurNode.u32_SubDeviceIndex < rc_CurNode.pc_DeviceDefinition->c_SubDevices.size());

            orc_ApplicationsToWrite[u8_Node].c_OtherAcceptedDeviceNames =
               rc_CurNode.pc_DeviceDefinition->c_SubDevices[rc_CurNode.u32_SubDeviceIndex].c_OtherAcceptedNames;
         }
      }
   }

   mh_GetWarningsAndErrors(orc_WarningMessages, orc_ErrorMessage);

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  This function shall check if certain files are existent in the update package. (internal function)

   The update package can be a created directory after unpacking a ".syde_sup" file or a directory itself,
   if there was no ".syde_sup" file.   In order to successfully perform the actual update we need to make sure that
   whether the "syde_sup" nor the plain directory got "corrupted". There are some defensive checks for files which are
   necessary to perform an update routine and if those are missing, we want to handle this gracefully.
   Files we need to make sure are present:
      * ".syde_sysdef" //the expected name MUST NOT be changed
      * ".syde_defsup" //the expected name MUST NOT be changed
      * devices.ini    //the expected name MUST NOT be changed
   Those are the absolute basics we need. This is an all or nothing approach. If one file is missing -> fail.

   This function does not claim to be 100% defensive. We surely could go more into detail regarding the checks for
   certain files, but for now it looks like the existing engines handle everything gracefully.

   \param[in]  orc_PackagePath   path to unzipped package folder or plain directory

   \retval   Errc::success    directory contains all necessary files
   \retval   Errc::default_   at least one file is missing in given directory
                              (due to lack of alternatives C_DEFAULT was chosen to have a unique error to redirect to
                               SYDEsup specific error codes in C_SYDEsup::Update)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_CheckSupFiles(const std::string & orc_PackagePath)
{
   std::error_code c_Return = Errc::success;

   std::vector<std::string> c_NecessaryFiles; //those are the files we look for

   c_NecessaryFiles.push_back(C_OscSupDefinitionFiler::hc_PACKAGE_UPDATE_DEF); //".syde_supdef"
   c_NecessaryFiles.push_back(mhc_SUP_SYSDEF);                                 //".syde_sysdef"

   c_Return = C_OscSpaServicePackageLoadUtil::h_SearchFilesInPath(orc_PackagePath,
                                                                                    c_NecessaryFiles);

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check params to process package

   \param[in]      orc_PackagePath        (see function h_ProcessPackage)
   \param[in,out]  orc_TargetUnzipPath    (see function h_ProcessPackage)
   \param[in]      oq_IsZip               (see function h_ProcessPackage)

   \return
   std::error_code

   \retval   Errc::success   success
   \retval   Errc::config    could not find update package archive or update package directory
   \retval   Errc::busy      could not erase pre-existing target path (note: can result in partially erased target path)
   \retval   Errc::rd_wr     could not unzip update package from disk to target path
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_CheckParamsToProcessPackage(const std::string & orc_PackagePath,
                                                                                 std::string & orc_TargetUnzipPath,
                                                                                 const bool oq_IsZip)
{
   std::error_code c_Return = Errc::success;

   if (oq_IsZip)
   {
      c_Return = C_OscSpaServicePackageLoadUtil::h_CheckParamsToProcessZipPackage(
                                      orc_PackagePath,
                                      orc_TargetUnzipPath,
                                      "Unpacking Update Package",
                                      mhc_ErrorMessage);

      //check if all files are present
      if (!c_Return)
      {
         c_Return = mh_CheckSupFiles(orc_TargetUnzipPath);
         if (c_Return)
         {
            mhc_ErrorMessage = "Could not find necessary files within \"" + orc_TargetUnzipPath +
                               "\" for update package to be complete.";
            osc_write_log_error("Unpacking Update Package", mhc_ErrorMessage);
         }
      }
   }
   else
   {
      //no zip -> we have a plain directory. Does it even exist?
      if (TglDirectoryExists(orc_PackagePath) == true)
      {
         //We need the base path for handling relative paths of files to transfer:
         orc_TargetUnzipPath = TglFileIncludeTrailingDelimiter(orc_PackagePath);

         //check if necessary files are present
         c_Return = mh_CheckSupFiles(orc_PackagePath);
         if (c_Return)
         {
            mhc_ErrorMessage = "Could not find necessary files within \"" + orc_PackagePath +
                               "\" for update package to be complete.";
            osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
         }
      }
      else
      {
         mhc_ErrorMessage = "Directory \"" + orc_PackagePath + "\" does not exist.";
         osc_write_log_error("Processing Update Package", mhc_ErrorMessage);
         c_Return = Errc::config;
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapts nodes update order by interchanging index with value to match openSYDE API functions.

   \param[in]   orc_UpdateOrderByNodes    key is node position, value is update position
   \param[out]  orc_NodesUpdateOrder      update position is index and value is node position

   \return
   Errc::success   successful
   Errc::warn      contains no nodes for update order
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_SetNodesUpdateOrder(
   const map<uint32_t, uint32_t> & orc_UpdateOrderByNodes, vector<uint32_t> & orc_NodesUpdateOrder)
{
   std::error_code c_Return = Errc::success;

   if (orc_UpdateOrderByNodes.size() == 0)
   {
      c_Return = Errc::warn;
   }
   else
   {
      orc_NodesUpdateOrder.resize(orc_UpdateOrderByNodes.size());
      map<uint32_t, uint32_t>::const_iterator c_Iter;
      for (c_Iter = orc_UpdateOrderByNodes.begin(); c_Iter != orc_UpdateOrderByNodes.end(); ++c_Iter)
      {
         // interchange index with value
         orc_NodesUpdateOrder[c_Iter->second] = c_Iter->first;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Unpack and load nodes

   \param[in]      orc_SystemDefinition      System definition
   \param[in]      orc_PackageFiles          Package files
   \param[in]      orc_TargetUnzipPath       Target unzip path
   \param[in]      orc_ActiveNodes           Active nodes
   \param[in]      orc_UpdatePosition        Update position
   \param[in,out]  orc_ApplicationsToWrite   Applications to write
   \param[in,out]  orc_NodesUpdateOrder      Nodes update order
   \param[in]      orc_DecryptNodes          list of all nodes contains information which node is decrypted
   \param[in]      orc_DecryptNodesPassword  list of all nodes contains information which node uses which password
   \param[in]      orc_NodeSignatureKeys     list of all nodes contains information which key to use for which node
   \param[in]      oq_UseMinorVersion1       Use minor version 1

   \return
   std::error_code

   \retval   Errc::success    successful
   \retval   Errc::warn       contains no nodes for update order
   \retval   Errc::rd_wr      could not unpack archive to target path
   \retval   Errc::noact      invalid signature of a sub-package
   \retval   Errc::checksum   size of orc_EncryptNodes does not match system definition
                              or wrong password
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_UnpackAndLoadNodes(
   const C_OscSystemDefinition & orc_SystemDefinition, const std::vector<std::string> & orc_PackageFiles,
   const std::string & orc_TargetUnzipPath, const std::vector<uint8_t> & orc_ActiveNodes,
   const std::vector<uint32_t> & orc_UpdatePosition,
   std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::vector<uint32_t> & orc_NodesUpdateOrder,
   const std::vector<uint8_t> & orc_DecryptNodes, const std::vector<std::string> & orc_DecryptNodesPassword,
   const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys, const bool oq_UseMinorVersion1)
{
   std::error_code c_Return = mh_CheckCommonSecurityParameters(orc_DecryptNodes, orc_DecryptNodesPassword,
                                                               orc_NodeSignatureKeys,
                                                               static_cast<uint32_t>(orc_SystemDefinition.c_Nodes.size()), "decrypt",
                                                               "Processing Update Package");

   if (!c_Return)
   {
      std::vector<std::string> c_NodeFoldersRel;
      std::vector<std::string> c_NodeFoldersAbs;

      mh_GetNodeFolderNames(orc_SystemDefinition, orc_TargetUnzipPath, c_NodeFoldersAbs, c_NodeFoldersRel);

      c_Return =
         mh_UnpackNodes(orc_DecryptNodes, orc_DecryptNodesPassword,
                        static_cast<uint32_t>(orc_SystemDefinition.c_Nodes.size()),
                        orc_PackageFiles, orc_TargetUnzipPath, c_NodeFoldersAbs);

      if (!c_Return)
      {
         std::map<uint32_t, uint32_t> c_UpdateOrderByNodes; // to store node update positions which are represented by
                                                            // index at
         std::vector<std::string> c_RelFiles;
         std::vector<std::string> c_AbsFiles;
         std::vector<std::string> c_Signatures;
         mh_GetSydeSecureDefFileNames(orc_SystemDefinition,
                                      orc_TargetUnzipPath, c_AbsFiles, c_RelFiles);
         c_Return = C_OscSupNodeDefinitionFiler::h_LoadNodes(c_AbsFiles, c_NodeFoldersAbs, orc_ActiveNodes,
                                                             orc_ApplicationsToWrite, c_UpdateOrderByNodes,
                                                             orc_UpdatePosition, c_Signatures, oq_UseMinorVersion1);

         if (!c_Return)
         {
            c_Return = mh_SetNodesUpdateOrder(c_UpdateOrderByNodes, orc_NodesUpdateOrder);
         }

         if (!c_Return)
         {
            c_Return =
               mh_VerifySignatures(orc_ApplicationsToWrite, orc_ActiveNodes,
                                   orc_NodeSignatureKeys, c_Signatures, c_AbsFiles);
         }
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Unpack nodes

   \param[in]  orc_DecryptNodes           list of all nodes contains information which node is decrypted
   \param[in]  orc_DecryptNodesPassword   list of all nodes contains information which node uses which password
   \param[in]  ou32_NodeCount             Node count
   \param[in]  orc_PackageFiles           Package files
   \param[in]  orc_TargetUnzipPath        Target unzip path
   \param[in]  orc_NodeFoldersAbs         Node folders abs

   \return
   std::error_code

   \retval   Errc::success    Encrypted zip file unpacked
   \retval   Errc::rd_wr      input file does not exist or could not be loaded
                              could not unpack archive to target path
   \retval   Errc::busy       Problems with deleting the temporary file
   \retval   Errc::config     input file size is not a multiple of 16
   \retval   Errc::checksum   input file is invalid; PKCS#7 value is > 16 or > file size (checked after decryption)
   \retval   Errc::noact      output file already exists or could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_UnpackNodes(const std::vector<uint8_t> & orc_DecryptNodes,
                                                                 const std::vector<std::string> & orc_DecryptNodesPassword, const uint32_t ou32_NodeCount,
                                                                 const std::vector<std::string> & orc_PackageFiles,
                                                                 const std::string & orc_TargetUnzipPath,
                                                                 const std::vector<std::string> & orc_NodeFoldersAbs)
{
   std::error_code c_Return = Errc::success;

   std::vector<uint8_t> c_DecryptNodes;
   std::vector<std::string> c_DecryptNodesPassword;
   mh_AdaptEncryptionParameters(orc_DecryptNodes, orc_DecryptNodesPassword,
                                ou32_NodeCount, c_DecryptNodes, c_DecryptNodesPassword);
   for (uint32_t u32_ItPackage = 0UL; (u32_ItPackage < orc_PackageFiles.size()) && (!c_Return);
        ++u32_ItPackage)
   {
      if (!orc_PackageFiles[u32_ItPackage].empty())
      {
         const std::string c_FinalZipPath = orc_TargetUnzipPath + orc_PackageFiles[u32_ItPackage];
         const std::string c_TargetFolder = orc_NodeFoldersAbs[u32_ItPackage];
         //TglCreateDirectory returns plain 0/non-zero, not an STW error code, so map it
         //explicitly rather than bridging it as though it were one
         if (TglCreateDirectory(c_TargetFolder) != 0)
         {
            osc_write_log_error("Unpacking package",
                                "Could not create folder \"" + c_TargetFolder + "\".");
            c_Return = Errc::rd_wr;
         }
         if (!c_Return)
         {
            if (c_DecryptNodes[u32_ItPackage] == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE)
            {
               c_Return = C_OscSecurityAesFile::h_UnpackEncryptedZipFile(c_FinalZipPath, c_TargetFolder,
                                                                         c_DecryptNodesPassword[u32_ItPackage],
                                                                         &mhc_ErrorMessage);
            }
            else
            {
               c_Return = C_OscZipFile::h_UnpackZipFile(c_FinalZipPath, c_TargetFolder,
                                                                          &mhc_ErrorMessage);
            }
         }
      }
   }
   if (c_Return)
   {
      mhc_ErrorMessage = "Could not unpack all files. Maybe wrong password or missing permissions?";
      osc_write_log_error("unpack all node files", mhc_ErrorMessage);
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Verify signatures

   \param[in]  orc_ApplicationsToWrite          Applications to write
   \param[in]  orc_ActiveNodes                  Active nodes
   \param[in]  orc_NodeSignatureKeys            Node signature keys
   \param[in]  orc_Signatures                   Signatures
   \param[in]  orc_AbsSydeSecureDefFileNames    Abs syde secure def file names

   \return
   std::error_code

   \retval   Errc::success   No err
   \retval   Errc::noact     invalid signature or signature check failed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_VerifySignatures(
   const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
   const std::vector<uint8_t> & orc_ActiveNodes, const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys,
   const std::vector<std::string> & orc_Signatures, const std::vector<std::string> & orc_AbsSydeSecureDefFileNames)
{
   std::error_code c_Retval = Errc::success;

   std::vector<std::vector<uint8_t> > c_NodeSignatureKeys;
   mh_AdaptCommonSignatureParameters(orc_NodeSignatureKeys,
                                     static_cast<uint32_t>(orc_ActiveNodes.size()),
                                     c_NodeSignatureKeys);
   tgl_assert(((((orc_ActiveNodes.size() == c_NodeSignatureKeys.size())) &&
                (orc_ActiveNodes.size() == orc_Signatures.size())) &&
               (orc_ActiveNodes.size() == orc_ApplicationsToWrite.size())) &&
              (orc_ActiveNodes.size() == orc_AbsSydeSecureDefFileNames.size()));
   if (((((orc_ActiveNodes.size() == c_NodeSignatureKeys.size())) &&
         (orc_ActiveNodes.size() == orc_Signatures.size())) &&
        (orc_ActiveNodes.size() == orc_ApplicationsToWrite.size())) &&
       (orc_ActiveNodes.size() == orc_AbsSydeSecureDefFileNames.size()))
   {
      for (uint32_t u32_ItNode = 0UL; (u32_ItNode < orc_ApplicationsToWrite.size()) && (!c_Retval);
           ++u32_ItNode)
      {
         if (orc_ActiveNodes[u32_ItNode] == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE)
         {
            if (orc_Signatures[u32_ItNode].empty() == false)
            {
               c_Retval = mh_VerifySignature(orc_ApplicationsToWrite[u32_ItNode], c_NodeSignatureKeys[u32_ItNode],
                                             orc_Signatures[u32_ItNode], orc_AbsSydeSecureDefFileNames[u32_ItNode]);
            }
         }
      }
   }
   if (c_Retval)
   {
      mhc_ErrorMessage = "Could not verify signature, or invalid signature. Maybe wrong pem file?";
      osc_write_log_error("verify signature for all nodes", mhc_ErrorMessage);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Verify signature

   \param[in]  orc_ApplicationsToWrite       Applications to write
   \param[in]  orc_NodeSignatureKeys         Node signature keys
   \param[in]  orc_Signature                 Signature (DER encoded in hex string format)
   \param[in]  orc_AbsSydeSecureDefFileName  Abs syde secure def file name

   \return
   std::error_code

   \retval   Errc::success   No err
   \retval   Errc::noact     invalid signature or signature check failed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageLoad::mh_VerifySignature(
   const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite, const std::vector<uint8_t> & orc_NodeSignatureKeys,
   const std::string & orc_Signature, const std::string & orc_AbsSydeSecureDefFileName)
{
   C_OscSecurityEcdsa::C_Ecdsa256Signature c_Signature;

   std::error_code c_Retval = c_Signature.SetFromDerString(orc_Signature);

   if (c_Retval)
   {
      c_Retval = Errc::noact;
   }
   else
   {
      std::set<std::string> c_Files;
      uint8_t au8_BinDigest[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH];
      mh_GetDigestFiles(orc_ApplicationsToWrite, orc_AbsSydeSecureDefFileName, c_Files);

      c_Retval = mh_CalcDigest("", c_Files, au8_BinDigest, true);
      if (!c_Retval)
      {
         std::string c_Log;
         uint8_t au8_PublicKey[C_OscSecurityEcdsa::hu32_SECP256R1_PUBLIC_KEY_LENGTH];
         mh_DigestToString(au8_BinDigest, c_Log);
         osc_write_log_info("Processing Update Package", "Calculated security digest: " + c_Log);

         c_Retval = C_OscSecurityEcdsa::h_ExtractPublicKeyFromX509Certificate(orc_NodeSignatureKeys,
                                                                              au8_PublicKey);
         if (!c_Retval)
         {
            bool q_Valid;
            c_Retval = C_OscSecurityEcdsa::h_VerifyEcdsaSecp256r1Signature(au8_PublicKey, c_Signature,
                                                                           au8_BinDigest, q_Valid);
            if ((c_Retval) || (q_Valid == false))
            {
               osc_write_log_error("Processing Update Package", "ECDSA signature check failed.");
               c_Retval = Errc::noact;
            }
         }
         else
         {
            osc_write_log_error("Processing Update Package", "Could not extract public EC key from certificate data.");
            c_Retval = Errc::noact;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get digest files

   \param[in]      orc_ApplicationsToWrite         Applications to write
   \param[in]      orc_AbsSydeSecureDefFileName    Abs syde secure def file name
   \param[in,out]  orc_Files                       Files
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageLoad::mh_GetDigestFiles(const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
                                                         const std::string & orc_AbsSydeSecureDefFileName,
                                                         std::set<std::string> & orc_Files)
{
   for (uint32_t u32_It = 0UL; u32_It < orc_ApplicationsToWrite.c_FilesToFlash.size(); ++u32_It)
   {
      orc_Files.insert(orc_ApplicationsToWrite.c_FilesToFlash[u32_It]);
   }
   for (uint32_t u32_It = 0UL; u32_It < orc_ApplicationsToWrite.c_FilesToWriteToNvm.size(); ++u32_It)
   {
      orc_Files.insert(orc_ApplicationsToWrite.c_FilesToWriteToNvm[u32_It]);
   }
   if (orc_ApplicationsToWrite.c_PemFile.empty() == false)
   {
      orc_Files.insert(orc_ApplicationsToWrite.c_PemFile);
   }
   orc_Files.insert(orc_AbsSydeSecureDefFileName);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get pem file content

   \param[in]      orc_NodeSignaturePemFiles    Node signature pem files
   \param[in,out]  orc_NodeSignatureKeys        Node signature keys
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageLoad::mh_GetPemFileContent(const std::vector<std::string> & orc_NodeSignaturePemFiles,
                                                            std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys)
{
   orc_NodeSignatureKeys.reserve(orc_NodeSignaturePemFiles.size());
   for (uint32_t u32_ItNode = 0UL; u32_ItNode < orc_NodeSignaturePemFiles.size();
        ++u32_ItNode)
   {
      std::string c_Err;
      C_OscSecurityPemSecUpdate c_Pem;
      const std::error_code c_Retval = c_Pem.LoadFromFile(orc_NodeSignaturePemFiles[u32_ItNode].c_str(), c_Err);
      if (c_Retval)
      {
         osc_write_log_warning("Reading pem files", mhc_ErrorMessage);
      }
      orc_NodeSignatureKeys.push_back(c_Pem.GetKeyInfo().GetX509CertificateData());
   }
}
