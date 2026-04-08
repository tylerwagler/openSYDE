//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for the CANopen managers attached to a node (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerFiler.hpp"
#include "C_OscCanOpenManagerInfoFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerFiler::save(const QHash<uint8_t, C_OscCanOpenManagerInfo> & orc_Managers)
{
   QJsonObject c_Json;
   QJsonArray c_Array;

   for (auto c_It = orc_Managers.constBegin(); c_It != orc_Managers.constEnd(); ++c_It)
   {
      QJsonObject c_Entry;
      c_Entry["interface_number"] = static_cast<qint64>(c_It.key());
      c_Entry["manager"] = C_OscCanOpenManagerInfoFiler::save(c_It.value());
      c_Array.append(c_Entry);
   }
   c_Json["managers"] = c_Array;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerFiler::load(const QJsonObject & orc_Json,
                                       QHash<uint8_t, C_OscCanOpenManagerInfo> & orc_Managers)
{
   QJsonArray c_Array;
   int32_t s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "managers", c_Array);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   orc_Managers.clear();
   orc_Managers.reserve(c_Array.size());
   for (const QJsonValue & rc_Value : c_Array)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      const QJsonObject c_Entry = rc_Value.toObject();

      uint8_t u8_InterfaceNumber = 0U;
      s32_Result = C_OscJsonUtil::h_GetU8(c_Entry, "interface_number", u8_InterfaceNumber);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }

      QJsonObject c_ManagerObj;
      s32_Result = C_OscJsonUtil::h_GetObject(c_Entry, "manager", c_ManagerObj);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }

      C_OscCanOpenManagerInfo c_Manager;
      s32_Result = C_OscCanOpenManagerInfoFiler::load(c_ManagerObj, c_Manager);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Managers.insert(u8_InterfaceNumber, c_Manager);
   }

   return C_NO_ERR;
}
