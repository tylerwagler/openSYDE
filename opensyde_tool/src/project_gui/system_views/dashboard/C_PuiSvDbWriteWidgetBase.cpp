//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base data class for write widgets

   Base data class for write widgets

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_OscHashUtil.hpp"
#include "C_PuiSvDbWriteWidgetBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

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
C_PuiSvDbWriteWidgetBase::C_PuiSvDbWriteWidgetBase() :
   C_PuiSvDbWidgetBase(),
   q_AutoWriteOnConnect(false),
   e_ElementWriteMode(eWM_MANUAL),
   e_InitialValueMode(eIVM_DISABLED)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvDbWriteWidgetBase::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                  this->q_AutoWriteOnConnect,
                                                  this->e_ElementWriteMode,
                                                  this->e_InitialValueMode,
                                                  this->c_InitialValue);

   C_PuiSvDbWidgetBase::CalcHash(oru32_HashValue);
}
