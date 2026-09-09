//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol security sub layer

   For details cf. documentation in .h file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include <cstdint>
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscProtocolSecuritySubLayer.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"
#include "TglUtils.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
std::map<C_OscProtocolDriverOsyNode, C_OscProtocolSecuritySubLayer> C_OscProtocolSecuritySubLayer::mhc_TheConfig;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Initialize with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolSecuritySubLayerBase::C_OscProtocolSecuritySubLayerBase() :
   mq_EncryptionIsActive(false),
   mq_EcdhKeysInitialized(false),
   mq_AesKeyDerived(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   destructor

   Clean up
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolSecuritySubLayerBase::~C_OscProtocolSecuritySubLayerBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Initialize with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolSecuritySubLayer::C_OscProtocolSecuritySubLayer() :
   C_OscProtocolSecuritySubLayerBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   destructor

   Clean up
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolSecuritySubLayer::~C_OscProtocolSecuritySubLayer()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get config with specified NodeId

   If no entry exists add one to our map.
   So we keep a static map with multiple known busIds + nodeIds and their current traffic encryption configuration.
   The combination of busId + nodeId is guaranteed to be unique on a valid system.
   Known nodes and their states can be cleaned up with h_ClearAll.

   \param[in]    orc_NodeId   node id to get config for

   \return  pointer to config
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProtocolSecuritySubLayer * C_OscProtocolSecuritySubLayer::h_GetConfigByNodeId(
   const C_OscProtocolDriverOsyNode & orc_NodeId)
{
   //either get existing entry define for that node id or create new (empty) one
   return &mhc_TheConfig[orc_NodeId];
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clear all known nodes

   Clear list of all known nodes from map.
   Beware that this will render previous pointer returned by h_GetConfigByNodeId invalid.
   So only call this at central spots where there is no risk of the pointers still being in use.

   Effectively this will result in all new nodes being (re-)added when calling h_GetConfigByNodeId.
   So they will start with uninitialized keys and encryption off.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolSecuritySubLayer::h_ClearAll()
{
   mhc_TheConfig.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize ECDH keys

   If not done so already create set of private and public ECDH keys and store in class fields.

   \return
   std::error_code with Errc::success on success, Errc::noact on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::InitEcdhKeys()
{
   //once per life time of instance
   if (mq_EcdhKeysInitialized == false)
   {
      const std::error_code c_Result = mc_Ecdh.CreateEcKeys(mau8_EcdhPublicKey);
      if (c_Result == Errc::success)
      {
         this->mq_EcdhKeysInitialized = true;
      }
      return c_Result;
   }
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get created ECDH public key

   If no key was created yet the function will try to do so.

   \param[out]    orc_TrafficEncryptionPublicClientKey   ECDH public key in compressed format

   \return
   std::error_code with Errc::success on success, Errc::noact on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::GetEcdhPublicKey(
   std::vector<uint8_t> & orc_TrafficEncryptionPublicClientKey)
{
   const std::error_code c_Result = this->InitEcdhKeys();

   if (c_Result == Errc::success)
   {
      orc_TrafficEncryptionPublicClientKey.resize(C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH);
      (void)std::memcpy(&orc_TrafficEncryptionPublicClientKey[0], &this->mau8_EcdhPublicKey[0],
                        C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH);
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Derive AES key

   Derive AES from own private key and other's public key.
   Store result in class field.
   Can be called multiple times after ECDH key initialization to create new shared AES key.

   \param[in]   orc_OthersPublicKey    Others public key

   \return
   std::error_code with Errc::success on success,
   Errc::range if invalid parameter,
   Errc::noact if could not derive key,
   Errc::config if own ECDH keys were not initialized
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::DeriveAesKey(const std::vector<uint8_t> & orc_OthersPublicKey)
{
   if (mq_EcdhKeysInitialized == false)
   {
      return Errc::config;
   }

   if (orc_OthersPublicKey.size() != C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH)
   {
      return Errc::range;
   }

   uint8_t au8_OthersKey[C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH];
   (void)std::memcpy(&au8_OthersKey[0], &orc_OthersPublicKey[0], C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH);
   const std::error_code c_Result = mc_Ecdh.DeriveAesKey(au8_OthersKey);
   if (c_Result == Errc::success)
   {
      mq_AesKeyDerived = true;
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set AES init vector

   Set init vector for the the AES-CBC encryption/decryption

   \param[out]    orc_InitVector   AES init vector

   \return
   std::error_code with Errc::success on success, Errc::range if invalid vector
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::SetAesInitVector(const std::vector<uint8_t> & orc_InitVector)
{
   if (orc_InitVector.size() != C_OscSecurityAesCbc::hu32_IV_LENGTH)
   {
      return Errc::range;
   }

   (void)std::memcpy(&this->mau8_AesInitVectorRequest[0], &orc_InitVector[0], C_OscSecurityAesCbc::hu32_IV_LENGTH);
   (void)std::memcpy(&this->mau8_AesInitVectorResponse[0], &orc_InitVector[0], C_OscSecurityAesCbc::hu32_IV_LENGTH);
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set flag to activate/deactivate encryption

   Flag is just "pass through" in this class.

   \param[out]    oq_IsActive    true: mark encryption as active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProtocolSecuritySubLayerBase::SetEncryptionIsActive(const bool oq_IsActive)
{
   this->mq_EncryptionIsActive = oq_IsActive;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get flag to check active encryption

   Flag is just "pass through" in this class.

   \return   true: encryption flag is active
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscProtocolSecuritySubLayerBase::GetEncryptionIsActive() const
{
   return this->mq_EncryptionIsActive;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Perform encryption

   Uses known AES key and request init vector.

   \param[in]     orc_Input       data to encrypt
   \param[out]    orc_Output      encrypted data

   \return
   std::error_code with Errc::success on success, Errc::config on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::m_Encrypt(const std::vector<uint8_t> & orc_Input,
                                                         std::vector<uint8_t> & orc_Output)
{
   return mc_Ecdh.AesEncrypt(this->mau8_AesInitVectorRequest, orc_Input, orc_Output);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Perform decryption

   Uses known AES key and response init vector.

   \param[in]     orc_Input       data to decrypt
   \param[out]    orc_Output      decrypted data

   \return
   std::error_code with Errc::success on success, Errc::config on failure
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayer::m_Decrypt(const std::vector<uint8_t> & orc_Input,
                                                         std::vector<uint8_t> & orc_Output)
{
   return mc_Ecdh.AesDecrypt(this->mau8_AesInitVectorResponse, orc_Input, orc_Output);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Encrypt and wrap up service

   Encrypt service data and wrap up into a SecuredDataTransmission service.

   \param[in]     orc_UnwrappedService    unencrypted service to wrap up
   \param[out]    orc_WrappedService      wrapped up, encrypted service

   \return
   std::error_code with Errc::success on success,
   Errc::config if encryption engine not initialized or not set active,
   Errc::checksum if encryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayerBase::WrapRequest(
   const C_OscProtocolDriverOsyService & orc_UnwrappedService,
   C_OscProtocolDriverOsyService & orc_WrappedService)
{
   if ((this->mq_EncryptionIsActive == false) || (this->mq_AesKeyDerived == false))
   {
      return Errc::config;
   }

   std::vector<uint8_t> c_EncryptedData;
   const std::error_code c_Result = m_Encrypt(orc_UnwrappedService.c_Data, c_EncryptedData);
   if (c_Result != Errc::success)
   {
      return Errc::checksum;
   }

   orc_WrappedService.q_CanTransferWithoutFlowControl = false; //this service always needs a flow control

   orc_WrappedService.c_Data.resize(4U + c_EncryptedData.size());
   orc_WrappedService.c_Data[0] = mhu8_OSY_SI_SECURED_DATA_TRANSMISSION;
   orc_WrappedService.c_Data[1] = 0x00U; // 16bit APAR: Bit0 is set (request message); other bits are constants
   orc_WrappedService.c_Data[2] = 0x19U;
   orc_WrappedService.c_Data[3] = 0x00U; // algorithm: AES-CBC-128 with PKCS#7
   (void)std::memcpy(&orc_WrappedService.c_Data[4], &c_EncryptedData[0], c_EncryptedData.size());

   tgl_assert(c_EncryptedData.size() >= C_OscSecurityAesCbc::hu32_IV_LENGTH);

   //remember last 16 bytes of encrypted result as new init vector for following services:
   (void)std::memcpy(&this->mau8_AesInitVectorRequest[0],
                     &c_EncryptedData[c_EncryptedData.size() - C_OscSecurityAesCbc::hu32_IV_LENGTH],
                     C_OscSecurityAesCbc::hu32_IV_LENGTH);

   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Unwrap and decrypt service response

   Unwrap incoming SecuredDataTransmission response and decrypt payload.

   \param[in]     orc_WrappedService    encrypted service to unwrap
   \param[out]    orc_UnwrappedService  unwrapped, decrypted service

   \return
   std::error_code with Errc::success on success,
   Errc::range if orc_WrappedService is not a valid SecuredDataTransmission service,
   Errc::config if encryption engine not initialized or not set active,
   Errc::checksum if decryption failed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProtocolSecuritySubLayerBase::UnwrapResponse(
   const C_OscProtocolDriverOsyService & orc_WrappedService,
   C_OscProtocolDriverOsyService & orc_UnwrappedService)
{
   if ((this->mq_EncryptionIsActive == false) || (this->mq_AesKeyDerived == false))
   {
      return Errc::config;
   }

   if (orc_WrappedService.c_Data.size() < 4U)
   {
      return Errc::range;
   }

   if ((orc_WrappedService.c_Data[0] != (mhu8_OSY_SI_SECURED_DATA_TRANSMISSION | 0x40U)) ||
       (orc_WrappedService.c_Data[1] != 0x40U) ||
       (orc_WrappedService.c_Data[2] != 0x19U) ||
       (orc_WrappedService.c_Data[3] != 0x00U))
   {
      return Errc::range;
   }

   const std::vector<uint8_t> c_EncryptedData(orc_WrappedService.c_Data.begin() + 4,
                                               orc_WrappedService.c_Data.end());

   std::vector<uint8_t> c_DecryptedData;
   const std::error_code c_Result = m_Decrypt(c_EncryptedData, c_DecryptedData);
   if (c_Result != Errc::success)
   {
      return Errc::checksum;
   }

   orc_UnwrappedService.c_Data.swap(c_DecryptedData);

   tgl_assert(c_EncryptedData.size() >= C_OscSecurityAesCbc::hu32_IV_LENGTH);

   //remember last 16 bytes of encrypted result as new init vector for following services:
   (void)std::memcpy(&this->mau8_AesInitVectorResponse[0],
                     &c_EncryptedData[c_EncryptedData.size() - C_OscSecurityAesCbc::hu32_IV_LENGTH],
                     C_OscSecurityAesCbc::hu32_IV_LENGTH);

   return Errc::success;
}
