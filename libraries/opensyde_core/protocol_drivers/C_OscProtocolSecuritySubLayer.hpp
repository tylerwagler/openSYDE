//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol security sub layer

   Wraps up encryption/decryption of protocol traffic.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPROTOCOLSECURITYSUBELAYERHPP
#define C_OSCPROTOCOLSECURITYSUBELAYERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <map>
#include <system_error>
#include "stwtypes.hpp"

#include "C_OscSecurityEcdhAes.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//base with generic implementation
class C_OscProtocolSecuritySubLayerBase
{
public:
   C_OscProtocolSecuritySubLayerBase();
   virtual ~C_OscProtocolSecuritySubLayerBase();

   virtual std::error_code InitEcdhKeys() = 0;
   virtual std::error_code GetEcdhPublicKey(std::vector<uint8_t> & orc_TrafficEncryptionPublicClientKey) = 0;
   virtual std::error_code DeriveAesKey(const std::vector<uint8_t> & orc_OthersPublicKey) = 0;
   virtual std::error_code SetAesInitVector(const std::vector<uint8_t> & orc_InitVector) = 0;

   void SetEncryptionIsActive(const bool oq_IsActive);
   bool GetEncryptionIsActive() const;

   std::error_code WrapRequest(const C_OscProtocolDriverOsyService & orc_UnwrappedService,
                               C_OscProtocolDriverOsyService & orc_WrappedService);
   std::error_code UnwrapResponse(const C_OscProtocolDriverOsyService & orc_WrappedService,
                                  C_OscProtocolDriverOsyService & orc_UnwrappedService);

protected:
   virtual std::error_code m_Encrypt(const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output) = 0;
   virtual std::error_code m_Decrypt(const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output) = 0;

   static const uint8_t mhu8_OSY_SI_SECURED_DATA_TRANSMISSION = 0x84U;

   uint8_t mau8_AesInitVectorRequest[C_OscSecurityAesCbc::hu32_IV_LENGTH];
   uint8_t mau8_AesInitVectorResponse[C_OscSecurityAesCbc::hu32_IV_LENGTH];
   bool mq_EncryptionIsActive;
   bool mq_EcdhKeysInitialized;
   bool mq_AesKeyDerived;
};

//----------------------------------------------------------------------------------------------------------------------
//implementation using specific AES and ECDH libraries
class C_OscProtocolSecuritySubLayer :
   public C_OscProtocolSecuritySubLayerBase
{
public:
   C_OscProtocolSecuritySubLayer();
   virtual ~C_OscProtocolSecuritySubLayer();

   virtual std::error_code InitEcdhKeys();
   virtual std::error_code GetEcdhPublicKey(std::vector<uint8_t> & orc_TrafficEncryptionPublicClientKey);
   virtual std::error_code DeriveAesKey(const std::vector<uint8_t> & orc_OthersPublicKey);
   virtual std::error_code SetAesInitVector(const std::vector<uint8_t> & orc_InitVector);

   static C_OscProtocolSecuritySubLayer * h_GetConfigByNodeId(const C_OscProtocolDriverOsyNode & orc_NodeId);
   static void h_ClearAll();

protected:
   static std::map<C_OscProtocolDriverOsyNode, C_OscProtocolSecuritySubLayer> mhc_TheConfig;

   C_OscSecurityEcdhAes mc_Ecdh;

   uint8_t mau8_EcdhPublicKey[C_OscSecurityEcdhAes::hu32_PUBLIC_KEY_LENGTH];

   virtual std::error_code m_Encrypt(const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output);
   virtual std::error_code m_Decrypt(const std::vector<uint8_t> & orc_Input, std::vector<uint8_t> & orc_Output);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
