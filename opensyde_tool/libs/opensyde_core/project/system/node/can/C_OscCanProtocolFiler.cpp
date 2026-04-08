//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanProtocol (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanProtocolFiler.hpp"
#include "C_OscCanMessageContainerFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscCanProtocol::E_Type> hac_ProtocolTypeTable[] = {
   { C_OscCanProtocol::eLAYER2,          "layer2"          },
   { C_OscCanProtocol::eCAN_OPEN_SAFETY, "can_open_safety" },
   { C_OscCanProtocol::eECES,            "eces"            },
   { C_OscCanProtocol::eCAN_OPEN,        "can_open"        },
   { C_OscCanProtocol::eJ1939,           "j1939"           }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanProtocolFiler::save(const C_OscCanProtocol & orc_Protocol)
{
   QJsonObject c_Json;

   c_Json["protocol_type"] = C_OscJsonUtil::h_EnumToString(orc_Protocol.e_Type, hac_ProtocolTypeTable);
   c_Json["data_pool_index"] = static_cast<qint64>(orc_Protocol.u32_DataPoolIndex);

   QJsonArray c_Containers;
   for (const C_OscCanMessageContainer & rc_Container : orc_Protocol.c_ComMessages)
   {
      c_Containers.append(C_OscCanMessageContainerFiler::save(rc_Container));
   }
   c_Json["interface_containers"] = c_Containers;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanProtocolFiler::load(const QJsonObject & orc_Json, C_OscCanProtocol & orc_Protocol)
{
   QString c_TypeStr;
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "protocol_type", c_TypeStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_TypeStr, hac_ProtocolTypeTable, orc_Protocol.e_Type);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "data_pool_index", orc_Protocol.u32_DataPoolIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_Containers;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "interface_containers", c_Containers);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   orc_Protocol.c_ComMessages.clear();
   orc_Protocol.c_ComMessages.reserve(c_Containers.size());
   for (const QJsonValue & rc_Value : c_Containers)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      C_OscCanMessageContainer c_Container;
      s32_Result = C_OscCanMessageContainerFiler::load(rc_Value.toObject(), c_Container);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Protocol.c_ComMessages.append(c_Container);
   }

   return C_NO_ERR;
}
