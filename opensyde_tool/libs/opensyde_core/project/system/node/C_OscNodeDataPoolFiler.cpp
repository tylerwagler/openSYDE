//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPool (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolListFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscNodeDataPool::E_Type> hac_TypeTable[] = {
   { C_OscNodeDataPool::eDIAG,     "diag"     },
   { C_OscNodeDataPool::eNVM,      "nvm"      },
   { C_OscNodeDataPool::eCOM,      "com"      },
   { C_OscNodeDataPool::eHALC,     "halc"     },
   { C_OscNodeDataPool::eHALC_NVM, "halc_nvm" }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolFiler::save(const C_OscNodeDataPool & orc_DataPool)
{
   QJsonObject c_Json;
   c_Json["type"] = C_OscJsonUtil::h_EnumToString(orc_DataPool.e_Type, hac_TypeTable);
   c_Json["name"] = orc_DataPool.c_Name;
   c_Json["comment"] = orc_DataPool.c_Comment;

   QJsonArray c_Version;
   c_Version.append(static_cast<qint64>(orc_DataPool.au8_Version[0]));
   c_Version.append(static_cast<qint64>(orc_DataPool.au8_Version[1]));
   c_Version.append(static_cast<qint64>(orc_DataPool.au8_Version[2]));
   c_Json["version"] = c_Version;

   c_Json["definition_crc_version"] = static_cast<qint64>(orc_DataPool.u16_DefinitionCrcVersion);
   c_Json["related_data_block_index"] = static_cast<qint64>(orc_DataPool.s32_RelatedDataBlockIndex);
   c_Json["is_safety"] = orc_DataPool.q_IsSafety;
   c_Json["scope_is_private"] = orc_DataPool.q_ScopeIsPrivate;
   c_Json["nvm_start_address"] = static_cast<qint64>(orc_DataPool.u32_NvmStartAddress);
   c_Json["nvm_size"] = static_cast<qint64>(orc_DataPool.u32_NvmSize);

   QJsonArray c_Lists;
   for (const C_OscNodeDataPoolList & rc_List : orc_DataPool.c_Lists)
   {
      c_Lists.append(C_OscNodeDataPoolListFiler::save(rc_List));
   }
   c_Json["lists"] = c_Lists;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::load(const QJsonObject & orc_Json, C_OscNodeDataPool & orc_DataPool)
{
   QString c_TypeStr;
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "type", c_TypeStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_TypeStr, hac_TypeTable, orc_DataPool.e_Type);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_DataPool.c_Name);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   C_OscJsonUtil::h_GetStringOr(orc_Json, "comment", QString(), orc_DataPool.c_Comment);

   QJsonArray c_Version;
   if (C_OscJsonUtil::h_GetArray(orc_Json, "version", c_Version) != C_NO_ERR)
   {
      return C_CONFIG;
   }
   if (c_Version.size() != 3)
   {
      return C_CONFIG;
   }
   for (int32_t s32_Index = 0; s32_Index < 3; ++s32_Index)
   {
      if (!c_Version.at(s32_Index).isDouble())
      {
         return C_CONFIG;
      }
      orc_DataPool.au8_Version[s32_Index] = static_cast<uint8_t>(c_Version.at(s32_Index).toInteger());
   }

   uint32_t u32_DefCrcVersion = 0U;
   C_OscJsonUtil::h_GetU32Or(orc_Json, "definition_crc_version", 1U, u32_DefCrcVersion);
   orc_DataPool.u16_DefinitionCrcVersion = static_cast<uint16_t>(u32_DefCrcVersion);

   int32_t s32_RelatedDataBlockIndex = -1;
   if (C_OscJsonUtil::h_GetS32(orc_Json, "related_data_block_index", s32_RelatedDataBlockIndex) == C_NO_ERR)
   {
      orc_DataPool.s32_RelatedDataBlockIndex = s32_RelatedDataBlockIndex;
   }
   else
   {
      orc_DataPool.s32_RelatedDataBlockIndex = -1;
   }

   C_OscJsonUtil::h_GetBoolOr(orc_Json, "is_safety", false, orc_DataPool.q_IsSafety);
   C_OscJsonUtil::h_GetBoolOr(orc_Json, "scope_is_private", false, orc_DataPool.q_ScopeIsPrivate);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_start_address", 0U, orc_DataPool.u32_NvmStartAddress);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_size", 0U, orc_DataPool.u32_NvmSize);

   QJsonArray c_Lists;
   if (C_OscJsonUtil::h_GetArray(orc_Json, "lists", c_Lists) == C_NO_ERR)
   {
      orc_DataPool.c_Lists.clear();
      orc_DataPool.c_Lists.reserve(c_Lists.size());
      for (const QJsonValue & rc_Value : c_Lists)
      {
         if (!rc_Value.isObject())
         {
            return C_CONFIG;
         }
         C_OscNodeDataPoolList c_List;
         s32_Result = C_OscNodeDataPoolListFiler::load(rc_Value.toObject(), c_List);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_DataPool.c_Lists.append(c_List);
      }
   }

   return C_NO_ERR;
}
