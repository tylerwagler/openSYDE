//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core ECDH/AES encryption utility

   see header in .hpp file for details.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/sha.h>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "TglUtils.hpp"

#include "C_OscSecurityEcdhAes.hpp"
#include "C_OscSecurityAesCbc.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace std;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSecurityEcdhAes::C_OscSecurityEcdhAes() :
   mpc_TheKey(nullptr),
   mpu8_AesKey(nullptr)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSecurityEcdhAes::~C_OscSecurityEcdhAes()
{
   if (mpc_TheKey != nullptr)
   {
      EVP_PKEY_free(mpc_TheKey);
      mpc_TheKey = nullptr;
   }
   if (mpu8_AesKey != nullptr)
   {
      OPENSSL_secure_clear_free(mpu8_AesKey, hu32_AES_KEY_LENGTH);
      mpu8_AesKey = nullptr;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get AES key

   Note that this function should only be used for testing purposes.
   For production code the key should be kept as "local" as possible.

   \param[out]       orau8_AesKey          AES key

   \return
   std::error_code with Errc::success on success, Errc::noact if no key available
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::GetAesKey(uint8_t(&orau8_AesKey)[hu32_AES_KEY_LENGTH]) const
{
   if (C_OscSecurityEcdhAes::mpu8_AesKey != nullptr)
   {
      (void)std::memcpy(&orau8_AesKey[0], C_OscSecurityEcdhAes::mpu8_AesKey, hu32_AES_KEY_LENGTH);
      return Errc::success;
   }
   return Errc::noact;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract public key

   Get public key in compressed format.

   \param[out]       orau8_PublicKey          Public key

   \return
   std::error_code with Errc::success if the key is available, Errc::noact on an error trying to extract it
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::m_ExtractCompressedPublicKey(
   uint8_t (&orau8_PublicKey)[hu32_PUBLIC_KEY_LENGTH]) const
{
   std::error_code c_Result = Errc::noact;

   if (mpc_TheKey != nullptr)
   {
      size_t x_KeyLength; //lint !e8080 //using type to match library interface
      //We did configure compressed format at initialization already. So just grab the data.
      const int x_Result = //lint !e970 !e8080 //using type to match library interface
                           EVP_PKEY_get_octet_string_param(mpc_TheKey, OSSL_PKEY_PARAM_PUB_KEY, &orau8_PublicKey[0],
                                                           hu32_PUBLIC_KEY_LENGTH, &x_KeyLength);
      if ((x_Result == 1) && (x_KeyLength == hu32_PUBLIC_KEY_LENGTH))
      {
         //sizes other than 33 should not happen; a compressed key should always be 33 bytes
         c_Result = Errc::success;
      }
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create OpenSSL EVP_PKEY from raw EC public key

   Constructs an EVP_PKEY from a compressed secp256r1 public key using the OpenSSL 3.0
   EVP_PKEY_fromdata API with OSSL_PARAM_BLD.

   \param[in]    orau8_PublicKey      Public key in compressed format (33 bytes of buffer)

   \retval  NULL     could not create key
   \retval  !NULL   created key (needs to be EVP_PKEY_free'd by caller)
*/
//----------------------------------------------------------------------------------------------------------------------
EVP_PKEY * C_OscSecurityEcdhAes::mh_CreateEvpPkeyFromRawPublicKey(
   const uint8_t(&orau8_PublicKey)[hu32_PUBLIC_KEY_LENGTH])
{
   EVP_PKEY * pc_Pkey = nullptr;

   OSSL_PARAM_BLD * const pc_Bld = OSSL_PARAM_BLD_new();
   if (pc_Bld != nullptr)
   {
      int x_Result = OSSL_PARAM_BLD_push_utf8_string(pc_Bld, OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "prime256v1", 0); //lint !e970 !e8080
      if (x_Result == 1)
      {
         x_Result = OSSL_PARAM_BLD_push_octet_string(pc_Bld, OSSL_PKEY_PARAM_PUB_KEY,
                                                      &orau8_PublicKey[0],
                                                      hu32_PUBLIC_KEY_LENGTH);
      }

      OSSL_PARAM * pc_Params = nullptr;
      if (x_Result == 1)
      {
         pc_Params = OSSL_PARAM_BLD_to_param(pc_Bld);
      }
      OSSL_PARAM_BLD_free(pc_Bld);

      if (pc_Params != nullptr)
      {
         EVP_PKEY_CTX * const pc_KeyCtx = EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);
         if (pc_KeyCtx != nullptr)
         {
            if ((EVP_PKEY_fromdata_init(pc_KeyCtx) == 1) &&
                (EVP_PKEY_fromdata(pc_KeyCtx, &pc_Pkey, EVP_PKEY_PUBLIC_KEY, pc_Params) == 1))
            {
               //EVP_PKEY_fromdata set pc_Pkey on success
            }
            else
            {
               pc_Pkey = nullptr;
            }
            EVP_PKEY_CTX_free(pc_KeyCtx);
         }
         OSSL_PARAM_free(pc_Params);
      }
   }

   return pc_Pkey;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create elliptic curve key pair with secp256r1 curve

   The created ECDH instance including the private key will be stored in the class instance for further operations.

   \param[out]    orau8_PublicKey          Public key in compressed format

   \return
   std::error_code with Errc::success on success, Errc::noact on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::CreateEcKeys(uint8_t (&orau8_PublicKey)[hu32_PUBLIC_KEY_LENGTH])
{
   if (mpc_TheKey != nullptr)
   {
      EVP_PKEY_free(mpc_TheKey);
      mpc_TheKey = nullptr;
   }
   if (mpc_TheKey == nullptr)
   {
      EVP_PKEY_CTX * const pc_ParamContext = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);

      if (pc_ParamContext != nullptr)
      {
         EVP_PKEY_CTX * pc_KeyContext = nullptr;
         EVP_PKEY * pc_KeyParams = nullptr;
         int x_Result = EVP_PKEY_paramgen_init(pc_ParamContext); //lint !e970 !e8080 //using type to match library
         if (x_Result == 1)
         {
            // select the curve
            x_Result = EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pc_ParamContext, NID_X9_62_prime256v1);
         }
         if (x_Result == 1)
         {
            // generate parameters object
            x_Result = EVP_PKEY_paramgen(pc_ParamContext, &pc_KeyParams);
         }
         EVP_PKEY_CTX_free(pc_ParamContext); //not needed any longer
         if ((x_Result == 1) && (pc_KeyParams != nullptr))
         {
            // create key generation context
            pc_KeyContext = EVP_PKEY_CTX_new(pc_KeyParams, nullptr);
            EVP_PKEY_free(pc_KeyParams);
         }
         if (pc_KeyContext != nullptr)
         {
            x_Result = EVP_PKEY_keygen_init(pc_KeyContext);
            if (x_Result == 1)
            {
               // generate the keypair
               x_Result = EVP_PKEY_keygen(pc_KeyContext, &mpc_TheKey);
               EVP_PKEY_CTX_free(pc_KeyContext); //not needed any longer
            }
            if (x_Result == 1)
            {
               //we want to make sure we get the public key data in 33 byte compressed format when reading it
               // subsequently (for compact transfer to the server):
               x_Result = EVP_PKEY_set_utf8_string_param(mpc_TheKey,
                                                         OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT,
                                                         OSSL_PKEY_EC_POINT_CONVERSION_FORMAT_COMPRESSED);
            }
            if ((x_Result == 1) && (mpc_TheKey != nullptr))
            {
               //extract binary keys and remember:
               const std::error_code c_ExtractResult = m_ExtractCompressedPublicKey(orau8_PublicKey);
               if (!c_ExtractResult)
               {
                  EVP_PKEY_CTX_free(pc_KeyContext);
                  return Errc::success;
               }
            }
            EVP_PKEY_CTX_free(pc_KeyContext);
         }
      }
   }
   return Errc::noact;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Perform elliptic curve diffie hellman algorithm to create AES encryption key

   Can be called after creating a valid ECDH instance with CreateEcKeys.
   The resulting AES key will be stored in the corresponding class field.

   Steps:
   * perform ECDH algorithm to create a shared secret from the own private and other's public keys
   * use SHA-256 as a key derivation function (KDF) to create a hash value over the shared secret
   * return the first 16 bytes of the SHA-256 hash as an AES-KEY

   \param[in]   orau8_OthersPublicKey    Others public key

   \return
   std::error_code with Errc::success on success, Errc::noact on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::DeriveAesKey(const uint8_t (&orau8_OthersPublicKey)[hu32_PUBLIC_KEY_LENGTH])
{
   if (mpc_TheKey != nullptr)
   {
      EVP_PKEY_CTX * const pc_DeriveContext = EVP_PKEY_CTX_new(mpc_TheKey, nullptr);
      if (pc_DeriveContext != nullptr)
      {
         size_t x_SharedSecredSize = 0; //lint !e8080 //using type to match library
         uint8_t au8_SharedSecret[32U];

         int x_Result = EVP_PKEY_derive_init(pc_DeriveContext); //lint !e970 !e8080 //using type to match library
         if (x_Result == 1)
         {
            // set up a key from the binary public key data
            EVP_PKEY * const pc_PeerKey = mh_CreateEvpPkeyFromRawPublicKey(orau8_OthersPublicKey);
            if (pc_PeerKey != nullptr)
            {
               x_Result = EVP_PKEY_derive_set_peer(pc_DeriveContext, pc_PeerKey);
            }
            else
            {
               x_Result = -1;
            }
            EVP_PKEY_free(pc_PeerKey); //not needed any more
         }

         if (x_Result == 1)
         {
            // derive shared secret
            x_SharedSecredSize = sizeof(au8_SharedSecret);
            x_Result = EVP_PKEY_derive(pc_DeriveContext, &au8_SharedSecret[0], &x_SharedSecredSize);
         }
         EVP_PKEY_CTX_free(pc_DeriveContext);

         if (x_Result == 1)
         {
            uint8_t au8_Sha256Digest[SHA256_DIGEST_LENGTH];
            const uint8_t * const pu8_Value = SHA256(&au8_SharedSecret[0], x_SharedSecredSize, &au8_Sha256Digest[0]);

            if (pu8_Value != nullptr)
            {
               if (this->mpu8_AesKey == nullptr)
               {
                  //lint -e{9079} //casting is the correct approach to use the API
                  this->mpu8_AesKey = reinterpret_cast<uint8_t *>(OPENSSL_secure_malloc(hu32_AES_KEY_LENGTH));
               }
               if (this->mpu8_AesKey != nullptr)
               {
                  //use first 16bytes as AES key:
                  (void)std::memcpy(this->mpu8_AesKey, &au8_Sha256Digest[0], hu32_AES_KEY_LENGTH);
                  return Errc::success;
               }
            }
         }
      }
   }
   return Errc::noact;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Encrypt with AES-128

   Can be called after deriving a valid key with DeriveAesKey

   \param[in]   orau8_AesInitVector    AES init vector to use
   \param[in]   orc_Input              Input data
   \param[out]  orc_Output             Output data

   \return
   std::error_code with Errc::success on success, Errc::config on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::AesEncrypt(const uint8_t (&orau8_AesInitVector)[C_OscSecurityAesCbc::hu32_IV_LENGTH],
                                                  const std::vector<uint8_t> & orc_Input,
                                                  std::vector<uint8_t> & orc_Output) const
{
   tgl_assert(this->mpu8_AesKey != nullptr);
   const uint8_t(&orau8_Key)[C_OscSecurityAesCbc::hu32_IV_LENGTH] =
      reinterpret_cast<const uint8_t ( &)[C_OscSecurityAesCbc::hu32_IV_LENGTH]>(this->mpu8_AesKey[0]);
   return C_OscSecurityAesCbc::h_Encrypt(orau8_Key, orau8_AesInitVector, orc_Input, orc_Output);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decrypt with AES-128

   Can be called after deriving a valid key with DeriveAesKey

   \param[in]   orau8_AesInitVector    AES init vector to use
   \param[in]   orc_Input              Input data
   \param[out]  orc_Output             Output data

   \return
   std::error_code with Errc::success on success, Errc::config on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityEcdhAes::AesDecrypt(const uint8_t (&orau8_AesInitVector)[C_OscSecurityAesCbc::hu32_IV_LENGTH],
                                                  const std::vector<uint8_t> & orc_Input,
                                                  std::vector<uint8_t> & orc_Output) const
{
   tgl_assert(this->mpu8_AesKey != nullptr);
   const uint8_t(&orau8_Key)[C_OscSecurityAesCbc::hu32_IV_LENGTH] =
      reinterpret_cast<const uint8_t ( &)[C_OscSecurityAesCbc::hu32_IV_LENGTH]>(this->mpu8_AesKey[0]);
   return C_OscSecurityAesCbc::h_Decrypt(orau8_Key, orau8_AesInitVector, orc_Input, orc_Output);
}
