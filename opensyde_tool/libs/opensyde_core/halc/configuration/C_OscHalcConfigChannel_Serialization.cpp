//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcConfigChannel

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcConfigChannel.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigChannel::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << c_Name;
   ro_DataStream << c_Comment;
   ro_DataStream << q_SafetyRelevant;
   ro_DataStream << static_cast<qint32>(u32_UseCaseIndex);
   
   // Serialize parameters
   ro_DataStream << static_cast<qint32>(c_Parameters.size());
   for (const auto &c_Param : c_Parameters) {
      c_Param.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigChannel::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> c_Name;
   ro_DataStream >> c_Comment;
   ro_DataStream >> q_SafetyRelevant;
   
   qint32 s_UseCaseIndex;
   ro_DataStream >> s_UseCaseIndex;
   u32_UseCaseIndex = static_cast<uint32_t>(s_UseCaseIndex);
   
   // Deserialize parameters
   qint32 s_ParamSize;
   ro_DataStream >> s_ParamSize;
   c_Parameters.clear();
   for (qint32 s_I = 0; s_I < s_ParamSize; ++s_I) {
      C_OscHalcConfigParameterStruct c_Param;
      c_Param.FromQDataStream(ro_DataStream);
      c_Parameters.append(c_Param);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigChannel::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["name"] = c_Name;
   c_Obj["comment"] = c_Comment;
   c_Obj["safety-relevant"] = q_SafetyRelevant;
   c_Obj["use-case-index"] = static_cast<qint64>(u32_UseCaseIndex);
   
   // Serialize parameters
   QJsonArray c_ParamsArray;
   for (const auto &c_Param : c_Parameters) {
      c_ParamsArray.append(c_Param.ToJsonObject());
   }
   c_Obj["parameters"] = c_ParamsArray;
   
   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigChannel::FromJsonObject(const QJsonObject &orc_Object) {
   if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
   if (orc_Object.contains("comment")) c_Comment = orc_Object["comment"].toString();
   if (orc_Object.contains("safety-relevant")) q_SafetyRelevant = orc_Object["safety-relevant"].toBool();
   if (orc_Object.contains("use-case-index")) u32_UseCaseIndex = static_cast<uint32_t>(orc_Object["use-case-index"].toInt());
   
   // Deserialize parameters
   if (orc_Object.contains("parameters")) {
      QJsonArray c_ParamsArray = orc_Object["parameters"].toArray();
      c_Parameters.clear();
      for (const auto &c_Value : c_ParamsArray) {
         C_OscHalcConfigParameterStruct c_Param;
         c_Param.FromJsonObject(c_Value.toObject());
         c_Parameters.append(c_Param);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcConfigChannel::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
   c_Element.setAttribute("name", c_Name);
   c_Element.setAttribute("comment", c_Comment);
   c_Element.setAttribute("safety-relevant", q_SafetyRelevant ? "true" : "false");
   c_Element.setAttribute("use-case-index", QString::number(u32_UseCaseIndex));
   
   // Serialize parameters
   for (const auto &c_Param : c_Parameters) {
      QDomElement c_ParamElem = c_Param.ToQDomElement(orc_Doc, "parameter");
      c_Element.appendChild(c_ParamElem);
   }
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigChannel::FromQDomElement(const QDomElement &orc_Element) {
   if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
   if (orc_Element.hasAttribute("comment")) c_Comment = orc_Element.attribute("comment");
   if (orc_Element.hasAttribute("safety-relevant")) q_SafetyRelevant = (orc_Element.attribute("safety-relevant") == "true");
   if (orc_Element.hasAttribute("use-case-index")) u32_UseCaseIndex = orc_Element.attribute("use-case-index").toUInt();
   
   // Deserialize parameters
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "parameter") {
         C_OscHalcConfigParameterStruct c_Param;
         c_Param.FromQDomElement(c_Elem);
         c_Parameters.append(c_Param);
      }
      c_Node = c_Node.nextSibling();
   }
}
