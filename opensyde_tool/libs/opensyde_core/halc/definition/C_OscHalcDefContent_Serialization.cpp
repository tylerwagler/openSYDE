//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefContent

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcDefContent.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContent::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize base class (C_OscNodeDataPoolContent)
   C_OscNodeDataPoolContent::ToQDataStream(ro_DataStream);

   // Serialize complex type
   ro_DataStream << static_cast<int32_t>(me_ComplexType);

   // Serialize enum items
   ro_DataStream << static_cast<int32_t>(mc_EnumItems.size());
   for (const auto &c_Item : mc_EnumItems) {
      ro_DataStream << c_Item.first;
      c_Item.second.ToQDataStream(ro_DataStream);
   }

   // Serialize bitmask items
   ro_DataStream << static_cast<int32_t>(mc_BitmaskItems.size());
   for (const auto &c_Item : mc_BitmaskItems) {
      ro_DataStream << c_Item.c_Display;
      ro_DataStream << c_Item.c_Comment;
      ro_DataStream << c_Item.q_ApplyValueSetting;
      ro_DataStream << c_Item.u64_Value;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContent::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize base class
   C_OscNodeDataPoolContent::FromQDataStream(ro_DataStream);

   // Deserialize complex type
   int32_t s32_ComplexType;
   ro_DataStream >> s32_ComplexType;
   me_ComplexType = static_cast<E_ComplexType>(s32_ComplexType);

   // Deserialize enum items
   int32_t s32_EnumCount;
   ro_DataStream >> s32_EnumCount;
   mc_EnumItems.clear();
   for (int32_t s32_It = 0; s32_It < s32_EnumCount; ++s32_It) {
      QString c_DisplayName;
      C_OscNodeDataPoolContent c_Value;
      ro_DataStream >> c_DisplayName;
      c_Value.FromQDataStream(ro_DataStream);
      mc_EnumItems.push_back(QPair<QString, C_OscNodeDataPoolContent>(c_DisplayName, c_Value));
   }

   // Deserialize bitmask items
   int32_t s32_BitmaskCount;
   ro_DataStream >> s32_BitmaskCount;
   mc_BitmaskItems.clear();
   for (int32_t s32_It = 0; s32_It < s32_BitmaskCount; ++s32_It) {
      C_OscHalcDefContentBitmaskItem c_Item;
      ro_DataStream >> c_Item.c_Display;
      ro_DataStream >> c_Item.c_Comment;
      ro_DataStream >> c_Item.q_ApplyValueSetting;
      ro_DataStream >> c_Item.u64_Value;
      mc_BitmaskItems.push_back(c_Item);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefContent::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize base class
   const QJsonObject c_BaseObj = C_OscNodeDataPoolContent::ToJsonObject();
   c_Obj["base"] = c_BaseObj;

   // Serialize complex type
   c_Obj["complexType"] = static_cast<int32_t>(me_ComplexType);

   // Serialize enum items
   QJsonArray c_EnumArray;
   for (const auto &c_Item : mc_EnumItems) {
      QJsonObject c_EnumItem;
      c_EnumItem["displayName"] = c_Item.first;
      c_EnumItem["value"] = c_Item.second.ToJsonObject();
      c_EnumArray.append(c_EnumItem);
   }
   c_Obj["enumItems"] = c_EnumArray;

   // Serialize bitmask items
   QJsonArray c_BitmaskArray;
   for (const auto &c_Item : mc_BitmaskItems) {
      QJsonObject c_BitmaskItem;
      c_BitmaskItem["display"] = c_Item.c_Display;
      c_BitmaskItem["comment"] = c_Item.c_Comment;
      c_BitmaskItem["applyValueSetting"] = c_Item.q_ApplyValueSetting;
      c_BitmaskItem["value"] = static_cast<double>(c_Item.u64_Value);
      c_BitmaskArray.append(c_BitmaskItem);
   }
   c_Obj["bitmaskItems"] = c_BitmaskArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContent::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize base class
   if (orc_Object.contains("base")) {
      C_OscNodeDataPoolContent::FromJsonObject(orc_Object["base"].toObject());
   }

   // Deserialize complex type
   if (orc_Object.contains("complexType")) {
      me_ComplexType = static_cast<E_ComplexType>(orc_Object["complexType"].toInt());
   }

   // Deserialize enum items
   if (orc_Object.contains("enumItems")) {
      mc_EnumItems.clear();
      const QJsonArray c_EnumArray = orc_Object["enumItems"].toArray();
      for (const QJsonValue &c_Value : c_EnumArray) {
         const QJsonObject c_Item = c_Value.toObject();
         if (c_Item.contains("displayName") && c_Item.contains("value")) {
            QString c_DisplayName = c_Item["displayName"].toString();
            C_OscNodeDataPoolContent c_ValueObj;
            c_ValueObj.FromJsonObject(c_Item["value"].toObject());
            mc_EnumItems.push_back(QPair<QString, C_OscNodeDataPoolContent>(c_DisplayName, c_ValueObj));
         }
      }
   }

   // Deserialize bitmask items
   if (orc_Object.contains("bitmaskItems")) {
      mc_BitmaskItems.clear();
      const QJsonArray c_BitmaskArray = orc_Object["bitmaskItems"].toArray();
      for (const QJsonValue &c_Value : c_BitmaskArray) {
         const QJsonObject c_Item = c_Value.toObject();
         C_OscHalcDefContentBitmaskItem c_BitmaskItem;
         if (c_Item.contains("display")) {
            c_BitmaskItem.c_Display = c_Item["display"].toString();
         }
         if (c_Item.contains("comment")) {
            c_BitmaskItem.c_Comment = c_Item["comment"].toString();
         }
         if (c_Item.contains("applyValueSetting")) {
            c_BitmaskItem.q_ApplyValueSetting = c_Item["applyValueSetting"].toBool();
         }
         if (c_Item.contains("value")) {
            c_BitmaskItem.u64_Value = static_cast<uint64_t>(c_Item["value"].toDouble());
         }
         mc_BitmaskItems.push_back(c_BitmaskItem);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefContent::ToQDomDocument(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize base class
   QDomElement c_BaseElem = C_OscNodeDataPoolContent::ToQDomDocument(orc_Doc, "base");
   c_Element.appendChild(c_BaseElem);

   // Serialize complex type
   QDomElement c_ComplexTypeElem = orc_Doc.createElement("complexType");
   c_ComplexTypeElem.setAttribute("value", static_cast<int32_t>(me_ComplexType));
   c_Element.appendChild(c_ComplexTypeElem);

   // Serialize enum items
   QDomElement c_EnumElem = orc_Doc.createElement("enumItems");
   for (const auto &c_Item : mc_EnumItems) {
      QDomElement c_EnumItemElem = orc_Doc.createElement("item");
      c_EnumItemElem.setAttribute("displayName", c_Item.first);
      QDomElement c_ValueElem = c_Item.second.ToQDomDocument(orc_Doc, "value");
      c_EnumItemElem.appendChild(c_ValueElem);
      c_EnumElem.appendChild(c_EnumItemElem);
   }
   c_Element.appendChild(c_EnumElem);

   // Serialize bitmask items
   QDomElement c_BitmaskElem = orc_Doc.createElement("bitmaskItems");
   for (const auto &c_Item : mc_BitmaskItems) {
      QDomElement c_BitmaskItemElem = orc_Doc.createElement("item");
      c_BitmaskItemElem.setAttribute("display", c_Item.c_Display);
      if (!c_Item.c_Comment.isEmpty()) {
         c_BitmaskItemElem.setAttribute("comment", c_Item.c_Comment);
      }
      c_BitmaskItemElem.setAttribute("applyValueSetting", c_Item.q_ApplyValueSetting ? "true" : "false");
      c_BitmaskItemElem.setAttribute("value", QString::number(c_Item.u64_Value));
      c_BitmaskElem.appendChild(c_BitmaskItemElem);
   }
   c_Element.appendChild(c_BitmaskElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContent::FromQDomDocument(const QDomElement &orc_Element) {
   // Deserialize base class
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "base") {
         C_OscNodeDataPoolContent::FromQDomDocument(c_Elem);
         break;
      }
      c_Node = c_Node.nextSibling();
   }

   // Deserialize complex type
   QDomNodeList c_List = orc_Element.elementsByTagName("complexType");
   if (!c_List.isEmpty()) {
      QDomElement c_ComplexTypeElem = c_List.item(0).toElement();
      bool q_Valid;
      int32_t s32_ComplexType = c_ComplexTypeElem.attribute("value").toInt(&q_Valid);
      if (q_Valid) {
         me_ComplexType = static_cast<E_ComplexType>(s32_ComplexType);
      }
   }

   // Deserialize enum items
   c_List = orc_Element.elementsByTagName("enumItems");
   if (!c_List.isEmpty()) {
      QDomElement c_EnumElem = c_List.item(0).toElement();
      mc_EnumItems.clear();
      QDomNode c_ItemNode = c_EnumElem.firstChild();
      while (!c_ItemNode.isNull()) {
         QDomElement c_ItemElem = c_ItemNode.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "item") {
            QString c_DisplayName = c_ItemElem.attribute("displayName");
            QDomNode c_ValueNode = c_ItemElem.firstChild();
            while (!c_ValueNode.isNull()) {
               QDomElement c_ValueElem = c_ValueNode.toElement();
               if (!c_ValueElem.isNull() && c_ValueElem.tagName() == "value") {
                  C_OscNodeDataPoolContent c_Value;
                  c_Value.FromQDomDocument(c_ValueElem);
                  mc_EnumItems.push_back(QPair<QString, C_OscNodeDataPoolContent>(c_DisplayName, c_Value));
                  break;
               }
               c_ValueNode = c_ValueNode.nextSibling();
            }
         }
         c_ItemNode = c_ItemNode.nextSibling();
      }
   }

   // Deserialize bitmask items
   c_List = orc_Element.elementsByTagName("bitmaskItems");
   if (!c_List.isEmpty()) {
      QDomElement c_BitmaskElem = c_List.item(0).toElement();
      mc_BitmaskItems.clear();
      QDomNode c_ItemNode = c_BitmaskElem.firstChild();
      while (!c_ItemNode.isNull()) {
         QDomElement c_ItemElem = c_ItemNode.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "item") {
            C_OscHalcDefContentBitmaskItem c_BitmaskItem;
            c_BitmaskItem.c_Display = c_ItemElem.attribute("display");
            if (c_ItemElem.hasAttribute("comment")) {
               c_BitmaskItem.c_Comment = c_ItemElem.attribute("comment");
            }
            c_BitmaskItem.q_ApplyValueSetting = (c_ItemElem.attribute("applyValueSetting") == "true");
            bool q_Valid;
            c_BitmaskItem.u64_Value = static_cast<uint64_t>(c_ItemElem.attribute("value").toULongLong(&q_Valid));
            mc_BitmaskItems.push_back(c_BitmaskItem);
         }
         c_ItemNode = c_ItemNode.nextSibling();
      }
   }
}
