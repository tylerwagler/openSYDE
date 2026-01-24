//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Package loading util

   openSYDE: Service Package loading util

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QDir>
#include <QFileInfo>


#include "C_OscLoggingHandler.hpp"
#include "C_OscSpaServicePackageLoadUtil.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"


/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get unzip path

   \param[in]  orc_TargetUnzipPath  Target unzip path

   \return
   Unzip path
 */
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSpaServicePackageLoadUtil::h_GetUnzipPath(
    const QString &orc_TargetUnzipPath) {
  QString c_TargetUnzipPath;
  if (orc_TargetUnzipPath != "") {
    // add trailing path delimiter in case there is none
    c_TargetUnzipPath =
        stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(
            orc_TargetUnzipPath);
  } else {
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
   \retval   C_CONFIG   could not find X-Config package archive or X-Config
   package directory
   \retval   C_BUSY     could not erase pre-existing target path (note: can
   result in partially erased target path)
   \retval   C_RD_WR    could not unzip X-Config package from disk to target
   path
 */
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageLoadUtil::h_CheckParamsToProcessZipPackage(
    const QString &orc_PackagePath, const QString &orc_TargetUnzipPath,
    const QString &orc_UseCase, QString &orc_ErrorMessage) {
  int32_t s32_Return = C_NO_ERR;

  // check if zip archive exists
  if (!QFileInfo(orc_PackagePath).exists() ||
      !QFileInfo(orc_PackagePath).isFile()) {
    orc_ErrorMessage =
        "Zip archive \"" + orc_PackagePath + "\" does not exist.";
    osc_write_log_error(orc_UseCase, orc_ErrorMessage);
    s32_Return = C_CONFIG;
  }

  // erase target path if it exists
  if (s32_Return == C_NO_ERR) {
    if (QFileInfo(orc_TargetUnzipPath).isDir()) {
      s32_Return = (QDir(orc_TargetUnzipPath).removeRecursively() ? 0 : -1);
      if (s32_Return != 0) {
        orc_ErrorMessage = "Could not remove folder \"" + orc_TargetUnzipPath +
                           "\" to extract contents of zip archive.";
        osc_write_log_error(orc_UseCase, orc_ErrorMessage);
        s32_Return = C_BUSY;
      }
    }
  }

  // create target unzip path
  if (s32_Return == C_NO_ERR) {
    // create target folder (from bottom-up if required):
    s32_Return = C_OscUtils::h_CreateFolderRecursively(orc_TargetUnzipPath);
    if (s32_Return != C_NO_ERR) {
      orc_ErrorMessage = "Could not create folder \"" + orc_TargetUnzipPath +
                         "\" for zip archive.";
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
      s32_Return = C_RD_WR;
    }
  }

  // open zip file and unpack contents to target folder
  if (s32_Return == C_NO_ERR) {
    s32_Return = C_OscZipFile::h_UnpackZipFile(
        orc_PackagePath, orc_TargetUnzipPath, &orc_ErrorMessage);
    if (s32_Return != C_NO_ERR) {
      osc_write_log_error(orc_UseCase, orc_ErrorMessage);
    }
  }
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Search files in path

   \param[in]  orc_PackagePath      Package path
   \param[in]  orc_NecessaryFiles   Necessary files

   \retval   C_NO_ERR   directory contains all necessary files
   \retval   C_DEFAULT  at least one file is missing in given directory
                        (due to lack of alternatives C_DEFAULT was chosen to
   have a unique error to redirect to tool specific error codes)
 */
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSpaServicePackageLoadUtil::h_SearchFilesInPath(
    const QString &orc_PackagePath,
    const std::vector<QString> &orc_NecessaryFiles) {
  int32_t s32_Return = C_NO_ERR;

  QString c_PackagePath =
      stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(
          orc_PackagePath);
  QDir c_QDir(c_PackagePath);

  for (uint32_t u32_It = 0; u32_It < orc_NecessaryFiles.size(); ++u32_It) {
    if (s32_Return == C_DEFAULT) {
      break;
    }

    const QString c_FileExt =
        "." + QFileInfo(orc_NecessaryFiles[u32_It]).suffix();
    QStringList c_Filter;
    c_Filter << "*" + c_FileExt;

    QFileInfoList c_InfoList =
        c_QDir.entryInfoList(c_Filter, QDir::Files | QDir::NoDotAndDotDot);

    // only one of the specified files is allowed
    if (c_InfoList.count() == 1) {
      // if the correct amount of files is present, we need to have a match on
      // the exact name, otherwise -> fail.
      if (c_InfoList.at(0).fileName() != orc_NecessaryFiles[u32_It]) {
        s32_Return = C_DEFAULT;
      }
    } else {
      // more than one file of the specified type, smells fishy -> abort
      s32_Return = C_DEFAULT;
    }
  }

  return s32_Return;
}
