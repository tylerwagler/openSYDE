//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job properties

   Data logger job properties

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscDataLoggerJobProperties.hpp"
#include "C_OscHashUtil.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
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
C_OscDataLoggerJobProperties::C_OscDataLoggerJobProperties()
    : c_Name("LogJob"), e_UseCase(eUC_MANUAL), e_LogFileFormat(eLFF_CSV),
      u32_MaxLogEntries(1000), u32_MaxLogDurationSec(60),
      u32_LogIntervalMs(1000), e_LocalLogTrigger(eLLT_ON_CHANGE) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobProperties::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_Name, this->c_Comment,
                              this->e_UseCase, this->e_LogFileFormat,
                              this->u32_MaxLogEntries, this->u32_MaxLogDurationSec,
                              this->u32_LogIntervalMs, this->e_LocalLogTrigger,
                              this->c_LogDestinationDirectory, this->c_AdditionalTriggerProperties);
}
