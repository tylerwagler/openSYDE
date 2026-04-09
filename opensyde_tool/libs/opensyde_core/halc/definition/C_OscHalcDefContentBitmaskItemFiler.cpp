//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefContentBitmaskItem (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefContentBitmaskItemFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefContentBitmaskItemFiler::save(const C_OscHalcDefContentBitmaskItem & orc_Item)
{
   QJsonObject c_Json;

   c_Json["display"] = orc_Item.c_Display;
   c_Json["comment"] = orc_Item.c_Comment;
   c_Json["apply_value_setting"] = orc_Item.q_ApplyValueSetting;
   C_OscJsonUtil::h_SetU64(c_Json, "value", orc_Item.u64_Value);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentBitmaskItemFiler::load(const QJsonObject & orc_Json, C_OscHalcDefContentBitmaskItem & orc_Item)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "display", orc_Item.c_Display);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "comment", orc_Item.c_Comment);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "apply_value_setting", orc_Item.q_ApplyValueSetting);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU64(orc_Json, "value", orc_Item.u64_Value);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   return C_NO_ERR;
}
