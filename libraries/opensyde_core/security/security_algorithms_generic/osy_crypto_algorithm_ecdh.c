//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Elliptic curve diffie hellman algorithms

   Software implementation of elliptic curve diffie hellman algorithms using secp256r1 curve.

   Features:
   * generate private and public ec keys
   * derive shared secret using ECDH with SHA-256 as KDF

   Note that the used OpenSSL is most likely not available on bare metal targets, but a HW module usually is.

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
/*STW Deviation Coding Rules v4*/
/*Violated Rule: no explicit rule*/
/*Circumstances: Library header includes same file twice.*/
/*Reason: Decision of that library.*/
/*Potential risks: Bad build performance; unexpected behavior.*/
/*Prevention of risks: Library was tested by supplier "as is"*/
/* PRQA S 0838 L1 */ //For unknown reasons "++" "--" syntax does not work here
#include <openssl/evp.h>
/* PRQA L:L1 */
#include <openssl/ec.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/sha.h>

#include "stwtypes.h"
#include "stwerrors.h"
#include "osy_crypto_algorithm_ecdh.h"

/* -- Defines ------------------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
static uint8 mau8_PublicKey[OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY]; //in compressed format
static uint8 mau8_PrivateKey[OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY];

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */
static sint16 m_extract_compressed_public_key(const EVP_PKEY * const opt_Key);
static sint16 m_extract_private_key(const EVP_PKEY * const opt_Key);
static EVP_PKEY * m_create_evp_pkey_from_raw_pubkey(const uint8 opu8_PublicKey[OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY]);
static EVP_PKEY * m_create_evp_pkey_from_raw_private_key(
   const uint8 opu8_PrivateKey[OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY]);

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract public key

   Get public key in compressed format and store locally.

   \param[in]        pt_Key                   OpenSSL key structure

   \retval  C_NO_ERR    key available
   \retval  C_NOACT     error trying to extract key
*/
//----------------------------------------------------------------------------------------------------------------------
static sint16 m_extract_compressed_public_key(const EVP_PKEY * const opt_Key)
{
   size_t un_KeyLength;
   sint16 s16_Result = C_NOACT;

   //We did configure compressed format in osy_udc_trg_sec_get_ec_keys already. So just grab the data.
   const sintn sn_Result = EVP_PKEY_get_octet_string_param(opt_Key, OSSL_PKEY_PARAM_PUB_KEY, &mau8_PublicKey[0],
                                                           OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY, &un_KeyLength);

   if ((sn_Result == 1) && (un_KeyLength == OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY))
   {
      //sizes other than 33 should not happen; a compressed key should always be 33 bytes
      s16_Result = C_NO_ERR;
   }
   return s16_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract private key

   Get private key in binary format and store locally.

   \param[in]        pt_Key                   OpenSSL key structure

   \retval  C_NO_ERR    key available
   \retval  C_NOACT     error trying to extract key
*/
//----------------------------------------------------------------------------------------------------------------------
static sint16 m_extract_private_key(const EVP_PKEY * const opt_Key)
{
   sint16 s16_Result = C_NOACT;
   BIGNUM * pt_PrivateKeyBn = NULL;

   //Extract private key as BIGNUM using OpenSSL 3.0+ API
   //Note: EVP_PKEY_get_bn_param allocates a new BIGNUM; caller must free
   const sintn sn_GetResult = EVP_PKEY_get_bn_param(opt_Key, OSSL_PKEY_PARAM_PRIV_KEY, &pt_PrivateKeyBn);

   if ((sn_GetResult == 1) && (pt_PrivateKeyBn != NULL))
   {
      //The key can occasionally be smaller than 32 bytes (due to leading zeroes).
      //In this case BN_bn2bin will only copy over the bytes that are occupied.
      //Storage is big endian. So we need the leading zeroes on the left.
      //So fill in values "to the right" of the leading zeroes, memset leading zeroes explicitly

      /*STW Deviation Coding Rules v4*/
      /*Violated Rule: no explicit rule*/
      /*Circumstances: Using macro that does integer division*/
      /*Reason: macro is documented approach to using API*/
      /*Potential risks: Undefined behavior*/
      /*Prevention of risks: Macro is implemented with consideration of UB*/
      /*PRQA S 3103 1*/
      const sintn sn_NumBytes = BN_num_bytes(pt_PrivateKeyBn);
      //sanity check:
      if (sn_NumBytes <= (sintn)OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY)
      {
         const sintn sn_OffsetToWriteTo = ((sintn)OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY - sn_NumBytes);
         const sintn sn_CopiedLength = BN_bn2bin(pt_PrivateKeyBn, &mau8_PrivateKey[sn_OffsetToWriteTo]);
         if (sn_CopiedLength == sn_NumBytes)
         {
            sintn sn_LeadingZeroIndex;
            //set leading zeroes to zero:
            for (sn_LeadingZeroIndex = 0; sn_LeadingZeroIndex < sn_OffsetToWriteTo; sn_LeadingZeroIndex++)
            {
               mau8_PrivateKey[sn_LeadingZeroIndex] = 0U;
            }
            s16_Result = C_NO_ERR;
         }
      }
      BN_clear_free(pt_PrivateKeyBn);
   }
   return s16_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create and provide elliptic curve key pair with secp256r1 curve

   Here: SW implementation using OpenSSL.
   The implementation is generic and can be used for ECDA key generation in general.

   The function implements a state machine to make sure the keys are only created once until the node is reset.
   But creation can be forces using the ou8_ForceCreation parameter.

   \param[in]     ou8_ForceCreation        0: regular operation; create key only once at first call
                                           1: force re-creation of key
   \param[out]    opu8_PublicKey           If NULL: nothing
                                           If not NULL: Public key in compressed format
                                             (caller must provide 33 bytes of buffer)
   \param[out]    opu8_PrivateKey          If NULL: nothing
                                           If not NULL: Private key
                                             (caller must provide 32 bytes of buffer)

   \retval  C_NO_ERR    keys available (and returned if requested)
   \retval  C_NOACT     error trying to perform operation
*/
//----------------------------------------------------------------------------------------------------------------------
sint16 osy_cry_alg_ecdh_get_ec_keys(const uint8 ou8_ForceCreation,
                                    uint8 opu8_PublicKey[OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY],
                                    uint8 opu8_PrivateKey[OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY])
{
   static uint8 hu8_KeysInitialized = 0U;
   sint16 s16_Result = C_NOACT;

   if ((hu8_KeysInitialized == 0U) || (ou8_ForceCreation == 1U))
   {
      EVP_PKEY_CTX * const pt_ParamContext = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
      if (pt_ParamContext != NULL)
      {
         EVP_PKEY_CTX * pt_KeyContext = NULL;
         EVP_PKEY * pt_KeyParams = NULL;
         sintn sn_Result = EVP_PKEY_paramgen_init(pt_ParamContext);
         if (sn_Result == 1)
         {
            // select the curve
            sn_Result = EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pt_ParamContext, NID_X9_62_prime256v1);
         }
         if (sn_Result == 1)
         {
            // generate parameters object
            sn_Result = EVP_PKEY_paramgen(pt_ParamContext, &pt_KeyParams);
         }
         EVP_PKEY_CTX_free(pt_ParamContext); //not needed any longer
         if ((sn_Result == 1) && (pt_KeyParams != NULL))
         {
            // create key generation context
            pt_KeyContext = EVP_PKEY_CTX_new(pt_KeyParams, NULL);
            EVP_PKEY_free(pt_KeyParams);
         }
         if (pt_KeyContext != NULL)
         {
            EVP_PKEY * pt_TheKey = NULL;

            sn_Result = EVP_PKEY_keygen_init(pt_KeyContext);
            if (sn_Result == 1)
            {
               // generate the keypair
               sn_Result = EVP_PKEY_keygen(pt_KeyContext, &pt_TheKey);
               EVP_PKEY_CTX_free(pt_KeyContext); //not needed any longer
            }
            if (sn_Result == 1)
            {
               //we want to make sure we get the public key data in 33 byte compressed format when reading it
               // subsequently (for compact transfer to the client):
               sn_Result = EVP_PKEY_set_utf8_string_param(pt_TheKey,
                                                          OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT,
                                                          OSSL_PKEY_EC_POINT_CONVERSION_FORMAT_COMPRESSED);
            }
            if ((sn_Result == 1) && (pt_TheKey != NULL))
            {
               //extract binary keys and remember:
               s16_Result = m_extract_compressed_public_key(pt_TheKey);
               if (s16_Result == C_NO_ERR)
               {
                  s16_Result = m_extract_private_key(pt_TheKey);
               }
               if (s16_Result == C_NO_ERR)
               {
                  hu8_KeysInitialized = 1U;
                  s16_Result = C_NO_ERR;
               }
               EVP_PKEY_free(pt_TheKey);
            }
         }
      }
   }
   //return keys if initialized and desired by caller:
   if (hu8_KeysInitialized == 1U)
   {
      uint8 u8_Index;
      s16_Result = C_NO_ERR;
      if (opu8_PublicKey != NULL)
      {
         //use loop instead of memcpy to be independent of external libraries
         for (u8_Index = 0U; u8_Index < OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY; u8_Index++)
         {
            opu8_PublicKey[u8_Index] = mau8_PublicKey[u8_Index];
         }
      }
      if (opu8_PrivateKey != NULL)
      {
         for (u8_Index = 0U; u8_Index < OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY; u8_Index++)
         {
            opu8_PrivateKey[u8_Index] = mau8_PrivateKey[u8_Index];
         }
      }
   }
   return s16_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create OpenSSL EVP_PKEY from raw EC public key

   Constructs an EVP_PKEY from a compressed public key using the OpenSSL 3.0+ OSSL_PARAM_BLD API.

   \param[in]    opu8_PublicKey      Public key in compressed format (33 bytes of buffer)

   \retval  NULL     could not create key
   \retval  else     created key (needs to be EVP_PKEY_free'd by caller)
*/
//----------------------------------------------------------------------------------------------------------------------
static EVP_PKEY * m_create_evp_pkey_from_raw_pubkey(const uint8 opu8_PublicKey[OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY])
{
   EVP_PKEY * pt_Pkey = NULL;
   OSSL_PARAM_BLD * pt_Bld = NULL;
   OSSL_PARAM * pt_Params = NULL;
   EVP_PKEY_CTX * pt_PkeyCtx = NULL;
   sintn sn_Result = -1;

   pt_Bld = OSSL_PARAM_BLD_new();
   if (pt_Bld != NULL)
   {
      sn_Result = OSSL_PARAM_BLD_push_utf8_string(pt_Bld, OSSL_PKEY_PARAM_GROUP_NAME, "prime256v1", 0);
   }
   if (sn_Result == 1)
   {
      sn_Result = OSSL_PARAM_BLD_push_octet_string(pt_Bld, OSSL_PKEY_PARAM_PUB_KEY,
                                                     opu8_PublicKey, OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY);
   }
   if (sn_Result == 1)
   {
      pt_Params = OSSL_PARAM_BLD_to_param(pt_Bld);
   }
   if (pt_Params != NULL)
   {
      pt_PkeyCtx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
   }
   if (pt_PkeyCtx != NULL)
   {
      sn_Result = EVP_PKEY_fromdata_init(pt_PkeyCtx);
   }
   if (sn_Result == 1)
   {
      sn_Result = EVP_PKEY_fromdata(pt_PkeyCtx, &pt_Pkey, EVP_PKEY_PUBLIC_KEY, pt_Params);
      if (sn_Result != 1)
      {
         pt_Pkey = NULL;
      }
   }

   //no action if parameters are NULL
   EVP_PKEY_CTX_free(pt_PkeyCtx);
   OSSL_PARAM_free(pt_Params);
   OSSL_PARAM_BLD_free(pt_Bld);

   return pt_Pkey;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create OpenSSL EVP_PKEY from raw EC private key

   Constructs an EVP_PKEY from a raw private key using the OpenSSL 3.0+ OSSL_PARAM_BLD API.

   \param[in]    opu8_PrivateKey      Private key in binary format (32 bytes of buffer)

   \retval  NULL     could not create key
   \retval  else     created key (needs to be EVP_PKEY_free'd by caller)
*/
//----------------------------------------------------------------------------------------------------------------------
static EVP_PKEY * m_create_evp_pkey_from_raw_private_key(const uint8 opu8_PrivateKey[OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY])
{
   EVP_PKEY * pt_Pkey = NULL;
   OSSL_PARAM_BLD * pt_Bld = NULL;
   OSSL_PARAM * pt_Params = NULL;
   EVP_PKEY_CTX * pt_PkeyCtx = NULL;
   BIGNUM * pt_TheBigNum = NULL;
   sintn sn_Result = -1;

   pt_TheBigNum = BN_bin2bn(opu8_PrivateKey, (sintn)OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY, NULL);
   if (pt_TheBigNum != NULL)
   {
      pt_Bld = OSSL_PARAM_BLD_new();
   }
   if (pt_Bld != NULL)
   {
      sn_Result = OSSL_PARAM_BLD_push_utf8_string(pt_Bld, OSSL_PKEY_PARAM_GROUP_NAME, "prime256v1", 0);
   }
   if (sn_Result == 1)
   {
      sn_Result = OSSL_PARAM_BLD_push_BN(pt_Bld, OSSL_PKEY_PARAM_PRIV_KEY, pt_TheBigNum);
   }
   if (sn_Result == 1)
   {
      pt_Params = OSSL_PARAM_BLD_to_param(pt_Bld);
   }
   if (pt_Params != NULL)
   {
      pt_PkeyCtx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
   }
   if (pt_PkeyCtx != NULL)
   {
      sn_Result = EVP_PKEY_fromdata_init(pt_PkeyCtx);
   }
   if (sn_Result == 1)
   {
      sn_Result = EVP_PKEY_fromdata(pt_PkeyCtx, &pt_Pkey, EVP_PKEY_KEYPAIR, pt_Params);
      if (sn_Result != 1)
      {
         pt_Pkey = NULL;
      }
   }

   //no action if parameters are NULL
   EVP_PKEY_CTX_free(pt_PkeyCtx);
   OSSL_PARAM_free(pt_Params);
   OSSL_PARAM_BLD_free(pt_Bld);
   BN_clear_free(pt_TheBigNum);

   return pt_Pkey;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Perform elliptic curve diffie hellman algorithm to create AES encryption key

   Here: SW implementation using OpenSSL.

   Steps:
   * perform ECDH algorithm to create a shared secret from the private and public keys
   * use SHA-256 as a key derivation function (KDF) to create an hash value over the shared secret
   * return the first 16 bytes of the SHA-256 hash as an AES-KEY

   \param[in]    opu8_OthersPublicKey    Public key in compressed format (caller will provide 33 bytes of buffer)
   \param[in]    opu8_OwnPrivateKey      Private key in binary format (caller will provide 32 bytes of buffer)
   \param[out]   opu8_AesKey             Derived AES key (caller will provide 16 bytes of buffer)

   \retval  C_NO_ERR    derivation done; key is in opu8_AesKey
   \retval  C_NOACT     error trying to perform operation
*/
//----------------------------------------------------------------------------------------------------------------------
sint16 osy_cry_alg_ecdh_derive_aes_key(const uint8 opu8_OthersPublicKey[OSY_CRY_ALG_ECDH_SIZE_PUBLIC_KEY],
                                       const uint8 opu8_OwnPrivateKey[OSY_CRY_ALG_ECDH_SIZE_PRIVATE_KEY],
                                       uint8 opu8_AesKey[OSY_CRY_ALG_ECDH_SIZE_AES_KEY])
{
   sint16 s16_Result = C_NOACT;

   // set up private key from binary key data:
   EVP_PKEY * const pt_OwnKey = m_create_evp_pkey_from_raw_private_key(opu8_OwnPrivateKey);

   if (pt_OwnKey != NULL)
   {
      EVP_PKEY_CTX * const pt_DeriveContext = EVP_PKEY_CTX_new(pt_OwnKey, NULL);
      EVP_PKEY_free(pt_OwnKey); //not needed any more

      if (pt_DeriveContext != NULL)
      {
         size_t un_SharedSecredSize;
         uint8 au8_SharedSecret[32U];

         sintn sn_Result = EVP_PKEY_derive_init(pt_DeriveContext);
         if (sn_Result == 1)
         {
            // set up a key from the binary public key data
            EVP_PKEY * const pt_PeerKey = m_create_evp_pkey_from_raw_pubkey(opu8_OthersPublicKey);
            if (pt_PeerKey != NULL)
            {
               sn_Result = EVP_PKEY_derive_set_peer(pt_DeriveContext, pt_PeerKey);
            }
            else
            {
               sn_Result = -1;
            }
            EVP_PKEY_free(pt_PeerKey); //not needed any more
         }

         if (sn_Result == 1)
         {
            // derive shared secret
            un_SharedSecredSize = sizeof(au8_SharedSecret);
            sn_Result = EVP_PKEY_derive(pt_DeriveContext, &au8_SharedSecret[0], &un_SharedSecredSize);
         }
         EVP_PKEY_CTX_free(pt_DeriveContext);

         if (sn_Result == 1)
         {
            uint8 au8_Sha256Digest[SHA256_DIGEST_LENGTH];
            const uint8 * const pu8_Value = SHA256(&au8_SharedSecret[0], un_SharedSecredSize, &au8_Sha256Digest[0]);

            if (pu8_Value != NULL)
            {
               //use first 16bytes as AES key:
               //use loop instead of memcpy to be independent of external libraries
               uint8 u8_Index;
               for (u8_Index = 0U; u8_Index < OSY_CRY_ALG_ECDH_SIZE_AES_KEY; u8_Index++)
               {
                  opu8_AesKey[u8_Index] = au8_Sha256Digest[u8_Index];
               }

               s16_Result = C_NO_ERR;
            }
            else
            {
               s16_Result  = C_NOACT;
            }
         }
      }
   }
   return s16_Result;
}
