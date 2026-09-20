//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Signatures, PEM material and key agreement: the pieces the secure update flow is built on

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "C_OscErrorCategory.hpp"
#include "C_OscSecurityAesCbc.hpp"
#include "C_OscSecurityEcdhAes.hpp"
#include "C_OscSecurityEcdsa.hpp"
#include "C_OscSecurityPem.hpp"
#include "C_OscSecurityPemDatabase.hpp"
#include "C_OscSecurityPemSecUpdate.hpp"
#include "C_OscSecurityRsa.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A fresh key with a self-signed certificate, in every encoding the classes under test consume
*/
//----------------------------------------------------------------------------------------------------------------------
class C_TestIdentity
{
public:
   std::vector<uint8_t> c_CertDer;   ///< what the verifiers take as "public key"
   std::vector<uint8_t> c_Pkcs8Der;  ///< what the RSA signer takes as "private key"
   std::vector<uint8_t> c_Serial;    ///< the certificate serial's content bytes: how the PEM database keys entries
   std::string c_Pem;                ///< certificate followed by the unencrypted PKCS#8 private key
   EVP_PKEY * pc_Key = nullptr;

   ~C_TestIdentity()
   {
      EVP_PKEY_free(pc_Key);
   }
};

std::vector<uint8_t> h_Der(int (* const opr_Encode)(void *, unsigned char **), void * const opv_Object)
{
   unsigned char * pu8_Buffer = nullptr;
   const int x_Len = opr_Encode(opv_Object, &pu8_Buffer);
   std::vector<uint8_t> c_Out;

   if (x_Len > 0)
   {
      c_Out.assign(pu8_Buffer, pu8_Buffer + x_Len);
   }
   OPENSSL_free(pu8_Buffer);
   return c_Out;
}

//key usage strings are the OpenSSL configuration spellings ("digitalSignature", "emailProtection"); nullptr = none
C_TestIdentity h_MakeIdentityFromKey(EVP_PKEY * const opc_Key, const long ox_Serial, const char * const opcn_KeyUsage,
                                     const char * const opcn_ExtendedKeyUsage)
{
   C_TestIdentity c_Id;

   c_Id.pc_Key = opc_Key;
   X509 * const pc_Cert = X509_new();
   (void)X509_set_version(pc_Cert, 2);
   (void)ASN1_INTEGER_set(X509_get_serialNumber(pc_Cert), ox_Serial);
   (void)X509_gmtime_adj(X509_getm_notBefore(pc_Cert), 0);
   (void)X509_gmtime_adj(X509_getm_notAfter(pc_Cert), 60L * 60L * 24L * 365L);
   X509_NAME * const pc_Name = X509_get_subject_name(pc_Cert);
   (void)X509_NAME_add_entry_by_txt(pc_Name, "CN", MBSTRING_ASC,
                                    reinterpret_cast<const unsigned char *>("osy test identity"), -1, -1, 0);
   (void)X509_set_issuer_name(pc_Cert, pc_Name);
   (void)X509_set_pubkey(pc_Cert, c_Id.pc_Key);
   for (const auto & rc_Ext : std::vector<std::pair<int, const char *> >{{NID_key_usage, opcn_KeyUsage},
                                                                          {NID_ext_key_usage, opcn_ExtendedKeyUsage}})
   {
      if (rc_Ext.second != nullptr)
      {
         X509_EXTENSION * const pc_Ext = X509V3_EXT_conf_nid(nullptr, nullptr, rc_Ext.first, rc_Ext.second);
         (void)X509_add_ext(pc_Cert, pc_Ext, -1);
         X509_EXTENSION_free(pc_Ext);
      }
   }
   (void)X509_sign(pc_Cert, c_Id.pc_Key, EVP_sha256());

   c_Id.c_CertDer = h_Der(reinterpret_cast<int (*)(void *, unsigned char **)>(i2d_X509), pc_Cert);
   {
      //the loader keeps the INTEGER's content and drops the two-byte tag/length header (short form: serials are
      //never longer than 20 bytes)
      const std::vector<uint8_t> c_Der = h_Der(reinterpret_cast<int (*)(void *, unsigned char **)>(i2d_ASN1_INTEGER),
                                               X509_get_serialNumber(pc_Cert));
      c_Id.c_Serial.assign(c_Der.begin() + 2, c_Der.end());
   }
   PKCS8_PRIV_KEY_INFO * const pc_P8 = EVP_PKEY2PKCS8(c_Id.pc_Key);
   c_Id.c_Pkcs8Der = h_Der(reinterpret_cast<int (*)(void *, unsigned char **)>(i2d_PKCS8_PRIV_KEY_INFO), pc_P8);
   PKCS8_PRIV_KEY_INFO_free(pc_P8);

   BIO * const pc_Bio = BIO_new(BIO_s_mem());
   (void)PEM_write_bio_X509(pc_Bio, pc_Cert);
   (void)PEM_write_bio_PKCS8PrivateKey(pc_Bio, c_Id.pc_Key, nullptr, nullptr, 0, nullptr, nullptr);
   char * pcn_Text = nullptr;
   const long x_TextLen = BIO_get_mem_data(pc_Bio, &pcn_Text);
   c_Id.c_Pem.assign(pcn_Text, static_cast<size_t>(x_TextLen));
   BIO_free(pc_Bio);
   X509_free(pc_Cert);
   return c_Id;
}

C_TestIdentity h_MakeIdentity(const bool oq_Rsa, const long ox_Serial, const char * const opcn_KeyUsage,
                              const char * const opcn_ExtendedKeyUsage)
{
   return h_MakeIdentityFromKey(oq_Rsa ? EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", static_cast<size_t>(2048)) :
                                EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "P-256"),
                                ox_Serial, opcn_KeyUsage, opcn_ExtendedKeyUsage);
}

///A P-256 key whose named scalar (the private key, or the public point's x or y) is below 2^248, so that its
///big-endian form has a leading zero byte. About one key in 256 per scalar; generated by trying.
EVP_PKEY * h_MakeP256KeyWithLeadingZero(const char * const opcn_Param)
{
   for (uint32_t u32_Try = 0U; u32_Try < 20000U; ++u32_Try)
   {
      EVP_PKEY * const pc_Key = EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "P-256");
      BIGNUM * pc_Value = nullptr;
      bool q_Short = false;
      if (std::strcmp(opcn_Param, OSSL_PKEY_PARAM_PRIV_KEY) == 0)
      {
         q_Short = (EVP_PKEY_get_bn_param(pc_Key, OSSL_PKEY_PARAM_PRIV_KEY, &pc_Value) == 1) &&
                   (BN_num_bytes(pc_Value) < 32);
      }
      else
      {
         q_Short = (EVP_PKEY_get_bn_param(pc_Key, opcn_Param, &pc_Value) == 1) && (BN_num_bytes(pc_Value) < 32);
      }
      BN_free(pc_Value);
      if (q_Short)
      {
         return pc_Key;
      }
      EVP_PKEY_free(pc_Key);
   }
   return nullptr;
}

std::string h_WritePem(const std::filesystem::path & orc_Path, const std::string & orc_Text)
{
   std::ofstream c_Out(orc_Path, std::ios::binary);

   c_Out << orc_Text;
   return orc_Path.string();
}

std::vector<uint8_t> h_Message(void)
{
   std::vector<uint8_t> c_Msg(32U);

   for (uint32_t u32_It = 0U; u32_It < c_Msg.size(); ++u32_It)
   {
      c_Msg[u32_It] = static_cast<uint8_t>(0x30U + u32_It);
   }
   return c_Msg;
}
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, RsaSignsAndVerifiesWithCertificateAndPkcs8Key)
{
   const C_TestIdentity c_Id = h_MakeIdentity(true, 7L, nullptr, nullptr);
   const std::vector<uint8_t> c_Msg = h_Message();
   std::vector<uint8_t> c_Sig;

   ASSERT_FALSE(static_cast<bool>(C_OscSecurityRsa::h_SignSignature(c_Id.c_Pkcs8Der, c_Msg, c_Sig)));
   EXPECT_EQ(256U, c_Sig.size()) << "2048-bit RSA";

   bool q_Valid = false;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityRsa::h_VerifySignature(c_Id.c_CertDer, c_Msg, c_Sig, q_Valid)));
   EXPECT_TRUE(q_Valid);

   std::vector<uint8_t> c_Other = c_Msg;
   c_Other[5] ^= 0x01U;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityRsa::h_VerifySignature(c_Id.c_CertDer, c_Other, c_Sig, q_Valid)));
   EXPECT_FALSE(q_Valid) << "one flipped bit in the message";

   const C_TestIdentity c_Stranger = h_MakeIdentity(true, 8L, nullptr, nullptr);
   const std::error_code c_Res = C_OscSecurityRsa::h_VerifySignature(c_Stranger.c_CertDer, c_Msg, c_Sig, q_Valid);
   EXPECT_TRUE(static_cast<bool>(c_Res) || (q_Valid == false)) << "another key must not verify it";
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, PemFileYieldsCertificateKeySerialAndUsage)
{
   const C_TestIdentity c_Id = h_MakeIdentity(true, 0x1234L, "digitalSignature,keyEncipherment", "emailProtection");
   const std::string c_Path = h_WritePem(std::filesystem::temp_directory_path() / "osy_sec_rsa.pem", c_Id.c_Pem);
   C_OscSecurityPem c_Pem;
   std::string c_Error;

   ASSERT_FALSE(static_cast<bool>(c_Pem.LoadFromFile(c_Path, c_Error))) << c_Error;
   EXPECT_EQ(c_Id.c_CertDer, c_Pem.GetKeyInfo().GetX509CertificateData());
   EXPECT_EQ(c_Id.c_Pkcs8Der, c_Pem.GetKeyInfo().GetPrivateKey());
   EXPECT_EQ(c_Id.c_Serial, c_Pem.GetKeyInfo().GetCertificateSerialNumber());
   const C_OscSecurityPemKeyInfo::C_CertificateKeyUsageInformation & rc_Usage = c_Pem.GetKeyInfo().GetKeyUsageInformation();
   EXPECT_TRUE(rc_Usage.q_KeyUsageDefined);
   EXPECT_TRUE(rc_Usage.q_KeyUsageDigitalSignature);
   EXPECT_TRUE(rc_Usage.q_KeyUsageKeyEncipherment);
   EXPECT_FALSE(rc_Usage.q_KeyUsageCrlSign);
   EXPECT_TRUE(rc_Usage.q_ExtendedKeyUsageDefined);
   EXPECT_TRUE(rc_Usage.q_ExtendedKeyUsageEmailProtection);
   EXPECT_FALSE(rc_Usage.q_ExtendedKeyUsageCodeSigning);
   EXPECT_NE(std::string::npos, c_Pem.GetMetaInfos().find("osy test identity")) << c_Pem.GetMetaInfos();

   std::vector<uint8_t> c_Modulus;
   std::vector<uint8_t> c_Exponent;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityPem::h_ExtractModulusAndExponentFromFile(c_Path, c_Modulus, c_Exponent,
                                                                                         c_Error))) << c_Error;
   EXPECT_EQ(256U, c_Modulus.size());
   EXPECT_EQ((std::vector<uint8_t>{0x01U, 0x00U, 0x01U}), c_Exponent) << "F4";
   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, PemDatabaseIndexesFolderBySerialNumber)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_sec_pemdb";

   (void)std::filesystem::remove_all(c_Dir);
   ASSERT_TRUE(std::filesystem::create_directories(c_Dir));
   const C_TestIdentity c_A = h_MakeIdentity(true, 21L, nullptr, nullptr);
   const C_TestIdentity c_B = h_MakeIdentity(true, 22L, nullptr, nullptr);
   (void)h_WritePem(c_Dir / "a.pem", c_A.c_Pem);
   (void)h_WritePem(c_Dir / "b.pem", c_B.c_Pem);
   //a certificate without its private key is not usable and must be skipped, not fail the folder
   (void)h_WritePem(c_Dir / "public_only.pem", c_B.c_Pem.substr(0, c_B.c_Pem.find("-----BEGIN PRIVATE KEY-----")));

   C_OscSecurityPemDatabase c_Db;
   ASSERT_FALSE(static_cast<bool>(c_Db.ParseFolder(c_Dir.string())));
   EXPECT_EQ(2U, c_Db.GetSizeOfDatabase());
   const C_OscSecurityPemKeyInfo * const pc_A = c_Db.GetPemFileBySerialNumber(c_A.c_Serial);
   ASSERT_NE(nullptr, pc_A);
   EXPECT_EQ(c_A.c_Pkcs8Der, pc_A->GetPrivateKey());
   EXPECT_EQ(nullptr, c_Db.GetPemFileBySerialNumber(std::vector<uint8_t>{0x63U})) << "serial 99";
   EXPECT_EQ(nullptr, c_Db.GetLevel7PemInformation());
   ASSERT_FALSE(static_cast<bool>(c_Db.AddLevel7PemFile((c_Dir / "b.pem").string())));
   ASSERT_NE(nullptr, c_Db.GetLevel7PemInformation());
   EXPECT_EQ(c_B.c_Serial, c_Db.GetLevel7PemInformation()->GetCertificateSerialNumber());
   EXPECT_EQ(Errc::range, c_Db.ParseFolder((c_Dir / "absent").string()));
   (void)std::filesystem::remove_all(c_Dir);
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, SecureUpdatePemRequiresSigningAndEmailProtectionUsage)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path();
   const C_TestIdentity c_Good = h_MakeIdentity(false, 31L, "digitalSignature", "emailProtection");
   const C_TestIdentity c_NoEku = h_MakeIdentity(false, 32L, "digitalSignature", nullptr);
   const C_TestIdentity c_NoUsage = h_MakeIdentity(false, 33L, nullptr, "emailProtection");
   std::string c_Error;

   {
      C_OscSecurityPemSecUpdate c_Pem;
      EXPECT_FALSE(static_cast<bool>(c_Pem.LoadFromFile(h_WritePem(c_Dir / "osy_sec_good.pem", c_Good.c_Pem),
                                                        c_Error))) << c_Error;
      EXPECT_EQ(c_Good.c_CertDer, c_Pem.GetKeyInfo().GetX509CertificateData());
      EXPECT_FALSE(c_Pem.GetKeyInfo().GetPrivateKey().empty());
   }
   {
      C_OscSecurityPemSecUpdate c_Pem;
      EXPECT_EQ(Errc::overflow, c_Pem.LoadFromFile(h_WritePem(c_Dir / "osy_sec_noeku.pem", c_NoEku.c_Pem), c_Error));
   }
   {
      C_OscSecurityPemSecUpdate c_Pem;
      EXPECT_EQ(Errc::overflow,
                c_Pem.LoadFromFile(h_WritePem(c_Dir / "osy_sec_nousage.pem", c_NoUsage.c_Pem), c_Error));
   }
   for (const char * const pcn_Name : {"osy_sec_good.pem", "osy_sec_noeku.pem", "osy_sec_nousage.pem"})
   {
      (void)std::filesystem::remove(c_Dir / pcn_Name);
   }
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, EcdsaSignsVerifiesAndRoundTripsThroughDer)
{
   const C_TestIdentity c_Id = h_MakeIdentity(false, 41L, nullptr, nullptr);
   uint8_t au8_Private[C_OscSecurityEcdsa::hu32_SECP256R1_PRIVATE_KEY_LENGTH];
   {
      BIGNUM * pc_Priv = nullptr;
      ASSERT_EQ(1, EVP_PKEY_get_bn_param(c_Id.pc_Key, OSSL_PKEY_PARAM_PRIV_KEY, &pc_Priv));
      ASSERT_EQ(static_cast<int>(sizeof(au8_Private)), BN_bn2binpad(pc_Priv, au8_Private, sizeof(au8_Private)));
      BN_free(pc_Priv);
   }
   uint8_t au8_Public[C_OscSecurityEcdsa::hu32_SECP256R1_PUBLIC_KEY_LENGTH];
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_ExtractPublicKeyFromX509Certificate(c_Id.c_CertDer,
                                                                                            au8_Public)));

   const std::vector<uint8_t> c_Msg = h_Message();
   uint8_t au8_Digest[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH];
   {
      C_OscSecurityEcdsa c_Hash;
      ASSERT_FALSE(static_cast<bool>(c_Hash.Sha256Init()));
      ASSERT_FALSE(static_cast<bool>(c_Hash.Sha256Update(&c_Msg[0], static_cast<uint32_t>(c_Msg.size()))));
      ASSERT_FALSE(static_cast<bool>(c_Hash.Sha256GetDigest(au8_Digest)));
   }
   C_OscSecurityEcdsa::C_Ecdsa256Signature c_Sig;
   std::string c_Error;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_CalcEcdsaSecp256r1Signature(au8_Digest, au8_Private, c_Sig,
                                                                                    c_Error))) << c_Error;
   EXPECT_GT(c_Sig.u8_NumBytesUsedRpart, 0U);
   EXPECT_GT(c_Sig.u8_NumBytesUsedSpart, 0U);

   bool q_Valid = false;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_VerifyEcdsaSecp256r1Signature(au8_Public, c_Sig, au8_Digest,
                                                                                      q_Valid)));
   EXPECT_TRUE(q_Valid);

   //DER text is how the signature travels inside a package
   std::string c_Der;
   ASSERT_FALSE(static_cast<bool>(c_Sig.GetAsDerString(c_Der)));
   C_OscSecurityEcdsa::C_Ecdsa256Signature c_Back;
   ASSERT_FALSE(static_cast<bool>(c_Back.SetFromDerString(c_Der)));
   EXPECT_EQ(c_Sig.u8_NumBytesUsedRpart, c_Back.u8_NumBytesUsedRpart);
   EXPECT_EQ(0, std::memcmp(c_Sig.au8_Rpart, c_Back.au8_Rpart, sizeof(c_Sig.au8_Rpart)));
   EXPECT_EQ(0, std::memcmp(c_Sig.au8_Spart, c_Back.au8_Spart, sizeof(c_Sig.au8_Spart)));
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_VerifyEcdsaSecp256r1Signature(au8_Public, c_Back, au8_Digest,
                                                                                      q_Valid)));
   EXPECT_TRUE(q_Valid);

   au8_Digest[0] ^= 0x80U;
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_VerifyEcdsaSecp256r1Signature(au8_Public, c_Sig, au8_Digest,
                                                                                      q_Valid)));
   EXPECT_FALSE(q_Valid) << "one flipped bit in the digest";
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, EcdhPartiesAgreeOnTheAesKey)
{
   C_OscSecurityEcdhAes c_Alice;
   C_OscSecurityEcdhAes c_Bob;
   uint8_t au8_PubAlice[C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH];
   uint8_t au8_PubBob[C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH];

   ASSERT_FALSE(static_cast<bool>(c_Alice.CreateEcKeys(au8_PubAlice)));
   ASSERT_FALSE(static_cast<bool>(c_Bob.CreateEcKeys(au8_PubBob)));
   EXPECT_NE(0, std::memcmp(au8_PubAlice, au8_PubBob, sizeof(au8_PubAlice)));
   ASSERT_FALSE(static_cast<bool>(c_Alice.DeriveAesKey(au8_PubBob)));
   ASSERT_FALSE(static_cast<bool>(c_Bob.DeriveAesKey(au8_PubAlice)));

   uint8_t au8_KeyAlice[C_OscSecurityEcdhAes::hu32_AES_KEY_LENGTH];
   uint8_t au8_KeyBob[C_OscSecurityEcdhAes::hu32_AES_KEY_LENGTH];
   ASSERT_FALSE(static_cast<bool>(c_Alice.GetAesKey(au8_KeyAlice)));
   ASSERT_FALSE(static_cast<bool>(c_Bob.GetAesKey(au8_KeyBob)));
   EXPECT_EQ(0, std::memcmp(au8_KeyAlice, au8_KeyBob, sizeof(au8_KeyAlice)));

   const uint8_t au8_Iv[C_OscSecurityAesCbc::hu32_IV_LENGTH] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U,
                                                                 13U, 14U, 15U, 16U};
   const std::vector<uint8_t> c_Plain = h_Message();
   std::vector<uint8_t> c_Cipher;
   std::vector<uint8_t> c_Back;
   ASSERT_FALSE(static_cast<bool>(c_Alice.AesEncrypt(au8_Iv, c_Plain, c_Cipher)));
   EXPECT_NE(c_Plain, c_Cipher);
   ASSERT_FALSE(static_cast<bool>(c_Bob.AesDecrypt(au8_Iv, c_Cipher, c_Back)));
   EXPECT_EQ(c_Plain, c_Back);
}

//----------------------------------------------------------------------------------------------------------------------
TEST(SecuritySignatures, AesCbcRoundTripsAndPads)
{
   const uint8_t au8_Key[C_OscSecurityAesCbc::hu32_KEY_LENGTH] = {0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U,
                                                                  0x17U, 0x18U, 0x19U, 0x1AU, 0x1BU, 0x1CU, 0x1DU,
                                                                  0x1EU, 0x1FU};
   const uint8_t au8_Iv[C_OscSecurityAesCbc::hu32_IV_LENGTH] = {0U};
   std::vector<uint8_t> c_Plain(20U, 0xA5U);
   std::vector<uint8_t> c_Cipher;
   std::vector<uint8_t> c_Back;

   ASSERT_FALSE(static_cast<bool>(C_OscSecurityAesCbc::h_Encrypt(au8_Key, au8_Iv, c_Plain, c_Cipher)));
   EXPECT_EQ(32U, c_Cipher.size()) << "20 bytes padded to two blocks";
   ASSERT_FALSE(static_cast<bool>(C_OscSecurityAesCbc::h_Decrypt(au8_Key, au8_Iv, c_Cipher, c_Back)));
   EXPECT_EQ(c_Plain, c_Back);

   uint8_t au8_Wrong[C_OscSecurityAesCbc::hu32_KEY_LENGTH];
   std::memcpy(au8_Wrong, au8_Key, sizeof(au8_Wrong));
   au8_Wrong[0] ^= 0xFFU;
   const std::error_code c_Res = C_OscSecurityAesCbc::h_Decrypt(au8_Wrong, au8_Iv, c_Cipher, c_Back);
   EXPECT_TRUE(static_cast<bool>(c_Res) || (c_Back != c_Plain)) << "the wrong key must not yield the plaintext";
}

//----------------------------------------------------------------------------------------------------------------------
/* Elliptic-curve scalars are numbers. One key in about 256 has a coordinate or a private scalar
   below 2^248, whose big-endian form is 31 bytes, and both readers below used to size their
   output by the number rather than by the curve: the certificate reader rejected the key, the
   PEM reader handed out 31 bytes that the package creator then refused. Found because the
   ECDSA round-trip test above, which generates a fresh key every run, failed once in ~128 runs. */

TEST(SecuritySignatures, PublicKeyWithALeadingZeroCoordinateIsExtractedPadded)
{
   for (const char * const pcn_Param : {OSSL_PKEY_PARAM_EC_PUB_X, OSSL_PKEY_PARAM_EC_PUB_Y})
   {
      EVP_PKEY * const pc_Key = h_MakeP256KeyWithLeadingZero(pcn_Param);
      ASSERT_NE(nullptr, pc_Key) << pcn_Param;
      const C_TestIdentity c_Id = h_MakeIdentityFromKey(pc_Key, 51L, nullptr, nullptr);

      uint8_t au8_Public[C_OscSecurityEcdsa::hu32_SECP256R1_PUBLIC_KEY_LENGTH];
      ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_ExtractPublicKeyFromX509Certificate(c_Id.c_CertDer,
                                                                                               au8_Public)))
         << pcn_Param;

      //the padded coordinates are what OpenSSL itself reports
      for (const char * const pcn_Check : {OSSL_PKEY_PARAM_EC_PUB_X, OSSL_PKEY_PARAM_EC_PUB_Y})
      {
         BIGNUM * pc_Value = nullptr;
         ASSERT_EQ(1, EVP_PKEY_get_bn_param(c_Id.pc_Key, pcn_Check, &pc_Value));
         uint8_t au8_Expected[32];
         ASSERT_EQ(32, BN_bn2binpad(pc_Value, au8_Expected, 32));
         BN_free(pc_Value);
         const uint8_t * const pu8_Actual = (std::strcmp(pcn_Check, OSSL_PKEY_PARAM_EC_PUB_X) == 0) ? &au8_Public[0] :
                                            &au8_Public[32];
         EXPECT_EQ(0, std::memcmp(au8_Expected, pu8_Actual, 32U)) << pcn_Param << " / " << pcn_Check;
      }

      //and a signature made with the matching private key verifies against the extracted public key
      uint8_t au8_Private[32];
      {
         BIGNUM * pc_Priv = nullptr;
         ASSERT_EQ(1, EVP_PKEY_get_bn_param(c_Id.pc_Key, OSSL_PKEY_PARAM_PRIV_KEY, &pc_Priv));
         ASSERT_EQ(32, BN_bn2binpad(pc_Priv, au8_Private, 32));
         BN_free(pc_Priv);
      }
      uint8_t au8_Digest[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH] = {0x5AU};
      C_OscSecurityEcdsa::C_Ecdsa256Signature c_Sig;
      std::string c_Error;
      ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_CalcEcdsaSecp256r1Signature(au8_Digest, au8_Private, c_Sig,
                                                                                       c_Error))) << c_Error;
      bool q_Valid = false;
      ASSERT_FALSE(static_cast<bool>(C_OscSecurityEcdsa::h_VerifyEcdsaSecp256r1Signature(au8_Public, c_Sig,
                                                                                         au8_Digest, q_Valid)));
      EXPECT_TRUE(q_Valid) << pcn_Param;
   }
}

TEST(SecuritySignatures, PemPrivateKeyWithALeadingZeroIsThirtyTwoBytes)
{
   EVP_PKEY * const pc_Key = h_MakeP256KeyWithLeadingZero(OSSL_PKEY_PARAM_PRIV_KEY);
   ASSERT_NE(nullptr, pc_Key);
   const C_TestIdentity c_Id = h_MakeIdentityFromKey(pc_Key, 52L, "digitalSignature", "emailProtection");
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_sec_shortpriv.pem";
   std::string c_Error;

   C_OscSecurityPemSecUpdate c_Pem;
   ASSERT_FALSE(static_cast<bool>(c_Pem.LoadFromFile(h_WritePem(c_Path, c_Id.c_Pem), c_Error))) << c_Error;
   const std::vector<uint8_t> c_Private = c_Pem.GetKeyInfo().GetPrivateKey();
   ASSERT_EQ(C_OscSecurityEcdsa::hu32_SECP256R1_PRIVATE_KEY_LENGTH, c_Private.size());
   EXPECT_EQ(0U, c_Private[0]); //the padding byte is what makes it usable

   //and it is the same key OpenSSL has, padded
   BIGNUM * pc_Priv = nullptr;
   ASSERT_EQ(1, EVP_PKEY_get_bn_param(c_Id.pc_Key, OSSL_PKEY_PARAM_PRIV_KEY, &pc_Priv));
   uint8_t au8_Expected[32];
   ASSERT_EQ(32, BN_bn2binpad(pc_Priv, au8_Expected, 32));
   BN_free(pc_Priv);
   EXPECT_EQ(0, std::memcmp(au8_Expected, c_Private.data(), 32U));

   (void)std::filesystem::remove(c_Path);
}
