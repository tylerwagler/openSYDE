//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Application information block embedded in a hex file (header).

   Handling of the openSYDE "application_info" block written by ECU firmware into its hex file.

   \copyright   Copyright 2015 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCAPPLICATIONINFOBLOCK_HPP
#define C_OSCAPPLICATIONINFOBLOCK_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <string>
#include <vector>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

//constants for application information block magic pattern
const uint8_t APPLICATION_INFO_MAGIC_LENGTH_V1 = 7U;
const uint8_t APPLICATION_INFO_MAGIC_LENGTH_V2 = 8U;
extern const char_t APPLICATION_INFO_MAGIC_V1[APPLICATION_INFO_MAGIC_LENGTH_V1 + 1U]; /// +1 for \0
extern const char_t APPLICATION_INFO_MAGIC_V2[APPLICATION_INFO_MAGIC_LENGTH_V2];      /// \0 already included

/* -- Types --------------------------------------------------------------------------------------------------------- */

///Application information block
class C_OscApplicationInfoBlock
{
private:
   std::string m_GetNonTerminatedString(const char_t * opcn_Chars, const uint8_t ou8_MaxLength) const;

   int32_t m_ParsePayload(const uint8_t * const opu8_Data, const uint16_t ou16_NumBytesAvailable,
                          const uint8_t ou8_BlockVersion);

public:
   //sizes of content fields by version (1st element: V1, 2nd element: V2, 3rd element: V3)
   static const uint8_t hau8_LENGTHS_DEVICE_ID[3];
   static const uint8_t hau8_LENGTHS_DATE[3];
   static const uint8_t hau8_LENGTHS_TIME[3];
   static const uint8_t hau8_LENGTHS_PROJECT_NAME[3];
   static const uint8_t hau8_LENGTHS_PROJECT_VERSION[3];

   C_OscApplicationInfoBlock(void);

   void ClearContents(void);
   uint16_t GetMaxSizeOnECU(void) const;

   int32_t ParseFromBLOB(const uint8_t * const opu8_Data, const uint16_t ou16_NumBytesAvailable);

   char_t acn_Magic[APPLICATION_INFO_MAGIC_LENGTH_V2]; //Maximum length ...
   uint8_t u8_StructVersion;                           ///< = 1
   uint8_t u8_ContentMap;                              ///< Bit0 = contains device ID  \n
   ///< Bit1 = contains date+time \n
   ///< Bit2 = contains project name \n
   ///< Bit3 = contains project version \n
   ///< Bit4 = contains additional info
   //the lenghts of the char-arrays are set to accomodate the longest possible texts of all defined versions
   char_t acn_DeviceID[29];       ///< ASCII device ID e.g. "ESX2" (optional; recommended)
   char_t acn_Date[12];           ///< date of compilation(format: __DATE__ ANSI C macro) (optional)
   char_t acn_Time[9];            ///< time of compilation (format: __TIME__ ANSI C macro) (optional)
   char_t acn_ProjectName[33];    ///< short description of project (optional)
   char_t acn_ProjectVersion[17]; ///< version number of project; (optional)
   uint8_t u8_LenAdditionalInfo;
   char_t acn_AdditionalInfo[255]; ///< does not hurt on PC side, if we simply use the theoretical max. of 255 bytes

   std::string GetDeviceID(void) const;
   std::string GetDate(void) const;
   std::string GetTime(void) const;
   std::string GetProjectName(void) const;
   std::string GetProjectVersion(void) const;
   std::string GetAdditionalInfo(void) const;

   void AddInfoToList(stw::scl::std::vector<std::string> & orc_List) const;

   int32_t GetInfoLevel(void) const;
   std::string GetInfoLevelAsString(void) const;

   bool ContainsDeviceID(void) const;
   bool ContainsDateAndTime(void) const;
   bool ContainsProjectName(void) const;
   bool ContainsProjectVersion(void) const;
   bool ContainsAdditionalInfo(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
