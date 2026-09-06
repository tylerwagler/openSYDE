//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Hex file class

   For details cf. documentation in .h file.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#include "precomp_headers.hpp" //pre-compiled headers

#include <cstring>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscHexFile.hpp"
#include "C_SclStringCompat.hpp"
#include <string>
#include "C_SclChecksums.hpp"
#include "TglUtils.hpp"

using namespace stw::errors;
using namespace stw::hex_file;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   convert error reported by CHexFile to textual representation

   \param[in]    ou32_ErrorCode      error code returned by CHexFile

   \return
   Textual representation of error.
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHexFile::ErrorCodeToErrorText(const std::error_code & orc_Error) const
{
   std::string c_Text;

   //The category supplies the description; the context that used to be packed
   //into the error value is read back from the instance that produced it.
   if (orc_Error == stw::hex_file::HexFileErrc::record_overlay)
   {
      c_Text = PrintFormattedCompat("Error in Hexfile: Address 0x%08x used twice !",
                                    this->GetLastOverlayErrorAddress());
   }
   else if ((orc_Error == stw::hex_file::HexFileErrc::hexline_syntax) ||
            (orc_Error == stw::hex_file::HexFileErrc::hexline_checksum) ||
            (orc_Error == stw::hex_file::HexFileErrc::hexline_command))
   {
      c_Text = "Error reading hex file: " + orc_Error.message() + " " +
               PrintFormattedCompat("in line %d", this->GetLastErrorLineNumber());
   }
   else
   {
      c_Text = "Error reading hex file: " + orc_Error.message();
   }
   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get application information blocks from hex-file

   Extract all application information block information from hex-file

   \param[out]     orc_InfoBlocks            list of detected application information blocks
   \param[in]      ou32_SearchStartAddress   linear offset address to start search at
   \param[in]      oq_OnlyOneBlock           true -> read only one block then stop searching more
   \param[in]      oq_ExactAddressMatch      true -> info block must be at exactly ou32_SearchStartAddress
   \param[in]      oq_Block0Only             true -> only find "Block0" information blocks
                                                    These are the blocks with application information.
                                                    Other Blocks (1..9) can contain information about various libraries.
                                                    e.g. 1 = "BIOS".

   \return
   C_NO_ERR     everything OK (data in oat_InfoBlocks); but maybe there are 0 blocks
   C_NOACT      if oq_ExactAddressMatch: no block found at specified address
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHexFile::GetApplicationInformationBlocks(std::vector<C_OscApplicationInfoBlock> & orc_InfoBlocks,
                                                      const uint32_t ou32_SearchStartAddress,
                                                      const bool oq_OnlyOneBlock,
                                                      const bool oq_ExactAddressMatch, const bool oq_Block0Only)
{
   int32_t s32_Return;
   uint32_t u32_Address = ou32_SearchStartAddress;
   C_OscApplicationInfoBlock c_Block;
   uint16_t u16_Size = static_cast<uint16_t>(c_Block.GetMaxSizeOnECU());
   uint8_t * const pu8_Buffer = new uint8_t[u16_Size];
   uint16_t u16_Help;

   s32_Return = C_NO_ERR;
   char_t acn_Magic[APPLICATION_INFO_MAGIC_LENGTH_V2];

   orc_InfoBlocks.clear();

   while (true)
   {
      //exact match required:
      //V1 and V2 have the same beginning:
      s32_Return = this->FindPattern(u32_Address, 5, reinterpret_cast<const uint8_t *>("Lx_?z"));
      if (s32_Return != C_NO_ERR)
      {
         break; //only stop if we cannot find any more pattern; in all other cases: continue searching
      }

      //get part of dump:
      u16_Help = APPLICATION_INFO_MAGIC_LENGTH_V2;
      s32_Return = this->GetDataByAddress(u32_Address, u16_Help, reinterpret_cast<uint8_t *>(&acn_Magic[0]));
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = C_CONFIG;
         //"Block0" ?
         if ((acn_Magic[6] == '.') || (oq_Block0Only == false))
         {
            //V1 ?
            if (acn_Magic[5] == '2')
            {
               s32_Return = C_NO_ERR;
               (void)memcpy(&c_Block.acn_Magic[0], &acn_Magic[0], APPLICATION_INFO_MAGIC_LENGTH_V1);
            }
            //V2 or V3 ?
            else if ((acn_Magic[5] == 'g') && (acn_Magic[7] == '\0'))
            {
               s32_Return = C_NO_ERR;
               (void)memcpy(&c_Block.acn_Magic[0], &acn_Magic[0], APPLICATION_INFO_MAGIC_LENGTH_V2);
            }
            else
            {
               //nothing we know about ...
            }
         }
      }

      if (oq_ExactAddressMatch == true)
      {
         if ((u32_Address != ou32_SearchStartAddress) || (s32_Return != C_NO_ERR))
         {
            //we searched the start address and did not find anything !
            delete[] pu8_Buffer;
            return C_NOACT;
         }
      }

      if (s32_Return == C_NO_ERR)
      {
         //for performance reasons we try to read the maximum possible size of the struct on the ECU:
         u16_Size = static_cast<uint16_t>(c_Block.GetMaxSizeOnECU());

         //the alignment on PC is not neccessarily the same as on the ECU
         //-> we have to use a temporary buffer to copy the data over ...
         (void)memset(pu8_Buffer, 0, u16_Size);
         s32_Return = this->GetDataByAddress(u32_Address, u16_Size, pu8_Buffer);
         switch (s32_Return)
         {
         case 0:
            break; //great ...
         case -2:  //data read but not fully
            s32_Return = 0;
            if (u16_Size < 2U) //we need at least 2 bytes for the header information
            {
               continue;
            }
            break;
         case -1:
         default:
            s32_Return = 0;
            continue; //nothing we can handle or undefined error -> continue
         }

         s32_Return = c_Block.ParseFromBLOB(pu8_Buffer, u16_Size);
         if (s32_Return == C_NO_ERR)
         {
            //we have all the data !
            //-> add to array
            orc_InfoBlocks.emplace_back(c_Block);
            if (oq_OnlyOneBlock == true)
            {
               break;
            }
         }
      }
      u32_Address += APPLICATION_INFO_MAGIC_LENGTH_V1; //done with this block ...
   }
   delete[] pu8_Buffer;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   calculate checksum over all data in the hexfile

   Calculate a 32bit checksum over all the bytes in the opened file.
   The checksum is calculated over address offsets and all bytes.
   So the sequence of hex-lines in the physical file will not affect the resulting checksum.
   If you need to check for a modified file, an MD5 checksum will be more helpful.
   The used algorithm is a CRC32 with a fixed start value.
   For a correct checksum we need to count in the address offsets and data bytes.
   We want the checksum to match for identical data. So we have to make sure to
   have a fixed concept of when to count in the address offset. Doing so at the
   beginning of a hex-line will not work, as then the checksum will change for a
   file with the same content but different record-lengths.
   So wo use the binary-block dump of the data and count in the address offset
    at the beginning of each block.

   \param[out]    oru32_Checksum   calculated checksum

   \return
   C_NO_ERR      checksum calculated
   C_CONFIG      error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHexFile::CalcFileChecksum(uint32_t & oru32_Checksum)
{
   std::error_code c_Return;
   const C_HexDataDump * pc_Dump;

   pc_Dump = this->GetDataDump(c_Return);
   if (c_Return)
   {
      return C_CONFIG;
   }

   oru32_Checksum = static_cast<uint32_t>(~0x56489437U); //fixed start value !
   for (uint32_t u32_Index = 0U; u32_Index < static_cast<uint32_t>(pc_Dump->at_Blocks.size()); u32_Index++)
   {
      //address (serialize to make the code endian-safe):
      const uint32_t u32_AddressOffset = pc_Dump->at_Blocks[u32_Index].u32_AddressOffset;
      const uint8_t au8_AddressOffset[4] =
      {
         static_cast<uint8_t>(u32_AddressOffset),
         static_cast<uint8_t>(u32_AddressOffset >> 8U),
         static_cast<uint8_t>(u32_AddressOffset >> 16U),
         static_cast<uint8_t>(u32_AddressOffset >> 24U),
      };

      //address:
      C_SclChecksums::CalcCRC32(&au8_AddressOffset[0], 4U, oru32_Checksum);

      //data:
      C_SclChecksums::CalcCRC32(&pc_Dump->at_Blocks[u32_Index].au8_Data[0],
                                pc_Dump->at_Blocks[u32_Index].au8_Data.size(), oru32_Checksum);
   }

   oru32_Checksum = ~oru32_Checksum;

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get address of signature block from hex file

   Identify address of signature block in file.
   The address of the first found block is returned.

   Possible approach when we have the flash packages: put the address into the package information.
    The signature generator has this information anyway.

   \param[out]     oru32_Address             address of signature block

   \return
   C_NO_ERR     everything OK; block found
   C_NOACT      no block found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHexFile::GetSignatureBlockAddress(uint32_t & oru32_Address)
{
   int32_t s32_Return;

   oru32_Address = this->mu32_MinAdr;

   //lint -e{926}
   s32_Return = this->FindPattern(oru32_Address, 10, reinterpret_cast<const uint8_t *>(";zwm2KgUZ!"));
   if (s32_Return == -1)
   {
      s32_Return = C_NOACT;
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   extract device ID from hex-file

   Scan through the hex-file and try to find the "application_info" structure.
   Then extract the device ID from it and return it.
   If the application_info structure with the device ID information is in the hex file more than once, AND
    the device ID in at least one of the copies is different, we fail.

   \param[out]    orc_DeviceId                     device ID found in hex-file

   \return
   C_NO_ERR     everything OK (device ID in orc_DeviceId)
   C_NOACT      device-ID not found
   C_CONFIG     ambiguous device-IDs in hex-file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHexFile::ScanDeviceIdFromHexFile(std::string & orc_DeviceId)
{
   int32_t s32_Return;
   std::string c_DeviceId = "";

   std::vector<C_OscApplicationInfoBlock> c_InfoBlocks;
   s32_Return = this->GetApplicationInformationBlocks(c_InfoBlocks, 0x0U, false, false, true);
   if ((s32_Return == C_NO_ERR) && (c_InfoBlocks.size() > 0))
   {
      int32_t s32_Index;
      for (s32_Index = 0; s32_Index < c_InfoBlocks.size(); s32_Index++)
      {
         if (c_InfoBlocks[s32_Index].ContainsDeviceID() == true)
         {
            const std::string c_Help = c_InfoBlocks[s32_Index].GetDeviceID();
            if (c_DeviceId == "") //no device-ID seen yet ...
            {
               c_DeviceId = c_Help;
            }
            else
            {
               if (c_DeviceId != c_Help)
               {
                  s32_Return = C_CONFIG; //ambiguous device IDs !
                  break;
               }
            }
         }
      }
   }
   if (s32_Return == C_NO_ERR)
   {
      if (c_DeviceId != "")
      {
         orc_DeviceId = c_DeviceId;
      }
      else
      {
         s32_Return = C_NOACT;
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   extract application information block from hex-file

   Scan through the hex-file and try to find the "application_info" structure.
   Then extract the information from it and return it.
   Multiple instances of the application_info structure are considered an error
   or warning depending on the device names reported in all application blocks.

   \param[out]    orc_InfoBlock        application info block found in hex file

   \return
   C_NO_ERR     everything OK
   C_WARN       multiple application blocks detected in hex file but device names match
                  output in this case is the first found application block
   C_NOACT      no application information block detected in hex file
   C_OVERFLOW   multiple application information blocks detected in hex file and device names differ
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHexFile::ScanApplicationInformationBlockFromHexFile(C_OscApplicationInfoBlock & orc_InfoBlock)
{
   int32_t s32_Return;

   std::vector<C_OscApplicationInfoBlock> c_InfoBlocks;
   s32_Return = this->GetApplicationInformationBlocks(c_InfoBlocks, 0x0U, false, false, true);
   tgl_assert(s32_Return == C_NO_ERR); //no plausible reasons documented

   if (c_InfoBlocks.size() == 0)
   {
      s32_Return = C_NOACT;
   }
   else if (c_InfoBlocks.size() > 1)
   {
      for (int32_t s32_Pos = 1; s32_Pos < c_InfoBlocks.size(); s32_Pos++)
      {
         // compare every device name with first device name, this is enough because all must be equal
         if (c_InfoBlocks[0].GetDeviceID() != c_InfoBlocks[s32_Pos].GetDeviceID())
         {
            s32_Return = C_OVERFLOW;
         }
         else
         {
            orc_InfoBlock = c_InfoBlocks[0];
            s32_Return = C_WARN;
         }
      }
   }
   else
   {
      //one block found !
      orc_InfoBlock = c_InfoBlocks[0];
      s32_Return = C_NO_ERR;
   }

   return s32_Return;
}
