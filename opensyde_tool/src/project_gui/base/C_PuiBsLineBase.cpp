//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI base class for segmented line based data: stores UI information (implementation)

   UI base class for segmented line based data: stores UI information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_PuiBsLineBase.hpp"

#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
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
C_PuiBsLineBase::C_PuiBsLineBase(void) :
   c_UiColor(Qt::GlobalColor::cyan),
   s32_UiWidthPixels(3),
   f64_ZetOrder(0.0)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiBsLineBase::~C_PuiBsLineBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiBsLineBase::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_UiColor.red(), this->c_UiColor.green(),
                              this->c_UiColor.blue(), this->c_UiColor.alpha(),
                              this->s32_UiWidthPixels, this->f64_ZetOrder);

   for (const QPointF & rc_Point : this->c_UiInteractionPoints)
   {
      hash_util::CalcHashMembers(oru32_HashValue, rc_Point.x(), rc_Point.y());
   }
}
