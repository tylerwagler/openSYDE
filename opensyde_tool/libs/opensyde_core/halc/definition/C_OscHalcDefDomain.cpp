//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition domain

   Detailed description (optional). The module shall be described detailed if it
   is not described completely by short description.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscHalcDefDomain.hpp"
#include "C_OscHashUtil.hpp"

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
C_OscHalcDefDomain::C_OscHalcDefDomain(void)
    : e_Category(C_OscHalcDefDomain::eCA_OTHER) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefDomain::~C_OscHalcDefDomain(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefDomain::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_Id, this->c_Name, this->c_Comment, this->c_SingularName,
                              this->e_Category, this->c_Channels, this->c_ChannelUseCases,
                              this->c_DomainValues, this->c_ChannelValues);
}
