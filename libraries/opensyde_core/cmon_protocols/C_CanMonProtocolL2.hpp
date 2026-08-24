//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to textual representation

   \copyright   Copyright 2003 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLL2HPP
#define CCMONPROTOCOLL2HPP

#include "stwtypes.hpp"
#include "C_CanMonProtocolBase.hpp"
#include <string>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

///Interpretation as layer two protocol (no L7 interpretation)
class C_CanMonProtocolL2 :
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
