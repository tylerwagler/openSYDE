//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core zip/unzip utilities

   \class       stw::opensyde_core::C_OscZipFile
   \brief       openSYDE Core zip/unzip file utilities

   Wrapper for the miniz library.
   Provides functions to zip/unzip data to/from file

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCZIPFILE_HPP
#define C_OSCZIPFILE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <set>
#include "stwtypes.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscZipFile
{
public:
   static int32_t h_CreateZipFile(const std::string & orc_SourcePath,
                                  const std::set<std::string> & orc_SupFiles,
                                  const std::string & orc_ZipArchivePath,
                                  std::string * const opc_ErrorText = NULL);

   static int32_t h_UnpackZipFile(const std::string & orc_SourcePath,
                                  const std::string & orc_TargetUnzipPath,
                                  std::string * const opc_ErrorText = NULL);

   static void h_AppendFilesRelative(std::set<std::string> & orc_Set,
                                     const std::vector<std::string> & orc_Files,
                                     const std::string & orc_BasePath);

   static int32_t h_IsZipFile(const std::string & orc_FilePath);

private:
   static int32_t mh_AddContentToZipFile(const std::string & orc_ZipArchivePath,
                                         const std::string & orc_ItemName, const char_t * const opcn_Content,
                                         const uint32_t ou32_ContentSize, const std::string & orc_ItemType,
                                         std::string * const opc_ErrorText = NULL);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
