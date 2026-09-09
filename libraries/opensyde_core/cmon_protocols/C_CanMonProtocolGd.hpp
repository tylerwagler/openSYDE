//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to Generic Driver protocol L7 interpretation

   \copyright   Copyright 2007 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLGDHPP
#define CCMONPROTOCOLGDHPP

#include <cstdint>
#include "C_CanMonProtocolBase.hpp"
#include <string>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

///Interpretation of Generic Driver protocol
class C_CanMonProtocolGd :
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
