//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bitmask item

   Bitmask item

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <sstream>

#include "stwerrors.hpp"
#include "C_SclChecksums.hpp"
#include "C_OscHalcDefContentBitmaskItem.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

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
C_OscHalcDefContentBitmaskItem::C_OscHalcDefContentBitmaskItem(void) :
   q_ApplyValueSetting(false),
   u64_Value(0ULL)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Denstructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefContentBitmaskItem::~C_OscHalcDefContentBitmaskItem()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set mask value by string

   \param[in]  orc_Item    Mask string

   \return
   C_NO_ERR All fine
   C_RANGE  String invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentBitmaskItem::SetValueByString(const QString & orc_Item)
{
   return C_OscHalcDefContentBitmaskItem::mh_ParseUintFromString(orc_Item, this->u64_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContentBitmaskItem::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Display.toUtf8().data(),
                                      static_cast<uint32_t>(this->c_Display.toUtf8().size()), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Comment.toUtf8().data(),
                                      static_cast<uint32_t>(this->c_Comment.toUtf8().size()), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_ApplyValueSetting, sizeof(this->q_ApplyValueSetting), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u64_Value, sizeof(this->u64_Value), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over structure only

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContentBitmaskItem::CalcHashStructure(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Display.toUtf8().data(),
                                      static_cast<uint32_t>(this->c_Display.toUtf8().size()), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Comment.toUtf8().data(),
                                      static_cast<uint32_t>(this->c_Comment.toUtf8().size()), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u64_Value, sizeof(this->u64_Value), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse uint value from string

   \param[in]  orc_Item       Generic value string
   \param[in]  oru64_Value    Value to set

   \return
   C_NO_ERR All fine
   C_RANGE  String invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentBitmaskItem::mh_ParseUintFromString(const QString & orc_Item,
                                                               uint64_t & oru64_Value)
{
   int32_t s32_Retval = C_NO_ERR;

   if (orc_Item.toLower() == "true")
   {
      oru64_Value = 1ULL;
   }
   else if (orc_Item.toLower() == "false")
   {
      oru64_Value = 0ULL;
   }
   else
   {
      if (orc_Item.startsWith("0x", Qt::CaseInsensitive))
      {
         bool q_Ok = false;
         oru64_Value = orc_Item.toULongLong(&q_Ok, 16);
         if (q_Ok == false)
         {
            s32_Retval = C_RANGE;
         }
      }
      else
      {
         bool q_Ok = false;
         oru64_Value = orc_Item.toULongLong(&q_Ok, 10);
         if (q_Ok == false)
         {
            s32_Retval = C_RANGE;
         }
      }
   }
   return s32_Retval;
}
