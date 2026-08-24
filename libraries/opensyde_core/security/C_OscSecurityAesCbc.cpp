//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core AES text encryption utility

   see header in .h file for details.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include <openssl/evp.h>

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscSecurityAesCbc.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Encrypt given array with AES-128 using CBC mode and PKCS#7 padding

   Uses OpenSSL EVP to perform AES-128-CBC encryption with PKCS#7 padding.

   Steps:
   * create cipher context
   * set up AES-128-CBC encryption with PKCS#7 padding
   * perform encryption
   * finalize (flushes any remaining ciphertext from padding)
   * write to output array

   \param[in]   orau8_Key           128bit key to use for encryption
   \param[in]   orau8_InitVector    128bit initialization vector
   \param[in]   orc_Input           Input data
   \param[out]  orc_Output          Output data

   \return
   std::error_code with Errc::success on success, Errc::config on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesCbc::h_Encrypt(const uint8_t (&orau8_Key)[hu32_KEY_LENGTH],
                                               const uint8_t (&orau8_InitVector)[hu32_IV_LENGTH],
                                               const std::vector<uint8_t> & orc_Input,
                                               std::vector<uint8_t> & orc_Output)
{
   // Allocate cipher context
   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();
   if (pc_Ctx == NULL)
   {
      return Errc::config;
   }

   // Set up AES-128-CBC encryption
   const int x_Result = EVP_EncryptInit_ex(pc_Ctx, EVP_aes_128_cbc(), NULL, orau8_Key, orau8_InitVector);
   if (x_Result != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      return Errc::config;
   }

   // PKCS#7 padding is enabled by default in OpenSSL EVP
   // Determine maximum output size (may be up to one block larger than input due to padding)
   const size_t u32_InputSize = orc_Input.size();
   const size_t u32_MaxOutputSize = u32_InputSize + 16U; // AES block size is 16 bytes
   std::vector<uint8_t> c_TempOutput(u32_MaxOutputSize);
   int x_OutLen = 0;
   int x_FinalLen = 0;

   // Perform encryption
   if (EVP_EncryptUpdate(pc_Ctx, &c_TempOutput[0], &x_OutLen,
                         &orc_Input[0], static_cast<int>(u32_InputSize)) != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      return Errc::config;
   }

   // Finalize (flush any remaining ciphertext from padding)
   if (EVP_EncryptFinal_ex(pc_Ctx, &c_TempOutput[static_cast<size_t>(x_OutLen)], &x_FinalLen) != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      return Errc::config;
   }

   // Copy output
   orc_Output.assign(c_TempOutput.begin(),
                     c_TempOutput.begin() + static_cast<size_t>(x_OutLen) + static_cast<size_t>(x_FinalLen));

   EVP_CIPHER_CTX_free(pc_Ctx);
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Decrypt given text with AES-128 using CBC mode and PKCS#7 padding

   Uses OpenSSL EVP to perform AES-128-CBC decryption with PKCS#7 padding.

   Steps:
   * create cipher context
   * set up AES-128-CBC decryption with PKCS#7 padding
   * perform decryption
   * finalize (PKCS#7 padding is verified and stripped automatically)
   * write to output array

   \param[in]   orau8_Key           128bit key to use for decryption
   \param[in]   orau8_InitVector    128bit initialization vector
   \param[in]   orc_Input           Encrypted input data
   \param[out]  orc_Output          Decrypted output data

   \return
   std::error_code with Errc::success on success,
   Errc::config if input length is not a multiple of 16 bytes or decryption failed,
   Errc::checksum if padding is invalid (likely wrong key or corrupted data)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityAesCbc::h_Decrypt(const uint8_t (&orau8_Key)[hu32_KEY_LENGTH],
                                               const uint8_t (&orau8_InitVector)[hu32_IV_LENGTH],
                                               const std::vector<uint8_t> & orc_Input,
                                               std::vector<uint8_t> & orc_Output)
{
   const size_t u32_InputSize = orc_Input.size();

   // check inputs: input text correctly padded to 16 bytes?
   if ((u32_InputSize % 16U) != 0U)
   {
      return Errc::config;
   }

   // Allocate cipher context
   EVP_CIPHER_CTX * const pc_Ctx = EVP_CIPHER_CTX_new();
   if (pc_Ctx == NULL)
   {
      return Errc::config;
   }

   // Set up AES-128-CBC decryption
   const int x_Result = EVP_DecryptInit_ex(pc_Ctx, EVP_aes_128_cbc(), NULL, orau8_Key, orau8_InitVector);
   if (x_Result != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      return Errc::config;
   }

   // PKCS#7 padding is enabled by default in OpenSSL EVP
   // Output size will be at most input size
   std::vector<uint8_t> c_TempOutput(u32_InputSize);
   int x_OutLen = 0;
   int x_FinalLen = 0;

   // Perform decryption
   if (EVP_DecryptUpdate(pc_Ctx, &c_TempOutput[0], &x_OutLen,
                         &orc_Input[0], static_cast<int>(u32_InputSize)) != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      return Errc::config;
   }

   // Finalize: this is where PKCS#7 padding is verified.
   // If the padding is invalid (wrong key or corrupted data), this will fail.
   if (EVP_DecryptFinal_ex(pc_Ctx, &c_TempOutput[static_cast<size_t>(x_OutLen)], &x_FinalLen) != 1)
   {
      EVP_CIPHER_CTX_free(pc_Ctx);
      // Invalid padding -> likely incorrect key or corrupted data
      return Errc::checksum;
   }

   // Copy output (padding already stripped by EVP)
   orc_Output.assign(c_TempOutput.begin(),
                     c_TempOutput.begin() + static_cast<size_t>(x_OutLen) +
                        static_cast<size_t>(x_FinalLen));

   EVP_CIPHER_CTX_free(pc_Ctx);
   return Errc::success;
}
