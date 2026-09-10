//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for device info block handling of EDS/DCF files

   Class for device info block handling of EDS/DCF files

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_SclChecksums.hpp"
#include "C_OscCanOpenEdsDeviceInfoBlock.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenEdsDeviceInfoBlock::C_OscCanOpenEdsDeviceInfoBlock() :
   q_BaudRate10(false),
   q_BaudRate20(false),
   q_BaudRate50(false),
   q_BaudRate125(false),
   q_BaudRate250(false),
   q_BaudRate500(false),
   q_BaudRate800(false),
   q_BaudRate1000(false),
   q_SimpleBootUpMaster(false),
   q_SimpleBootUpSlave(false),
   u8_Granularity(0),
   q_GroupMessaging(false),
   u16_NrOfRxPdo(0),
   u16_NrOfTxPdo(0),
   q_LssSupported(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with unit [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenEdsDeviceInfoBlock::CalcHash(uint32_t & oru32_HashValue) const
{
   C_SclChecksums::CalcCRC32(this->c_VendorName.c_str(), this->c_VendorName.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_VendorNumber.c_str(), this->c_VendorNumber.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_ProductName.c_str(), this->c_ProductName.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_ProductNumber.c_str(), this->c_ProductNumber.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_RevisionNumber.c_str(), this->c_RevisionNumber.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_OrderCode.c_str(), this->c_OrderCode.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate10, sizeof(this->q_BaudRate10), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate20, sizeof(this->q_BaudRate20), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate50, sizeof(this->q_BaudRate50), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate125, sizeof(this->q_BaudRate125), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate250, sizeof(this->q_BaudRate250), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate500, sizeof(this->q_BaudRate500), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate800, sizeof(this->q_BaudRate800), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_BaudRate1000, sizeof(this->q_BaudRate1000), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_SimpleBootUpMaster, sizeof(this->q_SimpleBootUpMaster), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_SimpleBootUpSlave, sizeof(this->q_SimpleBootUpSlave), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u8_Granularity, sizeof(this->u8_Granularity), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_DynamicChannelsSupported.c_str(),
                             this->c_DynamicChannelsSupported.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_GroupMessaging, sizeof(this->q_GroupMessaging), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u16_NrOfRxPdo, sizeof(this->u16_NrOfRxPdo), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u16_NrOfTxPdo, sizeof(this->u16_NrOfTxPdo), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_LssSupported, sizeof(this->q_LssSupported), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load from ini

   \param[in,out]  orc_File         File
   \param[in,out]  orc_LastError    Last error

   \return
   STW error codes

   \retval   Errc::success   Values read
   \retval   Errc::config    At least one value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenEdsDeviceInfoBlock::LoadFromIni(stw::scl::C_SclIniFile & orc_File,
                                                            std::string & orc_LastError)
{
   //lint -e{8062} Kept for later error reporting
   const std::error_code c_Retval = Errc::success;
   const std::string c_SectionName = "DeviceInfo";

   orc_LastError = "";

   if (orc_File.SectionExists(c_SectionName))
   {
      //Maybe mandatory values
      this->c_VendorName = orc_File.ReadString(c_SectionName, "VendorName", "");
      this->c_ProductName = orc_File.ReadString(c_SectionName, "ProductName", "");
      this->q_BaudRate10 = orc_File.ReadBool(c_SectionName, "BaudRate_10", false);
      this->q_BaudRate20 = orc_File.ReadBool(c_SectionName, "BaudRate_20", false);
      this->q_BaudRate50 = orc_File.ReadBool(c_SectionName, "BaudRate_50", false);
      this->q_BaudRate125 = orc_File.ReadBool(c_SectionName, "BaudRate_125", false);
      this->q_BaudRate250 = orc_File.ReadBool(c_SectionName, "BaudRate_250", false);
      this->q_BaudRate500 = orc_File.ReadBool(c_SectionName, "BaudRate_500", false);
      this->q_BaudRate800 = orc_File.ReadBool(c_SectionName, "BaudRate_800", false);
      this->q_BaudRate1000 = orc_File.ReadBool(c_SectionName, "BaudRate_1000", false);
      this->q_SimpleBootUpMaster = orc_File.ReadBool(c_SectionName, "SimpleBootUpMaster", false);
      this->q_SimpleBootUpSlave = orc_File.ReadBool(c_SectionName, "SimpleBootUpSlave", false);
      this->u8_Granularity = orc_File.ReadUint8(c_SectionName, "Granularity", 0);
      this->c_DynamicChannelsSupported = orc_File.ReadString(c_SectionName, "DynamicChannelsSupported", "0");
      this->c_RevisionNumber = orc_File.ReadString(c_SectionName, "RevisionNumber", "");
      //Optional
      this->c_RevisionNumber = orc_File.ReadString(c_SectionName, "RevisionNumber", "");
      this->q_GroupMessaging = orc_File.ReadBool(c_SectionName, "GroupMessaging", false);
      this->u16_NrOfRxPdo = orc_File.ReadUint16(c_SectionName, "NrOfRxPDO", 0);
      this->u16_NrOfTxPdo = orc_File.ReadUint16(c_SectionName, "NrOfTxPDO", 0);
      this->q_LssSupported = orc_File.ReadBool(c_SectionName, "LSS_Supported", false);
      this->c_VendorNumber = orc_File.ReadString(c_SectionName, "VendorNumber", "");
      this->c_ProductNumber = orc_File.ReadString(c_SectionName, "ProductNumber", "");
      this->c_OrderCode = orc_File.ReadString(c_SectionName, "OrderCode", "");
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get granularity

   \return
   Granularity
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscCanOpenEdsDeviceInfoBlock::GetGranularity() const
{
   return this->u8_Granularity;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load string value from ini file

   \param[in,out]  orc_File            File
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_KeyName         Key name
   \param[in,out]  orc_OutputValue     Output value
   \param[in,out]  orc_ErrorMessage    Error message
   \param[in]      orc_DefaultValue    Default value

   \return
   STW error codes

   \retval   Errc::success   Value read
   \retval   Errc::config    Value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T, typename F_Get>
std::error_code C_OscCanOpenEdsDeviceInfoBlock::mh_LoadValueFromIniFile(stw::scl::C_SclIniFile & orc_File,
                                                                        const std::string & orc_SectionName,
                                                                        const std::string & orc_KeyName,
                                                                        T & orc_OutputValue,
                                                                        std::string & orc_ErrorMessage,
                                                                        const T & orc_DefaultValue,
                                                                        F_Get && orc_Get)
{
   std::error_code c_Retval = Errc::success;

   if (orc_File.ValueExists(orc_SectionName, orc_KeyName))
   {
      orc_OutputValue = orc_Get();
   }
   else
   {
      orc_OutputValue = orc_DefaultValue;
      c_Retval = Errc::config;
      C_OscCanOpenEdsDeviceInfoBlock::h_ReportMissingKeyError(orc_SectionName, orc_KeyName, orc_ErrorMessage);
   }
   return c_Retval;
}

std::error_code C_OscCanOpenEdsDeviceInfoBlock::h_LoadStringValueFromIniFile(stw::scl::C_SclIniFile & orc_File,
                                                                             const std::string & orc_SectionName,
                                                                             const std::string & orc_KeyName,
                                                                             std::string & orc_OutputValue,
                                                                             std::string & orc_ErrorMessage,
                                                                             const std::string & orc_DefaultValue)
{
   return mh_LoadValueFromIniFile(orc_File, orc_SectionName, orc_KeyName, orc_OutputValue, orc_ErrorMessage,
                                  orc_DefaultValue,
                                  [&]() { return orc_File.ReadString(orc_SectionName, orc_KeyName, orc_DefaultValue); });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load u8 value from ini file

   \param[in]      orc_File            File
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_KeyName         Key name
   \param[out]     oru8_OutputValue    Output value
   \param[in,out]  orc_ErrorMessage    Error message
   \param[in]      ou8_DefaultValue    Default value

   \return
   STW error codes

   \retval   Errc::success   Value read
   \retval   Errc::config    Value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenEdsDeviceInfoBlock::h_LoadU8ValueFromIniFile(stw::scl::C_SclIniFile & orc_File,
                                                                         const std::string & orc_SectionName,
                                                                         const std::string & orc_KeyName,
                                                                         uint8_t & oru8_OutputValue,
                                                                         std::string & orc_ErrorMessage,
                                                                         const uint8_t ou8_DefaultValue)
{
   return mh_LoadValueFromIniFile(orc_File, orc_SectionName, orc_KeyName, oru8_OutputValue, orc_ErrorMessage,
                                  ou8_DefaultValue,
                                  [&]() { return orc_File.ReadUint8(orc_SectionName, orc_KeyName, ou8_DefaultValue); });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load u16 value from ini file

   \param[in,out]  orc_File            File
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_KeyName         Key name
   \param[out]     oru16_OutputValue   Output value
   \param[in,out]  orc_ErrorMessage    Error message
   \param[in]      ou16_DefaultValue   Default value

   \return
   STW error codes

   \retval   Errc::success   Value read
   \retval   Errc::config    Value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenEdsDeviceInfoBlock::h_LoadU16ValueFromIniFile(stw::scl::C_SclIniFile & orc_File,
                                                                          const std::string & orc_SectionName,
                                                                          const std::string & orc_KeyName,
                                                                          uint16_t & oru16_OutputValue,
                                                                          std::string & orc_ErrorMessage,
                                                                          const uint16_t ou16_DefaultValue)
{
   return mh_LoadValueFromIniFile(orc_File, orc_SectionName, orc_KeyName, oru16_OutputValue, orc_ErrorMessage,
                                  ou16_DefaultValue,
                                  [&]() { return orc_File.ReadUint16(orc_SectionName, orc_KeyName, ou16_DefaultValue); });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load bool value from ini file

   \param[in,out]  orc_File            File
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_KeyName         Key name
   \param[out]     orq_OutputValue     Output value
   \param[in,out]  orc_ErrorMessage    Error message
   \param[in]      oq_DefaultValue     Default value

   \return
   STW error codes

   \retval   Errc::success   Value read
   \retval   Errc::config    Value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenEdsDeviceInfoBlock::h_LoadBoolValueFromIniFile(stw::scl::C_SclIniFile & orc_File,
                                                                           const std::string & orc_SectionName,
                                                                           const std::string & orc_KeyName,
                                                                           bool & orq_OutputValue,
                                                                           std::string & orc_ErrorMessage,
                                                                           const bool oq_DefaultValue)
{
   return mh_LoadValueFromIniFile(orc_File, orc_SectionName, orc_KeyName, orq_OutputValue, orc_ErrorMessage,
                                  oq_DefaultValue,
                                  [&]() { return orc_File.ReadBool(orc_SectionName, orc_KeyName, oq_DefaultValue); });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report missing section error

   \param[in]      orc_SectionName     Section name
   \param[in,out]  orc_ErrorMessage    Error message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenEdsDeviceInfoBlock::h_ReportMissingSectionError(const std::string & orc_SectionName,
                                                                 std::string & orc_ErrorMessage)
{
   orc_ErrorMessage = "Error: Could not find section \"" + orc_SectionName + "\".";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report missing key error

   \param[in]      orc_SectionName     Section name
   \param[in]      orc_KeyName         Key name
   \param[in,out]  orc_ErrorMessage    Error message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenEdsDeviceInfoBlock::h_ReportMissingKeyError(const std::string & orc_SectionName,
                                                             const std::string & orc_KeyName,
                                                             std::string & orc_ErrorMessage)
{
   orc_ErrorMessage = "Error: Could not find key \"" + orc_KeyName + "\" in section \"" + orc_SectionName + "\".";
}
