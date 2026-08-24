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

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_OscSecurityAesFile.hpp"
#include "C_Md5Checksum.hpp"
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
/*! \brief   Convert hex string to binary key

   Helper to convert a 32-character hex string (MD5 digest) to a 16-byte binary key.

   \param[in]  orc_HexKey    Hex string (32 characters)
   \param[out] orau8_Key     Binary key (16 bytes)

   \return
   C_NO_ERR    conversion successful
   C_RANGE     invalid hex string
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_HexToKey(const std::string & orc_HexKey, uint8_t (&orau8_Key)[16])
{
   int32_t s32_Return = C_NO_ERR;

   if (orc_HexKey.length() < 32)
   {
      s32_Return = C_RANGE;
   }
   else
   {
      for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
      {
         const std::string c_Text = "0x" + SubStringCompat(orc_HexKey,
                                                           (static_cast<uint32_t>(u8_Index) * 2U) + 1U, 2U);
         orau8_Key[u8_Index] = static_cast<uint8_t>(std::stoi(c_Text));
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   AES-128-ECB encrypt data in memory

   Uses OpenSSL EVP for AES-128-ECB encryption with PKCS#7 padding.

   \param[in]   orau8_Key        128bit key
   \param[in]   orc_Input        Plaintext input
   \param[out]  orc_Output       Ciphertext output

   \return
   C_NO_ERR    success
   C_CONFIG    encryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_EncryptEcb(const uint8_t (&orau8_Key)[16], const std::vector<uint8_t> & orc_Input,
                             std::vector<uint8_t> & orc_Output)
{
   int32_t s32_Return = C_NO_ERR;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();
   if (pc_Ctx == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      // Set up AES-128-ECB encryption (no IV needed for ECB)
      const int x_Result = EVP_EncryptInit_ex(pc_Ctx, EVP_aes_128_ecb(), NULL, orau8_Key, NULL);
      if (x_Result != 1)
      {
         s32_Return = C_CONFIG;
      }
      else
      {
         const size_t u32_InputSize = orc_Input.size();
         const size_t u32_MaxOutputSize = u32_InputSize + 16U;
         std::vector<uint8_t> c_TempOutput(u32_MaxOutputSize);
         int x_OutLen = 0;
         int x_FinalLen = 0;

         if (EVP_EncryptUpdate(pc_Ctx, &c_TempOutput[0], &x_OutLen,
                               &orc_Input[0], static_cast<int>(u32_InputSize)) != 1)
         {
            s32_Return = C_CONFIG;
         }
         else
         {
            if (EVP_EncryptFinal_ex(pc_Ctx, &c_TempOutput[static_cast<size_t>(x_OutLen)], &x_FinalLen) != 1)
            {
               s32_Return = C_CONFIG;
            }
            else
            {
               orc_Output.assign(c_TempOutput.begin(),
                                 c_TempOutput.begin() + static_cast<size_t>(x_OutLen) +
                                    static_cast<size_t>(x_FinalLen));
            }
         }
      }

      EVP_CIPHER_CTX_free(pc_Ctx);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   AES-128-ECB decrypt data in memory

   Uses OpenSSL EVP for AES-128-ECB decryption with PKCS#7 padding.

   \param[in]   orau8_Key        128bit key
   \param[in]   orc_Input        Ciphertext input
   \param[out]  orc_Output       Plaintext output

   \return
   C_NO_ERR    success
   C_CONFIG    decryption failed
   C_CHECKSUM  invalid padding (likely wrong key or corrupted data)
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_DecryptEcb(const uint8_t (&orau8_Key)[16], const std::vector<uint8_t> & orc_Input,
                             std::vector<uint8_t> & orc_Output)
{
   int32_t s32_Return = C_NO_ERR;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();
   if (pc_Ctx == NULL)
   {
      s32_Return = C_CONFIG;
   }
   else
   {
      // Set up AES-128-ECB decryption
      const int x_Result = EVP_DecryptInit_ex(pc_Ctx, EVP_aes_128_ecb(), NULL, orau8_Key, NULL);
      if (x_Result != 1)
      {
         s32_Return = C_CONFIG;
      }
      else
      {
         const size_t u32_InputSize = orc_Input.size();
         std::vector<uint8_t> c_TempOutput(u32_InputSize);
         int x_OutLen = 0;
         int x_FinalLen = 0;

         if (EVP_DecryptUpdate(pc_Ctx, &c_TempOutput[0], &x_OutLen,
                               &orc_Input[0], static_cast<int>(u32_InputSize)) != 1)
         {
            s32_Return = C_CONFIG;
         }
         else
         {
            // Finalize: verify PKCS#7 padding
            if (EVP_DecryptFinal_ex(pc_Ctx, &c_TempOutput[static_cast<size_t>(x_OutLen)], &x_FinalLen) != 1)
            {
               s32_Return = C_CHECKSUM;
            }
            else
            {
               orc_Output.assign(c_TempOutput.begin(),
                                 c_TempOutput.begin() + static_cast<size_t>(x_OutLen) +
                                    static_cast<size_t>(x_FinalLen));
            }
         }
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
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?& anyway
   const std::string c_AesKey = stw::md5::C_Md5Checksum::GetMD5(
      reinterpret_cast<const uint8_t *>(orc_Key.c_str()), orc_Key.length());

   tgl_assert(c_AesKey.length() == 32); //really should be 16 bytes, resp. 32 hex characters

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
   c_InputFileStream.read(reinterpret_cast<char_t *>(&c_InputData[0]), u32_InputFileSize);
   //check for error
   q_HasFailed = c_InputFileStream.fail();
   //close file
   c_InputFileStream.close();

   if (q_HasFailed == true)
   {
      return Errc::rd_wr;
   }

   //do the encryption:
   uint8_t au8_Key[16];
   vector<uint8_t> c_EncryptedData;
   std::ofstream c_OutputFileStream;

   //convert key from string to array:
   const int32_t s32_HexResult = mh_HexToKey(c_AesKey, au8_Key);
   if (s32_HexResult != C_NO_ERR)
   {
      return Errc::range;
   }

   const int32_t s32_EncResult = mh_EncryptEcb(au8_Key, c_InputData, c_EncryptedData);
   if (s32_EncResult != C_NO_ERR)
   {
      return Errc::config;
   }

   //save to output file:
   c_OutputFileStream.open(orc_OutFilePath.c_str(), std::ofstream::binary | std::ofstream::trunc);
   if (c_OutputFileStream.is_open())
   {
      //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep
      // !"=?& anyway
      c_OutputFileStream.write(reinterpret_cast<const char_t *>(&c_EncryptedData[0]),
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
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?& anyway
   const std::string c_AesKey = stw::md5::C_Md5Checksum::GetMD5(
      reinterpret_cast<const uint8_t *>(orc_Key.c_str()), orc_Key.length());

   tgl_assert(c_AesKey.length() == 32); //really should be 16 bytes, resp. 32 hex characters

   //check whether input file exists:
   if (TglFileExists(orc_InFilePath) == false)
   {
      return Errc::rd_wr;
   }

   //load data from input file:
   vector<uint8_t> c_InputData;
   std::ifstream c_InputFileStream;
   const uint32_t u32_InputFileSize = TglFileSize(orc_InFilePath);

   //is the file correctly padded ?
   if ((u32_InputFileSize % 16U) != 0U)
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
   c_InputFileStream.read(reinterpret_cast<char_t *>(&c_InputData[0]), c_InputData.size());
   //check for error
   q_HasFailed = c_InputFileStream.fail();
   //close file
   c_InputFileStream.close();

   if (q_HasFailed == true)
   {
      return Errc::rd_wr;
   }

   //do the decryption:
   uint8_t au8_Key[16];
   vector<uint8_t> c_DecryptedData;
   std::ofstream c_OutputFileStream;

   //convert key from string to array:
   const int32_t s32_HexResult = mh_HexToKey(c_AesKey, au8_Key);
   if (s32_HexResult != C_NO_ERR)
   {
      return Errc::range;
   }

   const int32_t s32_DecResult = mh_DecryptEcb(au8_Key, c_InputData, c_DecryptedData);
   if (s32_DecResult == C_CHECKSUM)
   {
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
      c_OutputFileStream.write(reinterpret_cast<const char_t *>(&c_DecryptedData[0]),
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
   std::error_code c_Return;

   const std::string c_ZipFileTmp = orc_PathForZipFile + std::string("_tmp");
   std::string c_ErrorText;

   const int32_t s32_ZipResult = C_OscZipFile::h_CreateZipFile(orc_FolderPathToZip, orc_SupFiles, c_ZipFileTmp,
                                                                &c_ErrorText);
   if (s32_ZipResult != C_NO_ERR)
   {
      osc_write_log_error("Creating Encrypted Zip File", "Creating zip file failed with error: " +
                          C_OscLoggingHandler::h_StwError(s32_ZipResult) +
                          " and error text: " + c_ErrorText.c_str());

      if (opc_ErrorMessage != NULL)
      {
         *opc_ErrorMessage = c_ErrorText;
      }
      // Map the int32_t error to our error_code
      return std::error_code(s32_ZipResult, STWErrorCategory::Instance());
   }

   if (orc_Key != "")
   {
      c_Return = C_OscSecurityAesFile::h_EncryptFile(orc_Key, c_ZipFileTmp, orc_PathForZipFile);
   }
   else
   {
      // No key, just copy the original zip file as result
      osc_write_log_info("Creating Encrypted Zip File", "No key defined. Encryption not necessary.");

      const int32_t s32_CopyResult = C_OscUtils::h_CopyFile(c_ZipFileTmp, orc_PathForZipFile, NULL, &c_ErrorText);
      if (s32_CopyResult != C_NO_ERR)
      {
         c_Return = std::error_code(s32_CopyResult, STWErrorCategory::Instance());
      }
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
   std::error_code c_Return;
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
      const int32_t s32_UnzipResult = C_OscZipFile::h_UnpackZipFile(c_ZipFileTmp, orc_FolderPathToUnzip, &c_ErrorText);

      if (q_TemporaryFileUsed == true)
      {
         // Remove the non encrypted temporary file
         if (std::remove(c_ZipFileTmp.c_str()) != 0)
         {
            if (s32_UnzipResult == C_NO_ERR)
            {
               c_Return = Errc::busy;
            }
         }
      }

      if (s32_UnzipResult != C_NO_ERR)
      {
         c_Return = std::error_code(s32_UnzipResult, STWErrorCategory::Instance());
      }

      if (c_Return != Errc::success)
      {
         osc_write_log_error("Unpacking Encrypted Zip File", "Unpacking decrypted zip file failed with error: " +
                             C_OscLoggingHandler::h_StwError(static_cast<int32_t>(c_Return.value())) +
                             " and error text: " + c_ErrorText.c_str());

         if (opc_ErrorMessage != NULL)
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
