//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package loading util

   openSYDE: Service Package loading util

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "TglFile.hpp"
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSpaServicePackageLoadUtil.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
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
/*! \brief  Get unzip path

   \param[in]  orc_TargetUnzipPath  Target unzip path

   \return
   Unzip path
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscSpaServicePackageLoadUtil::h_GetUnzipPath(const std::string & orc_TargetUnzipPath)
{
   std::string c_TargetUnzipPath;
   if (orc_TargetUnzipPath != "")
   {
      // add trailing path delimiter in case there is none
      c_TargetUnzipPath = TglFileIncludeTrailingDelimiter(orc_TargetUnzipPath);
   }
   else
   {
      c_TargetUnzipPath = "";
   }
   return c_TargetUnzipPath;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check params to process package

   \param[in]      orc_PackagePath        Package path
   \param[in,out]  orc_TargetUnzipPath    Target unzip path
   \param[in]      orc_UseCase            Use case
   \param[in,out]  orc_ErrorMessage       Error message

   \return
   STW error codes

   \retval   C_NO_ERR   success
   \retval   C_CONFIG   could not find X-Config package archive or X-Config package directory
   \retval   C_BUSY     could not erase pre-existing target path (note: can result in partially erased target path)
   \retval   C_RD_WR    could not unzip X-Config package from disk to target path
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSpaServicePackageLoadUtil::h_CheckParamsToProcessZipPackage(const std::string & orc_PackagePath,
                                                                         const std::string & orc_TargetUnzipPath, const std::string & orc_UseCase,
                                                                         std::string & orc_ErrorMessage)
{
   std::error_code c_Return = Errc::success;

   // check if zip archive exists
   if (TglFileExists(orc_PackagePath) == false)
   {
      orc_ErrorMessage = "Zip archive \"" + orc_PackagePath + "\" does not exist.";
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      c_Return = Errc::config;
   }

   //erase target path if it exists
   if (!c_Return)
   {
      if (TglDirectoryExists(orc_TargetUnzipPath) == true)
      {
         //TglRemoveDirectory reports a plain 0/non-zero status, not an STW error code
         const int32_t s32_RemoveResult = TglRemoveDirectory(orc_TargetUnzipPath, false);
         if (s32_RemoveResult != 0)
         {
            orc_ErrorMessage = "Could not remove folder \"" + orc_TargetUnzipPath +
                               "\" to extract contents of zip archive.";
            osc_write_log_error(orc_UseCase, orc_ErrorMessage);
            c_Return = Errc::busy;
         }
      }
   }

   // create target unzip path
   if (!c_Return)
   {
      //create target folder (from bottom-up if required):
      c_Return = C_OscUtils::h_CreateFolderRecursively(orc_TargetUnzipPath);
      if (c_Return)
      {
         orc_ErrorMessage = "Could not create folder \"" + orc_TargetUnzipPath + "\" for zip archive.";
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
         c_Return = Errc::rd_wr;
      }
   }

   // open zip file and unpack contents to target folder
   if (!c_Return)
   {
      c_Return = C_OscZipFile::h_UnpackZipFile(orc_PackagePath, orc_TargetUnzipPath, &orc_ErrorMessage);
      if (c_Return)
      {
         osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Search files in path

   \param[in]  orc_PackagePath      Package path
   \param[in]  orc_NecessaryFiles   Necessary files

   \retval   C_NO_ERR   directory contains all necessary files
   \retval   C_DEFAULT  at least one file is missing in given directory
                        (due to lack of alternatives C_DEFAULT was chosen to have a unique error to redirect to
                         tool specific error codes)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSpaServicePackageLoadUtil::h_SearchFilesInPath(const std::string & orc_PackagePath,
                                                            const std::vector<std::string> & orc_NecessaryFiles)
{
   std::error_code c_Return = Errc::success;

   std::vector<C_TglFileSearchRecord> c_Files; //storage for found files

   for (uint32_t u32_It = 0; u32_It < orc_NecessaryFiles.size(); ++u32_It)
   {
      if (c_Return == Errc::default_)
      {
         break;
      }
      const std::string c_FileExt = TglExtractFileExtension(orc_NecessaryFiles[u32_It]);
      //define search pattern for TGL_FileFind including the package path, otherwise function would search the whole
      //system. This probably would slow us down. We first search for a certain extension and in second step for
      //specific name.
      const std::string c_SearchPattern = TglFileIncludeTrailingDelimiter(orc_PackagePath) + "*" + c_FileExt;

      TglFileFind(c_SearchPattern, c_Files);

      //only one of the specified files is allowed
      if (c_Files.size() == 1)
      {
         //if the correct amount of files is present, we need to have a match on the exact name, otherwise -> fail.
         if (c_Files[0].c_FileName != orc_NecessaryFiles[u32_It])
         {
            c_Return = Errc::default_;
         }
      }
      else
      {
         //more than one file of the specified type, smells fishy -> abort
         c_Return = Errc::default_;
      }
   }

   return c_Return;
}
