//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolList (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolListFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolDataSetFiler.hpp"
#include "C_OscNodeDataPoolListElementFiler.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolListFiler::save(const C_OscNodeDataPoolList & orc_List)
{
   QJsonObject c_Json;
   c_Json["name"] = orc_List.c_Name;
   c_Json["comment"] = orc_List.c_Comment;
   c_Json["nvm_crc_active"] = orc_List.q_NvmCrcActive;
   c_Json["nvm_crc"] = static_cast<qint64>(orc_List.u32_NvmCrc);
   c_Json["nvm_start_address"] = static_cast<qint64>(orc_List.u32_NvmStartAddress);
   c_Json["nvm_size"] = static_cast<qint64>(orc_List.u32_NvmSize);

   QJsonArray c_Elements;
   for (const C_OscNodeDataPoolListElement & rc_Element : orc_List.c_Elements)
   {
      c_Elements.append(C_OscNodeDataPoolListElementFiler::save(rc_Element));
   }
   c_Json["elements"] = c_Elements;

   QJsonArray c_DataSets;
   for (const C_OscNodeDataPoolDataSet & rc_DataSet : orc_List.c_DataSets)
   {
      c_DataSets.append(C_OscNodeDataPoolDataSetFiler::save(rc_DataSet));
   }
   c_Json["data_sets"] = c_DataSets;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolListFiler::load(const QJsonObject & orc_Json, C_OscNodeDataPoolList & orc_List)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_List.c_Name);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   C_OscJsonUtil::h_GetStringOr(orc_Json, "comment", QString(), orc_List.c_Comment);
   C_OscJsonUtil::h_GetBoolOr(orc_Json, "nvm_crc_active", false, orc_List.q_NvmCrcActive);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_crc", 0U, orc_List.u32_NvmCrc);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_start_address", 0U, orc_List.u32_NvmStartAddress);
   C_OscJsonUtil::h_GetU32Or(orc_Json, "nvm_size", 0U, orc_List.u32_NvmSize);

   QJsonArray c_Elements;
   if (C_OscJsonUtil::h_GetArray(orc_Json, "elements", c_Elements) == C_NO_ERR)
   {
      orc_List.c_Elements.clear();
      orc_List.c_Elements.reserve(c_Elements.size());
      for (const QJsonValue & rc_Value : c_Elements)
      {
         if (!rc_Value.isObject())
         {
            return C_CONFIG;
         }
         C_OscNodeDataPoolListElement c_Element;
         s32_Result = C_OscNodeDataPoolListElementFiler::load(rc_Value.toObject(), c_Element);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_List.c_Elements.append(c_Element);
      }
   }

   QJsonArray c_DataSets;
   if (C_OscJsonUtil::h_GetArray(orc_Json, "data_sets", c_DataSets) == C_NO_ERR)
   {
      orc_List.c_DataSets.clear();
      orc_List.c_DataSets.reserve(c_DataSets.size());
      for (const QJsonValue & rc_Value : c_DataSets)
      {
         if (!rc_Value.isObject())
         {
            return C_CONFIG;
         }
         C_OscNodeDataPoolDataSet c_DataSet;
         s32_Result = C_OscNodeDataPoolDataSetFiler::load(rc_Value.toObject(), c_DataSet);
         if (s32_Result != C_NO_ERR)
         {
            return s32_Result;
         }
         orc_List.c_DataSets.append(c_DataSet);
      }
   }

   return C_NO_ERR;
}
