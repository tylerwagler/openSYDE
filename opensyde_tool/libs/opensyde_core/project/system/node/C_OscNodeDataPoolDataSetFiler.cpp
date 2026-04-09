//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolDataSet (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolDataSetFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolDataSetFiler::save(const C_OscNodeDataPoolDataSet & orc_DataSet)
{
   QJsonObject c_Json;
   c_Json["name"] = orc_DataSet.c_Name;
   c_Json["comment"] = orc_DataSet.c_Comment;
   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolDataSetFiler::load(const QJsonObject & orc_Json, C_OscNodeDataPoolDataSet & orc_DataSet)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_DataSet.c_Name);
   if (s32_Result == C_NO_ERR)
   {
      C_OscJsonUtil::h_GetStringOr(orc_Json, "comment", QString(), orc_DataSet.c_Comment);
   }
   return s32_Result;
}
