//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to UDS (ISO 14229) L7 interpretation

   Translate L2 CAN message to Unified Diagnostic Services (ISO 14229) L7 interpretation

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CMONPROTOCOLUDS_HPP
#define C_CMONPROTOCOLUDS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_CanMonProtocolBase.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace cmon_protocol
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Interpretation of UDS (ISO 14229) protocol
class C_CanMonProtocolUds :
   public C_CanMonProtocolBase
{
private:
   static std::string mh_ServiceIdToText(const uint8_t ou8_Sid, const bool oq_IsResponse,
                                                    const bool oq_IsNegativeResponse);
   static std::string mh_SubFunctionToText(const uint8_t ou8_ServiceId, const uint8_t ou8_SubFunc);
   static std::string mh_NegativeResponseCodeToText(const uint8_t ou8_Nrc);
   static std::string mh_SessionToText(const uint8_t ou8_Session);
   static std::string mh_ResetTypeToText(const uint8_t ou8_ResetType);
   static std::string mh_DataIdentifierToText(const uint16_t ou16_Did);
   static std::string mh_RoutineIdentifierToText(const uint16_t ou16_Rid);
   static std::string mh_AccessTypeToText(const uint8_t ou8_AccessType);

public:
   C_CanMonProtocolUds(void);

   virtual std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual std::string GetProtocolName(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
