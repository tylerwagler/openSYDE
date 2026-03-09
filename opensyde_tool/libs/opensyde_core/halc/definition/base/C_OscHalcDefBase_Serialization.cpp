//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefBase

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcDefBase.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefBase::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << u32_ContentVersion;
   ro_DataStream << c_DeviceName;
   ro_DataStream << c_FileString;
   ro_DataStream << c_OriginalFileName;
   ro_DataStream << static_cast<int32_t>(e_SafetyMode);
   ro_DataStream << u8_NumConfigCopies;
   ro_DataStream << q_NvmBasedConfig;

   // Serialize NVM address/offset lists
   ro_DataStream << static_cast<int32_t>(c_NvmSafeAddressOffset.size());
   for (const auto &c_Value : c_NvmSafeAddressOffset) {
      ro_DataStream << c_Value;
   }

   ro_DataStream << static_cast<int32_t>(c_NvmNonSafeAddressOffset.size());
   for (const auto &c_Value : c_NvmNonSafeAddressOffset) {
      ro_DataStream << c_Value;
   }

   // Serialize reserved list sizes
   ro_DataStream << u32_NvmReservedListSizeParameters;
   ro_DataStream << u32_NvmReservedListSizeInputValues;
   ro_DataStream << u32_NvmReservedListSizeOutputValues;
   ro_DataStream << u32_NvmReservedListSizeStatusValues;
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefBase::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> u32_ContentVersion;
   ro_DataStream >> c_DeviceName;
   ro_DataStream >> c_FileString;
   ro_DataStream >> c_OriginalFileName;

   int32_t s32_SafetyMode;
   ro_DataStream >> s32_SafetyMode;
   e_SafetyMode = static_cast<E_SafetyMode>(s32_SafetyMode);

   ro_DataStream >> u8_NumConfigCopies;
   ro_DataStream >> q_NvmBasedConfig;

   // Deserialize NVM address/offset lists
   int32_t s32_SafeCount;
   ro_DataStream >> s32_SafeCount;
   c_NvmSafeAddressOffset.clear();
   for (int32_t s32_It = 0; s32_It < s32_SafeCount; ++s32_It) {
      uint32_t u32_Value;
      ro_DataStream >> u32_Value;
      c_NvmSafeAddressOffset.push_back(u32_Value);
   }

   int32_t s32_NonSafeCount;
   ro_DataStream >> s32_NonSafeCount;
   c_NvmNonSafeAddressOffset.clear();
   for (int32_t s32_It = 0; s32_It < s32_NonSafeCount; ++s32_It) {
      uint32_t u32_Value;
      ro_DataStream >> u32_Value;
      c_NvmNonSafeAddressOffset.push_back(u32_Value);
   }

   // Deserialize reserved list sizes
   ro_DataStream >> u32_NvmReservedListSizeParameters;
   ro_DataStream >> u32_NvmReservedListSizeInputValues;
   ro_DataStream >> u32_NvmReservedListSizeOutputValues;
   ro_DataStream >> u32_NvmReservedListSizeStatusValues;
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefBase::ToJsonObject() const {
   QJsonObject c_Obj;

   c_Obj["contentVersion"] = static_cast<int32_t>(u32_ContentVersion);
   c_Obj["deviceName"] = c_DeviceName;
   c_Obj["fileString"] = c_FileString;
   c_Obj["originalFileName"] = c_OriginalFileName;
   c_Obj["safetyMode"] = static_cast<int32_t>(e_SafetyMode);
   c_Obj["numConfigCopies"] = static_cast<int32_t>(u8_NumConfigCopies);
   c_Obj["nvmBasedConfig"] = q_NvmBasedConfig;

   // Serialize NVM address/offset lists
   QJsonArray c_SafeArray;
   for (const auto &c_Value : c_NvmSafeAddressOffset) {
      c_SafeArray.append(static_cast<double>(c_Value));
   }
   c_Obj["nvmSafeAddressOffset"] = c_SafeArray;

   QJsonArray c_NonSafeArray;
   for (const auto &c_Value : c_NvmNonSafeAddressOffset) {
      c_NonSafeArray.append(static_cast<double>(c_Value));
   }
   c_Obj["nvmNonSafeAddressOffset"] = c_NonSafeArray;

   // Serialize reserved list sizes
   c_Obj["nvmReservedListSizeParameters"] = static_cast<int32_t>(u32_NvmReservedListSizeParameters);
   c_Obj["nvmReservedListSizeInputValues"] = static_cast<int32_t>(u32_NvmReservedListSizeInputValues);
   c_Obj["nvmReservedListSizeOutputValues"] = static_cast<int32_t>(u32_NvmReservedListSizeOutputValues);
   c_Obj["nvmReservedListSizeStatusValues"] = static_cast<int32_t>(u32_NvmReservedListSizeStatusValues);

   return c_Obj;
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefBase::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize basic fields
   if (orc_Object.contains("contentVersion")) {
      u32_ContentVersion = static_cast<uint32_t>(orc_Object["contentVersion"].toInt());
   }
   if (orc_Object.contains("deviceName")) {
      c_DeviceName = orc_Object["deviceName"].toString();
   }
   if (orc_Object.contains("fileString")) {
      c_FileString = orc_Object["fileString"].toString();
   }
   if (orc_Object.contains("originalFileName")) {
      c_OriginalFileName = orc_Object["originalFileName"].toString();
   }
   if (orc_Object.contains("safetyMode")) {
      e_SafetyMode = static_cast<E_SafetyMode>(orc_Object["safetyMode"].toInt());
   }
   if (orc_Object.contains("numConfigCopies")) {
      u8_NumConfigCopies = static_cast<uint8_t>(orc_Object["numConfigCopies"].toInt());
   }
   if (orc_Object.contains("nvmBasedConfig")) {
      q_NvmBasedConfig = orc_Object["nvmBasedConfig"].toBool();
   }

   // Deserialize NVM address/offset lists
   if (orc_Object.contains("nvmSafeAddressOffset")) {
      c_NvmSafeAddressOffset.clear();
      const QJsonArray c_Array = orc_Object["nvmSafeAddressOffset"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         c_NvmSafeAddressOffset.push_back(static_cast<uint32_t>(c_Value.toDouble()));
      }
   }

   if (orc_Object.contains("nvmNonSafeAddressOffset")) {
      c_NvmNonSafeAddressOffset.clear();
      const QJsonArray c_Array = orc_Object["nvmNonSafeAddressOffset"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         c_NvmNonSafeAddressOffset.push_back(static_cast<uint32_t>(c_Value.toDouble()));
      }
   }

   // Deserialize reserved list sizes
   if (orc_Object.contains("nvmReservedListSizeParameters")) {
      u32_NvmReservedListSizeParameters = 
         static_cast<uint32_t>(orc_Object["nvmReservedListSizeParameters"].toInt());
   }
   if (orc_Object.contains("nvmReservedListSizeInputValues")) {
      u32_NvmReservedListSizeInputValues = 
         static_cast<uint32_t>(orc_Object["nvmReservedListSizeInputValues"].toInt());
   }
   if (orc_Object.contains("nvmReservedListSizeOutputValues")) {
      u32_NvmReservedListSizeOutputValues = 
         static_cast<uint32_t>(orc_Object["nvmReservedListSizeOutputValues"].toInt());
   }
   if (orc_Object.contains("nvmReservedListSizeStatusValues")) {
      u32_NvmReservedListSizeStatusValues = 
         static_cast<uint32_t>(orc_Object["nvmReservedListSizeStatusValues"].toInt());
   }
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefBase::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("contentVersion", static_cast<int32_t>(u32_ContentVersion));
   c_Element.setAttribute("deviceName", c_DeviceName);
   if (!c_FileString.isEmpty()) {
      c_Element.setAttribute("fileString", c_FileString);
   }
   c_Element.setAttribute("originalFileName", c_OriginalFileName);
   c_Element.setAttribute("safetyMode", static_cast<int32_t>(e_SafetyMode));
   c_Element.setAttribute("numConfigCopies", static_cast<int32_t>(u8_NumConfigCopies));
   c_Element.setAttribute("nvmBasedConfig", q_NvmBasedConfig ? "true" : "false");

   // Serialize NVM address/offset lists
   QDomElement c_SafeElem = orc_Doc.createElement("nvmSafeAddressOffset");
   for (const auto &c_Value : c_NvmSafeAddressOffset) {
      QDomElement c_ItemElem = orc_Doc.createElement("value");
      c_ItemElem.setAttribute("value", QString::number(c_Value));
      c_SafeElem.appendChild(c_ItemElem);
   }
   c_Element.appendChild(c_SafeElem);

   QDomElement c_NonSafeElem = orc_Doc.createElement("nvmNonSafeAddressOffset");
   for (const auto &c_Value : c_NvmNonSafeAddressOffset) {
      QDomElement c_ItemElem = orc_Doc.createElement("value");
      c_ItemElem.setAttribute("value", QString::number(c_Value));
      c_NonSafeElem.appendChild(c_ItemElem);
   }
   c_Element.appendChild(c_NonSafeElem);

   // Serialize reserved list sizes
   QDomElement c_ReservedElem = orc_Doc.createElement("nvmReservedListSizes");
   QDomElement c_ParamElem = orc_Doc.createElement("parameters");
   c_ParamElem.setAttribute("value", static_cast<int32_t>(u32_NvmReservedListSizeParameters));
   c_ReservedElem.appendChild(c_ParamElem);

   QDomElement c_InputElem = orc_Doc.createElement("inputValues");
   c_InputElem.setAttribute("value", static_cast<int32_t>(u32_NvmReservedListSizeInputValues));
   c_ReservedElem.appendChild(c_InputElem);

   QDomElement c_OutputElem = orc_Doc.createElement("outputValues");
   c_OutputElem.setAttribute("value", static_cast<int32_t>(u32_NvmReservedListSizeOutputValues));
   c_ReservedElem.appendChild(c_OutputElem);

   QDomElement c_StatusElem = orc_Doc.createElement("statusValues");
   c_StatusElem.setAttribute("value", static_cast<int32_t>(u32_NvmReservedListSizeStatusValues));
   c_ReservedElem.appendChild(c_StatusElem);

   c_Element.appendChild(c_ReservedElem);

   return c_Element;
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefBase::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize basic fields from attributes
   if (orc_Element.hasAttribute("contentVersion")) {
      bool q_Valid;
      u32_ContentVersion = static_cast<uint32_t>(orc_Element.attribute("contentVersion").toUInt(&q_Valid));
      Q_ASSERT(q_Valid);
   }
   if (orc_Element.hasAttribute("deviceName")) {
      c_DeviceName = orc_Element.attribute("deviceName");
   }
   if (orc_Element.hasAttribute("fileString")) {
      c_FileString = orc_Element.attribute("fileString");
   }
   if (orc_Element.hasAttribute("originalFileName")) {
      c_OriginalFileName = orc_Element.attribute("originalFileName");
   }
   if (orc_Element.hasAttribute("safetyMode")) {
      bool q_Valid;
      e_SafetyMode = static_cast<E_SafetyMode>(orc_Element.attribute("safetyMode").toInt(&q_Valid));
      Q_ASSERT(q_Valid);
   }
   if (orc_Element.hasAttribute("numConfigCopies")) {
      bool q_Valid;
      u8_NumConfigCopies = static_cast<uint8_t>(orc_Element.attribute("numConfigCopies").toUInt(&q_Valid));
      Q_ASSERT(q_Valid);
   }
   if (orc_Element.hasAttribute("nvmBasedConfig")) {
      q_NvmBasedConfig = (orc_Element.attribute("nvmBasedConfig") == "true");
   }

   // Deserialize NVM address/offset lists
   c_NvmSafeAddressOffset.clear();
   QDomNodeList c_List = orc_Element.elementsByTagName("nvmSafeAddressOffset");
   if (!c_List.isEmpty()) {
      QDomElement c_ParentElem = c_List.item(0).toElement();
      QDomNode c_Node = c_ParentElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_Elem = c_Node.toElement();
         if (!c_Elem.isNull() && c_Elem.tagName() == "value") {
            bool q_Valid;
            uint32_t u32_Value = static_cast<uint32_t>(c_Elem.attribute("value").toUInt(&q_Valid));
            if (q_Valid) {
               c_NvmSafeAddressOffset.push_back(u32_Value);
            }
         }
         c_Node = c_Node.nextSibling();
      }
   }

   c_NvmNonSafeAddressOffset.clear();
   c_List = orc_Element.elementsByTagName("nvmNonSafeAddressOffset");
   if (!c_List.isEmpty()) {
      QDomElement c_ParentElem = c_List.item(0).toElement();
      QDomNode c_Node = c_ParentElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_Elem = c_Node.toElement();
         if (!c_Elem.isNull() && c_Elem.tagName() == "value") {
            bool q_Valid;
            uint32_t u32_Value = static_cast<uint32_t>(c_Elem.attribute("value").toUInt(&q_Valid));
            if (q_Valid) {
               c_NvmNonSafeAddressOffset.push_back(u32_Value);
            }
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize reserved list sizes
   c_List = orc_Element.elementsByTagName("nvmReservedListSizes");
   if (!c_List.isEmpty()) {
      QDomElement c_ParentElem = c_List.item(0).toElement();
      
      QDomNodeList c_ParamList = c_ParentElem.elementsByTagName("parameters");
      if (!c_ParamList.isEmpty()) {
         bool q_Valid;
         u32_NvmReservedListSizeParameters = 
            static_cast<uint32_t>(c_ParamList.item(0).toElement().attribute("value").toUInt(&q_Valid));
      }

      QDomNodeList c_InputList = c_ParentElem.elementsByTagName("inputValues");
      if (!c_InputList.isEmpty()) {
         bool q_Valid;
         u32_NvmReservedListSizeInputValues = 
            static_cast<uint32_t>(c_InputList.item(0).toElement().attribute("value").toUInt(&q_Valid));
      }

      QDomNodeList c_OutputList = c_ParentElem.elementsByTagName("outputValues");
      if (!c_OutputList.isEmpty()) {
         bool q_Valid;
         u32_NvmReservedListSizeOutputValues = 
            static_cast<uint32_t>(c_OutputList.item(0).toElement().attribute("value").toUInt(&q_Valid));
      }

      QDomNodeList c_StatusList = c_ParentElem.elementsByTagName("statusValues");
      if (!c_StatusList.isEmpty()) {
         bool q_Valid;
         u32_NvmReservedListSizeStatusValues = 
            static_cast<uint32_t>(c_StatusList.item(0).toElement().attribute("value").toUInt(&q_Valid));
      }
   }
  return stw::errors::C_NO_ERR;
}
