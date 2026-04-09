//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolListElement (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolListElementFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolContentFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscNodeDataPoolListElement::E_Access> hac_AccessTable[] = {
   { C_OscNodeDataPoolListElement::eACCESS_RW, "read_write" },
   { C_OscNodeDataPoolListElement::eACCESS_RO, "read_only"  }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolListElementFiler::save(const C_OscNodeDataPoolListElement & orc_Element)
{
   QJsonObject c_Json;
   c_Json["name"] = orc_Element.c_Name;
   c_Json["comment"] = orc_Element.c_Comment;
   c_Json["unit"] = orc_Element.c_Unit;
   c_Json["factor"] = orc_Element.f64_Factor;
   c_Json["offset"] = orc_Element.f64_Offset;
   c_Json["access"] = C_OscJsonUtil::h_EnumToString(orc_Element.e_Access, hac_AccessTable);
   c_Json["interpret_as_string"] = orc_Element.q_InterpretAsString;
   c_Json["diag_event_call"] = orc_Element.q_DiagEventCall;
   c_Json["nvm_start_address"] = static_cast<qint64>(orc_Element.u32_NvmStartAddress);
   c_Json["min_value"] = C_OscNodeDataPoolContentFiler::save(orc_Element.c_MinValue);
   c_Json["max_value"] = C_OscNodeDataPoolContentFiler::save(orc_Element.c_MaxValue);
   c_Json["value"] = C_OscNodeDataPoolContentFiler::save(orc_Element.c_Value);
   c_Json["nvm_value"] = C_OscNodeDataPoolContentFiler::save(orc_Element.c_NvmValue);
   c_Json["nvm_value_changed"] = orc_Element.q_NvmValueChanged;
   c_Json["nvm_value_is_valid"] = orc_Element.q_NvmValueIsValid;

   QJsonArray c_DataSetValues;
   for (const C_OscNodeDataPoolContent & rc_DataSetValue : orc_Element.c_DataSetValues)
   {
      c_DataSetValues.append(C_OscNodeDataPoolContentFiler::save(rc_DataSetValue));
   }
   c_Json["data_set_values"] = c_DataSetValues;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolListElementFiler::load(const QJsonObject & orc_Json,
                                                C_OscNodeDataPoolListElement & orc_Element)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_Element.c_Name);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   C_OscJsonUtil::h_GetStringOr(orc_Json, "comment", QString(), orc_Element.c_Comment);
   C_OscJsonUtil::h_GetStringOr(orc_Json, "unit", QString(), orc_Element.c_Unit);

   s32_Result = C_OscJsonUtil::h_GetF64(orc_Json, "factor", orc_Element.f64_Factor);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetF64(orc_Json, "offset", orc_Element.f64_Offset);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QString c_AccessStr;
   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "access", c_AccessStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_AccessStr, hac_AccessTable, orc_Element.e_Access);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   C_OscJsonUtil::h_GetBoolOr(orc_Json, "interpret_as_string", false, orc_Element.q_InterpretAsString);
   C_OscJsonUtil::h_GetBoolOr(orc_Json, "diag_event_call", false, orc_Element.q_DiagEventCall);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_start_address", 0U, orc_Element.u32_NvmStartAddress);
   C_OscJsonUtil::h_GetBoolOr(orc_Json, "nvm_value_changed", false, orc_Element.q_NvmValueChanged);
   C_OscJsonUtil::h_GetBoolOr(orc_Json, "nvm_value_is_valid", false, orc_Element.q_NvmValueIsValid);

   QJsonObject c_SubObject;
   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "min_value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscNodeDataPoolContentFiler::load(c_SubObject, orc_Element.c_MinValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "max_value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscNodeDataPoolContentFiler::load(c_SubObject, orc_Element.c_MaxValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscNodeDataPoolContentFiler::load(c_SubObject, orc_Element.c_Value);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   if (orc_Json.contains("nvm_value"))
   {
      s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "nvm_value", c_SubObject);
      if (s32_Result == C_NO_ERR)
      {
         s32_Result = C_OscNodeDataPoolContentFiler::load(c_SubObject, orc_Element.c_NvmValue);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
      }
   }

   QJsonArray c_DataSetValues;
   if (C_OscJsonUtil::h_GetArray(orc_Json, "data_set_values", c_DataSetValues) == C_NO_ERR)
   {
      orc_Element.c_DataSetValues.clear();
      orc_Element.c_DataSetValues.reserve(c_DataSetValues.size());
      for (const QJsonValue & rc_Value : c_DataSetValues)
      {
         if (!rc_Value.isObject())
         {
            return C_CONFIG;
         }
         C_OscNodeDataPoolContent c_DataSetValue;
         s32_Result = C_OscNodeDataPoolContentFiler::load(rc_Value.toObject(), c_DataSetValue);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Element.c_DataSetValues.append(c_DataSetValue);
      }
   }

   return C_NO_ERR;
}
