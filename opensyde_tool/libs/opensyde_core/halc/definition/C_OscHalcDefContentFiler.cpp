//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefContent (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefContentFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolContentFiler.hpp"
#include "C_OscHalcDefContentBitmaskItemFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscHalcDefContent::E_ComplexType> hac_ComplexTypeTable[] = {
   { C_OscHalcDefContent::eCT_PLAIN,    "plain"    },
   { C_OscHalcDefContent::eCT_ENUM,     "enum"     },
   { C_OscHalcDefContent::eCT_BIT_MASK, "bit_mask" },
   { C_OscHalcDefContent::eCT_STRING,   "string"   }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefContentFiler::save(const C_OscHalcDefContent & orc_Content)
{
   QJsonObject c_Json;

    // Save base C_OscNodeDataPoolContent fields
    c_Json = C_OscNodeDataPoolContentFiler::save(orc_Content);

    // Add HALC-specific extensions
    c_Json["complex_type"] = C_OscJsonUtil::h_EnumToString(orc_Content.GetComplexType(), hac_ComplexTypeTable);

    // Save string value for string-typed content
    if (orc_Content.GetComplexType() == C_OscHalcDefContent::eCT_STRING)
    {
       QString c_StringValue;
       orc_Content.GetStringValue(c_StringValue);
       c_Json["string_value"] = c_StringValue;
    }

    // Save enum items
   const QList<QPair<QString, C_OscNodeDataPoolContent>> & rc_EnumItems = orc_Content.GetEnumItems();
   QJsonArray c_EnumArray;
   for (const QPair<QString, C_OscNodeDataPoolContent> & rc_Item : rc_EnumItems)
   {
      QJsonObject c_EnumItem;
      c_EnumItem["display"] = rc_Item.first;
      c_EnumItem["value"] = C_OscNodeDataPoolContentFiler::save(rc_Item.second);
      c_EnumArray.append(c_EnumItem);
   }
   c_Json["enum_items"] = c_EnumArray;

   // Save bitmask items
   const QList<C_OscHalcDefContentBitmaskItem> & rc_BitmaskItems = orc_Content.GetBitmaskItems();
   QJsonArray c_BitmaskArray;
   for (const C_OscHalcDefContentBitmaskItem & rc_Item : rc_BitmaskItems)
   {
      c_BitmaskArray.append(C_OscHalcDefContentBitmaskItemFiler::save(rc_Item));
   }
   c_Json["bitmask_items"] = c_BitmaskArray;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentFiler::load(const QJsonObject & orc_Json, C_OscHalcDefContent & orc_Content)
{
   // Load base C_OscNodeDataPoolContent fields
   int32_t s32_Result = C_OscNodeDataPoolContentFiler::load(orc_Json, orc_Content);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

    // Load complex type
    QString c_ComplexTypeStr;
    s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "complex_type", c_ComplexTypeStr);
    if (s32_Result != C_NO_ERR)
    {
       return s32_Result;
    }
    C_OscHalcDefContent::E_ComplexType e_ComplexType;
    s32_Result = C_OscJsonUtil::h_StringToEnum(c_ComplexTypeStr, hac_ComplexTypeTable, e_ComplexType);
    if (s32_Result != C_NO_ERR)
    {
       return s32_Result;
    }
    orc_Content.SetComplexType(e_ComplexType);

    // Load string value for string-typed content
    if (orc_Content.GetComplexType() == C_OscHalcDefContent::eCT_STRING)
    {
       QString c_StringValue;
       s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "string_value", c_StringValue);
       if (s32_Result != C_NO_ERR)
       {
          return s32_Result;
       }
       s32_Result = orc_Content.SetStringValue(c_StringValue);
       if (s32_Result != C_NO_ERR)
       {
          return s32_Result;
       }
    }

    // Load enum items
   if (orc_Json.contains("enum_items"))
   {
      const QJsonArray c_EnumArray = orc_Json["enum_items"].toArray();
      for (const QJsonValue & rc_Item : c_EnumArray)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         const QJsonObject c_EnumItem = rc_Item.toObject();

         QString c_Display;
         s32_Result = C_OscJsonUtil::h_GetString(c_EnumItem, "display", c_Display);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }

         QJsonObject c_ValueObject;
         s32_Result = C_OscJsonUtil::h_GetObject(c_EnumItem, "value", c_ValueObject);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         C_OscNodeDataPoolContent c_Value;
         s32_Result = C_OscNodeDataPoolContentFiler::load(c_ValueObject, c_Value);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }

          s32_Result = orc_Content.AddEnumItem(c_Display, c_Value);
          if (s32_Result != C_NO_ERR)
          {
             return s32_Result;
          }
      }
   }

   // Load bitmask items
   if (orc_Json.contains("bitmask_items"))
   {
      const QJsonArray c_BitmaskArray = orc_Json["bitmask_items"].toArray();
      for (const QJsonValue & rc_Item : c_BitmaskArray)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefContentBitmaskItem c_BitmaskItem;
         s32_Result = C_OscHalcDefContentBitmaskItemFiler::load(rc_Item.toObject(), c_BitmaskItem);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Content.AddBitmaskItem(c_BitmaskItem);
      }
   }

   return C_NO_ERR;
}
