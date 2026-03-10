//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefElement

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefElement.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefElement::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << c_Id;
   ro_DataStream << c_Display;
   ro_DataStream << c_Comment;

   // Serialize initial, min, and max values
   c_InitialValue.ToQDataStream(ro_DataStream);
   c_MinValue.ToQDataStream(ro_DataStream);
   c_MaxValue.ToQDataStream(ro_DataStream);

   // Serialize use case availabilities
   ro_DataStream << static_cast<int32_t>(c_UseCaseAvailabilities.size());
   for (const auto &c_Availability : c_UseCaseAvailabilities) {
      ro_DataStream << c_Availability;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefElement::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> c_Id;
   ro_DataStream >> c_Display;
   ro_DataStream >> c_Comment;

   // Deserialize initial, min, and max values
   c_InitialValue.FromQDataStream(ro_DataStream);
   c_MinValue.FromQDataStream(ro_DataStream);
   c_MaxValue.FromQDataStream(ro_DataStream);

   // Deserialize use case availabilities
   int32_t s32_Count;
   ro_DataStream >> s32_Count;
   c_UseCaseAvailabilities.clear();
   for (int32_t s32_It = 0; s32_It < s32_Count; ++s32_It) {
      uint32_t u32_Value;
      ro_DataStream >> u32_Value;
      c_UseCaseAvailabilities.push_back(u32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefElement::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize basic fields
   c_Obj["id"] = c_Id;
   c_Obj["display"] = c_Display;
   c_Obj["comment"] = c_Comment;

   // Serialize initial, min, and max values
   c_Obj["initialValue"] = c_InitialValue.ToJsonObject();
   c_Obj["minValue"] = c_MinValue.ToJsonObject();
   c_Obj["maxValue"] = c_MaxValue.ToJsonObject();

   // Serialize use case availabilities
   QJsonArray c_AvailabilityArray;
   for (const auto &c_Availability : c_UseCaseAvailabilities) {
      c_AvailabilityArray.append(static_cast<double>(c_Availability));
   }
   c_Obj["useCaseAvailabilities"] = c_AvailabilityArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefElement::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize basic fields
   if (orc_Object.contains("id")) {
      c_Id = orc_Object["id"].toString();
   }
   if (orc_Object.contains("display")) {
      c_Display = orc_Object["display"].toString();
   }
   if (orc_Object.contains("comment")) {
      c_Comment = orc_Object["comment"].toString();
   }

   // Deserialize initial, min, and max values
   if (orc_Object.contains("initialValue")) {
      c_InitialValue.FromJsonObject(orc_Object["initialValue"].toObject());
   }
   if (orc_Object.contains("minValue")) {
      c_MinValue.FromJsonObject(orc_Object["minValue"].toObject());
   }
   if (orc_Object.contains("maxValue")) {
      c_MaxValue.FromJsonObject(orc_Object["maxValue"].toObject());
   }

   // Deserialize use case availabilities
   if (orc_Object.contains("useCaseAvailabilities")) {
      c_UseCaseAvailabilities.clear();
      const QJsonArray c_Array = orc_Object["useCaseAvailabilities"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         c_UseCaseAvailabilities.push_back(static_cast<uint32_t>(c_Value.toDouble()));
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefElement::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("id", c_Id);
   c_Element.setAttribute("display", c_Display);
   if (!c_Comment.isEmpty()) {
      c_Element.setAttribute("comment", c_Comment);
   }

   // Serialize initial, min, and max values
   QDomElement c_InitialElem = c_InitialValue.ToQDomElement(orc_Doc, "initialValue");
   c_Element.appendChild(c_InitialElem);

   QDomElement c_MinElem = c_MinValue.ToQDomElement(orc_Doc, "minValue");
   c_Element.appendChild(c_MinElem);

   QDomElement c_MaxElem = c_MaxValue.ToQDomElement(orc_Doc, "maxValue");
   c_Element.appendChild(c_MaxElem);

   // Serialize use case availabilities
   QDomElement c_AvailabilityElem = orc_Doc.createElement("useCaseAvailabilities");
   for (const auto &c_Availability : c_UseCaseAvailabilities) {
      QDomElement c_AvailableElem = orc_Doc.createElement("availability");
      c_AvailableElem.setAttribute("index", QString::number(c_Availability));
      c_AvailabilityElem.appendChild(c_AvailableElem);
   }
   c_Element.appendChild(c_AvailabilityElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefElement::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize basic fields from attributes
   if (orc_Element.hasAttribute("id")) {
      c_Id = orc_Element.attribute("id");
   }
   if (orc_Element.hasAttribute("display")) {
      c_Display = orc_Element.attribute("display");
   }
   if (orc_Element.hasAttribute("comment")) {
      c_Comment = orc_Element.attribute("comment");
   }

   // Deserialize initial, min, and max values
   QDomNodeList c_List = orc_Element.elementsByTagName("initialValue");
   if (!c_List.isEmpty()) {
      c_InitialValue.FromQDomElement(c_List.item(0).toElement());
   }

   c_List = orc_Element.elementsByTagName("minValue");
   if (!c_List.isEmpty()) {
      c_MinValue.FromQDomElement(c_List.item(0).toElement());
   }

   c_List = orc_Element.elementsByTagName("maxValue");
   if (!c_List.isEmpty()) {
      c_MaxValue.FromQDomElement(c_List.item(0).toElement());
   }

   // Deserialize use case availabilities
   c_List = orc_Element.elementsByTagName("useCaseAvailabilities");
   if (!c_List.isEmpty()) {
      QDomElement c_AvailabilityElem = c_List.item(0).toElement();
      c_UseCaseAvailabilities.clear();
      QDomNode c_Node = c_AvailabilityElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_AvailableElem = c_Node.toElement();
         if (!c_AvailableElem.isNull()) {
            bool q_Valid;
            uint32_t u32_Index = static_cast<uint32_t>(c_AvailableElem.attribute("index").toUInt(&q_Valid));
            if (q_Valid) {
               c_UseCaseAvailabilities.push_back(u32_Index);
            }
         }
         c_Node = c_Node.nextSibling();
      }
   }

   return C_NO_ERR;
}
