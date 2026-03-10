//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefStruct

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefStruct.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefStruct::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize base class
   C_OscHalcDefElement::ToQDataStream(ro_DataStream);

   // Serialize struct elements
   ro_DataStream << static_cast<int32_t>(c_StructElements.size());
   for (const auto &c_Element : c_StructElements) {
      c_Element.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefStruct::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize base class
   C_OscHalcDefElement::FromQDataStream(ro_DataStream);

   // Deserialize struct elements
   int32_t s32_ElementCount;
   ro_DataStream >> s32_ElementCount;
   c_StructElements.clear();
   for (int32_t s32_It = 0; s32_It < s32_ElementCount; ++s32_It) {
      C_OscHalcDefElement c_Element;
      c_Element.FromQDataStream(ro_DataStream);
      c_StructElements.push_back(c_Element);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefStruct::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize base class
   const QJsonObject c_BaseObj = C_OscHalcDefElement::ToJsonObject();
   c_Obj["base"] = c_BaseObj;

   // Serialize struct elements
   QJsonArray c_ElementArray;
   for (const auto &c_Element : c_StructElements) {
      c_ElementArray.append(c_Element.ToJsonObject());
   }
   c_Obj["structElements"] = c_ElementArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefStruct::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize base class
   if (orc_Object.contains("base")) {
      C_OscHalcDefElement::FromJsonObject(orc_Object["base"].toObject());
   }

   // Deserialize struct elements
   if (orc_Object.contains("structElements")) {
      c_StructElements.clear();
      const QJsonArray c_Array = orc_Object["structElements"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefElement c_Element;
         c_Element.FromJsonObject(c_Value.toObject());
         c_StructElements.push_back(c_Element);
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefStruct::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize base class
   QDomElement c_BaseElem = C_OscHalcDefElement::ToQDomElement(orc_Doc, "base");
   c_Element.appendChild(c_BaseElem);

   // Serialize struct elements
   QDomElement c_StructElem = orc_Doc.createElement("structElements");
   for (const auto &c_Element : c_StructElements) {
      QDomElement c_ItemElem = c_Element.ToQDomElement(orc_Doc, "element");
      c_StructElem.appendChild(c_ItemElem);
   }
   c_Element.appendChild(c_StructElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefStruct::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize base class
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "base") {
         C_OscHalcDefElement::FromQDomElement(c_Elem);
         break;
      }
      c_Node = c_Node.nextSibling();
   }

   // Deserialize struct elements
   QDomNodeList c_List = orc_Element.elementsByTagName("structElements");
   if (!c_List.isEmpty()) {
      QDomElement c_StructElem = c_List.item(0).toElement();
      c_StructElements.clear();
      QDomNode c_ItemNode = c_StructElem.firstChild();
      while (!c_ItemNode.isNull()) {
         QDomElement c_ItemElem = c_ItemNode.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "element") {
            C_OscHalcDefElement c_Element;
            c_Element.FromQDomElement(c_ItemElem);
            c_StructElements.push_back(c_Element);
         }
         c_ItemNode = c_ItemNode.nextSibling();
      }
   }

   return C_NO_ERR;
}
