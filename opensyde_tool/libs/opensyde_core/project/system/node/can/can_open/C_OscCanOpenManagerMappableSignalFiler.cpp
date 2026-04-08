//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerMappableSignal (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerMappableSignalFiler.hpp"
#include "C_OscCanSignalFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "C_OscNodeDataPoolListElementFiler.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerMappableSignalFiler::save(const C_OscCanOpenManagerMappableSignal & orc_Signal)
{
   QJsonObject c_Json;

   c_Json["auto_min_max_used"] = orc_Signal.q_AutoMinMaxUsed;
   c_Json["signal"] = C_OscCanSignalFiler::save(orc_Signal.c_SignalData);
   c_Json["datapool_element"] = C_OscNodeDataPoolListElementFiler::save(orc_Signal.c_DatapoolData);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerMappableSignalFiler::load(const QJsonObject & orc_Json,
                                                     C_OscCanOpenManagerMappableSignal & orc_Signal)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "auto_min_max_used", orc_Signal.q_AutoMinMaxUsed);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonObject c_SubObject;
   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "signal", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscCanSignalFiler::load(c_SubObject, orc_Signal.c_SignalData);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "datapool_element", c_SubObject);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscNodeDataPoolListElementFiler::load(c_SubObject, orc_Signal.c_DatapoolData);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   return C_NO_ERR;
}
