//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for file info block handling of EDS/DCF files

   Class for file info block handling of EDS/DCF files

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscCanOpenEdsDeviceInfoBlock.hpp"
#include "C_OscCanOpenEdsFileInfoBlock.hpp"
#include "C_OscHashUtil.hpp"
#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenEdsFileInfoBlock::C_OscCanOpenEdsFileInfoBlock()
    : u8_FileVersion(0), u8_FileRevision(0) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with unit [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenEdsFileInfoBlock::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_FileName, this->u8_FileVersion, this->u8_FileRevision,
                              this->c_EdsVersion, this->c_Description,
                              this->c_CreationTime, this->c_CreationDate, this->c_CreatedBy,
                              this->c_ModificationTime, this->c_ModificationDate, this->c_ModifiedBy);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load from ini

   \param[in,out]  orc_File         File
   \param[in,out]  orc_LastError    Last error

   \return
   STW error codes

   \retval   C_NO_ERR   Values read
   \retval   C_CONFIG   At least one value not found, for details see error
   message
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenEdsFileInfoBlock::LoadFromIni(QSettings &orc_File,
                                                  QString &orc_LastError) {
  // lint -e{8062} Kept for later error reporting
  const int32_t s32_Retval = C_NO_ERR;
  const QString c_SectionName = "FileInfo";

  orc_LastError = "";

  if (orc_File.childGroups().contains(c_SectionName)) {
    const QString c_Group = c_SectionName + "/";
    // Maybe mandatory values
    this->c_FileName = orc_File.value(c_Group + "FileName", "").toString();
    this->u8_FileVersion = static_cast<uint8_t>(
        orc_File.value(c_Group + "FileVersion", 0).toUInt());
    this->u8_FileRevision = static_cast<uint8_t>(
        orc_File.value(c_Group + "FileRevision", 0).toUInt());
    this->c_Description =
        orc_File.value(c_Group + "Description", "").toString();
    this->c_CreationTime =
        orc_File.value(c_Group + "CreationTime", "").toString();
    this->c_CreationDate =
        orc_File.value(c_Group + "CreationDate", "").toString();
    this->c_CreatedBy = orc_File.value(c_Group + "CreatedBy", "").toString();
    // Optional values
    this->c_EdsVersion =
        orc_File.value(c_Group + "EDSVersion", "3.0").toString();
    this->c_ModificationTime =
        orc_File.value(c_Group + "ModificationTime", "").toString();
    this->c_ModificationDate =
        orc_File.value(c_Group + "ModificationDate", "").toString();
    this->c_ModifiedBy = orc_File.value(c_Group + "ModifiedBy", "").toString();
  }

  return s32_Retval;
}
