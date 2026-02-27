//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       header for class C_XFLHexFile

   Comfort layer on top of the hex file class

   \copyright   Copyright 2002 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CXFLHEXFILEHPP
#define CXFLHEXFILEHPP

#include "CXFLECUInformation.hpp"
#include "C_HexFile.hpp"
#include "DiagLib_config.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>

//----------------------------------------------------------------------------------------------------------------------

namespace stw {
namespace diag_lib {
//----------------------------------------------------------------------------------------------------------------------
/// Comfort layer on top of the hex file class
class C_XFLHexFile : public stw::hex_file::C_HexFile {
public:
  QString ErrorCodeToErrorText(const uint32_t ou32_ErrorCode) const;

  // STW specific: find magic block(s) and return all information contained
  // there:
  int32_t GetECUInformationBlocks(QList<C_XFLECUInformation> &orc_InfoBlocks,
                                  const uint32_t ou32_SearchStartAddress,
                                  const bool oq_OnlyOneBlock,
                                  const bool oq_ExactAddressMatch,
                                  const bool oq_Block0Only);
  int32_t CalcFileChecksum(uint32_t &oru32_Checksum);
};

//----------------------------------------------------------------------------------------------------------------------
} // namespace diag_lib
} // namespace stw

#endif
