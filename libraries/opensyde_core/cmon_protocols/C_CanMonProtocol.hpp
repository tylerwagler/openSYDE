//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to textual interpretation

   \copyright   Copyright 2002 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLHPP
#define CCMONPROTOCOLHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscErrorCategory.hpp"

#include "stwtypes.hpp"
#include "stw_can.hpp"
#include "C_CanMonProtocolBase.hpp"
#include "C_CanMonProtocolCanOpen.hpp"
#include "C_CanMonProtocolGd.hpp"
#include "C_CanMonProtocolL2.hpp"
#include "C_CanMonProtocolJ1939.hpp"
#include "C_CanMonProtocolOpenSyde.hpp"
#include "C_CanMonProtocolUds.hpp"
#include <string>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

const int32_t gs32_CMON_NUM_PROTOCOLS = 7;

enum e_CanMonL7Protocols
{
   eCMON_L7_PROTOCOL_NONE = 0, //L2 only
   eCMON_L7_PROTOCOL_CAN_OPEN,
   eCMON_L7_PROTOCOL_GD,
   eCMON_L7_PROTOCOL_J1939,
   eCMON_L7_PROTOCOL_OPEN_SYDE,
   eCMON_L7_PROTOCOL_CAN_TP,  // ISO 15765-2 (transport layer, L2 interpretation)
   eCMON_L7_PROTOCOL_UDS      // ISO 14229 (Unified Diagnostic Services)
};

//----------------------------------------------------------------------------------------------------------------------
///Aggregator for all possible protocol interpretation types
class C_CanMonProtocols
{
private:
   bool mq_Decimal;

   C_CanMonProtocolL2 mc_ProtocolL2;
   C_CanMonProtocolCanOpen mc_ProtocolCanOpen;
   C_CanMonProtocolGd mc_ProtocolGd;
   C_CanMonProtocolJ1939 mc_ProtocolJ1939;
   C_CanMonProtocolOpenSyde mc_ProtocolOpenSyde;
   C_CanMonProtocolUds mc_ProtocolUds;

protected:
   C_CanMonProtocolBase * mapc_Protocols[gs32_CMON_NUM_PROTOCOLS];
   e_CanMonL7Protocols me_ActiveProtocol;

public:
   //general:
   C_CanMonProtocols(void);
   virtual ~C_CanMonProtocols(void);

   C_CanMonProtocols(const C_CanMonProtocols & orc_Source);               //copying will cause compiler error
   C_CanMonProtocols & operator = (const C_CanMonProtocols & orc_Source); //assignment will cause compiler error

   std::error_code SetProtocolMode(const e_CanMonL7Protocols oe_L7Protocol);
   e_CanMonL7Protocols GetProtocolMode(void) const;

   std::error_code GetProtocolName(const e_CanMonL7Protocols oe_L7Protocol, std::string & orc_Description) const;

   std::error_code SetDecimalMode(const bool oq_Decimal);
   bool GetDecimalMode(void) const;

   static std::string FormatTimeStamp(const uint64_t ou64_TimeStampUs, const bool oq_LeftFillBlanks = false);

   //for displaying on screen (will consider configured L7 protocol and decimal/hex setting):
   std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   std::string MessageToString(const stw::can::T_STWCAN_Msg_TX & orc_Msg) const;
   std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Message,
                                         const uint32_t ou32_Count) const;

   //for protocolling (e.g. to file; individual fields separated by semicolon; L2 AND L7 interpretation; always hex)
   std::string MessageToStringLog(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   std::string MessageToStringLog(const stw::can::T_STWCAN_Msg_TX & orc_Msg) const;

   std::error_code SaveProtocolParametersToIni(const std::string & orc_FileName,
                                       const std::string & orc_Section) const;
   std::error_code LoadProtocolParametersFromIni(const std::string & orc_FileName,
                                         const std::string & orc_Section) const;
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
