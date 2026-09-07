//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Application information block handling.

   Provides functions to handle openSYDE application information blocks:
   - deserialize
   - extract information

   \copyright   Copyright 2015 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <cstring>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscApplicationInfoBlock.hpp"
#include "C_SclStringCompat.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::opensyde_core;

//magic for application info block in hex file
const char_t stw::opensyde_core::APPLICATION_INFO_MAGIC_V1[APPLICATION_INFO_MAGIC_LENGTH_V1 + 1U] = "Lx_?z2.";
const char_t stw::opensyde_core::APPLICATION_INFO_MAGIC_V2[APPLICATION_INFO_MAGIC_LENGTH_V2] = "Lx_?zg."; /// \0 already
                                                                                                          // included

const uint8_t C_OscApplicationInfoBlock::hau8_LENGTHS_DEVICE_ID[3]       =
{
   16U, 17U, 29U
};
const uint8_t C_OscApplicationInfoBlock::hau8_LENGTHS_DATE[3]            =
{
   11U, 12U, 12U
};
const uint8_t C_OscApplicationInfoBlock::hau8_LENGTHS_TIME[3]            =
{
   8U,  9U,  9U
};
const uint8_t C_OscApplicationInfoBlock::hau8_LENGTHS_PROJECT_NAME[3]    =
{
   15U, 16U, 33U
};
const uint8_t C_OscApplicationInfoBlock::hau8_LENGTHS_PROJECT_VERSION[3] =
{
   10U, 11U, 17U
};

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_OscApplicationInfoBlock::C_OscApplicationInfoBlock(void) :
   u8_StructVersion(0U),
   u8_ContentMap(0U),
   u8_LenAdditionalInfo(0U)
{
   this->ClearContents();
}

//----------------------------------------------------------------------------------------------------------------------

void C_OscApplicationInfoBlock::ClearContents(void)
{
   (void)std::memset(&acn_Magic[0], 0, sizeof(acn_Magic));
   (void)std::memset(&acn_DeviceID[0], 0, sizeof(acn_DeviceID));
   (void)std::memset(&acn_Date[0], 0, sizeof(acn_Date));
   (void)std::memset(&acn_Time[0], 0, sizeof(acn_Time));
   (void)std::memset(&acn_ProjectName[0], 0, sizeof(acn_ProjectName));
   (void)std::memset(&acn_ProjectVersion[0], 0, sizeof(acn_ProjectVersion));
   (void)std::memset(&acn_AdditionalInfo[0], 0, sizeof(acn_AdditionalInfo));
}

//----------------------------------------------------------------------------------------------------------------------

uint16_t C_OscApplicationInfoBlock::GetMaxSizeOnECU(void) const
{
   //the data has to be "packed" on ECU side
   uint16_t u16_Size = 0U;

   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_Magic));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(u8_StructVersion));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(u8_ContentMap));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_DeviceID));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_Date));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_Time));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_ProjectName));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_ProjectVersion));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(u8_LenAdditionalInfo));
   u16_Size = static_cast<uint16_t>(u16_Size + sizeof(acn_AdditionalInfo));
   return u16_Size;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Parse application information payload data from BLOB

   Tries to parse the payload of an application information block.
   Supports V1, V2, V3 of the block definition.

   \param[in]     opu8_Data                 data to parse, starting with the payload
                                              (first byte after version resp. content map byte)
   \param[in]     ou16_NumBytesAvailable    number of bytes available in opu8_Data

   \return
   Errc::success      no errors, information stored in class members
   else               error parsing
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscApplicationInfoBlock::m_ParsePayload(const uint8_t * const opu8_Data,
                                                          const uint16_t ou16_NumBytesAvailable,
                                                          const uint8_t ou8_BlockVersion)
{
   uint16_t u16_Index = 0U;
   uint16_t u16_Length;
   std::error_code c_Return = Errc::success;
   uint8_t u8_VersionIndex = 0U;

   switch (ou8_BlockVersion)
   {
   case 1U:
      u8_VersionIndex = 0U;
      break;
   case 2U:
      u8_VersionIndex = 1U;
      break;
   case 3U:
      u8_VersionIndex = 2U;
      break;
   default:
      c_Return = Errc::range;
      break;
   }

   if ((!c_Return) && (this->ContainsDeviceID() == true))
   {
      u16_Length = hau8_LENGTHS_DEVICE_ID[u8_VersionIndex];
      if ((ou16_NumBytesAvailable - u16_Index)  <= u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         this->acn_DeviceID[sizeof(acn_DeviceID) - 1] = '\0';
         (void)memcpy(&this->acn_DeviceID[0], &opu8_Data[u16_Index], u16_Length);
         u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }
   }

   if ((!c_Return) && (this->ContainsDateAndTime() == true))
   {
      //extract date:
      u16_Length = hau8_LENGTHS_DATE[u8_VersionIndex];
      if ((ou16_NumBytesAvailable - u16_Index)  <= u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         this->acn_Date[sizeof(acn_Date) - 1] = '\0';
         (void)memcpy(&this->acn_Date[0], &opu8_Data[u16_Index], u16_Length);
         u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }

      //extract time:
      u16_Length = hau8_LENGTHS_TIME[u8_VersionIndex];
      if ((ou16_NumBytesAvailable - u16_Index)  <= u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         this->acn_Time[sizeof(acn_Time) - 1] = '\0';
         (void)memcpy(&this->acn_Time[0], &opu8_Data[u16_Index], u16_Length);
         u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }
   }

   if ((!c_Return) && (this->ContainsProjectName() == true))
   {
      u16_Length = hau8_LENGTHS_PROJECT_NAME[u8_VersionIndex];
      if ((ou16_NumBytesAvailable - u16_Index)  <= u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         this->acn_ProjectName[sizeof(acn_ProjectName) - 1] = '\0';
         (void)memcpy(&this->acn_ProjectName[0], &opu8_Data[u16_Index], u16_Length);
         u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }
   }

   if ((!c_Return) && (this->ContainsProjectName() == true))
   {
      if (this->ContainsProjectVersion() == true)
      {
         u16_Length = hau8_LENGTHS_PROJECT_VERSION[u8_VersionIndex];
         if ((ou16_NumBytesAvailable - u16_Index)  <= u16_Length)
         {
            c_Return = Errc::overflow;
         }
         else
         {
            this->acn_ProjectVersion[sizeof(acn_ProjectVersion) - 1] = '\0';
            (void)memcpy(&this->acn_ProjectVersion[0], &opu8_Data[u16_Index], u16_Length);
            u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
         }
      }
   }

   if ((!c_Return) && (this->ContainsAdditionalInfo() == true))
   {
      u16_Length = 1;
      if ((ou16_NumBytesAvailable - u16_Index) <= u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         this->u8_LenAdditionalInfo = opu8_Data[u16_Index];
         u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }

      u16_Length = this->u8_LenAdditionalInfo;
      if ((ou16_NumBytesAvailable - u16_Index)  < u16_Length)
      {
         c_Return = Errc::overflow;
      }
      else
      {
         (void)memcpy(&this->acn_AdditionalInfo[0], &opu8_Data[u16_Index], this->u8_LenAdditionalInfo);
         //            u16_Index = static_cast<uint16_t>(u16_Index + u16_Length);
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Parse application information data from BLOB

   Tries to parse an application information block.
   Supports V1, V2, V3 of the block definition.

   \param[in]     opu8_Data                 data to parse, starting with magic pattern
   \param[in]     ou16_NumBytesAvailable    number of bytes available in opu8_Data (must be >= than the area
                                             occupied by the information block itseld)

   \return
   Errc::success      no errors, information stored in class members
   else               error parsing
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscApplicationInfoBlock::ParseFromBLOB(const uint8_t * const opu8_Data,
                                                         const uint16_t ou16_NumBytesAvailable)
{
   C_OscApplicationInfoBlock c_Temp;
   std::error_code c_Return = Errc::success;
   bool q_V2OrV3Detected = false;
   uint16_t u16_Length;
   uint16_t u16_Index = 0U;

   //We need at least the magic + version + content map (or at least a few bytes of valid data for V3)
   if (ou16_NumBytesAvailable < (APPLICATION_INFO_MAGIC_LENGTH_V1 + 2))
   {
      c_Return = Errc::overflow;
   }

   if (!c_Return)
   {
      //memcmp reports "differs", not a status code, so it gets its own local rather than being
      //funnelled through the error_code.
      int32_t s32_MagicDiffers;

      //don't compare block type indicator ...
      s32_MagicDiffers = memcmp(&opu8_Data[0], APPLICATION_INFO_MAGIC_V1, APPLICATION_INFO_MAGIC_LENGTH_V1 - 1);
      if (s32_MagicDiffers != 0)
      {
         //try V2/V3:
         s32_MagicDiffers = memcmp(&opu8_Data[0], APPLICATION_INFO_MAGIC_V2, APPLICATION_INFO_MAGIC_LENGTH_V2 - 2);
         if (s32_MagicDiffers == 0)
         {
            //final byte zero ?
            if (opu8_Data[APPLICATION_INFO_MAGIC_LENGTH_V2 - 1] != 0U)
            {
               s32_MagicDiffers = -1;
            }
            else
            {
               q_V2OrV3Detected = true;
            }
         }
      }
      if (s32_MagicDiffers != 0)
      {
         c_Return = Errc::config;
      }
      else
      {
         c_Temp.acn_Magic[APPLICATION_INFO_MAGIC_LENGTH_V1] = '\0';
         u16_Length =
            (static_cast<uint16_t>(APPLICATION_INFO_MAGIC_LENGTH_V1) + ((q_V2OrV3Detected == true) ? 1 : 0));
         memcpy(&c_Temp.acn_Magic[0], &opu8_Data[0], u16_Length);
         u16_Index = u16_Length;
      }
   }

   if (!c_Return)
   {
      //the length check above only covers the shortest header (V1: 7 magic bytes). A V2/V3 magic is one byte
      //longer, so re-check before every read rather than assuming the header fits.
      if (u16_Index >= ou16_NumBytesAvailable)
      {
         c_Return = Errc::overflow;
      }
   }

   if (!c_Return)
   {
      c_Temp.u8_StructVersion = opu8_Data[u16_Index];
      if ((c_Temp.u8_StructVersion != 1U) && (c_Temp.u8_StructVersion != 2U) && (c_Temp.u8_StructVersion != 3U))
      {
         c_Return = Errc::config; //nothing we can handle ...
      }

      u16_Index += 1U; //skip version

      switch (c_Temp.u8_StructVersion)
      {
      case 1: //no break
      case 2:
         if (u16_Index >= ou16_NumBytesAvailable)
         {
            c_Return = Errc::overflow; //content map byte is not there
         }
         else
         {
            c_Temp.u8_ContentMap = opu8_Data[u16_Index];
            u16_Index += 1U; //skip content map
         }
         break;
      case 3:
         c_Temp.u8_ContentMap = 0x1FU; //all data mandatory here
         break;
      default:
         c_Return = Errc::config; //nothing we can handle ...
         break;
      }
   }

   if (!c_Return)
   {
      //m_ParsePayload takes the remaining length as uint16_t, so an index past the end would wrap to a huge
      //value and defeat every bounds check inside it. The guards above are what keep that from happening.
      c_Return = c_Temp.m_ParsePayload(&opu8_Data[u16_Index],
                                       static_cast<uint16_t>(ou16_NumBytesAvailable - u16_Index),
                                       c_Temp.u8_StructVersion);
   }

   if (!c_Return)
   {
      (*this) = c_Temp;
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::m_GetNonTerminatedString(const char_t * const opcn_Chars,
                                                                const uint8_t ou8_MaxLength) const
{
   std::string c_Help;
   char_t * pcn_Text;

   pcn_Text = new char_t[ou8_MaxLength + 1];
   pcn_Text[ou8_MaxLength] = '\0';
   (void)memcpy(pcn_Text, opcn_Chars, ou8_MaxLength);
   c_Help = pcn_Text;
   delete[] pcn_Text;
   c_Help = TrimRightCompat(c_Help);
   return c_Help;
}

//----------------------------------------------------------------------------------------------------------------------

void C_OscApplicationInfoBlock::AddInfoToList(C_SclStringList & orc_List) const
{
   (void)orc_List.Add("Block type:      " + GetInfoLevelAsString());
   (void)orc_List.Add("Version:         " + std::to_string(u8_StructVersion));

   if (ContainsDeviceID() == true)
   {
      (void)orc_List.Add("DeviceID:        " + GetDeviceID());
   }
   else
   {
      (void)orc_List.Add("DeviceID:        undefined");
   }
   if (ContainsDateAndTime() == true)
   {
      (void)orc_List.Add("Date:            " + GetDate());
      (void)orc_List.Add("Time:            " + GetTime());
   }
   else
   {
      (void)orc_List.Add("Date and Time:   undefined");
   }

   if (ContainsProjectName() == true)
   {
      (void)orc_List.Add("Project name:    " + GetProjectName());
   }
   else
   {
      (void)orc_List.Add("Project name:    undefined");
   }

   if (ContainsProjectVersion() == true)
   {
      (void)orc_List.Add("Project version: " + GetProjectVersion());
   }
   else
   {
      (void)orc_List.Add("Project version: undefined");
   }

   if (ContainsAdditionalInfo() == true)
   {
      (void)orc_List.Add("Additional info: " + GetAdditionalInfo());
   }
   else
   {
      (void)orc_List.Add("Additional info: undefined");
   }
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetInfoLevelAsString(void) const
{
   int32_t s32_Return;
   std::string c_Text;

   s32_Return = GetInfoLevel();
   switch (s32_Return)
   {
   case 0:
      c_Text = "Application";
      break;
   case 1:
      c_Text = "BIOS";
      break;
   case 2:
      c_Text = "Driver Library";
      break;
   case 3:
      c_Text = "Level3";
      break;
   case 4:
      c_Text = "Level4";
      break;
   case 5:
      c_Text = "Level5";
      break;
   case 6:
      c_Text = "Level6";
      break;
   case 7:
      c_Text = "Level7";
      break;
   case 8:
      c_Text = "Level8";
      break;
   case 9:
      c_Text = "Level9";
      break;
   default:
      c_Text = "Undefined";
      break;
   }
   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------

int32_t C_OscApplicationInfoBlock::GetInfoLevel(void) const
{
   //last character decides:
   int32_t s32_Return;

   switch (acn_Magic[APPLICATION_INFO_MAGIC_LENGTH_V1 - 1U])
   {
   case '.':
      s32_Return = 0;
      break;
   case '1':
      s32_Return = 1;
      break;
   case '2':
      s32_Return = 2;
      break;
   case '3':
      s32_Return = 3;
      break;
   case '4':
      s32_Return = 4;
      break;
   case '5':
      s32_Return = 5;
      break;
   case '6':
      s32_Return = 6;
      break;
   case '7':
      s32_Return = 7;
      break;
   case '8':
      s32_Return = 8;
      break;
   case '9':
      s32_Return = 9;
      break;
   default:
      s32_Return = C_RANGE;
      break;
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscApplicationInfoBlock::ContainsDeviceID(void) const
{
   return ((u8_ContentMap & 0x01U) == 0x01U) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscApplicationInfoBlock::ContainsDateAndTime(void) const
{
   return ((u8_ContentMap & 0x02U) == 0x02U) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscApplicationInfoBlock::ContainsProjectName(void) const
{
   return ((u8_ContentMap & 0x04U) == 0x04U) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscApplicationInfoBlock::ContainsProjectVersion(void) const
{
   return ((u8_ContentMap & 0x08U) == 0x08U) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscApplicationInfoBlock::ContainsAdditionalInfo(void) const
{
   return ((u8_ContentMap & 0x10U) == 0x10U) ? true : false;
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetDeviceID(void) const
{
   return m_GetNonTerminatedString(&acn_DeviceID[0], static_cast<uint8_t>(sizeof(acn_DeviceID)));
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetDate(void) const
{
   return m_GetNonTerminatedString(&acn_Date[0], static_cast<uint8_t>(sizeof(acn_Date)));
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetTime(void) const
{
   return m_GetNonTerminatedString(&acn_Time[0], static_cast<uint8_t>(sizeof(acn_Time)));
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetProjectName(void) const
{
   return m_GetNonTerminatedString(&acn_ProjectName[0], static_cast<uint8_t>(sizeof(acn_ProjectName)));
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetProjectVersion(void) const
{
   return m_GetNonTerminatedString(&acn_ProjectVersion[0], static_cast<uint8_t>(sizeof(acn_ProjectVersion)));
}

//----------------------------------------------------------------------------------------------------------------------

std::string C_OscApplicationInfoBlock::GetAdditionalInfo(void) const
{
   return m_GetNonTerminatedString(&acn_AdditionalInfo[0], u8_LenAdditionalInfo);
}
