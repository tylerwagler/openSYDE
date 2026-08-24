//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Hex file class

   openSYDE utility layer on top of the generic Intel HEX file parser.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHEXFILE_HPP
#define C_OSCHEXFILE_HPP

#include "stwtypes.hpp"
#include "C_HexFile.hpp"
#include "C_OscApplicationInfoBlock.hpp"
#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace opensyde_core
{
//----------------------------------------------------------------------------------------------------------------------
///Comfort layer on top of the hex file class.
class C_OscHexFile :
   public stw::hex_file::C_HexFile
{
public:
   std::string ErrorCodeToErrorText(const uint32_t ou32_ErrorCode) const;

   //STW specific: find application information block(s) and return all information contained there:
   int32_t GetApplicationInformationBlocks(std::vector<C_OscApplicationInfoBlock> & orc_InfoBlocks,
                                           const uint32_t ou32_SearchStartAddress, const bool oq_OnlyOneBlock,
                                           const bool oq_ExactAddressMatch, const bool oq_Block0Only);
   int32_t CalcFileChecksum(uint32_t & oru32_Checksum);

   int32_t GetSignatureBlockAddress(uint32_t & oru32_Address);
   int32_t ScanDeviceIdFromHexFile(std::string & orc_DeviceId);
   int32_t ScanApplicationInformationBlockFromHexFile(C_OscApplicationInfoBlock & orc_InfoBlock);
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
