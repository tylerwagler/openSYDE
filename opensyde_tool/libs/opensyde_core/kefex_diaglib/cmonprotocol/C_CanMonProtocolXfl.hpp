//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to L7 interpretation

   Translate L2 CAN message to STW flashloader protocol L7 interpretation

   \copyright   Copyright 2003 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLXFLHPP
#define CCMONPROTOCOLXFLHPP

#include "C_CanMonProtocolBase.hpp"
#include "stwtypes.hpp"
#include <QString>

//----------------------------------------------------------------------------------------------------------------------

namespace stw {
namespace cmon_protocol {
//----------------------------------------------------------------------------------------------------------------------

/// Interpretation of STW Flashloader protocol
class C_CanMonProtocolXfl : public C_CanMonProtocolBase {
private:
  uint32_t mu32_XFLSendId;

public:
  C_CanMonProtocolXfl(void);

  virtual QString
  MessageToString(const stw::can::T_STWCAN_Msg_RX &orc_Msg) const;
  virtual QString GetProtocolName(void) const;

  virtual int32_t SaveParamsToIni(QSettings &orc_IniFile,
                                  const QString &orc_Section);
  virtual int32_t LoadParamsFromIni(QSettings &orc_IniFile,
                                    const QString &orc_Section);

  uint32_t GetSendId(void) const;
  void SetSendId(const uint32_t ou32_SendId);
};

//----------------------------------------------------------------------------------------------------------------------
} // namespace cmon_protocol
} // namespace stw

#endif
