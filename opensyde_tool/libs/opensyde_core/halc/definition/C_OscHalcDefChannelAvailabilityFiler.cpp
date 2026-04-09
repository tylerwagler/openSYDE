//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelAvailability (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefChannelAvailabilityFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelAvailabilityFiler::save(const C_OscHalcDefChannelAvailability & orc_Availability)
{
   QJsonObject c_Json;

   c_Json["value_index"] = static_cast<qint64>(orc_Availability.u32_ValueIndex);

   QJsonArray c_DependentArray;
   for (uint32_t u32_Dependent : orc_Availability.c_DependentValues)
   {
      c_DependentArray.append(static_cast<qint64>(u32_Dependent));
   }
   c_Json["dependent_values"] = c_DependentArray;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelAvailabilityFiler::load(const QJsonObject & orc_Json, C_OscHalcDefChannelAvailability & orc_Availability)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "value_index", orc_Availability.u32_ValueIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   if (orc_Json.contains("dependent_values"))
   {
      const QJsonArray c_Array = orc_Json["dependent_values"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isDouble())
         {
            return C_CONFIG;
         }
         orc_Availability.c_DependentValues.append(static_cast<uint32_t>(rc_Item.toInteger()));
      }
   }

   return C_NO_ERR;
}
