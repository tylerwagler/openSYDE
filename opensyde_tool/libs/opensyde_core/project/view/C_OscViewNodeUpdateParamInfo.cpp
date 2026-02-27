//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update information for parameter sets (implementation)

   Node update information for parameter sets

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscViewNodeUpdateParamInfo.hpp"
#include "C_SclChecksums.hpp"
#include "stwtypes.hpp"
#include <QString>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::scl;
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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdateParamInfo::C_OscViewNodeUpdateParamInfo(void)
    : mc_FilePath(""), mu32_LastKnownCrc(0UL) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue Hash value with init [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdateParamInfo::CalcHash(uint32_t &oru32_HashValue) const {
  C_SclChecksums::CalcCRC32(&this->mu32_LastKnownCrc,
                            sizeof(this->mu32_LastKnownCrc), oru32_HashValue);
  C_SclChecksums::CalcCRC32(this->mc_FilePath.toUtf8().constData(),
                            this->mc_FilePath.length(), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set file information

   \param[in] orc_FilePath      File path
   \param[in] ou32_LastKnownCrc Last known CRC value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdateParamInfo::SetContent(
    const QString &orc_FilePath, const uint32_t ou32_LastKnownCrc) {
  this->mc_FilePath = orc_FilePath;
  this->mu32_LastKnownCrc = ou32_LastKnownCrc;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current path

   \return
   Current path
*/
//----------------------------------------------------------------------------------------------------------------------
const QString &C_OscViewNodeUpdateParamInfo::GetPath(void) const {
  return this->mc_FilePath;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get last known CRC value

   \return
   Last known CRC value
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscViewNodeUpdateParamInfo::GetLastKnownCrc(void) const {
  return this->mu32_LastKnownCrc;
}
