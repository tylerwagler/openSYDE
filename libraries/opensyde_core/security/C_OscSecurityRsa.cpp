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
#include <system_error>

#include "openssl/x509.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
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
   std::error_code with Errc::success if the message was signed,
   Errc::range on an invalid key or if key or message have zero length,
   Errc::noact if the message could not be signed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityRsa::h_SignSignature(const std::vector<uint8_t> & orc_PrivateKey,
                                          const std::vector<uint8_t> & orc_Message,
                                          std::vector<uint8_t> & orc_Signature)
{
   std::error_code c_Retval = Errc::range;

   if ((orc_PrivateKey.size() > 0) && (orc_Message.size() > 0))
   {
      const uint8_t * pu8_Data = &orc_PrivateKey[0];

      //Get private key information from PKCS#8 dump:
      PKCS8_PRIV_KEY_INFO * const pc_Key = d2i_PKCS8_PRIV_KEY_INFO(
         nullptr, &pu8_Data,
         static_cast<long>(orc_PrivateKey.size())); //lint !e970 //using type to match library interface
      if (pc_Key != nullptr)
      {
         //Convert PKCS#8 key to EVP_PKEY:
         EVP_PKEY * const pc_EvpKey = EVP_PKCS82PKEY(pc_Key);
         PKCS8_PRIV_KEY_INFO_free(pc_Key);

         if (pc_EvpKey != nullptr)
         {
            c_Retval = Errc::noact;
            EVP_PKEY_CTX * const pc_SignCtx = EVP_PKEY_CTX_new(pc_EvpKey, nullptr);
            EVP_PKEY_free(pc_EvpKey);

            if (pc_SignCtx != nullptr)
            {
               int x_Result; //lint !e970 !e8080 //using type to match library interface

               x_Result = EVP_PKEY_sign_init(pc_SignCtx);
               if (x_Result > 0)
               {
                  // Raw RSA PKCS#1 v1.5 operation on the passed message)
                  x_Result = EVP_PKEY_CTX_set_rsa_padding(pc_SignCtx, RSA_PKCS1_PADDING);
                  if (x_Result > 0)
                  {
                     size_t x_SignatureSize = 0; //lint !e8080  //using type to match library interface

                     //first call without output buffer to get size of required buffer:
                     x_Result = EVP_PKEY_sign(pc_SignCtx, nullptr, &x_SignatureSize, &orc_Message[0], orc_Message.size());
                     if ((x_Result > 0) && (x_SignatureSize > 0))
                     {
                        //dimension buffer based on result of first call
                        orc_Signature.resize(x_SignatureSize);

                        //second call: compute the signature:
                        x_Result = EVP_PKEY_sign(pc_SignCtx, &orc_Signature[0], &x_SignatureSize,
                                                 &orc_Message[0], orc_Message.size());
                        if (x_Result > 0)
                        {
                           //this should really be the same value as before, but let's be defensive
                           orc_Signature.resize(x_SignatureSize);
                           c_Retval = Errc::success;
                        }
                        else
                        {
                           orc_Signature.clear();
                        }
                     }
                  }
               }
               EVP_PKEY_CTX_free(pc_SignCtx);
            }
         }
      }
   }

   return c_Retval;
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
   std::error_code with Errc::success if the operation was done (check orq_Valid for the result),
   Errc::range on an invalid key or if key, message or signature have zero length,
   Errc::noact if the message could not be recovered
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityRsa::h_VerifySignature(const std::vector<uint8_t> & orc_PublicKey,
                                            const std::vector<uint8_t> & orc_ExpectedMessage,
                                            const std::vector<uint8_t> & orc_Signature, bool & orq_Valid)
{
   std::error_code c_Retval = Errc::range;

   orq_Valid = false;

   if ((orc_PublicKey.size() > 0) && (orc_ExpectedMessage.size() > 0) && (orc_Signature.size() > 0))
   {
      //get RSA structure from binary key:
      const uint8_t * pu8_Data = &orc_PublicKey[0];

      //Extract X509 data from binary key data:
      X509 * const pc_X509Data = d2i_X509(
         nullptr, &pu8_Data,
         static_cast<long>(orc_PublicKey.size())); //lint !e970 //using type to match library interface
      if (pc_X509Data != nullptr)
      {
         //Get key in EVP_PKEY format:
         EVP_PKEY * const pc_EvpKey = X509_get_pubkey(pc_X509Data);
         X509_free(pc_X509Data);

         if (pc_EvpKey != nullptr)
         {
            EVP_PKEY_CTX * const pc_VerifyCtx = EVP_PKEY_CTX_new(pc_EvpKey, nullptr);
            EVP_PKEY_free(pc_EvpKey);
            c_Retval = Errc::noact;

            if (pc_VerifyCtx != nullptr)
            {
               int x_Result; //lint !e970 !e8080 //using type to match library interface
               x_Result = EVP_PKEY_verify_recover_init(pc_VerifyCtx);
               if (x_Result > 0)
               {
                  // Raw RSA PKCS#1 v1.5 operation on the passed message
                  x_Result = EVP_PKEY_CTX_set_rsa_padding(pc_VerifyCtx, RSA_PKCS1_PADDING);
                  if (x_Result > 0)
                  {
                     //get original data from signed data
                     size_t x_DecryptedSize = 0; //lint !e8080  //using type to match library interface

                     //first call: get buffer size needed for result
                     x_Result = EVP_PKEY_verify_recover(pc_VerifyCtx, nullptr, &x_DecryptedSize, &orc_Signature[0],
                                                        orc_Signature.size());
                     if (x_Result > 0)
                     {
                        std::vector<uint8_t> c_DecryptedMessage;

                        if (x_DecryptedSize > 0)
                        {
                           c_DecryptedMessage.resize(x_DecryptedSize);
                           //second call: get original data
                           x_Result = EVP_PKEY_verify_recover(pc_VerifyCtx, &c_DecryptedMessage[0], &x_DecryptedSize,
                                                              &orc_Signature[0], orc_Signature.size());
                           if (x_Result > 0)
                           {
                              c_Retval = Errc::success;
                              //this should really be the same value as before, but let's be defensive
                              c_DecryptedMessage.resize(x_DecryptedSize);

                              //compare original data with expected message:
                              if (c_DecryptedMessage.size() == orc_ExpectedMessage.size())
                              {
                                 const int x_DiffResult = //lint !e970 !e8080 //using type to match library interface
                                                          std::memcmp(&c_DecryptedMessage[0], &orc_ExpectedMessage[0],
                                                                      orc_ExpectedMessage.size());
                                 if (x_DiffResult == 0)
                                 {
                                    orq_Valid = true; //we have a winner
                                 }
                              }
                           }
                        }
                     }
                  }
               }
               EVP_PKEY_CTX_free(pc_VerifyCtx);
            }
         }
      }
   }

   return c_Retval;
}
