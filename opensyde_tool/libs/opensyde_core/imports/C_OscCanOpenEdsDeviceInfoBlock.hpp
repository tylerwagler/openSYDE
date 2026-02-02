//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for device info block handling of EDS/DCF files
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENEDSDEVICEINFOBLOCK_HPP
#define C_OSCCANOPENEDSDEVICEINFOBLOCK_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <QString>
#include <QSettings>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanOpenEdsDeviceInfoBlock
{
public:
   C_OscCanOpenEdsDeviceInfoBlock();

   QString c_VendorName;     ///< Vendor name, max 244 characters
   QString c_VendorNumber;   ///< Vendor number, also see object dictionary index 1018, sub-index 1
   QString c_ProductName;    ///< Product name, max 243 characters
   QString c_ProductNumber;  ///< Product number, also see object dictionary index 1018, sub-index 2
   QString c_RevisionNumber; ///< Revision number, also see object dictionary index 1018, sub-index 3
   QString c_OrderCode;      ///< Product order code, max 245 characters
   bool q_BaudRate10;                      ///< Support of baud rate 10 kbit/s
   bool q_BaudRate20;                      ///< Support of baud rate 20 kbit/s
   bool q_BaudRate50;                      ///< Support of baud rate 50 kbit/s
   bool q_BaudRate125;                     ///< Support of baud rate 125 kbit/s
   bool q_BaudRate250;                     ///< Support of baud rate 250 kbit/s
   bool q_BaudRate500;                     ///< Support of baud rate 500 kbit/s
   bool q_BaudRate800;                     ///< Support of baud rate 800 kbit/s
   bool q_BaudRate1000;                    ///< Support of baud rate 1000 kbit/s
   bool q_SimpleBootUpMaster;              ///< Simple boot up master functionality
   bool q_SimpleBootUpSlave;               ///< Simple boot up slave functionality
   uint8_t u8_Granularity;                 ///< Minimum size of a mappable object in bits, allowed for the PDO mapping
                                           // on
   ///< this CANopen device
   QString c_DynamicChannelsSupported; ///< The value indicates the facility of dynamic variable
   ///< generation. If the value is unequal to "0", the additional section DynamicChannels exists
   bool q_GroupMessaging;  ///< The value indicates, if multiplexed PDOs configuration is supported
   uint16_t u16_NrOfRxPdo; ///< Number of supported receive PDOs
   uint16_t u16_NrOfTxPdo; ///< Number of supported transmit PDOs
   bool q_LssSupported;    ///< The value indicates information whether LSS functionality is supported.

   void CalcHash(uint32_t & oru32_HashValue) const;
   int32_t LoadFromIni(QSettings & orc_File, QString & orc_LastError);

   uint8_t GetGranularity(void) const;

   static int32_t h_LoadStringValueFromIniFile(QSettings & orc_File,
                                               const QString & orc_SectionName,
                                               const QString & orc_KeyName,
                                               QString & orc_OutputValue,
                                               QString & orc_ErrorMessage,
                                               const QString & orc_DefaultValue = "");
   static int32_t h_LoadU8ValueFromIniFile(QSettings & orc_File,
                                           const QString & orc_SectionName,
                                           const QString & orc_KeyName, uint8_t & oru8_OutputValue,
                                           QString & orc_ErrorMessage,
                                           const uint8_t ou8_DefaultValue = 0);
   static int32_t h_LoadU16ValueFromIniFile(QSettings & orc_File,
                                            const QString & orc_SectionName,
                                            const QString & orc_KeyName, uint16_t & oru16_OutputValue,
                                            QString & orc_ErrorMessage,
                                            const uint16_t ou16_DefaultValue = 0);
   static int32_t h_LoadBoolValueFromIniFile(QSettings & orc_File,
                                             const QString & orc_SectionName,
                                             const QString & orc_KeyName, bool & orq_OutputValue,
                                             QString & orc_ErrorMessage,
                                             const bool oq_DefaultValue = false);
   static void h_ReportMissingSectionError(const QString & orc_SectionName,
                                           QString & orc_ErrorMessage);
   static void h_ReportMissingKeyError(const QString & orc_SectionName,
                                       const QString & orc_KeyName,
                                       QString & orc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
