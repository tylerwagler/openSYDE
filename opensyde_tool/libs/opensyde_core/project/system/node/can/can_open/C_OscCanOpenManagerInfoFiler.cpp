//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerInfo (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerInfoFiler.hpp"
#include "C_OscCanOpenManagerDeviceInfoFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscCanOpenManagerInfo::E_NmtErrorBehaviourType> hac_NmtErrorTable[] = {
   { C_OscCanOpenManagerInfo::eRESTART_ALL_DEVICES,    "restart_all_devices"    },
   { C_OscCanOpenManagerInfo::eRESTART_FAILURE_DEVICE, "restart_failure_device" },
   { C_OscCanOpenManagerInfo::eSTOP_ALL_DEVICES,       "stop_all_devices"       }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerInfoFiler::save(const C_OscCanOpenManagerInfo & orc_Manager)
{
   QJsonObject c_Json;

   c_Json["use_opensyde_node_id"] = orc_Manager.q_UseOpenSydeNodeId;
   c_Json["node_id_value"] = static_cast<qint64>(orc_Manager.u8_NodeIdValue);
   c_Json["global_sdo_timeout_ms"] = static_cast<qint64>(orc_Manager.u16_GlobalSdoTimeoutMs);
   c_Json["autostart_can_open_manager"] = orc_Manager.q_AutostartCanOpenManager;
   c_Json["start_devices"] = orc_Manager.q_StartDevices;
   c_Json["nmt_start_all"] = orc_Manager.q_NmtStartAll;

   c_Json["nmt_error_behaviour"] =
      C_OscJsonUtil::h_EnumToString(orc_Manager.e_NmtErrorBehaviour, hac_NmtErrorTable);

   c_Json["enable_heartbeat_producing"] = orc_Manager.q_EnableHeartbeatProducing;
   c_Json["heartbeat_producer_time_ms"] = static_cast<qint64>(orc_Manager.u16_HeartbeatProducerTimeMs);

   c_Json["produce_sync_message"] = orc_Manager.q_ProduceSyncMessage;
   c_Json["sync_cycle_period_us"] = static_cast<qint64>(orc_Manager.u32_SyncCyclePeriodUs);
   c_Json["sync_window_length_us"] = static_cast<qint64>(orc_Manager.u32_SyncWindowLengthUs);

   QJsonArray c_Devices;
   for (auto c_It = orc_Manager.c_CanOpenDevices.constBegin();
        c_It != orc_Manager.c_CanOpenDevices.constEnd(); ++c_It)
   {
      QJsonObject c_Entry;
      c_Entry["device_node_index"] = static_cast<qint64>(c_It.key().u32_NodeIndex);
      c_Entry["device_interface_number"] = static_cast<qint64>(c_It.key().u8_InterfaceNumber);
      c_Entry["device"] = C_OscCanOpenManagerDeviceInfoFiler::save(c_It.value());
      c_Devices.append(c_Entry);
   }
   c_Json["can_open_devices"] = c_Devices;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerInfoFiler::load(const QJsonObject & orc_Json, C_OscCanOpenManagerInfo & orc_Manager)
{
   int32_t s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "use_opensyde_node_id", orc_Manager.q_UseOpenSydeNodeId);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "node_id_value", orc_Manager.u8_NodeIdValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "global_sdo_timeout_ms", orc_Manager.u16_GlobalSdoTimeoutMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "autostart_can_open_manager",
                                         orc_Manager.q_AutostartCanOpenManager);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "start_devices", orc_Manager.q_StartDevices);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "nmt_start_all", orc_Manager.q_NmtStartAll);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QString c_NmtStr;
   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "nmt_error_behaviour", c_NmtStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_NmtStr, hac_NmtErrorTable, orc_Manager.e_NmtErrorBehaviour);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "enable_heartbeat_producing",
                                         orc_Manager.q_EnableHeartbeatProducing);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "heartbeat_producer_time_ms",
                                        orc_Manager.u16_HeartbeatProducerTimeMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "produce_sync_message", orc_Manager.q_ProduceSyncMessage);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "sync_cycle_period_us", orc_Manager.u32_SyncCyclePeriodUs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "sync_window_length_us", orc_Manager.u32_SyncWindowLengthUs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_Devices;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "can_open_devices", c_Devices);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   orc_Manager.c_CanOpenDevices.clear();
   orc_Manager.c_CanOpenDevices.reserve(c_Devices.size());
   for (const QJsonValue & rc_Value : c_Devices)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      const QJsonObject c_Entry = rc_Value.toObject();

      C_OscCanInterfaceId c_Key;
      s32_Result = C_OscJsonUtil::h_GetU32(c_Entry, "device_node_index", c_Key.u32_NodeIndex);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      s32_Result = C_OscJsonUtil::h_GetU8(c_Entry, "device_interface_number", c_Key.u8_InterfaceNumber);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }

      QJsonObject c_DeviceObj;
      s32_Result = C_OscJsonUtil::h_GetObject(c_Entry, "device", c_DeviceObj);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      C_OscCanOpenManagerDeviceInfo c_Device;
      s32_Result = C_OscCanOpenManagerDeviceInfoFiler::load(c_DeviceObj, c_Device);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Manager.c_CanOpenDevices.insert(c_Key, c_Device);
   }

   return C_NO_ERR;
}
