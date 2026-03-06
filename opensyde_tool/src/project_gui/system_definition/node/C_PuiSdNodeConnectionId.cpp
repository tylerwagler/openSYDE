//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       short description (implementation)

   detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_PuiSdNodeConnectionId.hpp"

#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSdNodeConnectionId::C_PuiSdNodeConnectionId() :
   e_InterfaceType(stw::opensyde_core::C_OscSystemBus::E_Type::eCAN),
   u8_InterfaceNumber(0)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compare two node connection IDs

   \param[in] orc_Ref Node connection ID to compare to

   \return
   true:  equal
   false: not equal
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_PuiSdNodeConnectionId::operator ==(const C_PuiSdNodeConnectionId & orc_Ref) const
{
   bool q_Retval;

   if ((this->e_InterfaceType == orc_Ref.e_InterfaceType) && (this->u8_InterfaceNumber == orc_Ref.u8_InterfaceNumber))
   {
      q_Retval = true;
   }
   else
   {
      q_Retval = false;
   }

   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSdNodeConnectionId::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                   this->e_InterfaceType,
                                                   this->u8_InterfaceNumber);
}
