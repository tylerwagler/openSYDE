//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelValues (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefChannelValuesFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscHalcDefStructFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelValuesFiler::save(const C_OscHalcDefChannelValues & orc_Values)
{
   QJsonObject c_Json;

   QJsonArray c_Parameters;
   for (const C_OscHalcDefStruct & rc_Struct : orc_Values.c_Parameters)
   {
      c_Parameters.append(C_OscHalcDefStructFiler::save(rc_Struct));
   }
   c_Json["parameters"] = c_Parameters;

   QJsonArray c_InputValues;
   for (const C_OscHalcDefStruct & rc_Struct : orc_Values.c_InputValues)
   {
      c_InputValues.append(C_OscHalcDefStructFiler::save(rc_Struct));
   }
   c_Json["input_values"] = c_InputValues;

   QJsonArray c_OutputValues;
   for (const C_OscHalcDefStruct & rc_Struct : orc_Values.c_OutputValues)
   {
      c_OutputValues.append(C_OscHalcDefStructFiler::save(rc_Struct));
   }
   c_Json["output_values"] = c_OutputValues;

   QJsonArray c_StatusValues;
   for (const C_OscHalcDefStruct & rc_Struct : orc_Values.c_StatusValues)
   {
      c_StatusValues.append(C_OscHalcDefStructFiler::save(rc_Struct));
   }
   c_Json["status_values"] = c_StatusValues;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelValuesFiler::load(const QJsonObject & orc_Json, C_OscHalcDefChannelValues & orc_Values)
{
   int32_t s32_Result;

   if (orc_Json.contains("parameters"))
   {
      const QJsonArray c_Array = orc_Json["parameters"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefStruct c_Struct;
         s32_Result = C_OscHalcDefStructFiler::load(rc_Item.toObject(), c_Struct);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Values.c_Parameters.append(c_Struct);
      }
   }

   if (orc_Json.contains("input_values"))
   {
      const QJsonArray c_Array = orc_Json["input_values"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefStruct c_Struct;
         s32_Result = C_OscHalcDefStructFiler::load(rc_Item.toObject(), c_Struct);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Values.c_InputValues.append(c_Struct);
      }
   }

   if (orc_Json.contains("output_values"))
   {
      const QJsonArray c_Array = orc_Json["output_values"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefStruct c_Struct;
         s32_Result = C_OscHalcDefStructFiler::load(rc_Item.toObject(), c_Struct);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Values.c_OutputValues.append(c_Struct);
      }
   }

   if (orc_Json.contains("status_values"))
   {
      const QJsonArray c_Array = orc_Json["status_values"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefStruct c_Struct;
         s32_Result = C_OscHalcDefStructFiler::load(rc_Item.toObject(), c_Struct);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Values.c_StatusValues.append(c_Struct);
      }
   }

   return C_NO_ERR;
}
