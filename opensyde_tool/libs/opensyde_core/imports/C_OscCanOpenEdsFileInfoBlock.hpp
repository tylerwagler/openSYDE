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
#include "stwtypes.hpp"
#include <QString>
#include <QSettings>

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

   QString c_FileName;         ///< File name
   uint8_t u8_FileVersion;                   ///< File version
   uint8_t u8_FileRevision;                  ///< File revision
   QString c_EdsVersion;       ///< EDS version, in format X.y (3 chars)
   QString c_Description;      ///< File description, max 243 characters
   QString c_CreationTime;     ///< Creation time as hh:mm(AM|PM)
   QString c_CreationDate;     ///< Creation date as mm-dd-yyyy
   QString c_CreatedBy;        ///< Name or description of file creator, max 245 characters
   QString c_ModificationTime; ///< Modification time as hh:mm(AM|PM)
   QString c_ModificationDate; ///< Modification date as mm-dd-yyyy
   QString c_ModifiedBy;       ///< Name or description of file creator, max 244 characters

   void CalcHash(uint32_t & oru32_HashValue) const;
   int32_t LoadFromIni(QSettings & orc_File, QString & orc_LastError);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
