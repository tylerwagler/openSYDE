//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefDomain (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefDomainFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscHalcDefChannelDefFiler.hpp"
#include "C_OscHalcDefChannelUseCaseFiler.hpp"
#include "C_OscHalcDefChannelValuesFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscHalcDefDomain::E_Category> hac_CategoryTable[] = {
   { C_OscHalcDefDomain::eCA_INPUT,    "input"    },
   { C_OscHalcDefDomain::eCA_OUTPUT,   "output"   },
   { C_OscHalcDefDomain::eCA_OTHER,    "other"    }
};
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefDomainFiler::save(const C_OscHalcDefDomain & orc_Domain)
{
   QJsonObject c_Json;

   c_Json["id"] = orc_Domain.c_Id;
   c_Json["name"] = orc_Domain.c_Name;
   c_Json["singular_name"] = orc_Domain.c_SingularName;
   c_Json["comment"] = orc_Domain.c_Comment;

   QJsonArray c_Channels;
   for (const C_OscHalcDefChannelDef & rc_Channel : orc_Domain.c_Channels)
   {
      c_Channels.append(C_OscHalcDefChannelDefFiler::save(rc_Channel));
   }
   c_Json["channels"] = c_Channels;

   QJsonArray c_UseCases;
   for (const C_OscHalcDefChannelUseCase & rc_UseCase : orc_Domain.c_ChannelUseCases)
   {
      c_UseCases.append(C_OscHalcDefChannelUseCaseFiler::save(rc_UseCase));
   }
   c_Json["channel_use_cases"] = c_UseCases;

   c_Json["domain_values"] = C_OscHalcDefChannelValuesFiler::save(orc_Domain.c_DomainValues);
   c_Json["channel_values"] = C_OscHalcDefChannelValuesFiler::save(orc_Domain.c_ChannelValues);

   c_Json["category"] = C_OscJsonUtil::h_EnumToString(orc_Domain.e_Category, hac_CategoryTable);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefDomainFiler::load(const QJsonObject & orc_Json, C_OscHalcDefDomain & orc_Domain)
{
   int32_t s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "id", orc_Domain.c_Id);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_Domain.c_Name);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "singular_name", orc_Domain.c_SingularName);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "comment", orc_Domain.c_Comment);
   if (s32_Result != C_NO_ERR) return s32_Result;

   if (orc_Json.contains("channels"))
   {
      const QJsonArray c_Array = orc_Json["channels"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         C_OscHalcDefChannelDef c_Channel;
         s32_Result = C_OscHalcDefChannelDefFiler::load(rc_Item.toObject(), c_Channel);
         if (s32_Result != C_NO_ERR) return s32_Result;
         orc_Domain.c_Channels.append(c_Channel);
      }
   }

   if (orc_Json.contains("channel_use_cases"))
   {
      const QJsonArray c_Array = orc_Json["channel_use_cases"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         C_OscHalcDefChannelUseCase c_UseCase;
         s32_Result = C_OscHalcDefChannelUseCaseFiler::load(rc_Item.toObject(), c_UseCase);
         if (s32_Result != C_NO_ERR) return s32_Result;
         orc_Domain.c_ChannelUseCases.append(c_UseCase);
      }
   }

   QJsonObject c_ValuesObject;
   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "domain_values", c_ValuesObject);
   if (s32_Result != C_NO_ERR) return s32_Result;
   s32_Result = C_OscHalcDefChannelValuesFiler::load(c_ValuesObject, orc_Domain.c_DomainValues);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "channel_values", c_ValuesObject);
   if (s32_Result != C_NO_ERR) return s32_Result;
   s32_Result = C_OscHalcDefChannelValuesFiler::load(c_ValuesObject, orc_Domain.c_ChannelValues);
   if (s32_Result != C_NO_ERR) return s32_Result;

   QString c_CategoryStr;
   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "category", c_CategoryStr);
   if (s32_Result != C_NO_ERR) return s32_Result;
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_CategoryStr, hac_CategoryTable, orc_Domain.e_Category);
   if (s32_Result != C_NO_ERR) return s32_Result;

   return C_NO_ERR;
}
