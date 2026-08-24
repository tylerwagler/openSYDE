//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to openSYDE protocol L7 interpretation

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLOPENSYDEHPP
#define CCMONPROTOCOLOPENSYDEHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "stwtypes.hpp"
#include "C_CanMonProtocolBase.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace cmon_protocol
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Interpretation of openSYDE protocol
class C_CanMonProtocolOpenSyde :
   public C_CanMonProtocolBase
{
protected:
   struct T_NodeIdentifier
   {
      uint8_t u8_Subnet;
      uint8_t u8_NodeId;
   };

   struct T_CanAddressInformation ///< disassembled CAN identifier
   {
      T_NodeIdentifier t_NodeIdSource; ///< local CAN address and subnet
      T_NodeIdentifier t_NodeIdTarget; ///< target CAN address and subnet
      uint8_t u8_AddressMode;          ///< fixed addressing mode physical/functional
      uint8_t u8_RoutingMode;          ///< routing configured or not
   };

   virtual std::string m_AddressInformationToText(const T_CanAddressInformation & orc_CanAddressInformation)
   const;
   virtual std::string m_DataPoolIdentifierToText(const uint32_t ou32_DataPoolIdentifier,
                                                            const bool oq_IsResponse,
                                                            const T_CanAddressInformation & orc_CanAddressInformation)
   const;

private:
   static int32_t mh_DisassembleCanId(const uint32_t ou32_CanId, T_CanAddressInformation & or_CanAddressInformation);

   static std::string mh_ThreeByteVersionToString(const uint8_t * const opu8_Version);
   std::string m_RawDataToString(const uint8_t ou8_NumBytes, const uint8_t * const opu8_Data) const;
   std::string m_GetSupposedlyAsciiText(const uint8_t ou8_NumBytes, const uint8_t * const opu8_Data) const;
   std::string m_ServiceIdToText(const uint8_t ou8_ServiceData,
                                           const bool oq_IsNegativeResponse = false) const;
   std::string m_SessionIdToText(const uint8_t ou8_SessionId) const;
   std::string m_ServiceDataToText(const uint8_t * const opu8_ServiceData, const uint8_t ou8_ServiceSize,
                                             const T_CanAddressInformation & orc_CanAddressInformation,
                                             const bool oq_IsSingleFrame) const;
   std::string m_NegativeResponseCodeToText(const uint8_t ou8_NrCode) const;
   std::string m_DataIdentifierAndDataToText(const bool oq_IsWrite, const uint16_t ou16_DataIdentifier,
                                                       const bool oq_IsResponse, const uint8_t ou8_PayloadSize,
                                                       const uint8_t * const opu8_Payload) const;
   std::string m_RoutineDataToText(const uint16_t ou16_RoutineIdentifier, const bool oq_IsResponse,
                                             const uint8_t ou8_DataSize, const uint8_t * const opu8_Data) const;

public:
   C_CanMonProtocolOpenSyde(void);

   virtual std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual std::string GetProtocolName(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
