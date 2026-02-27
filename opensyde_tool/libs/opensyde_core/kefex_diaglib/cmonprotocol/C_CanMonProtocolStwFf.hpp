//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to STW-FF protocol L7 interpretation

   \copyright   Copyright 2005 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLSTWFFHPP
#define CCMONPROTOCOLSTWFFHPP

#include "C_CanMonProtocolBase.hpp"
#include "stwtypes.hpp"
#include <QString>

//----------------------------------------------------------------------------------------------------------------------

namespace stw {
namespace cmon_protocol {
//----------------------------------------------------------------------------------------------------------------------

/// Interpretation of STWFF protocol
class C_CanMonProtocolStwFf : public C_CanMonProtocolBase {
public:
  virtual QString
  MessageToString(const stw::can::T_STWCAN_Msg_RX &orc_Msg) const;
  virtual QString GetProtocolName(void) const;
};

//----------------------------------------------------------------------------------------------------------------------
} // namespace cmon_protocol
} // namespace stw

#endif
