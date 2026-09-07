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
#include "C_OscErrorCategory.hpp"
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
   Errc::success   everything OK (data in orc_InfoBlocks); but maybe there are 0 blocks
   Errc::noact     if oq_ExactAddressMatch: no block found at specified address
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHexFile::GetApplicationInformationBlocks(std::vector<C_OscApplicationInfoBlock> & orc_InfoBlocks,
                                                              const uint32_t ou32_SearchStartAddress,
                                                              const bool oq_OnlyOneBlock,
                                                              const bool oq_ExactAddressMatch,
                                                              const bool oq_Block0Only)
{
   std::error_code c_Return = Errc::success;
   uint32_t u32_Address = ou32_SearchStartAddress;
   C_OscApplicationInfoBlock c_Block;
   uint16_t u16_Size = static_cast<uint16_t>(c_Block.GetMaxSizeOnECU());
   uint8_t * const pu8_Buffer = new uint8_t[u16_Size];
   uint16_t u16_Help;

   char_t acn_Magic[APPLICATION_INFO_MAGIC_LENGTH_V2];

   orc_InfoBlocks.clear();

   while (true)
   {
      //FindPattern and GetDataByAddress are inherited from C_HexFile and report their own 0 / -1 / -2
      //convention - neither the STW codes nor the hex_file category - so they get a plain local.
      int32_t s32_HexResult;

      //exact match required:
      //V1 and V2 have the same beginning:
      s32_HexResult = this->FindPattern(u32_Address, 5, reinterpret_cast<const uint8_t *>("Lx_?z"));
      if (s32_HexResult != 0)
      {
         break; //only stop if we cannot find any more pattern; in all other cases: continue searching
      }

      //get part of dump:
      u16_Help = APPLICATION_INFO_MAGIC_LENGTH_V2;
      s32_HexResult = this->GetDataByAddress(u32_Address, u16_Help, reinterpret_cast<uint8_t *>(&acn_Magic[0]));
      c_Return = Errc::config; //until a magic we know about is recognised
      if (s32_HexResult == 0)
      {
         //"Block0" ?
         if ((acn_Magic[6] == '.') || (oq_Block0Only == false))
         {
            //V1 ?
            if (acn_Magic[5] == '2')
            {
               c_Return = Errc::success;
               (void)memcpy(&c_Block.acn_Magic[0], &acn_Magic[0], APPLICATION_INFO_MAGIC_LENGTH_V1);
            }
            //V2 or V3 ?
            else if ((acn_Magic[5] == 'g') && (acn_Magic[7] == '\0'))
            {
               c_Return = Errc::success;
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
         if ((u32_Address != ou32_SearchStartAddress) || c_Return)
         {
            //we searched the start address and did not find anything !
            delete[] pu8_Buffer;
            return make_error_code(Errc::noact);
         }
      }

      if (!c_Return)
      {
         //for performance reasons we try to read the maximum possible size of the struct on the ECU:
         u16_Size = static_cast<uint16_t>(c_Block.GetMaxSizeOnECU());

         //the alignment on PC is not neccessarily the same as on the ECU
         //-> we have to use a temporary buffer to copy the data over ...
         (void)memset(pu8_Buffer, 0, u16_Size);
         s32_HexResult = this->GetDataByAddress(u32_Address, u16_Size, pu8_Buffer);
         switch (s32_HexResult)
         {
         case 0:
            break; //great ...
         case -2:  //data read but not fully
            if (u16_Size < 2U) //we need at least 2 bytes for the header information
            {
               //skip past this pattern before retrying, or FindPattern below matches the same
               //address again and the loop never terminates
               u32_Address += APPLICATION_INFO_MAGIC_LENGTH_V1;
               continue;
            }
            break;
         case -1:
         default:
            //as above: advance, or this is an infinite loop
            u32_Address += APPLICATION_INFO_MAGIC_LENGTH_V1;
            continue; //nothing we can handle or undefined error -> continue
         }

         c_Return = c_Block.ParseFromBLOB(pu8_Buffer, u16_Size);
         if (!c_Return)
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
   return make_error_code(Errc::success);
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
   Errc::success   checksum calculated
   Errc::config    error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHexFile::CalcFileChecksum(uint32_t & oru32_Checksum)
{
   //GetDataDump reports the hex_file category, which is a different set of codes to the STW one this
   //function returns - hence the separate local.
   std::error_code c_DumpError = Errc::success;
   const C_HexDataDump * pc_Dump;

   pc_Dump = this->GetDataDump(c_DumpError);
   if (c_DumpError)
   {
      return make_error_code(Errc::config);
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

   return make_error_code(Errc::success);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   get address of signature block from hex file

   Identify address of signature block in file.
   The address of the first found block is returned.

   Possible approach when we have the flash packages: put the address into the package information.
    The signature generator has this information anyway.

   \param[out]     oru32_Address             address of signature block

   \return
   Errc::success   everything OK; block found
   Errc::noact     no block found
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHexFile::GetSignatureBlockAddress(uint32_t & oru32_Address)
{
   std::error_code c_Return = Errc::success;

   oru32_Address = this->mu32_MinAdr;

   //FindPattern is inherited from C_HexFile and reports 0 / -1, not an STW code.
   //lint -e{926}
   const int32_t s32_HexResult =
      this->FindPattern(oru32_Address, 10, reinterpret_cast<const uint8_t *>(";zwm2KgUZ!"));
   if (s32_HexResult != 0)
   {
      c_Return = Errc::noact;
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   extract device ID from hex-file

   Scan through the hex-file and try to find the "application_info" structure.
   Then extract the device ID from it and return it.
   If the application_info structure with the device ID information is in the hex file more than once, AND
    the device ID in at least one of the copies is different, we fail.

   \param[out]    orc_DeviceId                     device ID found in hex-file

   \return
   Errc::success   everything OK (device ID in orc_DeviceId)
   Errc::noact     device-ID not found
   Errc::config    ambiguous device-IDs in hex-file
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHexFile::ScanDeviceIdFromHexFile(std::string & orc_DeviceId)
{
   std::error_code c_Return = Errc::success;
   std::string c_DeviceId = "";

   std::vector<C_OscApplicationInfoBlock> c_InfoBlocks;
   c_Return = this->GetApplicationInformationBlocks(c_InfoBlocks, 0x0U, false, false, true);
   if ((!c_Return) && (c_InfoBlocks.size() > 0))
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
                  c_Return = Errc::config; //ambiguous device IDs !
                  break;
               }
            }
         }
      }
   }
   if (!c_Return)
   {
      if (c_DeviceId != "")
      {
         orc_DeviceId = c_DeviceId;
      }
      else
      {
         c_Return = Errc::noact;
      }
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   extract application information block from hex-file

   Scan through the hex-file and try to find the "application_info" structure.
   Then extract the information from it and return it.
   Multiple instances of the application_info structure are considered an error
   or warning depending on the device names reported in all application blocks.

   \param[out]    orc_InfoBlock        application info block found in hex file

   \return
   Errc::success    everything OK
   Errc::warn       multiple application blocks detected in hex file but device names match
                      output in this case is the first found application block
   Errc::noact      no application information block detected in hex file
   Errc::overflow   multiple application information blocks detected in hex file and device names differ
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHexFile::ScanApplicationInformationBlockFromHexFile(C_OscApplicationInfoBlock & orc_InfoBlock)
{
   std::error_code c_Return = Errc::success;

   std::vector<C_OscApplicationInfoBlock> c_InfoBlocks;
   c_Return = this->GetApplicationInformationBlocks(c_InfoBlocks, 0x0U, false, false, true);
   tgl_assert(!c_Return); //no plausible reasons documented

   if (c_InfoBlocks.size() == 0)
   {
      c_Return = Errc::noact;
   }
   else if (c_InfoBlocks.size() > 1)
   {
      for (uint32_t u32_Pos = 1U; u32_Pos < c_InfoBlocks.size(); u32_Pos++)
      {
         // compare every device name with first device name, this is enough because all must be equal
         if (c_InfoBlocks[0].GetDeviceID() != c_InfoBlocks[u32_Pos].GetDeviceID())
         {
            c_Return = Errc::overflow;
         }
         else
         {
            orc_InfoBlock = c_InfoBlocks[0];
            c_Return = Errc::warn;
         }
      }
   }
   else
   {
      //one block found !
      orc_InfoBlock = c_InfoBlocks[0];
      c_Return = Errc::success;
   }

   return c_Return;
}
