//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core AES file encryption utility

   \class       stw::opensyde_core::C_OscAesFile
   \brief       openSYDE Core zip/unzip file utilities

   Wrapper for the AES library by SergeyBel.
   Provides functions to encrypt/decrypt a file with AES.

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCAESFILE_HPP
#define C_OSCAESFILE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <set>
#include "stwtypes.hpp"
#include "C_SclString.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscAesFile
{
public:
   static int32_t h_EncryptFile(const QString & orc_Key, const QString & orc_InFilePath,
                                const QString & orc_OutFilePath);

   static int32_t h_DecryptFile(const QString & orc_Key, const QString & orc_InFilePath,
                                const QString & orc_OutFilePath);

   static int32_t h_CreateEncryptedZipFile(const QString & orc_FolderPathToZip,
                                           const std::set<QString> & orc_SupFiles,
                                           const QString & orc_PathForZipFile,
                                           const QString & orc_Key,
                                           QString * const opc_ErrorMessage);
   static int32_t h_UnpackEncryptedZipFile(const QString & orc_PathOfZipFile,
                                           const QString & orc_FolderPathToUnzip,
                                           const QString & orc_Key,
                                           QString * const opc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
