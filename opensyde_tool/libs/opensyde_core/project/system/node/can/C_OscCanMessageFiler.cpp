//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanMessage (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanMessageFiler.hpp"
#include "C_OscCanSignalFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscCanMessage::E_TxMethodType> hac_TxMethodTable[] = {
   { C_OscCanMessage::eTX_METHOD_CYCLIC,             "cyclic"                 },
   { C_OscCanMessage::eTX_METHOD_ON_CHANGE,          "on_change"              },
   { C_OscCanMessage::eTX_METHOD_ON_EVENT,           "on_event"               },
   { C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_0,    "can_open_type_0"        },
   { C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_1_TO_240, "can_open_type_1_to_240" },
   { C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_254,  "can_open_type_254"      },
   { C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_255,  "can_open_type_255"      }
};

QJsonObject mh_SaveOwnerInterface(const C_OscCanInterfaceId & orc_Id)
{
   QJsonObject c_Json;
   c_Json["node_index"] = static_cast<qint64>(orc_Id.u32_NodeIndex);
   c_Json["interface_number"] = static_cast<qint64>(orc_Id.u8_InterfaceNumber);
   return c_Json;
}

int32_t mh_LoadOwnerInterface(const QJsonObject & orc_Json, C_OscCanInterfaceId & orc_Id)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "node_index", orc_Id.u32_NodeIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   return C_OscJsonUtil::h_GetU8(orc_Json, "interface_number", orc_Id.u8_InterfaceNumber);
}
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanMessageFiler::save(const C_OscCanMessage & orc_Message)
{
   QJsonObject c_Json;

   c_Json["name"] = orc_Message.c_Name;
   c_Json["comment"] = orc_Message.c_Comment;
   c_Json["can_id"] = static_cast<qint64>(orc_Message.u32_CanId);
   c_Json["is_extended"] = orc_Message.q_IsExtended;
   c_Json["dlc"] = static_cast<qint64>(orc_Message.u16_Dlc);
   c_Json["tx_method"] = C_OscJsonUtil::h_EnumToString(orc_Message.e_TxMethod, hac_TxMethodTable);
   c_Json["cycle_time_ms"] = static_cast<qint64>(orc_Message.u32_CycleTimeMs);
   c_Json["delay_time_ms"] = static_cast<qint64>(orc_Message.u16_DelayTimeMs);
   c_Json["timeout_ms"] = static_cast<qint64>(orc_Message.u32_TimeoutMs);

   QJsonArray c_Signals;
   for (const C_OscCanSignal & rc_Signal : orc_Message.c_Signals)
   {
      c_Signals.append(C_OscCanSignalFiler::save(rc_Signal));
   }
   c_Json["signals"] = c_Signals;

   c_Json["can_open_owner_interface"] = mh_SaveOwnerInterface(orc_Message.c_CanOpenManagerOwnerNodeIndex);
   c_Json["can_open_cob_id_includes_node_id"] = orc_Message.q_CanOpenManagerCobIdIncludesNodeId;
   c_Json["can_open_cob_id_offset"] = static_cast<qint64>(orc_Message.u32_CanOpenManagerCobIdOffset);
   c_Json["can_open_message_active"] = orc_Message.q_CanOpenManagerMessageActive;
   c_Json["can_open_pdo_index"] = static_cast<qint64>(orc_Message.u16_CanOpenManagerPdoIndex);
   c_Json["can_open_tx_method_additional_info"] = static_cast<qint64>(orc_Message.u8_CanOpenTxMethodAdditionalInfo);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanMessageFiler::load(const QJsonObject & orc_Json, C_OscCanMessage & orc_Message)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "name", orc_Message.c_Name);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   C_OscJsonUtil::h_GetStringOr(orc_Json, "comment", QString(), orc_Message.c_Comment);

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "can_id", orc_Message.u32_CanId);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "is_extended", orc_Message.q_IsExtended);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "dlc", orc_Message.u16_Dlc);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QString c_TxMethodStr;
   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "tx_method", c_TxMethodStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_TxMethodStr, hac_TxMethodTable, orc_Message.e_TxMethod);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "cycle_time_ms", orc_Message.u32_CycleTimeMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "delay_time_ms", orc_Message.u16_DelayTimeMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "timeout_ms", orc_Message.u32_TimeoutMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_Signals;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "signals", c_Signals);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   orc_Message.c_Signals.clear();
   orc_Message.c_Signals.reserve(c_Signals.size());
   for (const QJsonValue & rc_Value : c_Signals)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      C_OscCanSignal c_Signal;
      s32_Result = C_OscCanSignalFiler::load(rc_Value.toObject(), c_Signal);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Message.c_Signals.append(c_Signal);
   }

   QJsonObject c_OwnerObj;
   s32_Result = C_OscJsonUtil::h_GetObject(orc_Json, "can_open_owner_interface", c_OwnerObj);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = mh_LoadOwnerInterface(c_OwnerObj, orc_Message.c_CanOpenManagerOwnerNodeIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "can_open_cob_id_includes_node_id",
                                         orc_Message.q_CanOpenManagerCobIdIncludesNodeId);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "can_open_cob_id_offset",
                                        orc_Message.u32_CanOpenManagerCobIdOffset);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "can_open_message_active",
                                         orc_Message.q_CanOpenManagerMessageActive);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "can_open_pdo_index",
                                        orc_Message.u16_CanOpenManagerPdoIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "can_open_tx_method_additional_info",
                                       orc_Message.u8_CanOpenTxMethodAdditionalInfo);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   return C_NO_ERR;
}
