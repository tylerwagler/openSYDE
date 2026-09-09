//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for file info block handling of EDS/DCF files
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENEDSFILEINFOBLOCK_HPP
#define C_OSCCANOPENEDSFILEINFOBLOCK_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include <cstdint>
#include <string>
#include "C_SclDateTime.hpp"
#include "C_SclIniFile.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanOpenEdsFileInfoBlock
{
public:
   C_OscCanOpenEdsFileInfoBlock();

   std::string c_FileName;         ///< File name
   uint8_t u8_FileVersion;                   ///< File version
   uint8_t u8_FileRevision;                  ///< File revision
   std::string c_EdsVersion;       ///< EDS version, in format X.y (3 chars)
   std::string c_Description;      ///< File description, max 243 characters
   std::string c_CreationTime;     ///< Creation time as hh:mm(AM|PM)
   std::string c_CreationDate;     ///< Creation date as mm-dd-yyyy
   std::string c_CreatedBy;        ///< Name or description of file creator, max 245 characters
   std::string c_ModificationTime; ///< Modification time as hh:mm(AM|PM)
   std::string c_ModificationDate; ///< Modification date as mm-dd-yyyy
   std::string c_ModifiedBy;       ///< Name or description of file creator, max 244 characters

   void CalcHash(uint32_t & oru32_HashValue) const;
   std::error_code LoadFromIni(stw::scl::C_SclIniFile & orc_File, std::string & orc_LastError);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
