//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to KEFEX protocol L7 interpretation

   \copyright   Copyright 2003 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLKEFEXHPP
#define CCMONPROTOCOLKEFEXHPP

#include "stwtypes.hpp"
#include "C_CanMonProtocolBase.hpp"
#include <QString>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------
///Interpretation of KEFEX protocol
class C_CanMonProtocolKefex :
   public C_CanMonProtocolKefexIva
{
private:
   uint16_t mu16_KfxBaseID;

   QString m_MessageToStringKefex(const stw::can::T_STWCAN_Msg_RX & orc_Msg);
   QString m_KfxIndexAndErrorToString(const char_t * const opcn_Text, const uint16_t ou16_Index,
                                                    const uint16_t ou16_Error,
                                                    const bool oq_IsKefexVarIndex = true) const;
   QString m_KfxTextAndValueToString(const char_t * const opcn_Text, const uint32_t ou32_Value) const;

public:
   C_CanMonProtocolKefex(void);

   virtual QString MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const;
   virtual QString GetProtocolName(void) const;

   virtual int32_t SaveParamsToIni(QSettings & orc_IniFile, const QString & orc_Section);
   virtual int32_t LoadParamsFromIni(QSettings & orc_IniFile, const QString & orc_Section);

   uint16_t GetBaseId(void) const;
   void SetBaseId(const uint16_t ou16_BaseId);
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
