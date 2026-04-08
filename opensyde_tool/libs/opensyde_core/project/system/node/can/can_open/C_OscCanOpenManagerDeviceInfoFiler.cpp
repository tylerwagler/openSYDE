//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerDeviceInfo (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerDeviceInfoFiler.hpp"
#include "C_OscCanOpenManagerMappableSignalFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonArray>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerDeviceInfoFiler::save(const C_OscCanOpenManagerDeviceInfo & orc_Device)
{
   QJsonObject c_Json;

   c_Json["project_eds_file_path"] = orc_Device.c_ProjectEdsFilePath;
   c_Json["original_eds_file_name"] = orc_Device.c_OriginalEdsFileName;

   c_Json["device_optional"] = orc_Device.q_DeviceOptional;
   c_Json["no_initialization"] = orc_Device.q_NoInitialization;
   c_Json["factory_settings_active"] = orc_Device.q_FactorySettingsActive;
   c_Json["reset_node_object_dictionary_sub_index"] =
      static_cast<qint64>(orc_Device.u8_ResetNodeObjectDictionarySubIndex);

   c_Json["enable_heartbeat_producing"] = orc_Device.q_EnableHeartbeatProducing;
   c_Json["heartbeat_producer_time_ms"] = static_cast<qint64>(orc_Device.u16_HeartbeatProducerTimeMs);

   c_Json["use_opensyde_node_id"] = orc_Device.q_UseOpenSydeNodeId;
   c_Json["node_id_value"] = static_cast<qint64>(orc_Device.u8_NodeIdValue);

   c_Json["enable_heartbeat_consuming"] = orc_Device.q_EnableHeartbeatConsuming;
   c_Json["heartbeat_consumer_time_ms"] = static_cast<qint64>(orc_Device.u16_HeartbeatConsumerTimeMs);
   c_Json["enable_heartbeat_consuming_auto_calculation"] = orc_Device.q_EnableHeartbeatConsumingAutoCalculation;

   QJsonArray c_Signals;
   for (const C_OscCanOpenManagerMappableSignal & rc_Signal : orc_Device.c_EdsFileMappableSignals)
   {
      c_Signals.append(C_OscCanOpenManagerMappableSignalFiler::save(rc_Signal));
   }
   c_Json["eds_file_mappable_signals"] = c_Signals;

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerDeviceInfoFiler::load(const QJsonObject & orc_Json,
                                                 C_OscCanOpenManagerDeviceInfo & orc_Device)
{
   C_OscJsonUtil::h_GetStringOr(orc_Json, "project_eds_file_path", QString(), orc_Device.c_ProjectEdsFilePath);
   C_OscJsonUtil::h_GetStringOr(orc_Json, "original_eds_file_name", QString(), orc_Device.c_OriginalEdsFileName);

   int32_t s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "device_optional", orc_Device.q_DeviceOptional);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "no_initialization", orc_Device.q_NoInitialization);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "factory_settings_active", orc_Device.q_FactorySettingsActive);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "reset_node_object_dictionary_sub_index",
                                       orc_Device.u8_ResetNodeObjectDictionarySubIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "enable_heartbeat_producing",
                                         orc_Device.q_EnableHeartbeatProducing);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "heartbeat_producer_time_ms",
                                        orc_Device.u16_HeartbeatProducerTimeMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "use_opensyde_node_id", orc_Device.q_UseOpenSydeNodeId);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "node_id_value", orc_Device.u8_NodeIdValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "enable_heartbeat_consuming",
                                         orc_Device.q_EnableHeartbeatConsuming);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "heartbeat_consumer_time_ms",
                                        orc_Device.u16_HeartbeatConsumerTimeMs);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "enable_heartbeat_consuming_auto_calculation",
                                         orc_Device.q_EnableHeartbeatConsumingAutoCalculation);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   QJsonArray c_Signals;
   s32_Result = C_OscJsonUtil::h_GetArray(orc_Json, "eds_file_mappable_signals", c_Signals);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   orc_Device.c_EdsFileMappableSignals.clear();
   orc_Device.c_EdsFileMappableSignals.reserve(c_Signals.size());
   for (const QJsonValue & rc_Value : c_Signals)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      C_OscCanOpenManagerMappableSignal c_Signal;
      s32_Result = C_OscCanOpenManagerMappableSignalFiler::load(rc_Value.toObject(), c_Signal);
      if (s32_Result != C_NO_ERR)
      {
         return s32_Result;
      }
      orc_Device.c_EdsFileMappableSignals.append(c_Signal);
   }

   return C_NO_ERR;
}
