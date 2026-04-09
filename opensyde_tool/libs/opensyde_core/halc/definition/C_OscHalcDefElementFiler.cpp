//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefElement (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefElementFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscHalcDefContentFiler.hpp"
#include "C_OscNodeDataPoolContentFiler.hpp"
#include "C_OscHalcDefContentBitmaskItemFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefElementFiler::save(const C_OscHalcDefElement & orc_Element)
{
   QJsonObject c_Json;

   c_Json["id"] = orc_Element.c_Id;
   c_Json["display"] = orc_Element.c_Display;
   c_Json["comment"] = orc_Element.c_Comment;

   // Save content fields (type metadata is embedded in each content object)
   c_Json["initial_value"] = C_OscHalcDefContentFiler::save(orc_Element.c_InitialValue);
   c_Json["min_value"] = C_OscHalcDefContentFiler::save(orc_Element.c_MinValue);
   c_Json["max_value"] = C_OscHalcDefContentFiler::save(orc_Element.c_MaxValue);

   // Save use-case availabilities
   QJsonArray c_AvailabilityArray;
   for (uint32_t u32_Avail : orc_Element.c_UseCaseAvailabilities)
   {
      c_AvailabilityArray.append(static_cast<qint64>(u32_Avail));
   }
   c_Json["use_case_availabilities"] = c_AvailabilityArray;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefElementFiler::load(const QJsonObject & orc_Json, C_OscHalcDefElement & orc_Element)
{
   int32_t s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "id", orc_Element.c_Id);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "display", orc_Element.c_Display);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "comment", orc_Element.c_Comment);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   // Load content fields (type metadata is embedded in each content object)
   QJsonObject c_SubObject;

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "initial_value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscHalcDefContentFiler::load(c_SubObject, orc_Element.c_InitialValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "min_value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscHalcDefContentFiler::load(c_SubObject, orc_Element.c_MinValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "max_value", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscHalcDefContentFiler::load(c_SubObject, orc_Element.c_MaxValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   // Load use-case availabilities
   if (orc_Json.contains("use_case_availabilities"))
   {
      const QJsonArray c_AvailabilityArray = orc_Json["use_case_availabilities"].toArray();
      for (const QJsonValue & rc_Item : c_AvailabilityArray)
      {
         if (!rc_Item.isDouble())
         {
            return C_CONFIG;
         }
         orc_Element.c_UseCaseAvailabilities.append(static_cast<uint32_t>(rc_Item.toInteger()));
      }
   }

   return C_NO_ERR;
}
