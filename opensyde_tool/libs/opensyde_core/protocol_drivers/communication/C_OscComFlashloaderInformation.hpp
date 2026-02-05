//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for flashloader information
   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCOMFLASHLOADERINFORMATION_H
#define C_OSCCOMFLASHLOADERINFORMATION_H

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscProtocolSerialNumber.hpp"
#include "stwtypes.hpp"
#include <QString>
#include <QStringList>


/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscComFlashloaderInformation {
public:
  C_OscComFlashloaderInformation();

  uint8_t au8_FlashloaderSoftwareVersion[3]; ///< Mmr
  uint8_t au8_FlashloaderProtocolVersion[3]; ///< Mmr
  uint8_t au8_ProtocolVersion[3];            ///< Mmr
  uint32_t u32_FlashCount; ///< how often was this device flashed already ?
  C_OscProtocolSerialNumber c_SerialNumber; ///< serial number of node
                                            // holds both variants: POS and FSN
                                            // depending of c_AvailableFeatures.
                                            // q_ExtendedSerialNumberModeImplemented
                                            // and the manufacturer format
  uint32_t u32_EcuArticleNumber; ///< article number of device
  QString c_EcuHardwareVersionNumber;
  uint8_t au8_FlashFingerprintDate[3]; ///< last date of flashing yy.mm.dd
  uint8_t au8_FlashFingerprintTime[3]; ///< last time of flashing hh.mm.ss
  QString c_FlashFingerprintUserName;

  C_OscProtocolDriverOsy::C_ListOfFeatures
      c_AvailableFeatures;             ///< Available features of flashloader
  uint16_t u16_MaxNumberOfBlockLength; ///< maximum size of service the server
                                       ///< can handle

  QString GetEcuSerialNumber(void) const;
  QString GetEcuSerialNumberFormatDescription(void) const;

  QStringList FlashloaderInformationToText() const;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
