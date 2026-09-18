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
#include <cstddef>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include <cstdint>
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_OscSecurityAesFile.hpp"
#include "C_OscEndian.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::opensyde_core;
using namespace std;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
namespace
{
///Byte offsets of the header fields. The layout table in the class header is the reference; the
///static_assert below keeps the two in step, so a layout change that forgets one of them fails
///to compile instead of writing a header the reader cannot parse.
constexpr uint32_t mu32_MAGIC_LENGTH = 6U;
constexpr uint32_t mu32_OFFSET_MAGIC = 0U;
constexpr uint32_t mu32_OFFSET_VERSION = mu32_OFFSET_MAGIC + mu32_MAGIC_LENGTH;
constexpr uint32_t mu32_OFFSET_ALGORITHM = mu32_OFFSET_VERSION + 1U;
constexpr uint32_t mu32_OFFSET_ITERATIONS = mu32_OFFSET_ALGORITHM + 1U;
constexpr uint32_t mu32_OFFSET_SALT = mu32_OFFSET_ITERATIONS + 4U;
constexpr uint32_t mu32_OFFSET_NONCE = mu32_OFFSET_SALT + C_OscSecurityAesFile::hu32_SALT_LENGTH;
constexpr uint32_t mu32_OFFSET_TAG = mu32_OFFSET_NONCE + C_OscSecurityAesFile::hu32_NONCE_LENGTH;
static_assert((mu32_OFFSET_TAG + C_OscSecurityAesFile::hu32_TAG_LENGTH) == C_OscSecurityAesFile::hu32_HEADER_LENGTH,
              "encrypted file header layout does not add up to hu32_HEADER_LENGTH");
constexpr uint8_t mau8_MAGIC[mu32_MAGIC_LENGTH] = {'O', 'S', 'Y', 'E', 'N', 'C'};
}

/* -- Types --------------------------------------------------------------------------------------------------------- */
namespace
{
///The per-file part of the header. Version and algorithm are fixed by this build and are not
///stored here: the reader rejects anything else.
struct T_Header
{
   uint32_t u32_Iterations;
   uint8_t au8_Salt[C_OscSecurityAesFile::hu32_SALT_LENGTH];
   uint8_t au8_Nonce[C_OscSecurityAesFile::hu32_NONCE_LENGTH];
   uint8_t au8_Tag[C_OscSecurityAesFile::hu32_TAG_LENGTH];
};

///Key material that wipes itself: the derived key must not stay on the stack after use, on any
///path out of the function, and a destructor is the one place that is true for every path.
class C_KeyBuffer
{
public:
   C_KeyBuffer(void) :
      au8_Key()
   {
   }

   ~C_KeyBuffer(void)
   {
      OPENSSL_cleanse(&au8_Key[0], C_OscSecurityAesFile::hu32_KEY_LENGTH);
   }

   C_KeyBuffer(const C_KeyBuffer &) = delete;
   C_KeyBuffer & operator =(const C_KeyBuffer &) = delete;

   uint8_t au8_Key[C_OscSecurityAesFile::hu32_KEY_LENGTH];
};
}

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load a whole file into memory

   \param[in]   orc_Path   File to read
   \param[out]  orc_Data   File content

   \return
   Errc::success   file read
   Errc::rd_wr     file does not exist, could not be opened or could not be read
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_ReadFile(const std::string & orc_Path, std::vector<uint8_t> & orc_Data)
{
   if (TglFileExists(orc_Path) == false)
   {
      return Errc::rd_wr;
   }

   std::ifstream c_Stream(orc_Path.c_str(), std::ifstream::binary);
   if (c_Stream.is_open() == false)
   {
      return Errc::rd_wr;
   }

   orc_Data.resize(TglFileSize(orc_Path));
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?& anyway
   c_Stream.read(reinterpret_cast<char *>(orc_Data.data()), static_cast<std::streamsize>(orc_Data.size()));
   if (c_Stream.fail())
   {
      return Errc::rd_wr;
   }
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write a whole file from memory, replacing any existing file

   \param[in]   orc_Path   File to write
   \param[in]   orc_Data   Content

   \return
   Errc::success   file written
   Errc::noact     file could not be opened or written
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_WriteFile(const std::string & orc_Path, const std::vector<uint8_t> & orc_Data)
{
   std::ofstream c_Stream(orc_Path.c_str(), std::ofstream::binary | std::ofstream::trunc);
   if (c_Stream.is_open() == false)
   {
      return Errc::noact;
   }
   //lint -e{9176} //no problems as long as char has the same size as uint8; if not we'd be in deep !"=?& anyway
   c_Stream.write(reinterpret_cast<const char *>(orc_Data.data()), static_cast<std::streamsize>(orc_Data.size()));
   if (c_Stream.fail())
   {
      return Errc::noact;
   }
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write the header into the first hu32_HEADER_LENGTH bytes of a buffer

   \param[in]   orc_Header   Per-file header fields
   \param[out]  opu8_Data    Buffer of at least hu32_HEADER_LENGTH bytes
*/
//----------------------------------------------------------------------------------------------------------------------
static void mh_SerializeHeader(const T_Header & orc_Header, uint8_t * const opu8_Data)
{
   (void)std::memcpy(&opu8_Data[mu32_OFFSET_MAGIC], &mau8_MAGIC[0], mu32_MAGIC_LENGTH);
   opu8_Data[mu32_OFFSET_VERSION] = C_OscSecurityAesFile::hu8_FORMAT_VERSION;
   opu8_Data[mu32_OFFSET_ALGORITHM] = C_OscSecurityAesFile::hu8_ALGO_AES256_GCM;
   C_OscEndian::h_SetU32Little(orc_Header.u32_Iterations, &opu8_Data[mu32_OFFSET_ITERATIONS]);
   (void)std::memcpy(&opu8_Data[mu32_OFFSET_SALT], &orc_Header.au8_Salt[0], C_OscSecurityAesFile::hu32_SALT_LENGTH);
   (void)std::memcpy(&opu8_Data[mu32_OFFSET_NONCE], &orc_Header.au8_Nonce[0],
                     C_OscSecurityAesFile::hu32_NONCE_LENGTH);
   (void)std::memcpy(&opu8_Data[mu32_OFFSET_TAG], &orc_Header.au8_Tag[0], C_OscSecurityAesFile::hu32_TAG_LENGTH);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read and validate the header at the start of an encrypted file

   Files written by the previous format carry no header at all and are rejected here rather than
   silently mis-decrypted, as is any version or algorithm this build does not know.

   \param[in]   orc_File     Whole file content
   \param[out]  orc_Header   Per-file header fields

   \return
   Errc::success   header valid
   Errc::config    too short, wrong magic, unknown version or algorithm, or a zero iteration count
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_ParseHeader(const std::vector<uint8_t> & orc_File, T_Header & orc_Header)
{
   if (orc_File.size() < C_OscSecurityAesFile::hu32_HEADER_LENGTH)
   {
      return Errc::config;
   }
   if (std::memcmp(&orc_File[mu32_OFFSET_MAGIC], &mau8_MAGIC[0], mu32_MAGIC_LENGTH) != 0)
   {
      return Errc::config;
   }
   if ((orc_File[mu32_OFFSET_VERSION] != C_OscSecurityAesFile::hu8_FORMAT_VERSION) ||
       (orc_File[mu32_OFFSET_ALGORITHM] != C_OscSecurityAesFile::hu8_ALGO_AES256_GCM))
   {
      return Errc::config;
   }
   orc_Header.u32_Iterations = C_OscEndian::h_GetU32Little(&orc_File[mu32_OFFSET_ITERATIONS]);
   if (orc_Header.u32_Iterations == 0U)
   {
      return Errc::config;
   }
   (void)std::memcpy(&orc_Header.au8_Salt[0], &orc_File[mu32_OFFSET_SALT], C_OscSecurityAesFile::hu32_SALT_LENGTH);
   (void)std::memcpy(&orc_Header.au8_Nonce[0], &orc_File[mu32_OFFSET_NONCE],
                     C_OscSecurityAesFile::hu32_NONCE_LENGTH);
   (void)std::memcpy(&orc_Header.au8_Tag[0], &orc_File[mu32_OFFSET_TAG], C_OscSecurityAesFile::hu32_TAG_LENGTH);
   return Errc::success;
}

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
   Errc::success   key derived
   Errc::config    derivation failed
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_DeriveKey(const std::string & orc_Password,
                                    const uint8_t (&orau8_Salt)[C_OscSecurityAesFile::hu32_SALT_LENGTH],
                                    const uint32_t ou32_Iterations,
                                    uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH])
{
   //OpenSSL reports 1 for success, the opposite polarity to the STW codes
   const int x_Result = PKCS5_PBKDF2_HMAC(orc_Password.c_str(), static_cast<int>(orc_Password.length()),
                                          &orau8_Salt[0], static_cast<int>(C_OscSecurityAesFile::hu32_SALT_LENGTH),
                                          static_cast<int>(ou32_Iterations), EVP_sha256(),
                                          static_cast<int>(C_OscSecurityAesFile::hu32_KEY_LENGTH), &orau8_Key[0]);

   return (x_Result == 1) ? Errc::success : Errc::config;
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
   Errc::success   encrypted
   Errc::config    encryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_EncryptGcm(const uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH],
                                     const uint8_t (&orau8_Nonce)[C_OscSecurityAesFile::hu32_NONCE_LENGTH],
                                     const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output,
                                     uint8_t (&orau8_Tag)[C_OscSecurityAesFile::hu32_TAG_LENGTH])
{
   std::error_code c_Return = Errc::success;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();

   if (pc_Ctx == nullptr)
   {
      c_Return = Errc::config;
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
         c_Return = Errc::config;
      }
      else if ((orc_Input.size() > 0U) &&
               (EVP_EncryptUpdate(pc_Ctx, orc_Output.data(), &x_OutLen, orc_Input.data(),
                                  static_cast<int>(orc_Input.size())) != 1))
      {
         c_Return = Errc::config;
      }
      // GCM adds no padding, so x_OutLen equals the input size and the finalise
      // pointer is one-past-the-end. data()+n is valid there; &vec[n] is not.
      else if (EVP_EncryptFinal_ex(pc_Ctx, orc_Output.data() + static_cast<size_t>(x_OutLen), &x_FinalLen) != 1)
      {
         c_Return = Errc::config;
      }
      else if (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_GET_TAG,
                                   static_cast<int>(C_OscSecurityAesFile::hu32_TAG_LENGTH), &orau8_Tag[0]) != 1)
      {
         c_Return = Errc::config;
      }
      else
      {
         orc_Output.resize(static_cast<size_t>(x_OutLen) + static_cast<size_t>(x_FinalLen));
      }

      EVP_CIPHER_CTX_free(pc_Ctx);
   }
   return c_Return;
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
   Errc::success    decrypted
   Errc::checksum   authentication failed (wrong password or tampered data)
   Errc::config     decryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
static std::error_code mh_DecryptGcm(const uint8_t (&orau8_Key)[C_OscSecurityAesFile::hu32_KEY_LENGTH],
                                     const uint8_t (&orau8_Nonce)[C_OscSecurityAesFile::hu32_NONCE_LENGTH],
                                     const uint8_t (&orau8_Tag)[C_OscSecurityAesFile::hu32_TAG_LENGTH],
                                     const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output)
{
   std::error_code c_Return = Errc::success;

   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();

   if (pc_Ctx == nullptr)
   {
      c_Return = Errc::config;
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
         c_Return = Errc::config;
      }
      else if ((orc_Input.size() > 0U) &&
               (EVP_DecryptUpdate(pc_Ctx, orc_Output.data(), &x_OutLen, orc_Input.data(),
                                  static_cast<int>(orc_Input.size())) != 1))
      {
         c_Return = Errc::config;
      }
      else if (EVP_CIPHER_CTX_ctrl(pc_Ctx, EVP_CTRL_GCM_SET_TAG,
                                   static_cast<int>(C_OscSecurityAesFile::hu32_TAG_LENGTH), &au8_TagCopy[0]) != 1)
      {
         c_Return = Errc::config;
      }
      // A non-positive result here means the tag did not verify.
      else if (EVP_DecryptFinal_ex(pc_Ctx, orc_Output.data() + static_cast<size_t>(x_OutLen), &x_FinalLen) <= 0)
      {
         orc_Output.clear();
         c_Return = Errc::checksum;
      }
      else
      {
         orc_Output.resize(static_cast<size_t>(x_OutLen) + static_cast<size_t>(x_FinalLen));
      }

      EVP_CIPHER_CTX_free(pc_Ctx);
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create encrypted file

   Steps:
   * load the input file to RAM
   * draw a fresh salt and nonce, derive the key with PBKDF2 (see mh_DeriveKey)
   * AES-256-GCM encrypt in RAM (so not suitable for very large files)
   * write header + ciphertext (layout in the class header)

   If the output file already exists it is overwritten.

   \param[in]  orc_Key           Password; the AES key is derived from it, never used directly
   \param[in]  orc_InFilePath    Path to input file
   \param[in]  orc_OutFilePath   Path to resulting output file

   \return
   Errc::success   file written
   Errc::rd_wr     input file does not exist or could not be loaded
   Errc::config    random source, key derivation or encryption failed
   Errc::noact     output could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_EncryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                                    const std::string & orc_OutFilePath)
{
   std::vector<uint8_t> c_Plaintext;
   std::error_code c_Result = mh_ReadFile(orc_InFilePath, c_Plaintext);
   if (c_Result)
   {
      return c_Result;
   }

   T_Header c_Header;
   c_Header.u32_Iterations = hu32_PBKDF2_ITERATIONS;
   //fresh salt and nonce per file; a repeated nonce under one key breaks GCM
   if ((RAND_bytes(&c_Header.au8_Salt[0], static_cast<int>(hu32_SALT_LENGTH)) != 1) ||
       (RAND_bytes(&c_Header.au8_Nonce[0], static_cast<int>(hu32_NONCE_LENGTH)) != 1))
   {
      return Errc::config;
   }

   C_KeyBuffer c_Key;
   c_Result = mh_DeriveKey(orc_Key, c_Header.au8_Salt, c_Header.u32_Iterations, c_Key.au8_Key);
   if (c_Result)
   {
      return c_Result;
   }

   std::vector<uint8_t> c_Ciphertext;
   c_Result = mh_EncryptGcm(c_Key.au8_Key, c_Header.au8_Nonce, c_Plaintext, c_Ciphertext, c_Header.au8_Tag);
   if (c_Result)
   {
      return c_Result;
   }

   std::vector<uint8_t> c_File(hu32_HEADER_LENGTH);
   mh_SerializeHeader(c_Header, c_File.data());
   c_File.insert(c_File.end(), c_Ciphertext.begin(), c_Ciphertext.end());

   return mh_WriteFile(orc_OutFilePath, c_File);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create decrypted file

   Steps:
   * load the input file to RAM
   * parse and validate the header (see mh_ParseHeader)
   * derive the key from the password and the stored salt
   * AES-256-GCM decrypt in RAM, verifying the stored tag
   * write the plaintext

   If the output file already exists it is overwritten.

   \param[in]  orc_Key           Password the file was encrypted with
   \param[in]  orc_InFilePath    Path to input file
   \param[in]  orc_OutFilePath   Path to resulting output file

   \return
   Errc::success    file written
   Errc::rd_wr      input file does not exist or could not be loaded
   Errc::config     no valid header (too short, wrong magic, unknown version or algorithm), or decryption failed
   Errc::checksum   authentication failed: wrong password, truncated or tampered data
   Errc::noact      output could not be written
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesFile::h_DecryptFile(const std::string & orc_Key, const std::string & orc_InFilePath,
                                                    const std::string & orc_OutFilePath)
{
   std::vector<uint8_t> c_File;
   std::error_code c_Result = mh_ReadFile(orc_InFilePath, c_File);
   if (c_Result)
   {
      return c_Result;
   }

   T_Header c_Header;
   c_Result = mh_ParseHeader(c_File, c_Header);
   if (c_Result)
   {
      return c_Result;
   }

   C_KeyBuffer c_Key;
   c_Result = mh_DeriveKey(orc_Key, c_Header.au8_Salt, c_Header.u32_Iterations, c_Key.au8_Key);
   if (c_Result)
   {
      return c_Result;
   }

   const std::vector<uint8_t> c_Ciphertext(c_File.begin() + static_cast<std::ptrdiff_t>(hu32_HEADER_LENGTH),
                                           c_File.end());
   std::vector<uint8_t> c_Plaintext;
   c_Result = mh_DecryptGcm(c_Key.au8_Key, c_Header.au8_Nonce, c_Header.au8_Tag, c_Ciphertext, c_Plaintext);
   if (c_Result)
   {
      //wrong password, truncation or tampering: all indistinguishable, all rejected as Errc::checksum
      return c_Result;
   }

   return mh_WriteFile(orc_OutFilePath, c_Plaintext);
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
