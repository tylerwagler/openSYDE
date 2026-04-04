//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscCanOpenManagerDeviceInfo

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscCanOpenManagerDeviceInfo.hpp"
#include "C_OscCanOpenObjectDictionary.hpp"
#include <QJsonArray>

using namespace stw::opensyde_core;

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << c_ProjectEdsFilePath;
   ro_DataStream << c_OriginalEdsFileName;

   // Serialize mappable signals
   ro_DataStream << static_cast<int32_t>(c_EdsFileMappableSignals.size());
   for (const C_OscCanOpenManagerMappableSignal &rc_Signal : c_EdsFileMappableSignals) {
      rc_Signal.ToQDataStream(ro_DataStream);
   }

   ro_DataStream << q_DeviceOptional;
   ro_DataStream << q_NoInitialization;
   ro_DataStream << q_FactorySettingsActive;
   ro_DataStream << static_cast<uint32_t>(u8_ResetNodeObjectDictionarySubIndex);
   ro_DataStream << q_EnableHeartbeatProducing;
   ro_DataStream << static_cast<uint32_t>(u16_HeartbeatProducerTimeMs);
   ro_DataStream << q_UseOpenSydeNodeId;
   ro_DataStream << static_cast<uint32_t>(u8_NodeIdValue);
   ro_DataStream << q_EnableHeartbeatConsuming;
   ro_DataStream << static_cast<uint32_t>(u16_HeartbeatConsumerTimeMs);
   ro_DataStream << q_EnableHeartbeatConsumingAutoCalculation;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> c_ProjectEdsFilePath;
   ro_DataStream >> c_OriginalEdsFileName;

   // Deserialize mappable signals
   int32_t s32_SignalCount;
   ro_DataStream >> s32_SignalCount;
   c_EdsFileMappableSignals.clear();
   c_EdsFileMappableSignals.reserve(s32_SignalCount);
   for (int32_t s32_It = 0; s32_It < s32_SignalCount; ++s32_It) {
      C_OscCanOpenManagerMappableSignal c_Signal;
      c_Signal.FromQDataStream(ro_DataStream);
      c_EdsFileMappableSignals.append(c_Signal);
   }

   ro_DataStream >> q_DeviceOptional;
   ro_DataStream >> q_NoInitialization;
   ro_DataStream >> q_FactorySettingsActive;
   uint32_t u32_ResetSubIndex;
   ro_DataStream >> u32_ResetSubIndex;
   u8_ResetNodeObjectDictionarySubIndex = static_cast<uint8_t>(u32_ResetSubIndex);
   ro_DataStream >> q_EnableHeartbeatProducing;
   uint32_t u32_HeartbeatProducer;
   ro_DataStream >> u32_HeartbeatProducer;
   u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(u32_HeartbeatProducer);
   ro_DataStream >> q_UseOpenSydeNodeId;
   uint32_t u32_NodeId;
   ro_DataStream >> u32_NodeId;
   u8_NodeIdValue = static_cast<uint8_t>(u32_NodeId);
   ro_DataStream >> q_EnableHeartbeatConsuming;
   uint32_t u32_HeartbeatConsumer;
   ro_DataStream >> u32_HeartbeatConsumer;
   u16_HeartbeatConsumerTimeMs = static_cast<uint16_t>(u32_HeartbeatConsumer);
   ro_DataStream >> q_EnableHeartbeatConsumingAutoCalculation;
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerDeviceInfo::ToJsonObject() const {
   QJsonObject c_Obj;

   c_Obj["projectEdsFilePath"] = c_ProjectEdsFilePath;
   c_Obj["originalEdsFileName"] = c_OriginalEdsFileName;

   // Serialize mappable signals
   QJsonArray c_SignalArray;
   for (const C_OscCanOpenManagerMappableSignal &rc_Signal : c_EdsFileMappableSignals) {
      c_SignalArray.append(rc_Signal.ToJsonObject());
   }
   c_Obj["edsFileMappableSignals"] = c_SignalArray;

   c_Obj["deviceOptional"] = q_DeviceOptional;
   c_Obj["noInitialization"] = q_NoInitialization;
   c_Obj["factorySettingsActive"] = q_FactorySettingsActive;
   c_Obj["resetNodeObjectDictionarySubIndex"] = static_cast<int32_t>(u8_ResetNodeObjectDictionarySubIndex);
   c_Obj["enableHeartbeatProducing"] = q_EnableHeartbeatProducing;
   c_Obj["heartbeatProducerTimeMs"] = static_cast<int32_t>(u16_HeartbeatProducerTimeMs);
   c_Obj["useOpenSydeNodeId"] = q_UseOpenSydeNodeId;
   c_Obj["nodeIdValue"] = static_cast<int32_t>(u8_NodeIdValue);
   c_Obj["enableHeartbeatConsuming"] = q_EnableHeartbeatConsuming;
   c_Obj["heartbeatConsumerTimeMs"] = static_cast<int32_t>(u16_HeartbeatConsumerTimeMs);
   c_Obj["enableHeartbeatConsumingAutoCalculation"] = q_EnableHeartbeatConsumingAutoCalculation;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::FromJsonObject(const QJsonObject &ro_Json) {
   if (ro_Json.contains("projectEdsFilePath")) {
      c_ProjectEdsFilePath = ro_Json["projectEdsFilePath"].toString();
   }
   if (ro_Json.contains("originalEdsFileName")) {
      c_OriginalEdsFileName = ro_Json["originalEdsFileName"].toString();
   }

   // Deserialize mappable signals
   if (ro_Json.contains("edsFileMappableSignals")) {
      const QJsonArray c_SignalArray = ro_Json["edsFileMappableSignals"].toArray();
      c_EdsFileMappableSignals.clear();
      c_EdsFileMappableSignals.reserve(c_SignalArray.size());
      for (const QJsonValue &c_Value : c_SignalArray) {
         C_OscCanOpenManagerMappableSignal c_Signal;
         c_Signal.FromJsonObject(c_Value.toObject());
         c_EdsFileMappableSignals.append(c_Signal);
      }
   }

   if (ro_Json.contains("deviceOptional")) {
      q_DeviceOptional = ro_Json["deviceOptional"].toBool();
   }
   if (ro_Json.contains("noInitialization")) {
      q_NoInitialization = ro_Json["noInitialization"].toBool();
   }
   if (ro_Json.contains("factorySettingsActive")) {
      q_FactorySettingsActive = ro_Json["factorySettingsActive"].toBool();
   }
   if (ro_Json.contains("resetNodeObjectDictionarySubIndex")) {
      u8_ResetNodeObjectDictionarySubIndex = static_cast<uint8_t>(ro_Json["resetNodeObjectDictionarySubIndex"].toInt());
   }
   if (ro_Json.contains("enableHeartbeatProducing")) {
      q_EnableHeartbeatProducing = ro_Json["enableHeartbeatProducing"].toBool();
   }
   if (ro_Json.contains("heartbeatProducerTimeMs")) {
      u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(ro_Json["heartbeatProducerTimeMs"].toInt());
   }
   if (ro_Json.contains("useOpenSydeNodeId")) {
      q_UseOpenSydeNodeId = ro_Json["useOpenSydeNodeId"].toBool();
   }
   if (ro_Json.contains("nodeIdValue")) {
      u8_NodeIdValue = static_cast<uint8_t>(ro_Json["nodeIdValue"].toInt());
   }
   if (ro_Json.contains("enableHeartbeatConsuming")) {
      q_EnableHeartbeatConsuming = ro_Json["enableHeartbeatConsuming"].toBool();
   }
   if (ro_Json.contains("heartbeatConsumerTimeMs")) {
      u16_HeartbeatConsumerTimeMs = static_cast<uint16_t>(ro_Json["heartbeatConsumerTimeMs"].toInt());
   }
   if (ro_Json.contains("enableHeartbeatConsumingAutoCalculation")) {
      q_EnableHeartbeatConsumingAutoCalculation = ro_Json["enableHeartbeatConsumingAutoCalculation"].toBool();
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscCanOpenManagerDeviceInfo::ToQDomElement(QDomDocument &ro_Doc,
                                                         const QString &orc_ElementName) const {
   QDomElement c_Element = ro_Doc.createElement(orc_ElementName);

   // Serialize basic fields
   QDomElement c_ProjectEdsPathElement = ro_Doc.createElement("projectEdsFilePath");
   c_ProjectEdsPathElement.appendChild(ro_Doc.createTextNode(c_ProjectEdsFilePath));
   c_Element.appendChild(c_ProjectEdsPathElement);

   QDomElement c_OriginalEdsNameElement = ro_Doc.createElement("originalEdsFileName");
   c_OriginalEdsNameElement.appendChild(ro_Doc.createTextNode(c_OriginalEdsFileName));
   c_Element.appendChild(c_OriginalEdsNameElement);

   // Serialize mappable signals
   QDomElement c_SignalsElement = ro_Doc.createElement("edsFileMappableSignals");
   for (const C_OscCanOpenManagerMappableSignal &rc_Signal : c_EdsFileMappableSignals) {
      QDomElement c_SignalElement = rc_Signal.ToQDomDocument(ro_Doc, "signal");
      c_SignalsElement.appendChild(c_SignalElement);
   }
   c_Element.appendChild(c_SignalsElement);

   QDomElement c_DeviceOptionalElement = ro_Doc.createElement("deviceOptional");
   c_DeviceOptionalElement.appendChild(ro_Doc.createTextNode(QString::number(q_DeviceOptional ? 1 : 0)));
   c_Element.appendChild(c_DeviceOptionalElement);

   QDomElement c_NoInitElement = ro_Doc.createElement("noInitialization");
   c_NoInitElement.appendChild(ro_Doc.createTextNode(QString::number(q_NoInitialization ? 1 : 0)));
   c_Element.appendChild(c_NoInitElement);

   QDomElement c_FactorySettingsElement = ro_Doc.createElement("factorySettingsActive");
   c_FactorySettingsElement.appendChild(ro_Doc.createTextNode(QString::number(q_FactorySettingsActive ? 1 : 0)));
   c_Element.appendChild(c_FactorySettingsElement);

   QDomElement c_ResetSubIndexElement = ro_Doc.createElement("resetNodeObjectDictionarySubIndex");
   c_ResetSubIndexElement.appendChild(ro_Doc.createTextNode(QString::number(u8_ResetNodeObjectDictionarySubIndex)));
   c_Element.appendChild(c_ResetSubIndexElement);

   QDomElement c_HeartbeatProdElement = ro_Doc.createElement("enableHeartbeatProducing");
   c_HeartbeatProdElement.appendChild(ro_Doc.createTextNode(QString::number(q_EnableHeartbeatProducing ? 1 : 0)));
   c_Element.appendChild(c_HeartbeatProdElement);

   QDomElement c_HeartbeatProdTimeElement = ro_Doc.createElement("heartbeatProducerTimeMs");
   c_HeartbeatProdTimeElement.appendChild(ro_Doc.createTextNode(QString::number(u16_HeartbeatProducerTimeMs)));
   c_Element.appendChild(c_HeartbeatProdTimeElement);

   QDomElement c_UseOpenSydeNodeIdElement = ro_Doc.createElement("useOpenSydeNodeId");
   c_UseOpenSydeNodeIdElement.appendChild(ro_Doc.createTextNode(QString::number(q_UseOpenSydeNodeId ? 1 : 0)));
   c_Element.appendChild(c_UseOpenSydeNodeIdElement);

   QDomElement c_NodeIdValueElement = ro_Doc.createElement("nodeIdValue");
   c_NodeIdValueElement.appendChild(ro_Doc.createTextNode(QString::number(u8_NodeIdValue)));
   c_Element.appendChild(c_NodeIdValueElement);

   QDomElement c_HeartbeatConsElement = ro_Doc.createElement("enableHeartbeatConsuming");
   c_HeartbeatConsElement.appendChild(ro_Doc.createTextNode(QString::number(q_EnableHeartbeatConsuming ? 1 : 0)));
   c_Element.appendChild(c_HeartbeatConsElement);

   QDomElement c_HeartbeatConsTimeElement = ro_Doc.createElement("heartbeatConsumerTimeMs");
   c_HeartbeatConsTimeElement.appendChild(ro_Doc.createTextNode(QString::number(u16_HeartbeatConsumerTimeMs)));
   c_Element.appendChild(c_HeartbeatConsTimeElement);

   QDomElement c_HeartbeatAutoCalcElement = ro_Doc.createElement("enableHeartbeatConsumingAutoCalculation");
   c_HeartbeatAutoCalcElement.appendChild(ro_Doc.createTextNode(QString::number(q_EnableHeartbeatConsumingAutoCalculation ? 1 : 0)));
   c_Element.appendChild(c_HeartbeatAutoCalcElement);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::FromQDomElement(const QDomElement &ro_Element) {
   QDomNode c_Node = ro_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull()) {
         const QString c_TagName = c_Elem.tagName();

         if (c_TagName == "projectEdsFilePath") {
            c_ProjectEdsFilePath = c_Elem.text();
         } else if (c_TagName == "originalEdsFileName") {
            c_OriginalEdsFileName = c_Elem.text();
         } else if (c_TagName == "edsFileMappableSignals") {
            QDomNode c_SignalNode = c_Elem.firstChild();
            while (!c_SignalNode.isNull()) {
               QDomElement c_SignalElem = c_SignalNode.toElement();
               if (!c_SignalElem.isNull() && c_SignalElem.tagName() == "signal") {
                  C_OscCanOpenManagerMappableSignal c_Signal;
                  c_Signal.FromQDomDocument(c_SignalElem);
                  c_EdsFileMappableSignals.append(c_Signal);
               }
               c_SignalNode = c_SignalNode.nextSibling();
            }
         } else if (c_TagName == "deviceOptional") {
            q_DeviceOptional = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "noInitialization") {
            q_NoInitialization = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "factorySettingsActive") {
            q_FactorySettingsActive = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "resetNodeObjectDictionarySubIndex") {
            u8_ResetNodeObjectDictionarySubIndex = static_cast<uint8_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "enableHeartbeatProducing") {
            q_EnableHeartbeatProducing = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "heartbeatProducerTimeMs") {
            u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "useOpenSydeNodeId") {
            q_UseOpenSydeNodeId = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "nodeIdValue") {
            u8_NodeIdValue = static_cast<uint8_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "enableHeartbeatConsuming") {
            q_EnableHeartbeatConsuming = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "heartbeatConsumerTimeMs") {
            u16_HeartbeatConsumerTimeMs = static_cast<uint16_t>(c_Elem.text().toUInt());
         } else if (c_TagName == "enableHeartbeatConsumingAutoCalculation") {
            q_EnableHeartbeatConsumingAutoCalculation = (c_Elem.text().toInt() == 1);
         }
      }
      c_Node = c_Node.nextSibling();
   }
}
