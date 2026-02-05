//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to J1939 protocol L7 interpretation

   \copyright   Copyright 2010 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLJ1939HPP
#define CCMONPROTOCOLJ1939HPP

#include "stwtypes.hpp"
#include "C_CanMonProtocolBase.hpp"
#include <QString>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------
///Interpretation of SAE J1939 protocol
class C_CanMonProtocolJ1939 :
   public C_CanMonProtocolBase
{
public:
   C_CanMonProtocolJ1939(void);
   virtual QString MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual QString GetProtocolName(void) const;

private:
   QString m_PgnToString(const uint32_t ou32_Pgn) const;
   uint32_t m_GetPgn(const uint8_t * const opu8_Data) const;
   QString m_GetMessageSize(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   QString m_GetData(const stw::can::T_STWCAN_Msg_RX & orc_Msg, const uint8_t ou8_StartIdx) const;
   QString m_GetName(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
