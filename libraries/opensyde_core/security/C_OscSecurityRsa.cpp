//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Access to RSA signature handling

   Access to RSA signature handling

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>

#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/evp.h"

#include "stwerrors.hpp"
#include "C_OscSecurityRsa.hpp"

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
/*! \brief  Create signature

   Create signature over message data using private key.

   Formats:
   Key:
   * binary in PKCS#8 format
   Message:
   * binary data
   Signature:
   * binary data

   \param[in]      orc_PrivateKey     Private key in PKCS#8 format
   \param[in]      orc_Message        Message
   \param[out]     orc_Signature      Created signature

   \return
   STW error codes

   \retval   C_NO_ERR   Message encrypted
   \retval   C_RANGE    Invalid key; key or message have zero length
   \retval   C_NOACT    Could not encrypt message
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSecurityRsa::h_SignSignature(const std::vector<uint8_t> & orc_PrivateKey,
                                          const std::vector<uint8_t> & orc_Message,
                                          std::vector<uint8_t> & orc_Signature)
{
   int32_t s32_Retval = C_RANGE;

   //Get private key information from PKCS#8 dump:
   PKCS8_PRIV_KEY_INFO * const pc_Key = d2i_PKCS8_PRIV_KEY_INFO(
      NULL, &pu8_Data,
      static_cast<long>(orc_PrivateKey.size())); //lint !e970 //using type to match library interface
   if (pc_Key != NULL)
   {
      const uint8_t * pu8_Data = &orc_PrivateKey[0];

      //Get private key information from PKCS#8 dump:
      PKCS8_PRIV_KEY_INFO * const pc_Key = d2i_PKCS8_PRIV_KEY_INFO(
         NULL, &pu8_Data,
         static_cast<long>(orc_PrivateKey.size())); //lint !e970 //using type to match library interface
      if (pc_Key != NULL)
      {
         //Use EVP_PKEY_sign API (OpenSSL 3.0+) instead of deprecated RSA_private_encrypt
         EVP_PKEY_CTX * const pc_Ctx = EVP_PKEY_CTX_new(pc_EvpKey, NULL);
         EVP_PKEY_free(pc_EvpKey);
         s32_Retval = C_NOACT;

         if (pc_Ctx != NULL)
         {
            int x_Result = EVP_PKEY_sign_init(pc_Ctx); //lint !e970 !e8080 //using type to match library interface
            if (x_Result == 1)
            {
               x_Result = EVP_PKEY_CTX_set_rsa_padding(pc_Ctx, RSA_PKCS1_PADDING);
            }
            if (x_Result == 1)
            {
               //Determine output size:
               size_t un_OutLen = 0;
               x_Result = EVP_PKEY_sign(pc_Ctx, NULL, &un_OutLen, &orc_Message[0], orc_Message.size());
               if (x_Result == 1)
               {
                  orc_EncryptedMessage.resize(un_OutLen);
                  //Perform the actual signing:
                  x_Result = EVP_PKEY_sign(pc_Ctx, &orc_EncryptedMessage[0], &un_OutLen,
                                           &orc_Message[0], orc_Message.size());
                  if (x_Result == 1)
                  {
                     orc_EncryptedMessage.resize(un_OutLen);
                     s32_Retval = C_NO_ERR;
                  }
               }
            }
            EVP_PKEY_CTX_free(pc_Ctx);
         }
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Verify signature

   Extracts data from specified signature using a public key.
   Compares extracted data to provided original data.

   Formats:
   Key:
   * is expected to be provided in X.509 format.
   * When looking at a .PEM file:
   ** effectively everything between the "BEGIN CERTIFICATE" and "END CERTIFICATE" lines converted from base64 to binary.
   ExpectedMessage:
   * binary data
   Signature:
   * binary data

   \param[in]   orc_PublicKey          Public key in X509 format
   \param[in]   orc_ExpectedMessage    Expected message to compare against
   \param[in]   orc_Signature          Signature to parse
   \param[out]  orq_Valid              true: expected message identical to data extracted from orc_EncryptedMessage
                                       false: not the thing above

   \return
   STW error codes

   \retval   C_NO_ERR   Operation done; check for result
   \retval   C_RANGE    Invalid key; key, message or encrypted message have zero length
   \retval   C_NOACT    Could not decrypt message
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSecurityRsa::h_VerifySignature(const std::vector<uint8_t> & orc_PublicKey,
                                            const std::vector<uint8_t> & orc_ExpectedMessage,
                                            const std::vector<uint8_t> & orc_Signature, bool & orq_Valid)
{
   int32_t s32_Retval = C_RANGE;

   orq_Valid = false;

   if ((orc_PublicKey.size() > 0) && (orc_ExpectedMessage.size() > 0) && (orc_Signature.size() > 0))
   {
      //get RSA structure from binary key:
      const uint8_t * pu8_Data = &orc_PublicKey[0];

      //Extract X509 data from binary key data:
      X509 * const pc_X509Data = d2i_X509(
         NULL, &pu8_Data,
         static_cast<long>(orc_PublicKey.size())); //lint !e970 //using type to match library interface
      if (pc_X509Data != NULL)
      {
         //Use EVP_PKEY_verify_recover API (OpenSSL 3.0+) instead of deprecated RSA_public_decrypt
         EVP_PKEY_CTX * const pc_Ctx = EVP_PKEY_CTX_new(pc_EvpKey, NULL);
         EVP_PKEY_free(pc_EvpKey);
         s32_Retval = C_NOACT;

         if (pc_Ctx != NULL)
         {
            int x_Result = EVP_PKEY_verify_recover_init(pc_Ctx); //lint !e970 !e8080 //using type to match library
                                                                  // interface
            if (x_Result == 1)
            {
               x_Result = EVP_PKEY_CTX_set_rsa_padding(pc_Ctx, RSA_PKCS1_PADDING);
            }
            if (x_Result == 1)
            {
               //Determine output size:
               size_t un_OutLen = 0;
               x_Result = EVP_PKEY_verify_recover(pc_Ctx, NULL, &un_OutLen,
                                                   &orc_EncryptedMessage[0], orc_EncryptedMessage.size());
               if (x_Result == 1)
               {
                  std::vector<uint8_t> c_DecryptedMessage;
                  c_DecryptedMessage.resize(un_OutLen);

                  //Perform the actual recovery (decryption with public key):
                  x_Result = EVP_PKEY_verify_recover(pc_Ctx, &c_DecryptedMessage[0], &un_OutLen,
                                                      &orc_EncryptedMessage[0], orc_EncryptedMessage.size());
                  if (x_Result == 1)
                  {
                     s32_Retval = C_NO_ERR;
                     c_DecryptedMessage.resize(un_OutLen);

                     //compare decrypted messages with expected message:
                     if (c_DecryptedMessage.size() == orc_Message.size())
                     {
                        const int x_DiffResult = //lint !e970 !e8080 //using type to match library interface
                                                 std::memcmp(&c_DecryptedMessage[0], &orc_Message[0],
                                                             orc_Message.size());
                        if (x_DiffResult == 0)
                        {
                           orq_Valid = true; //we have a winner
                        }
                     }
                  }
               }
               EVP_PKEY_CTX_free(pc_VerifyCtx);
            }
            EVP_PKEY_CTX_free(pc_Ctx);
         }
      }
   }

   return s32_Retval;
}
