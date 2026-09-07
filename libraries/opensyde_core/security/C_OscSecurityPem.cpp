//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle PEM content

   Handle PEM content

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <fstream>
#include <sstream>
#include <system_error>

#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/core_names.h"

#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_OscSecurityPem.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSecurityPem::C_OscSecurityPem() :
   C_OscSecurityPemBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read modulus and exponent from file

   \param[in]      orc_FileName        File name
   \param[in,out]  orc_Modulus         Modulus
   \param[in,out]  orc_Exponent        Exponent
   \param[in,out]  orc_ErrorMessage    Error message (does not include file name)

   \return
   std::error_code with Errc::success if the information was extracted,
   Errc::range if the file was not found,
   Errc::config on invalid file content,
   Errc::checksum if modulus and exponent could not be parsed from the key
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPem::h_ExtractModulusAndExponentFromFile(const std::string & orc_FileName,
                                                              std::vector<uint8_t> & orc_Modulus,
                                                              std::vector<uint8_t> & orc_Exponent,
                                                              std::string & orc_ErrorMessage)
{
   C_OscSecurityPem c_Pem;
   std::error_code c_Retval = c_Pem.LoadFromFile(orc_FileName, orc_ErrorMessage);

   if (!c_Retval)
   {
      c_Retval = C_OscSecurityPem::h_ExtractModulusAndExponent(c_Pem.GetKeyInfo().GetX509CertificateData(),
                                                                 orc_Modulus, orc_Exponent, orc_ErrorMessage);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read modulus and exponent

   \param[in]      orc_PubKeyTextDecoded  Public key text decoded
   \param[in,out]  orc_Modulus            Modulus
   \param[in,out]  orc_Exponent           Exponent
   \param[in,out]  orc_ErrorMessage       Error message

   \return
   std::error_code with Errc::success if the information was extracted,
   Errc::checksum if modulus and exponent could not be parsed from the key
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPem::h_ExtractModulusAndExponent(const std::vector<uint8_t> & orc_PubKeyTextDecoded,
                                                      std::vector<uint8_t> & orc_Modulus,
                                                      std::vector<uint8_t> & orc_Exponent,
                                                      std::string & orc_ErrorMessage)
{
   std::error_code c_Retval = Errc::success;
   X509 * pc_X509Key = X509_new();
   const uint8_t * pu8_DataPointer = &orc_PubKeyTextDecoded[0];

   const long x_DecodedSize = static_cast<long>(orc_PubKeyTextDecoded.size()); //lint !e970 !e8080 //use API type

   pc_X509Key = d2i_X509(&pc_X509Key, &pu8_DataPointer, x_DecodedSize);
   if (pc_X509Key != nullptr)
   {
      EVP_PKEY * const pc_PubKey = X509_get0_pubkey(pc_X509Key);
      if (pc_PubKey != nullptr)
      {
         BIGNUM * pc_Modulus = nullptr;
         BIGNUM * pc_Exponent = nullptr;

         const int x_ResultEn = //lint !e970 !e8080 //use API type
                                EVP_PKEY_get_bn_param(pc_PubKey, OSSL_PKEY_PARAM_RSA_N, &pc_Modulus);
         const int x_ResultEe = //lint !e970 !e8080 //use API type
                                EVP_PKEY_get_bn_param(pc_PubKey, OSSL_PKEY_PARAM_RSA_E, &pc_Exponent);

         if ((x_ResultEn == 1) && (x_ResultEe == 1))
         {
            orc_Modulus.resize(C_OscSecurityPem::mhu32_DEFAULT_BUFFER_SIZE);
            orc_Exponent.resize(C_OscSecurityPem::mhu32_DEFAULT_BUFFER_SIZE);
            {
               const int32_t s32_SizeModulus = BN_bn2bin(pc_Modulus, &orc_Modulus[0]);
               const int32_t s32_SizeExponent = BN_bn2bin(pc_Exponent, &orc_Exponent[0]);
               BN_clear_free(pc_Modulus);  //not needed any more
               BN_clear_free(pc_Exponent); //not needed any more
               if ((s32_SizeModulus > 0) && (s32_SizeExponent > 0))
               {
                  orc_Modulus.resize(s32_SizeModulus);
                  orc_Exponent.resize(s32_SizeExponent);
               }
               else
               {
                  c_Retval = Errc::checksum;
                  orc_ErrorMessage = "Could not read modulus or exponent";
               }
            }
         }
         else
         {
            c_Retval = Errc::checksum;
            orc_ErrorMessage = "Could not get modulus and exponent part from public key";
         }
      }
      else
      {
         c_Retval = Errc::checksum;
         orc_ErrorMessage = "Could not get public key from X509 input";
      }
      X509_free(pc_X509Key);
   }
   else
   {
      c_Retval = Errc::checksum;
      orc_ErrorMessage = "Could not read X509 input";
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read private key

   \param[in]      orc_FileContent     File content
   \param[in,out]  orc_ErrorMessage    Error message

   \return
   std::error_code with Errc::success if the information was extracted, Errc::config on invalid file content
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPem::m_ReadPrivateKey(const std::vector<uint8_t> & orc_FileContent,
                                                   std::string & orc_ErrorMessage)
{
   std::error_code c_Retval = Errc::success;
   //Private key
   const int x_ContentSize = static_cast<int>(orc_FileContent.size()); //lint !e970 !e8080 //use type expected by API
   BIO * const pc_PrivKeyFile = BIO_new_mem_buf(&orc_FileContent[0], x_ContentSize);

   if (pc_PrivKeyFile != nullptr)
   {
      PKCS8_PRIV_KEY_INFO * const pc_RsaPriv = PEM_read_bio_PKCS8_PRIV_KEY_INFO(pc_PrivKeyFile, nullptr, nullptr,
                                                                                nullptr);
      if (pc_RsaPriv != nullptr)
      {
         std::vector<uint8_t> c_PrivKeyTextDecoded;
         c_PrivKeyTextDecoded.resize(C_OscSecurityPem::mhu32_DEFAULT_BUFFER_SIZE);
         {
            uint8_t * pu8_PrivKeyTextPointer = &c_PrivKeyTextDecoded[0];
            const uint32_t u32_PrivKeyTextCount = i2d_PKCS8_PRIV_KEY_INFO(pc_RsaPriv, &pu8_PrivKeyTextPointer);
            c_PrivKeyTextDecoded.resize(u32_PrivKeyTextCount);
            this->mc_KeyInfo.SetPrivateKey(c_PrivKeyTextDecoded);
            if (u32_PrivKeyTextCount == 0)
            {
               c_Retval = Errc::config;
               orc_ErrorMessage = "Private key: could not convert private key section into bytes";
            }
         }
         PKCS8_PRIV_KEY_INFO_free(pc_RsaPriv);
      }
      else
      {
         //No error as private key part is only sometimes present
      }
      BIO_free(pc_PrivKeyFile);
   }
   else
   {
      c_Retval = Errc::config;
      orc_ErrorMessage = "Private key: could not read file content";
   }
   return c_Retval;
}
