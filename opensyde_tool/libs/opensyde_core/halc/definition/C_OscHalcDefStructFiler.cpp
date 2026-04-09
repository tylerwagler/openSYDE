//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefStruct (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefStructFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscHalcDefElementFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefStructFiler::save(const C_OscHalcDefStruct & orc_Struct)
{
   QJsonObject c_Json = C_OscHalcDefElementFiler::save(orc_Struct);

   // Save struct-specific: nested elements
   QJsonArray c_StructElementsArray;
   for (const C_OscHalcDefElement & rc_Element : orc_Struct.c_StructElements)
   {
      c_StructElementsArray.append(C_OscHalcDefElementFiler::save(rc_Element));
   }
   c_Json["struct_elements"] = c_StructElementsArray;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefStructFiler::load(const QJsonObject & orc_Json, C_OscHalcDefStruct & orc_Struct)
{
   // Load base C_OscHalcDefElement fields
   int32_t s32_Result = C_OscHalcDefElementFiler::load(orc_Json, orc_Struct);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   // Load struct-specific: nested elements
   if (orc_Json.contains("struct_elements"))
   {
      const QJsonArray c_StructElementsArray = orc_Json["struct_elements"].toArray();
      for (const QJsonValue & rc_Item : c_StructElementsArray)
      {
         if (!rc_Item.isObject())
         {
            return C_CONFIG;
         }
         C_OscHalcDefElement c_Element;
         s32_Result = C_OscHalcDefElementFiler::load(rc_Item.toObject(), c_Element);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_Struct.c_StructElements.append(c_Element);
      }
   }

   return C_NO_ERR;
}
