//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDef (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscHalcDefDomainFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscHalcDefBase::E_SafetyMode> hac_SafetyModeTable[] = {
   { C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING,       "two_levels_with_dropping"       },
   { C_OscHalcDefBase::eTWO_LEVELS_WITHOUT_DROPPING,    "two_levels_without_dropping"    },
   { C_OscHalcDefBase::eONE_LEVEL_ALL_SAFE,             "one_level_all_safe"             },
   { C_OscHalcDefBase::eONE_LEVEL_ALL_NON_SAFE,         "one_level_all_non_safe"         }
};
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefFiler::save(const C_OscHalcDef & orc_Definition)
{
   QJsonObject c_Json;

   c_Json["content_version"] = static_cast<qint64>(orc_Definition.u32_ContentVersion);
   c_Json["device_name"] = orc_Definition.c_DeviceName;
   c_Json["file_string"] = orc_Definition.c_FileString;
   c_Json["original_file_name"] = orc_Definition.c_OriginalFileName;
   c_Json["safety_mode"] = C_OscJsonUtil::h_EnumToString(orc_Definition.e_SafetyMode, hac_SafetyModeTable);
   c_Json["num_config_copies"] = static_cast<qint64>(orc_Definition.u8_NumConfigCopies);
   c_Json["nvm_based_config"] = orc_Definition.q_NvmBasedConfig;

   QJsonArray c_NvmSafeArray;
   for (uint32_t u32_Addr : orc_Definition.c_NvmSafeAddressOffset)
   {
      c_NvmSafeArray.append(static_cast<qint64>(u32_Addr));
   }
   c_Json["nvm_safe_address_offset"] = c_NvmSafeArray;

   QJsonArray c_NvmNonSafeArray;
   for (uint32_t u32_Addr : orc_Definition.c_NvmNonSafeAddressOffset)
   {
      c_NvmNonSafeArray.append(static_cast<qint64>(u32_Addr));
   }
   c_Json["nvm_non_safe_address_offset"] = c_NvmNonSafeArray;

   c_Json["nvm_reserved_list_size_parameters"] = static_cast<qint64>(orc_Definition.u32_NvmReservedListSizeParameters);
   c_Json["nvm_reserved_list_size_input_values"] = static_cast<qint64>(orc_Definition.u32_NvmReservedListSizeInputValues);
   c_Json["nvm_reserved_list_size_output_values"] = static_cast<qint64>(orc_Definition.u32_NvmReservedListSizeOutputValues);
   c_Json["nvm_reserved_list_size_status_values"] = static_cast<qint64>(orc_Definition.u32_NvmReservedListSizeStatusValues);

    QJsonArray c_Domains;
    uint32_t u32_DomainCount = orc_Definition.GetDomainSize();
    for (uint32_t u32_Index = 0; u32_Index < u32_DomainCount; ++u32_Index)
    {
       const C_OscHalcDefDomain *opc_Domain = orc_Definition.GetDomainDefDataConst(u32_Index);
       if (opc_Domain != nullptr)
       {
          c_Domains.append(C_OscHalcDefDomainFiler::save(*opc_Domain));
       }
    }
    c_Json["domains"] = c_Domains;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefFiler::load(const QJsonObject & orc_Json, C_OscHalcDef & orc_Definition)
{
   int32_t s32_Result;

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "content_version", orc_Definition.u32_ContentVersion);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "device_name", orc_Definition.c_DeviceName);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "file_string", orc_Definition.c_FileString);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "original_file_name", orc_Definition.c_OriginalFileName);
   if (s32_Result != C_NO_ERR) return s32_Result;

   QString c_SafetyModeStr;
   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "safety_mode", c_SafetyModeStr);
   if (s32_Result != C_NO_ERR) return s32_Result;
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_SafetyModeStr, hac_SafetyModeTable, orc_Definition.e_SafetyMode);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "num_config_copies", orc_Definition.u8_NumConfigCopies);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "nvm_based_config", orc_Definition.q_NvmBasedConfig);
   if (s32_Result != C_NO_ERR) return s32_Result;

   if (orc_Json.contains("nvm_safe_address_offset"))
   {
      const QJsonArray c_Array = orc_Json["nvm_safe_address_offset"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isDouble()) return C_CONFIG;
         orc_Definition.c_NvmSafeAddressOffset.append(static_cast<uint32_t>(rc_Item.toInteger()));
      }
   }

   if (orc_Json.contains("nvm_non_safe_address_offset"))
   {
      const QJsonArray c_Array = orc_Json["nvm_non_safe_address_offset"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         if (!rc_Item.isDouble()) return C_CONFIG;
         orc_Definition.c_NvmNonSafeAddressOffset.append(static_cast<uint32_t>(rc_Item.toInteger()));
      }
   }

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "nvm_reserved_list_size_parameters", orc_Definition.u32_NvmReservedListSizeParameters);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "nvm_reserved_list_size_input_values", orc_Definition.u32_NvmReservedListSizeInputValues);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "nvm_reserved_list_size_output_values", orc_Definition.u32_NvmReservedListSizeOutputValues);
   if (s32_Result != C_NO_ERR) return s32_Result;

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "nvm_reserved_list_size_status_values", orc_Definition.u32_NvmReservedListSizeStatusValues);
   if (s32_Result != C_NO_ERR) return s32_Result;

   if (orc_Json.contains("domains"))
   {
      const QJsonArray c_Array = orc_Json["domains"].toArray();
      for (const QJsonValue & rc_Item : c_Array)
      {
         C_OscHalcDefDomain c_Domain;
         s32_Result = C_OscHalcDefDomainFiler::load(rc_Item.toObject(), c_Domain);
         if (s32_Result != C_NO_ERR) return s32_Result;
         orc_Definition.AddDomain(c_Domain);
      }
   }

   return C_NO_ERR;
}
