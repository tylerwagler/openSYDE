//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to CANopen protocol L7 interpretation

   \copyright   Copyright 2006 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLCANOPENHPP
#define CCMONPROTOCOLCANOPENHPP

#include <cstdint>
#include "C_CanMonProtocolBase.hpp"
#include <string>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

///CANopen protocol converter
class C_CanMonProtocolCanOpen :
   public C_CanMonProtocolBase
{
public:
   virtual std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual std::string GetProtocolName(void) const;
};

//----------------------------------------------------------------------------------------------------------------------
}
}
#endif
