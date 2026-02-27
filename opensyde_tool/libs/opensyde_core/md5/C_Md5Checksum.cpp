//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       MD5 hashing class

   ANSI C++ MD5 hashing class using Qt's QCryptographicHash.
   For details see documentation in .hpp file.

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_Md5Checksum.hpp"
#include <QCryptographicHash>
#include <QFile>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::md5;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Get MD5 over data

   \param[in]  opu8_Data    data to calculate MD5 for
   \param[in]  ou32_Length  number of bytes referenced by opu8_Data

   \return
   Calculated MD5 as lowercase hex string (empty string if there are problems)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_Md5Checksum::GetMD5(const uint8_t *const opu8_Data,
                              const uint32_t ou32_Length) {
  if (opu8_Data == NULL) {
    return "";
  }

  const QByteArray c_Data =
      QByteArray::fromRawData(reinterpret_cast<const char *>(opu8_Data),
                              static_cast<qsizetype>(ou32_Length));
  const QByteArray c_Hash =
      QCryptographicHash::hash(c_Data, QCryptographicHash::Md5);

  return QString::fromLatin1(c_Hash.toHex());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Get MD5 of file.

   \param[in]  opc_File   C style file handle of file

   \return
   Calculated MD5 as lowercase hex string (empty string if there are problems)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_Md5Checksum::GetMD5(std::FILE *const opc_File) {
  if (opc_File == NULL) {
    return "";
  }

  QCryptographicHash c_Hash(QCryptographicHash::Md5);
  const uint32_t u32_BufferSize = 4096;
  char acn_Buffer[u32_BufferSize];

  // Read file in chunks and feed to hash
  while (true) {
    const size_t uq_BytesRead =
        std::fread(acn_Buffer, 1, u32_BufferSize, opc_File);
    if (uq_BytesRead == 0) {
      break;
    }
    c_Hash.addData(acn_Buffer, static_cast<qsizetype>(uq_BytesRead));
  }

  return QString::fromLatin1(c_Hash.result().toHex());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Get MD5 of file.

   \param[in]  orc_FilePath   absolute or relative path of file

   \return
   Calculated MD5 as lowercase hex string (empty string if there are problems)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_Md5Checksum::GetMD5(const QString &orc_FilePath) {
  QFile c_File(orc_FilePath);

  if (!c_File.open(QIODevice::ReadOnly)) {
    return "";
  }

  QCryptographicHash c_Hash(QCryptographicHash::Md5);

  if (!c_Hash.addData(&c_File)) {
    return "";
  }

  return QString::fromLatin1(c_Hash.result().toHex());
}
