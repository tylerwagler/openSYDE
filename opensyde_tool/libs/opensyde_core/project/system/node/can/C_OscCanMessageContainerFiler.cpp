//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanMessageContainer (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanMessageContainerFiler.hpp"
#include "C_OscCanMessageFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
QJsonArray mh_SaveMessageList(const QList<C_OscCanMessage> & orc_Messages)
{
   QJsonArray c_Json;
   for (const C_OscCanMessage & rc_Message : orc_Messages)
   {
      c_Json.append(C_OscCanMessageFiler::save(rc_Message));
   }
   return c_Json;
}

int32_t mh_LoadMessageList(const QJsonArray & orc_Json, QList<C_OscCanMessage> & orc_Messages)
{
   orc_Messages.clear();
   orc_Messages.reserve(orc_Json.size());
   for (const QJsonValue & rc_Value : orc_Json)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      C_OscCanMessage c_Message;
      const int32_t s32_Result = C_OscCanMessageFiler::load(rc_Value.toObject(), c_Message);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Messages.append(c_Message);
   }
   return C_NO_ERR;
}
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanMessageContainerFiler::save(const C_OscCanMessageContainer & orc_Container)
{
   QJsonObject c_Json;

   c_Json["is_protocol_used_by_interface"] = orc_Container.q_IsComProtocolUsedByInterface;
   c_Json["tx_messages"] = mh_SaveMessageList(orc_Container.c_TxMessages);
   c_Json["rx_messages"] = mh_SaveMessageList(orc_Container.c_RxMessages);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanMessageContainerFiler::load(const QJsonObject & orc_Json, C_OscCanMessageContainer & orc_Container)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "is_protocol_used_by_interface",
                                                 orc_Container.q_IsComProtocolUsedByInterface);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_TxArray;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "tx_messages", c_TxArray);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = mh_LoadMessageList(c_TxArray, orc_Container.c_TxMessages);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_RxArray;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "rx_messages", c_RxArray);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = mh_LoadMessageList(c_RxArray, orc_Container.c_RxMessages);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   return C_NO_ERR;
}
