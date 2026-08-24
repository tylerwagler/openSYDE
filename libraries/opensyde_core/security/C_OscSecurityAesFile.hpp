//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core AES file encryption utility

   \class       stw::opensyde_core::C_OscSecurityAesFile

   Wrapper for the AES library by SergeyBel.
   Provides functions to encrypt/decrypt a file with AES.

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYAESFILE_HPP
#define C_OSCSECURITYAESFILE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <set>
#include <system_error>
#include "stwtypes.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSecurityAesFile
{
public:
   static std::error_code h_EncryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                const std::string & orc_OutFilePath);

   static std::error_code h_DecryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                const std::string & orc_OutFilePath);

   static std::error_code h_CreateEncryptedZipFile(const std::string & orc_FolderPathToZip,
                                           const std::set<std::string> & orc_SupFiles,
                                           const std::string & orc_PathForZipFile,
                                           const std::string & orc_Key,
                                           std::string * const opc_ErrorMessage);
   static std::error_code h_UnpackEncryptedZipFile(const std::string & orc_PathOfZipFile,
                                           const std::string & orc_FolderPathToUnzip,
                                           const std::string & orc_Key,
                                           std::string * const opc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
