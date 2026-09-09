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

#include <system_error>

#include <cstdint>
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
   std::string ErrorCodeToErrorText(const std::error_code & orc_Error) const;

   //STW specific: find application information block(s) and return all information contained there:
   //These report the STW error category. Note that the inherited C_HexFile members report the separate
   //hex_file category instead - the two are not interchangeable.
   std::error_code GetApplicationInformationBlocks(std::vector<C_OscApplicationInfoBlock> & orc_InfoBlocks,
                                                   const uint32_t ou32_SearchStartAddress,
                                                   const bool oq_OnlyOneBlock,
                                                   const bool oq_ExactAddressMatch, const bool oq_Block0Only);
   std::error_code CalcFileChecksum(uint32_t & oru32_Checksum);

   std::error_code GetSignatureBlockAddress(uint32_t & oru32_Address);
   std::error_code ScanDeviceIdFromHexFile(std::string & orc_DeviceId);
   std::error_code ScanApplicationInformationBlockFromHexFile(C_OscApplicationInfoBlock & orc_InfoBlock);
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
