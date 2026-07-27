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
#include "stwtypes.hpp"
#include "C_CanMonProtocolBase.hpp"
#include "C_SclString.hpp"

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
   static stw::scl::C_SclString mh_ServiceIdToText(const uint8_t ou8_Sid, const bool oq_IsResponse,
                                                    const bool oq_IsNegativeResponse);
   static stw::scl::C_SclString mh_SubFunctionToText(const uint8_t ou8_ServiceId, const uint8_t ou8_SubFunc);
   static stw::scl::C_SclString mh_NegativeResponseCodeToText(const uint8_t ou8_Nrc);
   static stw::scl::C_SclString mh_SessionToText(const uint8_t ou8_Session);
   static stw::scl::C_SclString mh_ResetTypeToText(const uint8_t ou8_ResetType);
   static stw::scl::C_SclString mh_DataIdentifierToText(const uint16_t ou16_Did);
   static stw::scl::C_SclString mh_RoutineIdentifierToText(const uint16_t ou16_Rid);
   static stw::scl::C_SclString mh_AccessTypeToText(const uint8_t ou8_AccessType);

public:
   C_CanMonProtocolUds(void);

   virtual stw::scl::C_SclString MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual stw::scl::C_SclString GetProtocolName(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
