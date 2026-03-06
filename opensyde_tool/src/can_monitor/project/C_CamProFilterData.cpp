//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filter package and filter item data structure (implementation)

   A filter in openSYDE CAN monitor is more a filter package.
   Such a filter package consists of one or more openSYDE core
   filters and additional information such as a name and a comment.

   To avoid naming confusion consider:

      openSYDE CAN monitor       openSYDE core
      ---------------------------------------------------------
      filter item                filter
      filter                     package of one ore more filters

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_CamProFilterData.hpp"
#include "C_OscHashUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_core::hash_util;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor of filter class.
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamProFilterData::C_CamProFilterData(void)
{
   this->c_Name = "DefaultFilterName";
   this->c_Comment = "";
   this->q_Enabled = true;
   this->c_FilterItems.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamProFilterData::CalcHash(uint32_t & oru32_HashValue) const
{
   CalcHashMembers(oru32_HashValue, this->c_Name, this->c_Comment, this->q_Enabled, this->c_FilterItems);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor of filter item class.
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamProFilterItemData::C_CamProFilterItemData() :
   C_OscComMessageLoggerFilter()
{
   this->q_Enabled = true;
   if (this->u32_StartId == this->u32_EndId)
   {
      e_Type = eID_SINGLE;
   }
   else
   {
      e_Type = eID_RANGE;
   }

   this->c_DatabaseMatch = "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamProFilterItemData::CalcHash(uint32_t & oru32_HashValue) const
{
   CalcHashMembers(oru32_HashValue, this->q_PassFilter, this->u8_ExtendedId, this->u32_EndId,
                   this->u32_StartId, this->q_Enabled, this->e_Type);
}
