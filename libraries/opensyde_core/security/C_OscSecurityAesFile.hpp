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
#include <cstdint>
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
   ///Encrypted file layout, all little endian:
   ///   0   6  magic "OSYENC"
   ///   6   1  format version
   ///   7   1  algorithm id
   ///   8   4  PBKDF2 iteration count
   ///  12  16  salt
   ///  28  12  nonce
   ///  40  16  GCM authentication tag
   ///  56   .  ciphertext
   static constexpr uint32_t hu32_KEY_LENGTH = 32U;   ///< AES-256
   static constexpr uint32_t hu32_SALT_LENGTH = 16U;
   static constexpr uint32_t hu32_NONCE_LENGTH = 12U; ///< GCM standard nonce size
   static constexpr uint32_t hu32_TAG_LENGTH = 16U;
   static constexpr uint32_t hu32_HEADER_LENGTH = 56U;

   ///Bumped whenever the on-disk layout changes. The previous format had no
   ///header at all, so it cannot be detected — it is rejected, not migrated.
   static constexpr uint8_t hu8_FORMAT_VERSION = 1U;
   static constexpr uint8_t hu8_ALGO_AES256_GCM = 1U;

   ///OWASP guidance for PBKDF2-HMAC-SHA256. Stored in the header so the count
   ///can be raised later without breaking files written today.
   static constexpr uint32_t hu32_PBKDF2_ITERATIONS = 600000U;

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
