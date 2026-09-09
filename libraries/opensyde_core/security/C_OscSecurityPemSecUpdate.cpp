//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle content of PEM files for SecureUpdate feature

   Content and thus handling of SecureUpdate PEM files is slightly different than the PEM files of the
   SecureAuthentication feature.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "openssl/pem.h"
#include "openssl/evp.h"
#include "openssl/core_names.h"

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include <cstdint>
#include "C_OscUtils.hpp"
#include "TglFile.hpp"
#include "C_OscSecurityPemSecUpdate.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::tgl;

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
C_OscSecurityPemSecUpdate::C_OscSecurityPemSecUpdate() :
   C_OscSecurityPemBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load from file

   Loads information from PEM file using the base class.
   Checks for specific conditions required for PEM files used for secure update:
   * key usage flag digitalSignature must be set
   * extended key usage flag id-kp-emailProtection must be set

   Results will be stored in mc_KeyInfo and mc_MetaInfo.

   \param[in]      orc_FileName        File name
   \param[in,out]  orc_ErrorMessage    Error message (does not include file name)

   \return
   std::error_code with Errc::success if the information was extracted,
   Errc::range if the file was not found,
   Errc::config on invalid file content,
   Errc::overflow if the usage flags are not set as expected
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemSecUpdate::LoadFromFile(const std::string & orc_FileName,
                                                        std::string & orc_ErrorMessage)
{
   std::error_code c_Result = C_OscSecurityPemBase::LoadFromFile(orc_FileName, orc_ErrorMessage);

   if (!c_Result)
   {
      const C_OscSecurityPemKeyInfo::C_CertificateKeyUsageInformation c_UsageInfo = mc_KeyInfo.GetKeyUsageInformation();
      if ((c_UsageInfo.q_KeyUsageDefined == false) || (c_UsageInfo.q_KeyUsageDigitalSignature == false) ||
          (c_UsageInfo.q_ExtendedKeyUsageDefined == false) || (c_UsageInfo.q_ExtendedKeyUsageEmailProtection == false))
      {
         c_Result = Errc::overflow;
         if (orc_ErrorMessage.empty() == false)
         {
            orc_ErrorMessage += " ";
         }
         orc_ErrorMessage += "Certificate: Key usage flags not as expected in PEM file used for secure update "
                             "configuration. Expected flags \"digitalSignature\" and \"id-kp-emailProtection\" "
                             "to be set.";
      }
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read private key

   Here: we extract an elliptic curve private key and store it in mc_KeyInfo.

   \param[in]      orc_FileContent     File content
   \param[in,out]  orc_ErrorMessage    Error message

   \return
   std::error_code with Errc::success if the information was extracted, Errc::config on invalid file content
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemSecUpdate::m_ReadPrivateKey(const std::vector<uint8_t> & orc_FileContent,
                                                            std::string & orc_ErrorMessage)
{
   std::error_code c_Retval = Errc::success;

   const int x_ContentSize = static_cast<int>(orc_FileContent.size()); //lint !e970 !e8080 //use type expected by API
   //read pem file content into a BIO (= openSSL I/O stream)
   BIO * const pc_PrivKeyFile = BIO_new_mem_buf(&orc_FileContent[0], x_ContentSize);

   if (pc_PrivKeyFile != nullptr)
   {
      //read the private key portion from the BIO
      EVP_PKEY * const pc_PrivKey = PEM_read_bio_PrivateKey(pc_PrivKeyFile, nullptr, nullptr, nullptr);

      BIO_free(pc_PrivKeyFile);
      if ((pc_PrivKey != nullptr) && (EVP_PKEY_is_a(pc_PrivKey, "EC") == 1))
      {
         //extract the private key using OpenSSL 3.0 EVP API
         //EVP_PKEY_get_raw_private_key does not support EC keys (only X25519, Ed25519, etc.)
         //so we use the generic EVP_PKEY_get_bn_param instead.
         BIGNUM * pc_PrivBigNum = nullptr;
         if (EVP_PKEY_get_bn_param(pc_PrivKey, OSSL_PKEY_PARAM_PRIV_KEY, &pc_PrivBigNum) == 1)
         {
            const int x_Size = BN_num_bytes(pc_PrivBigNum); //lint !e970 !e8080 //use type expected by API
            std::vector<uint8_t> c_PrivKey(x_Size);

            //convert BIGNUM to byte array
            BN_bn2bin(pc_PrivBigNum, &c_PrivKey[0]);

            //write private key to our internal structure
            this->mc_KeyInfo.SetPrivateKey(c_PrivKey);

            BN_clear_free(pc_PrivBigNum);
         }
         EVP_PKEY_free(pc_PrivKey);
      }
      else
      {
         //No error as private key part is only sometimes present, but info could be useful in layers above
         orc_ErrorMessage = "Information: No private key present in given file.";
      }
   }
   else
   {
      c_Retval = Errc::config;
      orc_ErrorMessage = "Private key: could not read file content.";
   }

   return c_Retval;
}
