//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscCanOpenManagerInfo

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscCanOpenManagerInfo.hpp"
#include "C_OscCanOpenManagerDeviceInfo.hpp"
#include "stwerrors.hpp"
#include <QJsonArray>
#include <QJsonValue>
#include <QDomElement>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerInfo::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << q_UseOpenSydeNodeId;
   ro_DataStream << static_cast<uint32_t>(u8_NodeIdValue);
   ro_DataStream << u16_GlobalSdoTimeoutMs;
   ro_DataStream << q_AutostartCanOpenManager;
   ro_DataStream << q_StartDevices;
   ro_DataStream << q_NmtStartAll;
   ro_DataStream << static_cast<int32_t>(e_NmtErrorBehaviour);
   ro_DataStream << q_EnableHeartbeatProducing;
   ro_DataStream << u16_HeartbeatProducerTimeMs;
   ro_DataStream << q_ProduceSyncMessage;
   ro_DataStream << u32_SyncCyclePeriodUs;
   ro_DataStream << u32_SyncWindowLengthUs;

   // Serialize CANopen devices
   ro_DataStream << static_cast<int32_t>(c_CanOpenDevices.size());
   for (auto c_Iter = c_CanOpenDevices.constBegin(); c_Iter != c_CanOpenDevices.constEnd(); ++c_Iter) {
      c_Iter.key().ToQDataStream(ro_DataStream);
      c_Iter.value().ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerInfo::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> q_UseOpenSydeNodeId;
   uint32_t u32_NodeId;
   ro_DataStream >> u32_NodeId;
   u8_NodeIdValue = static_cast<uint8_t>(u32_NodeId);
   ro_DataStream >> u16_GlobalSdoTimeoutMs;
   ro_DataStream >> q_AutostartCanOpenManager;
   ro_DataStream >> q_StartDevices;
   ro_DataStream >> q_NmtStartAll;
   int32_t s32_ErrorBehaviour;
   ro_DataStream >> s32_ErrorBehaviour;
   e_NmtErrorBehaviour = static_cast<E_NmtErrorBehaviourType>(s32_ErrorBehaviour);
   ro_DataStream >> q_EnableHeartbeatProducing;
   ro_DataStream >> u16_HeartbeatProducerTimeMs;
   ro_DataStream >> q_ProduceSyncMessage;
   ro_DataStream >> u32_SyncCyclePeriodUs;
   ro_DataStream >> u32_SyncWindowLengthUs;

   // Deserialize CANopen devices
   int32_t s32_DeviceCount;
   ro_DataStream >> s32_DeviceCount;
   c_CanOpenDevices.clear();
   for (int32_t s32_It = 0; s32_It < s32_DeviceCount; ++s32_It) {
      C_OscCanInterfaceId c_InterfaceId;
      C_OscCanOpenManagerDeviceInfo c_Device;
      c_InterfaceId.FromQDataStream(ro_DataStream);
      c_Device.FromQDataStream(ro_DataStream);
      c_CanOpenDevices.insert(c_InterfaceId, c_Device);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerInfo::ToJsonObject() const {
   QJsonObject c_Obj;

   c_Obj["useOpenSydeNodeId"] = q_UseOpenSydeNodeId;
   c_Obj["nodeIdValue"] = static_cast<int32_t>(u8_NodeIdValue);
   c_Obj["globalSdoTimeoutMs"] = static_cast<int32_t>(u16_GlobalSdoTimeoutMs);
   c_Obj["autostartCanOpenManager"] = q_AutostartCanOpenManager;
   c_Obj["startDevices"] = q_StartDevices;
   c_Obj["nmtStartAll"] = q_NmtStartAll;
   c_Obj["nmtErrorBehaviour"] = static_cast<int32_t>(e_NmtErrorBehaviour);
   c_Obj["enableHeartbeatProducing"] = q_EnableHeartbeatProducing;
   c_Obj["heartbeatProducerTimeMs"] = static_cast<int32_t>(u16_HeartbeatProducerTimeMs);
   c_Obj["produceSyncMessage"] = q_ProduceSyncMessage;
   c_Obj["syncCyclePeriodUs"] = static_cast<double>(u32_SyncCyclePeriodUs);
   c_Obj["syncWindowLengthUs"] = static_cast<double>(u32_SyncWindowLengthUs);

   // Serialize CANopen devices
   QJsonArray c_DeviceArray;
   for (auto c_Iter = c_CanOpenDevices.constBegin(); c_Iter != c_CanOpenDevices.constEnd(); ++c_Iter) {
      QJsonObject c_DeviceObj;
      c_DeviceObj["interfaceId"] = c_Iter.key().ToJsonObject();
      c_DeviceObj["deviceInfo"] = c_Iter.value().ToJsonObject();
      c_DeviceArray.append(c_DeviceObj);
   }
   c_Obj["canOpenDevices"] = c_DeviceArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerInfo::FromJsonObject(const QJsonObject &orc_Object) {
   if (orc_Object.contains("useOpenSydeNodeId")) {
      q_UseOpenSydeNodeId = orc_Object["useOpenSydeNodeId"].toBool();
   }
   if (orc_Object.contains("nodeIdValue")) {
      u8_NodeIdValue = static_cast<uint8_t>(orc_Object["nodeIdValue"].toInt());
   }
   if (orc_Object.contains("globalSdoTimeoutMs")) {
      u16_GlobalSdoTimeoutMs = static_cast<uint16_t>(orc_Object["globalSdoTimeoutMs"].toInt());
   }
   if (orc_Object.contains("autostartCanOpenManager")) {
      q_AutostartCanOpenManager = orc_Object["autostartCanOpenManager"].toBool();
   }
   if (orc_Object.contains("startDevices")) {
      q_StartDevices = orc_Object["startDevices"].toBool();
   }
   if (orc_Object.contains("nmtStartAll")) {
      q_NmtStartAll = orc_Object["nmtStartAll"].toBool();
   }
   if (orc_Object.contains("nmtErrorBehaviour")) {
      e_NmtErrorBehaviour = static_cast<E_NmtErrorBehaviourType>(orc_Object["nmtErrorBehaviour"].toInt());
   }
   if (orc_Object.contains("enableHeartbeatProducing")) {
      q_EnableHeartbeatProducing = orc_Object["enableHeartbeatProducing"].toBool();
   }
   if (orc_Object.contains("heartbeatProducerTimeMs")) {
      u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(orc_Object["heartbeatProducerTimeMs"].toInt());
   }
   if (orc_Object.contains("produceSyncMessage")) {
      q_ProduceSyncMessage = orc_Object["produceSyncMessage"].toBool();
   }
   if (orc_Object.contains("syncCyclePeriodUs")) {
      u32_SyncCyclePeriodUs = static_cast<uint32_t>(orc_Object["syncCyclePeriodUs"].toInteger());
   }
   if (orc_Object.contains("syncWindowLengthUs")) {
      u32_SyncWindowLengthUs = static_cast<uint32_t>(orc_Object["syncWindowLengthUs"].toInteger());
   }

   // Deserialize CANopen devices
   if (orc_Object.contains("canOpenDevices")) {
      c_CanOpenDevices.clear();
      const QJsonArray c_DeviceArray = orc_Object["canOpenDevices"].toArray();
      for (const QJsonValue &c_Value : c_DeviceArray) {
         QJsonObject c_DeviceObj = c_Value.toObject();
         C_OscCanInterfaceId c_InterfaceId;
         C_OscCanOpenManagerDeviceInfo c_Device;
         if (c_DeviceObj.contains("interfaceId")) {
            c_InterfaceId.FromJsonObject(c_DeviceObj["interfaceId"].toObject());
         }
         if (c_DeviceObj.contains("deviceInfo")) {
            c_Device.FromJsonObject(c_DeviceObj["deviceInfo"].toObject());
         }
         c_CanOpenDevices.insert(c_InterfaceId, c_Device);
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscCanOpenManagerInfo::ToQDomElement(QDomDocument &orc_Doc,
                                                   const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields
   QDomElement c_UseOpenSydeElement = orc_Doc.createElement("useOpenSydeNodeId");
   c_UseOpenSydeElement.appendChild(orc_Doc.createTextNode(QString::number(q_UseOpenSydeNodeId ? 1 : 0)));
   c_Element.appendChild(c_UseOpenSydeElement);

   QDomElement c_NodeIdElement = orc_Doc.createElement("nodeIdValue");
   c_NodeIdElement.appendChild(orc_Doc.createTextNode(QString::number(u8_NodeIdValue)));
   c_Element.appendChild(c_NodeIdElement);

   QDomElement c_SdoTimeoutElement = orc_Doc.createElement("globalSdoTimeoutMs");
   c_SdoTimeoutElement.appendChild(orc_Doc.createTextNode(QString::number(u16_GlobalSdoTimeoutMs)));
   c_Element.appendChild(c_SdoTimeoutElement);

   QDomElement c_AutostartElement = orc_Doc.createElement("autostartCanOpenManager");
   c_AutostartElement.appendChild(orc_Doc.createTextNode(QString::number(q_AutostartCanOpenManager ? 1 : 0)));
   c_Element.appendChild(c_AutostartElement);

   QDomElement c_StartDevicesElement = orc_Doc.createElement("startDevices");
   c_StartDevicesElement.appendChild(orc_Doc.createTextNode(QString::number(q_StartDevices ? 1 : 0)));
   c_Element.appendChild(c_StartDevicesElement);

   QDomElement c_NmtStartAllElement = orc_Doc.createElement("nmtStartAll");
   c_NmtStartAllElement.appendChild(orc_Doc.createTextNode(QString::number(q_NmtStartAll ? 1 : 0)));
   c_Element.appendChild(c_NmtStartAllElement);

   QDomElement c_ErrorBehaviourElement = orc_Doc.createElement("nmtErrorBehaviour");
   QString c_ErrorBehaviourStr;
   switch (e_NmtErrorBehaviour) {
      case eRESTART_ALL_DEVICES:
         c_ErrorBehaviourStr = "restart-all";
         break;
      case eRESTART_FAILURE_DEVICE:
         c_ErrorBehaviourStr = "restart-failure";
         break;
      case eSTOP_ALL_DEVICES:
         c_ErrorBehaviourStr = "stop-all";
         break;
      default:
         c_ErrorBehaviourStr = "unknown";
         break;
   }
   c_ErrorBehaviourElement.appendChild(orc_Doc.createTextNode(c_ErrorBehaviourStr));
   c_Element.appendChild(c_ErrorBehaviourElement);

   QDomElement c_HeartbeatElement = orc_Doc.createElement("enableHeartbeatProducing");
   c_HeartbeatElement.appendChild(orc_Doc.createTextNode(QString::number(q_EnableHeartbeatProducing ? 1 : 0)));
   c_Element.appendChild(c_HeartbeatElement);

   QDomElement c_HeartbeatTimeElement = orc_Doc.createElement("heartbeatProducerTimeMs");
   c_HeartbeatTimeElement.appendChild(orc_Doc.createTextNode(QString::number(u16_HeartbeatProducerTimeMs)));
   c_Element.appendChild(c_HeartbeatTimeElement);

   QDomElement c_SyncElement = orc_Doc.createElement("produceSyncMessage");
   c_SyncElement.appendChild(orc_Doc.createTextNode(QString::number(q_ProduceSyncMessage ? 1 : 0)));
   c_Element.appendChild(c_SyncElement);

   QDomElement c_SyncCycleElement = orc_Doc.createElement("syncCyclePeriodUs");
   c_SyncCycleElement.appendChild(orc_Doc.createTextNode(QString::number(u32_SyncCyclePeriodUs)));
   c_Element.appendChild(c_SyncCycleElement);

   QDomElement c_SyncWindowElement = orc_Doc.createElement("syncWindowLengthUs");
   c_SyncWindowElement.appendChild(orc_Doc.createTextNode(QString::number(u32_SyncWindowLengthUs)));
   c_Element.appendChild(c_SyncWindowElement);

   // Serialize CANopen devices
   QDomElement c_DevicesElement = orc_Doc.createElement("canOpenDevices");
   for (auto c_Iter = c_CanOpenDevices.constBegin(); c_Iter != c_CanOpenDevices.constEnd(); ++c_Iter) {
      QDomElement c_DeviceElement = c_Iter.value().ToQDomElement(orc_Doc, "device");
      QDomElement c_InterfaceElement = c_Iter.key().ToQDomDocument(orc_Doc, "interfaceId");
      c_DeviceElement.appendChild(c_InterfaceElement);
      c_DevicesElement.appendChild(c_DeviceElement);
   }
   c_Element.appendChild(c_DevicesElement);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenManagerInfo::FromQDomElement(const QDomElement &orc_Element) {
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull()) {
         const QString c_TagName = c_Elem.tagName();

         if (c_TagName == "useOpenSydeNodeId") {
            q_UseOpenSydeNodeId = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "nodeIdValue") {
            u8_NodeIdValue = static_cast<uint8_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "globalSdoTimeoutMs") {
            u16_GlobalSdoTimeoutMs = static_cast<uint16_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "autostartCanOpenManager") {
            q_AutostartCanOpenManager = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "startDevices") {
            q_StartDevices = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "nmtStartAll") {
            q_NmtStartAll = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "nmtErrorBehaviour") {
            const QString c_Str = c_Elem.text();
            if (c_Str == "restart-all") {
               e_NmtErrorBehaviour = eRESTART_ALL_DEVICES;
            } else if (c_Str == "restart-failure") {
               e_NmtErrorBehaviour = eRESTART_FAILURE_DEVICE;
            } else if (c_Str == "stop-all") {
               e_NmtErrorBehaviour = eSTOP_ALL_DEVICES;
            }
         } else if (c_TagName == "enableHeartbeatProducing") {
            q_EnableHeartbeatProducing = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "heartbeatProducerTimeMs") {
            u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "produceSyncMessage") {
            q_ProduceSyncMessage = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "syncCyclePeriodUs") {
            u32_SyncCyclePeriodUs = c_Elem.text().toUInt();
         } else if (c_TagName == "syncWindowLengthUs") {
            u32_SyncWindowLengthUs = c_Elem.text().toUInt();
         } else if (c_TagName == "canOpenDevices") {
            QDomNode c_DeviceNode = c_Elem.firstChild();
            while (!c_DeviceNode.isNull()) {
               QDomElement c_DeviceElem = c_DeviceNode.toElement();
               if (!c_DeviceElem.isNull() && c_DeviceElem.tagName() == "device") {
                  C_OscCanInterfaceId c_InterfaceId;
                  C_OscCanOpenManagerDeviceInfo c_Device;
                  QDomNode c_InterfaceNode = c_DeviceElem.firstChild();
                  while (!c_InterfaceNode.isNull()) {
                     QDomElement c_InterfaceElem = c_InterfaceNode.toElement();
                     if (!c_InterfaceElem.isNull() && c_InterfaceElem.tagName() == "interfaceId") {
                        c_InterfaceId.FromQDomDocument(c_InterfaceElem);
                        break;
                     }
                     c_InterfaceNode = c_InterfaceNode.nextSibling();
                  }
                  c_Device.FromQDomElement(c_DeviceElem);
                  c_CanOpenDevices.insert(c_InterfaceId, c_Device);
               }
               c_DeviceNode = c_DeviceNode.nextSibling();
            }
         }
      }
      c_Node = c_Node.nextSibling();
   }

   return C_NO_ERR;
}
