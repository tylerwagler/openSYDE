//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core AES file encryption utility

   see header in .h file for details.

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_OscSecurityAesFile.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclStringCompat.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::opensyde_core;
using namespace std;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Derive an AES-256 key from a password

   PBKDF2-HMAC-SHA256 over the supplied password and a random per-file salt.

   The previous scheme was a single unsalted MD5 of the password. MD5 is fast and
   unsalted, so identical passwords produced identical keys across every file and
   the whole keyspace was precomputable. PBKDF2 with a per-file salt removes both
   properties, and the iteration count makes guessing cost real time.

   \param[in]   orc_Password     Password to derive from
   \param[in]   orau8_Salt       Per-file random salt
   \param[in]   ou32_Iterations  PBKDF2 iteration count
   \param[out]  orau8_Key        Derived 256bit key

   \return
   C_NO_ERR    key derived
   C_CONFIG    derivation failed
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_DeriveKey(const std::string & orc_Password,
                            const uint8_t (&orau8_Salt)[C_OscSecurityAesFile::hu32_SALT_LENGTH],
                            const uint32_t ou32_Iterations,
                            uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH])
{
   int32_t s32_Return = C_NO_ERR;

   const int x_Result = PKCS5_PBKDF2_HMAC(orc_Password.c_str(), static_cast<int>(orc_Password.length()),
                                          &orau8_Salt[0], static_cast<int>(C_OscSecurityAesFile::hu32_SALT_LENGTH),
                                          static_cast<int>(ou32_Iterations), EVP_sha256(),
                                          static_cast<int>(C_OscSecurityAesFile::hu32_KEY_LENGTH), &orau8_Key[0]);

   if (x_Result != 1)
   {
      s32_Return = C_CONFIG;
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   AES-256-GCM encrypt data in memory

   GCM is an authenticated mode: it produces a tag that detects any modification
   of the ciphertext. The previous mode was ECB, which encrypts each block
   independently — identical plaintext blocks produced identical ciphertext, the
   structure of the plaintext leaked, and nothing detected tampering.

   \param[in]   orau8_Key    256bit key
   \param[in]   orau8_Nonce  96bit nonce, must never repeat for a given key
   \param[in]   orc_Input    Plaintext input
   \param[out]  orc_Output   Ciphertext output
   \param[out]  orau8_Tag    Authentication tag

   \return
   C_NO_ERR    success
   C_CONFIG    encryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_EncryptGcm(const uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH],
                             const uint8_t (&orau8_Nonce)[C_OscSecurityAesFile::hu32_NONCE_LENGTH],
                             const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output,
                             uint8_t (&orau8_Tag)[C_OscSecurityAesFile::hu32_TAG_LENGTH])
{
   int32_t s32_Return = C_NO_ERR;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();

   if (pc_Ctx == nullptr)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      // GCM produces ciphertext the same length as the plaintext; no padding.
      orc_Output.resize(orc_Input.size());

      int x_OutLen = 0;
      int x_FinalLen = 0;

      if ((EVP_EncryptInit_ex(pc_Ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) ||
          (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_SET_IVLEN,
                               static_cast<int>(C_OscSecurityAesFile::hu32_NONCE_LENGTH), nullptr) != 1) ||
          (EVP_EncryptInit_ex(pc_Ctx, nullptr, nullptr, &orau8_Key[0], &orau8_Nonce[0]) != 1))
      {
         s32_Return = C_CONFIG;
      }
      else if ((orc_Input.size() > 0U) &&
               (EVP_EncryptUpdate(pc_Ctx, orc_Output.data(), &x_OutLen, orc_Input.data(),
                                  static_cast<int>(orc_Input.size())) != 1))
      {
         s32_Return = C_CONFIG;
      }
      // GCM adds no padding, so x_OutLen equals the input size and the finalise
      // pointer is one-past-the-end. data()+n is valid there; &vec[n] is not.
      else if (EVP_EncryptFinal_ex(pc_Ctx, orc_Output.data() + static_cast<size_t>(x_OutLen), &x_FinalLen) != 1)
      {
         s32_Return = C_CONFIG;
      }
      else if (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_GET_TAG,
                                   static_cast<int>(C_OscSecurityAesFile::hu32_TAG_LENGTH), &orau8_Tag[0]) != 1)
      {
         s32_Return = C_CONFIG;
      }
      else
      {
         orc_Output.resize(static_cast<size_t>(x_OutLen) + static_cast<size_t>(x_FinalLen));
      }

      EVP_CIPHER_CTX_free(pc_Ctx);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   AES-256-GCM decrypt data in memory

   Verifies the authentication tag. A wrong password, a truncated file or any
   modification of the ciphertext fails here rather than yielding garbage.

   \param[in]   orau8_Key    256bit key
   \param[in]   orau8_Nonce  96bit nonce
   \param[in]   orau8_Tag    Expected authentication tag
   \param[in]   orc_Input    Ciphertext input
   \param[out]  orc_Output   Plaintext output

   \return
   C_NO_ERR    success
   C_CHECKSUM  authentication failed (wrong password or tampered data)
   C_CONFIG    decryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_DecryptGcm(const uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH],
                             const uint8_t (&orau8_Nonce)[C_OscSecurityAesFile::hu32_NONCE_LENGTH],
                             const uint8_t (&orau8_Tag)[C_OscSecurityAesFile::hu32_TAG_LENGTH],
                             const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output)
{
   int32_t s32_Return = C_NO_ERR;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();

   if (pc_Ctx == nullptr)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      orc_Output.resize(orc_Input.size());

      int x_OutLen = 0;
      int x_FinalLen = 0;
      // EVP_CIPHER_CTX_ctrl takes a non-const tag pointer even though it only reads it here.
      uint8_t au8_TagCopy[C_OscSecurityAesFile::hu32_TAG_LENGTH];
      (void)std::memcpy(&au8_TagCopy[0], &orau8_Tag[0], C_OscSecurityAesFile::hu32_TAG_LENGTH);

      if ((EVP_DecryptInit_ex(pc_Ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) ||
          (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_SET_IVLEN,
                               static_cast<int>(C_OscSecurityAesFile::hu32_NONCE_LENGTH), nullptr) != 1) ||
          (EVP_DecryptInit_ex(pc_Ctx, nullptr, nullptr, &orau8_Key[0], &orau8_Nonce[0]) != 1))
      {
         s32_Return = C_CONFIG;
      }
      else if ((orc_Input.size() > 0U) &&
               (EVP_DecryptUpdate(pc_Ctx, orc_Output.data(), &x_OutLen, orc_Input.data(),
                                  static_cast<int>(orc_Input.size())) != 1))
      {
         s32_Return = C_CONFIG;
      }
      else if (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_SET_TAG,
                                   static_cast<int>(C_OscSecurityAesFile::hu32_TAG_LENGTH), &au8_TagCopy[0]) != 1)
      {
         s32_Return = C_CONFIG;
      }
      // A non-positive result here means the tag did not verify.
      else if (EVP_DecryptFinal_ex(pc_Ctx, orc_Output.data() + static_cast<size_t>(x_OutLen), &x_FinalLen) <= 0)
      {
         orc_Output.clear();
         s32_Return = C_CHECKSUM;
      }
      else
      {
         orc_Output.resize(static_cast<size_t>(x_OutLen) + static_cast<size_t>(x_FinalLen));
      }

      EVP_CIPHER_CTX_free(pc_Ctx);
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create encrypted file

   Create AES encrypted file using ECB mode.
   Uses OpenSSL EVP with PKCS#7 padding (handled automatically by EVP).

   Steps:
   * create an MD5 hash over the passed key to use as an 128bit AES encryption key
   * load input file to RAM
   * perform encryption in RAM (so not suitable for very large files)
   * write resulting output file

   If the output file already exists the function will overwrite it.

   \param[in]  orc_Key                       Key to use for encryption. Technically a MD5 hash over this key will
                                              be used as 128bit AES encryption key.
   \param[in]  orc_InFilePath                Path to input file
   \param[in]  orc_OutFilePath               Path to resulting output file

   \return
   std::error_code with Errc::success on success,
   Errc::rd_wr if input file does not exist or could not be loaded,
   Errc::noact if output could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_EncryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                                    const std::string & orc_OutFilePath)

{
   //check whether input file exists:
   if (TglFileExists(orc_InFilePath) == false)
   {
      return Errc::rd_wr;
   }

   //load data from input file:
   vector<uint8_t> c_InputData;
   std::ifstream c_InputFileStream;
   const uint32_t u32_InputFileSize = static_cast<uint32_t>(TglFileSize(orc_InFilePath));

   c_InputData.resize(u32_InputFileSize);

   c_InputFileStream.open(orc_InFilePath.c_str(), std::ifstream::binary);

   if (c_InputFileStream.is_open() == false)
   {
      return Errc::rd_wr;
   }

   //read file content
   bool q_HasFailed;
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?& anyway
   c_InputFileStream.read(reinterpret_cast<char_t *>(c_InputData.data()), u32_InputFileSize);
   //check for error
   q_HasFailed = c_InputFileStream.fail();
   //close file
   c_InputFileStream.close();

   if (q_HasFailed == true)
   {
      return Errc::rd_wr;
   }

   //do the encryption:
   uint8_t au8_Key[hu32_KEY_LENGTH];
   uint8_t au8_Salt[hu32_SALT_LENGTH];
   uint8_t au8_Nonce[hu32_NONCE_LENGTH];
   uint8_t au8_Tag[hu32_TAG_LENGTH];
   vector<uint8_t> c_EncryptedData;
   std::ofstream c_OutputFileStream;

   //fresh salt and nonce per file; a repeated nonce under one key breaks GCM
   if ((RAND_bytes(&au8_Salt[0], static_cast<int>(hu32_SALT_LENGTH)) != 1) ||
       (RAND_bytes(&au8_Nonce[0], static_cast<int>(hu32_NONCE_LENGTH)) != 1))
   {
      return Errc::config;
   }

   if (mh_DeriveKey(orc_Key, au8_Salt, hu32_PBKDF2_ITERATIONS, au8_Key) != C_NO_ERR)
   {
      OPENSSL_cleanse(&au8_Key[0], hu32_KEY_LENGTH);
      return Errc::config;
   }

   const int32_t s32_EncResult = mh_EncryptGcm(au8_Key, au8_Nonce, c_InputData, c_EncryptedData, au8_Tag);
   OPENSSL_cleanse(&au8_Key[0], hu32_KEY_LENGTH); //do not leave key material on the stack
   if (s32_EncResult != C_NO_ERR)
   {
      return Errc::config;
   }

   //prepend the header
   vector<uint8_t> c_Header(hu32_HEADER_LENGTH, 0U);
   (void)std::memcpy(&c_Header[0], "OSYENC", 6U);
   c_Header[6] = hu8_FORMAT_VERSION;
   c_Header[7] = hu8_ALGO_AES256_GCM;
   c_Header[8] = static_cast<uint8_t>(hu32_PBKDF2_ITERATIONS & 0xFFU);
   c_Header[9] = static_cast<uint8_t>((hu32_PBKDF2_ITERATIONS >> 8U) & 0xFFU);
   c_Header[10] = static_cast<uint8_t>((hu32_PBKDF2_ITERATIONS >> 16U) & 0xFFU);
   c_Header[11] = static_cast<uint8_t>((hu32_PBKDF2_ITERATIONS >> 24U) & 0xFFU);
   (void)std::memcpy(&c_Header[12], &au8_Salt[0], hu32_SALT_LENGTH);
   (void)std::memcpy(&c_Header[28], &au8_Nonce[0], hu32_NONCE_LENGTH);
   (void)std::memcpy(&c_Header[40], &au8_Tag[0], hu32_TAG_LENGTH);
   c_EncryptedData.insert(c_EncryptedData.begin(), c_Header.begin(), c_Header.end());

   //save to output file:
   c_OutputFileStream.open(orc_OutFilePath.c_str(), std::ofstream::binary | std::ofstream::trunc);
   if (c_OutputFileStream.is_open())
   {
      //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep
      // !"=?& anyway
      c_OutputFileStream.write(reinterpret_cast<const char_t *>(c_EncryptedData.data()),
                               static_cast<streamsize>(c_EncryptedData.size()));
      q_HasFailed = c_OutputFileStream.fail();
      c_OutputFileStream.close();
      if (q_HasFailed == true)
      {
         return Errc::noact;
      }
   }
   else
   {
      return Errc::noact;
   }

   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create decrypted file

   Steps:
   * create an MD5 hash over the passed key to use as an 128bit AES encryption key
   * load input file
   * perform decryption in RAM (so not suitable for very large files)
   * size decrypted data according to PKCS#7 algorithm
   * write resulting output file

   If the output file already exists the function will overwrite it.

   The size of the file must be a multiple of 16bytes.
   The file must have been written with PKCS#7 algorithm.

   \param[in]  orc_Key                       Key to use for decryption. Technically a MD5 hash over this key will
                                              be used as 128bit AES decryption key.
   \param[in]  orc_InFilePath                Path to input file
   \param[in]  orc_OutFilePath               Path to resulting output file

   \return
   std::error_code with Errc::success on success,
   Errc::rd_wr if input file does not exist or could not be loaded,
   Errc::config if input file size is not a multiple of 16,
   Errc::checksum if input file is invalid (wrong key or corrupted),
   Errc::noact if output file could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_DecryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                                    const std::string & orc_OutFilePath)

{
   //check whether input file exists:
   if (TglFileExists(orc_InFilePath) == false)
   {
      return Errc::rd_wr;
   }

   //load data from input file:
   vector<uint8_t> c_InputData;
   std::ifstream c_InputFileStream;
   const uint32_t u32_InputFileSize = TglFileSize(orc_InFilePath);

   //must at least carry a full header
   if (u32_InputFileSize < hu32_HEADER_LENGTH)
   {
      return Errc::config;
   }

   c_InputData.resize(u32_InputFileSize);

   c_InputFileStream.open(orc_InFilePath.c_str(), std::ifstream::binary);

   if (c_InputFileStream.is_open() == false)
   {
      return Errc::rd_wr;
   }

   //read file content
   bool q_HasFailed;
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?&
   // anyway
   c_InputFileStream.read(reinterpret_cast<char_t *>(c_InputData.data()), c_InputData.size());
   //check for error
   q_HasFailed = c_InputFileStream.fail();
   //close file
   c_InputFileStream.close();

   if (q_HasFailed == true)
   {
      return Errc::rd_wr;
   }

   //do the decryption:
   uint8_t au8_Key[hu32_KEY_LENGTH];
   uint8_t au8_Salt[hu32_SALT_LENGTH];
   uint8_t au8_Nonce[hu32_NONCE_LENGTH];
   uint8_t au8_Tag[hu32_TAG_LENGTH];
   vector<uint8_t> c_DecryptedData;
   std::ofstream c_OutputFileStream;

   //parse and validate the header. Files written by the previous format carry no
   //header at all and are rejected here rather than silently mis-decrypted.
   if (std::memcmp(c_InputData.data(), "OSYENC", 6U) != 0)
   {
      return Errc::config;
   }
   if ((c_InputData[6] != hu8_FORMAT_VERSION) || (c_InputData[7] != hu8_ALGO_AES256_GCM))
   {
      return Errc::config;
   }

   const uint32_t u32_Iterations = static_cast<uint32_t>(c_InputData[8]) |
                                   (static_cast<uint32_t>(c_InputData[9]) << 8U) |
                                   (static_cast<uint32_t>(c_InputData[10]) << 16U) |
                                   (static_cast<uint32_t>(c_InputData[11]) << 24U);
   if (u32_Iterations == 0U)
   {
      return Errc::config;
   }

   (void)std::memcpy(&au8_Salt[0], &c_InputData[12], hu32_SALT_LENGTH);
   (void)std::memcpy(&au8_Nonce[0], &c_InputData[28], hu32_NONCE_LENGTH);
   (void)std::memcpy(&au8_Tag[0], &c_InputData[40], hu32_TAG_LENGTH);

   const vector<uint8_t> c_CipherText(c_InputData.begin() + static_cast<int32_t>(hu32_HEADER_LENGTH),
                                      c_InputData.end());

   if (mh_DeriveKey(orc_Key, au8_Salt, u32_Iterations, au8_Key) != C_NO_ERR)
   {
      OPENSSL_cleanse(&au8_Key[0], hu32_KEY_LENGTH);
      return Errc::config;
   }

   const int32_t s32_DecResult = mh_DecryptGcm(au8_Key, au8_Nonce, au8_Tag, c_CipherText, c_DecryptedData);
   OPENSSL_cleanse(&au8_Key[0], hu32_KEY_LENGTH);
   if (s32_DecResult == C_CHECKSUM)
   {
      //wrong password, truncation or tampering: all indistinguishable, all rejected
      return Errc::checksum;
   }
   if (s32_DecResult != C_NO_ERR)
   {
      return Errc::config;
   }

   //save to output file:
   c_OutputFileStream.open(orc_OutFilePath.c_str(), std::ofstream::binary | std::ofstream::trunc);
   if (c_OutputFileStream.is_open())
   {
      //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep
      // !"=?& anyway
      c_OutputFileStream.write(reinterpret_cast<const char_t *>(c_DecryptedData.data()),
                               static_cast<streamsize>(c_DecryptedData.size()));
      q_HasFailed = c_OutputFileStream.fail();
      c_OutputFileStream.close();
      if (q_HasFailed == true)
      {
         return Errc::noact;
      }
   }
   else
   {
      return Errc::noact;
   }

   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create a zip file will all files of a folder and encrypts it with AES

   \param[in]   orc_FolderPathToZip    Base folder to pack into zip package
   \param[in]   orc_SupFiles           relative file paths for zip archive (files to add to archive)
   \param[in]   orc_PathForZipFile     File path for generated zip file
   \param[in]   orc_Key                Key for AES encryption (In case of an empty key, the file will not be encrypted)
   \param[out]  opc_ErrorMessage       Optional string for error messages

   \return
   std::error_code with Errc::success on success,
   Errc::config if at least one input file does not exist,
   Errc::rd_wr if could not open/write input/output file,
   Errc::noact if could not add data to zip file,
   Errc::busy if problems with deleting the temporary file
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_CreateEncryptedZipFile(const std::string & orc_FolderPathToZip,
                                                               const std::set<std::string> & orc_SupFiles,
                                                               const std::string & orc_PathForZipFile,
                                                               const std::string & orc_Key,
                                                               std::string * const opc_ErrorMessage)
{
   std::error_code c_Return = Errc::success;

   const std::string c_ZipFileTmp = orc_PathForZipFile + std::string("_tmp");
   std::string c_ErrorText;

   const std::error_code c_ZipResult = C_OscZipFile::h_CreateZipFile(orc_FolderPathToZip, orc_SupFiles, c_ZipFileTmp,
                                                                &c_ErrorText);
   if (c_ZipResult)
   {
      osc_write_log_error("Creating Encrypted Zip File", "Creating zip file failed with error: " +
                          c_ZipResult.message() +
                          " and error text: " + c_ErrorText.c_str());

      if (opc_ErrorMessage != nullptr)
      {
         *opc_ErrorMessage = c_ErrorText;
      }
      return c_ZipResult;
   }

   if (orc_Key != "")
   {
      c_Return = C_OscSecurityAesFile::h_EncryptFile(orc_Key, c_ZipFileTmp, orc_PathForZipFile);
   }
   else
   {
      // No key, just copy the original zip file as result
      osc_write_log_info("Creating Encrypted Zip File", "No key defined. Encryption not necessary.");

      c_Return = C_OscUtils::h_CopyFile(c_ZipFileTmp, orc_PathForZipFile, nullptr, &c_ErrorText);
   }

   // Remove the non encrypted temporary file
   if (std::remove(c_ZipFileTmp.c_str()) != 0)
   {
      if (c_Return == std::error_code(Errc::success))
      {
         // Do not overwrite an other error code
         c_Return = Errc::busy;
      }
   }

   if (c_Return != std::error_code(Errc::success))
   {
      osc_write_log_error("Creating Encrypted Zip File", "Encrypting zip file failed with error: " +
                          C_OscLoggingHandler::h_StwError(static_cast<int32_t>(c_Return.value())));
   }
   else
   {
      osc_write_log_info("Creating Encrypted Zip File", "Encrypted zip file created.");
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Decrypts and unpacks a zip file to a specific folder

   Sequence (if "orc_Key" is not an empty string):
   * decrypt file "orc_PathOfZipFile" to file "<orc_PathOfZipFile>_tmp"
   * extract content of "<orc_PathOfZipFile>_tmp" to path "orc_FolderPathToUnzip"
   * remove file "<orc_PathOfZipFile>_tmp"

   \param[in]  orc_PathOfZipFile       File path of encrypted zip package
   \param[in]  orc_FolderPathToUnzip   Folder to unpack zip package
   \param[in]  orc_Key                 Key for AES decryption (In case of an empty key, the file will not be decypted)
   \param[out] opc_ErrorMessage        Optional string for error messages

   \return
   std::error_code with Errc::success on success,
   Errc::rd_wr if input file does not exist or could not be loaded,
   Errc::busy if problems with deleting the temporary file,
   Errc::config if input file size is not a multiple of 16,
   Errc::checksum if input file is invalid (wrong key or corrupted),
   Errc::noact if output file already exists or could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_UnpackEncryptedZipFile(const std::string & orc_PathOfZipFile,
                                                               const std::string & orc_FolderPathToUnzip,
                                                               const std::string & orc_Key,
                                                               std::string * const opc_ErrorMessage)
{
   std::error_code c_Return = Errc::success;
   std::string c_ZipFileTmp = orc_PathOfZipFile + std::string("_tmp");
   bool q_TemporaryFileUsed = true;

   if (orc_Key != "")
   {
      c_Return = C_OscSecurityAesFile::h_DecryptFile(orc_Key, orc_PathOfZipFile, c_ZipFileTmp);
   }
   else
   {
      // No encrypted zip file. The original file can be used directly
      c_ZipFileTmp = orc_PathOfZipFile;
      q_TemporaryFileUsed = false;
      c_Return = Errc::success;
   }

   if (c_Return == Errc::success)
   {
      std::string c_ErrorText;
      const std::error_code c_UnzipResult = C_OscZipFile::h_UnpackZipFile(c_ZipFileTmp, orc_FolderPathToUnzip, &c_ErrorText);

      if (q_TemporaryFileUsed == true)
      {
         // Remove the non encrypted temporary file
         if (std::remove(c_ZipFileTmp.c_str()) != 0)
         {
            if (!c_UnzipResult)
            {
               c_Return = Errc::busy;
            }
         }
      }

      if (c_UnzipResult)
      {
         c_Return = c_UnzipResult;
      }

      if (c_Return != Errc::success)
      {
         osc_write_log_error("Unpacking Encrypted Zip File", "Unpacking decrypted zip file failed with error: " +
                             C_OscLoggingHandler::h_StwError(static_cast<int32_t>(c_Return.value())) +
                             " and error text: " + c_ErrorText.c_str());

         if (opc_ErrorMessage != nullptr)
         {
            *opc_ErrorMessage = c_ErrorText;
         }
      }
   }
   else
   {
      osc_write_log_error("Unpacking Encrypted Zip File", "Decrypting zip file failed with error: " +
                          C_OscLoggingHandler::h_StwError(static_cast<int32_t>(c_Return.value())));
   }

   return c_Return;
}
