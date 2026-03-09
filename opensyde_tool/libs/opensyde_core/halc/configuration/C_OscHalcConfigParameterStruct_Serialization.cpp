//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcConfigParameterStruct

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcConfigParameterStruct.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameterStruct::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize base class (C_OscHalcConfigParameter)
   // Note: C_OscHalcConfigParameter serialization would need to be added separately
   ro_DataStream << c_Name;
   ro_DataStream << c_Comment;
   
   // Serialize parameter elements
   ro_DataStream << static_cast<qint32>(c_ParameterElements.size());
   for (const auto &c_Element : c_ParameterElements) {
      c_Element.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameterStruct::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> c_Name;
   ro_DataStream >> c_Comment;
   
   // Deserialize parameter elements
   qint32 s_ElementSize;
   ro_DataStream >> s_ElementSize;
   c_ParameterElements.clear();
   for (qint32 s_I = 0; s_I < s_ElementSize; ++s_I) {
      C_OscHalcConfigParameter c_Element;
      c_Element.FromQDataStream(ro_DataStream);
      c_ParameterElements.append(c_Element);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigParameterStruct::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["name"] = c_Name;
   c_Obj["comment"] = c_Comment;
   
   // Serialize parameter elements
   QJsonArray c_ElementsArray;
   for (const auto &c_Element : c_ParameterElements) {
      c_ElementsArray.append(c_Element.ToJsonObject());
   }
   c_Obj["elements"] = c_ElementsArray;
   
   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameterStruct::FromJsonObject(const QJsonObject &orc_Object) {
   if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
   if (orc_Object.contains("comment")) c_Comment = orc_Object["comment"].toString();
   
   // Deserialize parameter elements
   if (orc_Object.contains("elements")) {
      QJsonArray c_ElementsArray = orc_Object["elements"].toArray();
      c_ParameterElements.clear();
      for (const auto &c_Value : c_ElementsArray) {
         C_OscHalcConfigParameter c_Element;
         c_Element.FromJsonObject(c_Value.toObject());
         c_ParameterElements.append(c_Element);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcConfigParameterStruct::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
   c_Element.setAttribute("name", c_Name);
   c_Element.setAttribute("comment", c_Comment);
   
   // Serialize parameter elements
   for (const auto &c_ParamElement : c_ParameterElements) {
      QDomElement c_ElementElem = c_ParamElement.ToQDomElement(orc_Doc, "element");
      c_Element.appendChild(c_ElementElem);
   }
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameterStruct::FromQDomElement(const QDomElement &orc_Element) {
   if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
   if (orc_Element.hasAttribute("comment")) c_Comment = orc_Element.attribute("comment");
   
   // Deserialize parameter elements
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "element") {
         C_OscHalcConfigParameter c_Element;
         c_Element.FromQDomElement(c_Elem);
         c_ParameterElements.append(c_Element);
      }
      c_Node = c_Node.nextSibling();
   }
}
