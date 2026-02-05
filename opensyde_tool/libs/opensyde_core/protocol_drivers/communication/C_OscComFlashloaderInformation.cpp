//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for flashloader information

   Collection of information that can be read from the flashloader, containing
   flashloader versions, flash count, serial number, article number, fingerprint
   data, etc.

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
// #include <QString>
#include "C_OscComFlashloaderInformation.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
// using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscComFlashloaderInformation::C_OscComFlashloaderInformation(void)
    : u32_FlashCount(0), u32_EcuArticleNumber(0),
      u16_MaxNumberOfBlockLength(0) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the Ecu serial number dependent of the extended serial
   number flag and the manufacturer format

   \return
   Formatted serial number string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscComFlashloaderInformation::GetEcuSerialNumber(void) const {
  return this->c_SerialNumber.GetSerialNumberAsFormattedString();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the Ecu serial number description

   Formats:
   * Standard
   * Extended
   ** Manufacturer format

   \return
   String with description of the serial number format
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscComFlashloaderInformation::GetEcuSerialNumberFormatDescription(
    void) const {
  QString c_Return = "(Format: ";

  if (this->c_AvailableFeatures.q_ExtendedSerialNumberModeImplemented ==
      false) {
    c_Return += "Standard";
  } else {
    c_Return +=
        "Extended with Manufacturer Format " +
        QString::number(this->c_SerialNumber.u8_SerialNumberManufacturerFormat);
  }
  c_Return += ")";

  return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert information read from openSYDE server node to string list

   Can be used for a no-frills approach to get a textual representation of the
   information held by this class.

   \return
   Textual represenation of this class
*/
//----------------------------------------------------------------------------------------------------------------------
QStringList
C_OscComFlashloaderInformation::FlashloaderInformationToText(void) const {
  QStringList c_Text;
  QString c_Line;

  c_Text.clear();
  c_Line = QString("Flashloader software version: V%1.%2r%3")
               .arg(au8_FlashloaderSoftwareVersion[0])
               .arg(au8_FlashloaderSoftwareVersion[1], 2, 10, QChar('0'))
               .arg(au8_FlashloaderSoftwareVersion[2]);
  c_Text.append(c_Line);

  c_Line = QString("Flashloader protocol version: V%1.%2r%3")
               .arg(au8_FlashloaderProtocolVersion[0])
               .arg(au8_FlashloaderProtocolVersion[1], 2, 10, QChar('0'))
               .arg(au8_FlashloaderProtocolVersion[2]);
  c_Text.append(c_Line);

  c_Line = QString("Protocol version: V%1.%2r%3")
               .arg(au8_ProtocolVersion[0])
               .arg(au8_ProtocolVersion[1], 2, 10, QChar('0'))
               .arg(au8_ProtocolVersion[2]);
  c_Text.append(c_Line);

  c_Text.append("Flash count: " + QString::number(u32_FlashCount));
  c_Line = "Device serial number: " + GetEcuSerialNumber() + " " +
           GetEcuSerialNumberFormatDescription();
  c_Text.append(c_Line);
  c_Text.append("Device article number: " +
                QString::number(u32_EcuArticleNumber));
  c_Text.append("Device article version: " + c_EcuHardwareVersionNumber);

  c_Line = QString("Flash fingerprint date: %1-%2-%3 (yy-mm-dd)")
               .arg(au8_FlashFingerprintDate[0], 2, 10, QChar('0'))
               .arg(au8_FlashFingerprintDate[1], 2, 10, QChar('0'))
               .arg(au8_FlashFingerprintDate[2], 2, 10, QChar('0'));
  c_Text.append(c_Line);

  c_Line = QString("Flash fingerprint time: %1:%2:%3")
               .arg(au8_FlashFingerprintTime[0], 2, 10, QChar('0'))
               .arg(au8_FlashFingerprintTime[1], 2, 10, QChar('0'))
               .arg(au8_FlashFingerprintTime[2], 2, 10, QChar('0'));
  c_Text.append(c_Line);

  c_Text.append("Flash fingerprint username: " + c_FlashFingerprintUserName);

  c_Line =
      QString("NVM writing available: %1")
          .arg((c_AvailableFeatures.q_FlashloaderCanWriteToNvm == true) ? 1
                                                                        : 0);
  c_Text.append(c_Line);

  c_Line = QString("Security supported: %1")
               .arg((c_AvailableFeatures.q_SupportsSecurity == true) ? 1 : 0);
  c_Text.append(c_Line);

  c_Line =
      QString("Disabling debugger supported: %1")
          .arg((c_AvailableFeatures.q_SupportsDebuggerOff == true) ? 1 : 0);
  c_Text.append(c_Line);

  c_Line = QString("Enabling debugger supported: %1")
               .arg((c_AvailableFeatures.q_SupportsDebuggerOn == true) ? 1 : 0);
  c_Text.append(c_Line);

  c_Line =
      QString("Maximum block size information available: %1")
          .arg((c_AvailableFeatures.q_MaxNumberOfBlockLengthAvailable == true)
                   ? 1
                   : 0);
  c_Text.append(c_Line);

  if (c_AvailableFeatures.q_MaxNumberOfBlockLengthAvailable == true) {
    c_Line = QString("Maximum block size: %1").arg(u16_MaxNumberOfBlockLength);
    c_Text.append(c_Line);
  }

  c_Line = QString("Ethernet2Ethernet routing supported: %1")
               .arg((c_AvailableFeatures.q_EthernetToEthernetRoutingSupported ==
                     true)
                        ? 1
                        : 0);
  c_Text.append(c_Line);

  c_Line =
      QString("FileBasedTransferExitResult available: %1")
          .arg((c_AvailableFeatures.q_FileBasedTransferExitResultAvailable ==
                true)
                   ? 1
                   : 0);
  c_Text.append(c_Line);

  return c_Text;
}
