//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for info block handling of EDS/DCF files

   Class for info block handling of EDS/DCF files

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "TglFile.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_SclChecksums.hpp"
#include "C_SclIniFile.hpp"
#include <string>
#include <vector>
#include "C_OscCanOpenEdsInfoBlock.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenEdsInfoBlock::C_OscCanOpenEdsInfoBlock()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with unit [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenEdsInfoBlock::CalcHash(uint32_t & oru32_HashValue) const
{
   this->c_FileInfo.CalcHash(oru32_HashValue);
   this->c_DeviceInfo.CalcHash(oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load from file

   \param[in]      orc_File         Opened eds file to load from
   \param[in,out]  orc_LastError    Last error

   \return
   STW error codes

   \retval   Errc::success   Values read
   \retval   Errc::config    At least one value not found, for details see error message
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenEdsInfoBlock::LoadFromFile(C_SclIniFile & orc_File, std::string & orc_LastError)
{
   std::error_code c_Retval = this->c_FileInfo.LoadFromIni(orc_File, orc_LastError);

   if (!c_Retval)
   {
      c_Retval = this->c_DeviceInfo.LoadFromIni(orc_File, orc_LastError);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get granularity

   \return
   Granularity
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscCanOpenEdsInfoBlock::GetGranularity() const
{
   return this->c_DeviceInfo.GetGranularity();
}
