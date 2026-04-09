//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelUseCase (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefChannelUseCaseFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolContentFiler.hpp"
#include "C_OscHalcDefChannelAvailabilityFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelUseCaseFiler::save(const C_OscHalcDefChannelUseCase & orc_UseCase)
{
   QJsonObject c_Json;

   c_Json["id"] = orc_UseCase.c_Id;
   c_Json["display"] = orc_UseCase.c_Display;
   c_Json["comment"] = orc_UseCase.c_Comment;
   c_Json["value"] = C_OscNodeDataPoolContentFiler::save(orc_UseCase.c_Value);

   QJsonArray c_AvailabilityArray;
   for (const C_OscHalcDefChannelAvailability & rc_Avail : orc_UseCase.c_Availability)
   {
      c_AvailabilityArray.append(C_OscHalcDefChannelAvailabilityFiler::save(rc_Avail));
   }
   c_Json["availability"] = c_AvailabilityArray;

   QJsonArray c_DefaultChannels;
   for (uint32_t u32_Channel : orc_UseCase.c_DefaultChannels)
   {
      c_DefaultChannels.append(static_cast<qint64>(u32_Channel));
   }
   c_Json["default_channels"] = c_DefaultChannels;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelUseCaseFiler::load(const QJsonObject & orc_Json, C_OscHalcDefChannelUseCase & orc_UseCase)
{
   int32_t s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "id", orc_UseCase.c_Id);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "display", orc_UseCase.c_Display);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "comment", orc_UseCase.c_Comment);
   if (s32_Result != C_NO_ERR) return s32_Result;

   QJsonObject c_ValueObject;
   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "value", c_ValueObject);
   if (s32_Result != C_NO_ERR) return s32_Result;
   s32_Result = C_OscNodeDataPoolContentFiler::load(c_ValueObject, orc_UseCase.c_Value);
   if (s32_Result != C_NO_ERR) return s32_Result;

   if (orc_Json.contains("availability"))
   {
      const QJsonArray c_Array = orc_Json["availability"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         C_OscHalcDefChannelAvailability c_Avail;
         s32_Result = C_OscHalcDefChannelAvailabilityFiler::load(rc_Item.toObject(), c_Avail);
         if (s32_Result != C_NO_ERR) return s32_Result;
         orc_UseCase.c_Availability.append(c_Avail);
      }
   }

   if (orc_Json.contains("default_channels"))
   {
      const QJsonArray c_Array = orc_Json["default_channels"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isDouble())
         {
            return C_CONFIG;
         }
         orc_UseCase.c_DefaultChannels.append(static_cast<uint32_t>(rc_Item.toInteger()));
      }
   }

   return C_NO_ERR;
}
