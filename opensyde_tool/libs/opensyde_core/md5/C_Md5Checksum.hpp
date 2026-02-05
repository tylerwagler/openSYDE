//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief        MD5 hashing class

   ANSI C++ MD5 hashing class.

   Provides functions to calculate an MD5 over a BLOB of data or over a file.
   Technically not a "checksum". Class name kept for compatibility.

   This implementation uses Qt's QCryptographicHash internally.

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CMD5CHECKSUMHPP
#define CMD5CHECKSUMHPP

#include <cstdio>
#include <QString>
#include "stwtypes.hpp"

namespace stw
{
namespace md5
{
class C_Md5Checksum
{
public:
   static QString GetMD5(const uint8_t * const opu8_Data, const uint32_t ou32_Length);
   static QString GetMD5(std::FILE * const opc_File);
   static QString GetMD5(const QString & orc_FilePath);
};
}
}

#endif
