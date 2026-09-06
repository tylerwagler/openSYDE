//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package creation util

   openSYDE: Service Package creation util

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <filesystem>
#include <map>
#include <system_error>

#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemDefinitionFiler.hpp"
#include "C_OscSpaServicePackageCreateUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check package path param

   \param[in]      orc_PackagePath           Package path
   \param[in]      orc_UseCase               Use case
   \param[in]      orc_PackageExtension      Package extension
   \param[in]      orc_PackageExtensionTmp   Package extension tmp folder
   \param[in,out]  orc_ErrorMessage          Error message
   \param[in]      oq_CheckFileExist         Check file exist

   \return
   C_NO_ERR    success
   C_RANGE     target package file already exists
               target directory for package does not exist
               invalid package name with no package extension
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageCreateUtil::h_CheckPackagePathParam(const C_SclString & orc_PackagePath,
                                                                  const C_SclString & orc_UseCase,
                                                                  const C_SclString & orc_PackageExtension,
                                                                  const C_SclString & orc_PackageExtensionTmp,
                                                                  C_SclString & orc_ErrorMessage,
                                                                  const bool oq_CheckFileExist)
{
   int32_t s32_Return = C_NO_ERR;

   // does package already exist ?
   const bool q_FileExists = TglFileExists(orc_PackagePath);
   const bool q_DirectoryExists = TglDirectoryExists(orc_PackagePath);

   if ((oq_CheckFileExist) && ((q_FileExists == true) || (q_DirectoryExists == true)))
   {
      orc_ErrorMessage = "Update Package \"" + orc_PackagePath + "\" exists already.";
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      s32_Return = C_RANGE;
   }

   // does target directory for package exist ?
   if (s32_Return == C_NO_ERR)
   {
      const C_SclString c_TargetDir = TglExtractFilePath(orc_PackagePath);

      // package name without path or './' leads to target directory "",
      // which should not lead to error as it will result in creating package next to executable
      if ((c_TargetDir.IsEmpty() == false) && (TglDirectoryExists(c_TargetDir) == false))
      {
         orc_ErrorMessage = "Target directory \"" + c_TargetDir + "\" does not exist.";
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         s32_Return = C_RANGE;
      }
   }

   // valid package name? Only relevant if we save SUP as file
   if (s32_Return == C_NO_ERR)
   {
      // package name must contain ".syde_sup" and ".syde_sup" must be the last characters of the given path
      if ((orc_PackagePath.Pos(orc_PackageExtension) == 0) ||
          (orc_PackagePath.Length() !=
           ((orc_PackagePath.Pos(orc_PackageExtension) + orc_PackageExtension.Length()) - 1)))
      {
         orc_ErrorMessage = "Package name must have extension \"" + orc_PackageExtension + "\".";
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         s32_Return = C_RANGE;
      }
   }

   // does temporary result folder exist ? (would be really strange, but who knows)
   if (s32_Return == C_NO_ERR)
   {
      C_SclString c_TmpPackagePath = orc_PackagePath.SubString(1, orc_PackagePath.Pos(orc_PackageExtension) - 1) +
                                     orc_PackageExtensionTmp;
      // add trailing path delimiter to temporary folder if not present
      c_TmpPackagePath = TglFileIncludeTrailingDelimiter(c_TmpPackagePath);
      if ((TglFileExists(c_TmpPackagePath) == true) || (TglDirectoryExists(c_TmpPackagePath) == true))
      {
         orc_ErrorMessage = "Temporary result folder \"" + c_TmpPackagePath +
                            "\" to create zip archive already exists.";
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         s32_Return = C_RANGE;
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get temp folder name

   \param[in]      orc_PackagePath                 Package path
   \param[in]      orc_TemporaryDirectory          Temporary directory
   \param[in]      orc_UseCase                     Use case
   \param[in]      orc_PackageExtension            Package extension
   \param[in]      orc_TemporaryPackageExtension   Temporary package extension
   \param[in,out]  orc_UsedTempPath                Used temp path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSpaServicePackageCreateUtil::h_GetTempFolderName(const C_SclString & orc_PackagePath,
                                                           const C_SclString & orc_TemporaryDirectory,
                                                           const C_SclString & orc_UseCase,
                                                           const C_SclString & orc_PackageExtension,
                                                           const C_SclString & orc_TemporaryPackageExtension,
                                                           C_SclString & orc_UsedTempPath)
{
   // path for temporary folder to create contents
   if (orc_TemporaryDirectory.IsEmpty() == true)
   {
      orc_UsedTempPath = orc_PackagePath;
   }
   else
   {
      // use sub folder named like package
      orc_UsedTempPath = TglFileIncludeTrailingDelimiter(orc_TemporaryDirectory) +
                         TglExtractFileName(orc_PackagePath);
      // TglExpandFileName() in the linux tgl only works for existing paths, but our path does not yet exist,
      // so we need to expand the path manually
   }
   orc_UsedTempPath = orc_UsedTempPath.SubString(1, orc_UsedTempPath.Pos(orc_PackageExtension) - 1) +
                      orc_TemporaryPackageExtension;

   // add trailing path delimiter to temporary folder if not present
   orc_UsedTempPath = TglFileIncludeTrailingDelimiter(orc_UsedTempPath);

   osc_write_log_info(orc_UseCase, "Temporary folder path: " + orc_UsedTempPath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create temp folder and sub folders

   \param[in]      orc_PackagePath                 Package path
   \param[in]      orc_TemporaryDirectory          Temporary directory
   \param[in]      orc_UseCase                     Use case
   \param[in]      orc_PackageExtension            Package extension
   \param[in]      orc_TemporaryPackageExtension   Temporary package extension
   \param[in]      orc_AllStaticSubFolders         All static sub folders
   \param[in,out]  orc_UsedTempPath                Used temp path
   \param[in,out]  orc_ErrorMessage                Error message

   \return
   STW error codes

   \retval   C_NO_ERR   All folders created
   \retval   C_RD_WR    could not create temporary folders
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageCreateUtil::h_CreateTempFolderAndSubFolders(const C_SclString & orc_PackagePath,
                                                                          const C_SclString & orc_TemporaryDirectory,
                                                                          const C_SclString & orc_UseCase,
                                                                          const C_SclString & orc_PackageExtension,
                                                                          const C_SclString & orc_TemporaryPackageExtension, const std::vector<C_SclString> & orc_AllStaticSubFolders, C_SclString & orc_UsedTempPath,
                                                                          C_SclString & orc_ErrorMessage)
{
   int32_t s32_Return = C_NO_ERR;

   stw::scl::C_SclString c_ErrorPath;
   C_OscSpaServicePackageCreateUtil::h_GetTempFolderName(orc_PackagePath, orc_TemporaryDirectory,
                                                         orc_UseCase, orc_PackageExtension,
                                                         orc_TemporaryPackageExtension, orc_UsedTempPath);

   //erase target path if it exists:
   if (TglDirectoryExists(orc_UsedTempPath) == true)
   {
      s32_Return = TglRemoveDirectory(orc_UsedTempPath, false);
   }

   if (s32_Return == C_NO_ERR)
   {
      c_ErrorPath = orc_UsedTempPath;
      //create target folder (from bottom-up if required):
      s32_Return = C_OscUtils::h_CreateFolderRecursively(c_ErrorPath);
   }
   if (s32_Return == C_NO_ERR)
   {
      for (uint32_t u32_It = 0UL; (u32_It < orc_AllStaticSubFolders.size()) && (s32_Return == C_NO_ERR); ++u32_It)
      {
         c_ErrorPath = orc_UsedTempPath + orc_AllStaticSubFolders[u32_It];
         //create target folder (from bottom-up if required):
         s32_Return = C_OscUtils::h_CreateFolderRecursively(c_ErrorPath);
      }
   }
   if (s32_Return != C_NO_ERR)
   {
      // very strange! normally the precondition check should
      // guarantee a correct behavior of h_CreateTemporaryFolder
      orc_ErrorMessage = "Could not create temporary folder \"" +
                         c_ErrorPath + "\".";
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      s32_Return = C_RD_WR; // redefine because we only have a few error codes
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save system definition

   \param[in]      orc_SystemDefinition            System definition
   \param[in]      orc_SystemDefinitionFileName    System definition file name
   \param[in]      orc_UseCase                     Use case
   \param[in]      orc_UsedTempPath                Used temp path
   \param[in]      orc_OutFilePrefix               Out file prefix
   \param[in,out]  orc_AllCreatedFiles             All created files
   \param[in,out]  orc_ErrorMessage                Error message

   \return
   STW error codes

   \retval   C_NO_ERR   success
   \retval   C_RD_WR    could not save system definition file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageCreateUtil::h_SaveSystemDefinition(const C_OscSystemDefinition & orc_SystemDefinition,
                                                                 const C_SclString & orc_SystemDefinitionFileName,
                                                                 const C_SclString & orc_UseCase,
                                                                 const C_SclString & orc_UsedTempPath,
                                                                 const C_SclString & orc_OutFilePrefix,
                                                                 std::set<C_SclString> & orc_AllCreatedFiles,
                                                                 C_SclString & orc_ErrorMessage)
{
   // take current system definition of view (is required) and store to file
   const C_SclString c_SysDefPath = orc_UsedTempPath + orc_SystemDefinitionFileName;

   std::vector<C_SclString> c_AdditionalFiles;
   int32_t s32_Return = C_OscSystemDefinitionFiler::h_SaveSystemDefinitionFile(orc_SystemDefinition,
                                                                               c_SysDefPath, &c_AdditionalFiles);
   if (s32_Return == C_NO_ERR)
   {
      //Add files to pack
      for (uint32_t u32_ItFile = 0UL; u32_ItFile < c_AdditionalFiles.size(); ++u32_ItFile)
      {
         orc_AllCreatedFiles.insert(orc_OutFilePrefix + c_AdditionalFiles[u32_ItFile]);
      }
   }

   if (s32_Return != C_NO_ERR)
   {
      // write log
      orc_ErrorMessage = "Could not save System Definition file to path \"" + orc_UsedTempPath + "\".";
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      s32_Return = C_RD_WR;
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save device definitions and ini

   \param[in]      orc_SystemDefinition   System definition
   \param[in]      orc_UseCase            Use case
   \param[in]      orc_UsedTempPath       Used temp path
   \param[in]      orc_OutFilePrefix      Out file prefix
   \param[in,out]  orc_AllCreatedFiles    All created files
   \param[in,out]  orc_ErrorMessage       Error message

   \return
   STW error codes

   \retval   C_NO_ERR   success
   \retval   C_RD_WR    could not save device definitions and ini file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageCreateUtil::h_SaveDeviceDefinitionsAndIni(
   const C_OscSystemDefinition & orc_SystemDefinition, const C_SclString & orc_UseCase,
   const C_SclString & orc_UsedTempPath, const C_SclString & orc_OutFilePrefix,
   std::set<C_SclString> & orc_AllCreatedFiles, C_SclString & orc_ErrorMessage)
{
   namespace fs = std::filesystem;
   int32_t s32_Return = C_NO_ERR;

   // Collect unique device manifests, keyed by source file path so duplicate uses across
   // multiple nodes only produce one bundle. Value is the device's canonical name, which
   // becomes the bundle folder name in the package.
   std::map<C_SclString, C_SclString> c_DevicesByPath;
   for (uint32_t u32_Pos = 0U; u32_Pos < orc_SystemDefinition.c_Nodes.size(); ++u32_Pos)
   {
      const C_OscDeviceDefinition * const pc_DeviceDefinition =
         orc_SystemDefinition.c_Nodes[u32_Pos].pc_DeviceDefinition;
      tgl_assert(pc_DeviceDefinition != nullptr);
      if (pc_DeviceDefinition != nullptr)
      {
         c_DevicesByPath[pc_DeviceDefinition->c_FilePath] = pc_DeviceDefinition->c_DeviceName;
      }
   }

   const C_SclString c_TempPathTrailing = TglFileIncludeTrailingDelimiter(orc_UsedTempPath);

   // For each unique device, write a folder bundle: <device_name>/device.syd
   for (std::map<C_SclString, C_SclString>::const_iterator c_It = c_DevicesByPath.begin();
        (c_It != c_DevicesByPath.end()) && (s32_Return == C_NO_ERR);
        ++c_It)
   {
      const C_SclString & rc_SrcPath = c_It->first;
      const C_SclString & rc_DeviceName = c_It->second;

      const C_SclString c_TargetDir = c_TempPathTrailing + rc_DeviceName;
      const C_SclString c_TargetFile = c_TargetDir + "/device.syd";

      std::error_code c_Ec;
      fs::create_directories(fs::path(c_TargetDir.c_str()), c_Ec);
      if (c_Ec)
      {
         orc_ErrorMessage = "Could not create device-bundle folder \"" + c_TargetDir +
                            "\": " + C_SclString(c_Ec.message().c_str());
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         s32_Return = C_RD_WR;
         continue;
      }

      s32_Return = C_OscUtils::h_CopyFile(rc_SrcPath, c_TargetFile, nullptr, &orc_ErrorMessage);
      if (s32_Return != C_NO_ERR)
      {
         orc_ErrorMessage = "Could not save device manifest for \"" + rc_DeviceName +
                            "\" from \"" + rc_SrcPath + "\" to \"" + c_TargetFile + "\".";
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         s32_Return = C_RD_WR;
         continue;
      }

      orc_AllCreatedFiles.insert(orc_OutFilePrefix + rc_DeviceName + "/device.syd");
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create zip

   \param[in]      orc_UseCase            Use case
   \param[in]      orc_UsedTempPath       Used temp path
   \param[in]      orc_ZipFilePath        Zip file path
   \param[in]      orc_AllCreatedFiles    All created files
   \param[in,out]  orc_ErrorMessage       Error message

   \return
   STW error codes

   \retval   C_NO_ERR   success
   \retval   C_CONFIG    at least one input file does not exist
   \retval   C_RD_WR     could not open input file
   \retval   C_NOACT     could not add data to zip file (does the path to the file exist ?)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageCreateUtil::h_CreateZip(const C_SclString & orc_UseCase,
                                                      const C_SclString & orc_UsedTempPath,
                                                      const C_SclString & orc_ZipFilePath,
                                                      const std::set<C_SclString> & orc_AllCreatedFiles,
                                                      C_SclString & orc_ErrorMessage)
{
   const int32_t s32_Return =
      C_OscZipFile::h_CreateZipFile(orc_UsedTempPath, orc_AllCreatedFiles, orc_ZipFilePath, &orc_ErrorMessage);

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clean up temp folder

   \param[in]      orc_UseCase         Use case
   \param[in]      orc_UsedTempPath    Used temp path
   \param[out]     ors32_ErrVal        Err val
   \param[in,out]  orc_ErrorMessage    Error message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSpaServicePackageCreateUtil::h_CleanUpTempFolder(const C_SclString & orc_UseCase,
                                                           const C_SclString & orc_UsedTempPath, int32_t & ors32_ErrVal,
                                                           C_SclString & orc_ErrorMessage)
{
   const int32_t s32_Tmp = TglRemoveDirectory(orc_UsedTempPath, false);

   if (s32_Tmp != 0)
   {
      const C_SclString c_Message = "Could not delete temporary result folder \"" + orc_UsedTempPath + "\".";
      osc_write_log_error(orc_UseCase, c_Message);
      if ((ors32_ErrVal == C_NO_ERR) || (ors32_ErrVal == C_WARN)) // do not redefine error in case we had even a
      // problem
      // earlier
      {
         orc_ErrorMessage = c_Message;
         ors32_ErrVal = C_BUSY;
      }
   }
}

