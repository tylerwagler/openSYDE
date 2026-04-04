//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Logging data structure (implementation)

   Logging data structure containing e.g. directory, file name and flags.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_CamProLoggingData.hpp"
#include "C_OscHashUtil.hpp"
#include "C_Uti.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core::hash_util;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor.
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamProLoggingData::C_CamProLoggingData(void) :
   c_Directory(""),
   c_FileName(""),
   q_Enabled(false),
   e_OverwriteMode(eADD_TIMESTAMP),
   e_FileFormat(eASC)
{
   this->Clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamProLoggingData::CalcHash(uint32_t & oru32_HashValue) const
{
   CalcHashMembers(oru32_HashValue, this->c_FileName, this->c_Directory, this->q_Enabled,
                   this->e_OverwriteMode, this->e_FileFormat);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reset all member variables.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamProLoggingData::Clear(void)
{
   c_Directory = "";
   c_FileName = "CANlog";
   q_Enabled = false;
   e_OverwriteMode = eADD_TIMESTAMP;
   e_FileFormat = eASC;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert enum to string.

   \param[in]     oe_Format        format type

   \return
   format as string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_CamProLoggingData::h_FormatEnumToString(const C_CamProLoggingData::E_Format oe_Format)
{
   QString c_Return;

   Q_UNUSED(oe_Format)
   c_Return = "asc";

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert string to enum.

   \param[in]     orc_Format        format string

   \return
   format as enum
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamProLoggingData::E_Format C_CamProLoggingData::h_FormatStringToEnum(const QString & orc_Format)
{
   E_Format e_Return;

   Q_UNUSED(orc_Format)
   e_Return = eASC;

   return e_Return;
}
