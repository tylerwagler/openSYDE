//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefChannelUseCase

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefChannelUseCase.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelUseCase::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << c_Id;
   ro_DataStream << c_Display;
   ro_DataStream << c_Comment;

   // Serialize value (C_OscNodeDataPoolContent)
   c_Value.ToQDataStream(ro_DataStream);

   // Serialize availability
   ro_DataStream << static_cast<int32_t>(c_Availability.size());
   for (const auto &c_Available : c_Availability) {
      c_Available.ToQDataStream(ro_DataStream);
   }

   // Serialize default channels
   ro_DataStream << static_cast<int32_t>(c_DefaultChannels.size());
   for (const auto &c_Channel : c_DefaultChannels) {
      ro_DataStream << c_Channel;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelUseCase::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> c_Id;
   ro_DataStream >> c_Display;
   ro_DataStream >> c_Comment;

   // Deserialize value
   c_Value.FromQDataStream(ro_DataStream);

   // Deserialize availability
   int32_t s32_AvailabilityCount;
   ro_DataStream >> s32_AvailabilityCount;
   c_Availability.clear();
   for (int32_t s32_It = 0; s32_It < s32_AvailabilityCount; ++s32_It) {
      C_OscHalcDefChannelAvailability c_Available;
      c_Available.FromQDataStream(ro_DataStream);
      c_Availability.push_back(c_Available);
   }

   // Deserialize default channels
   int32_t s32_ChannelCount;
   ro_DataStream >> s32_ChannelCount;
   c_DefaultChannels.clear();
   for (int32_t s32_It = 0; s32_It < s32_ChannelCount; ++s32_It) {
      uint32_t u32_Value;
      ro_DataStream >> u32_Value;
      c_DefaultChannels.push_back(u32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelUseCase::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize basic fields
   c_Obj["id"] = c_Id;
   c_Obj["display"] = c_Display;
   c_Obj["comment"] = c_Comment;

   // Serialize value
   c_Obj["value"] = c_Value.ToJsonObject();

   // Serialize availability
   QJsonArray c_AvailabilityArray;
   for (const auto &c_Available : c_Availability) {
      c_AvailabilityArray.append(c_Available.ToJsonObject());
   }
   c_Obj["availability"] = c_AvailabilityArray;

   // Serialize default channels
   QJsonArray c_DefaultChannelsArray;
   for (const auto &c_Channel : c_DefaultChannels) {
      c_DefaultChannelsArray.append(static_cast<double>(c_Channel));
   }
   c_Obj["defaultChannels"] = c_DefaultChannelsArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelUseCase::FromJsonObject(const QJsonObject &orc_Object) {
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

   // Deserialize value
   if (orc_Object.contains("value")) {
      c_Value.FromJsonObject(orc_Object["value"].toObject());
   }

   // Deserialize availability
   if (orc_Object.contains("availability")) {
      c_Availability.clear();
      const QJsonArray c_Array = orc_Object["availability"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefChannelAvailability c_Available;
         c_Available.FromJsonObject(c_Value.toObject());
         c_Availability.push_back(c_Available);
      }
   }

   // Deserialize default channels
   if (orc_Object.contains("defaultChannels")) {
      c_DefaultChannels.clear();
      const QJsonArray c_Array = orc_Object["defaultChannels"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         c_DefaultChannels.push_back(static_cast<uint32_t>(c_Value.toDouble()));
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefChannelUseCase::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("id", c_Id);
   c_Element.setAttribute("display", c_Display);
   if (!c_Comment.isEmpty()) {
      c_Element.setAttribute("comment", c_Comment);
   }

   // Serialize value
   QDomElement c_ValueElem = c_Value.ToQDomElement(orc_Doc, "value");
   c_Element.appendChild(c_ValueElem);

   // Serialize availability
   QDomElement c_AvailabilityElem = orc_Doc.createElement("availability");
   for (const auto &c_Available : c_Availability) {
      QDomElement c_AvailableElem = c_Available.ToQDomElement(orc_Doc, "item");
      c_AvailabilityElem.appendChild(c_AvailableElem);
   }
   c_Element.appendChild(c_AvailabilityElem);

   // Serialize default channels
   QDomElement c_DefaultChannelsElem = orc_Doc.createElement("defaultChannels");
   for (const auto &c_Channel : c_DefaultChannels) {
      QDomElement c_ChannelElem = orc_Doc.createElement("channel");
      c_ChannelElem.setAttribute("index", QString::number(c_Channel));
      c_DefaultChannelsElem.appendChild(c_ChannelElem);
   }
   c_Element.appendChild(c_DefaultChannelsElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelUseCase::FromQDomElement(const QDomElement &orc_Element) {
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

   // Deserialize value
   QDomNodeList c_List = orc_Element.elementsByTagName("value");
   if (!c_List.isEmpty()) {
      c_Value.FromQDomElement(c_List.item(0).toElement());
   }

   // Deserialize availability
   c_List = orc_Element.elementsByTagName("availability");
   if (!c_List.isEmpty()) {
      QDomElement c_AvailabilityElem = c_List.item(0).toElement();
      c_Availability.clear();
      QDomNode c_Node = c_AvailabilityElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ItemElem = c_Node.toElement();
         if (!c_ItemElem.isNull()) {
            C_OscHalcDefChannelAvailability c_Available;
            c_Available.FromQDomElement(c_ItemElem);
            c_Availability.push_back(c_Available);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize default channels
   c_List = orc_Element.elementsByTagName("defaultChannels");
   if (!c_List.isEmpty()) {
      QDomElement c_DefaultChannelsElem = c_List.item(0).toElement();
      c_DefaultChannels.clear();
      QDomNode c_Node = c_DefaultChannelsElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ChannelElem = c_Node.toElement();
         if (!c_ChannelElem.isNull()) {
            bool q_Valid;
            uint32_t u32_Index = static_cast<uint32_t>(c_ChannelElem.attribute("index").toUInt(&q_Valid));
            if (q_Valid) {
               c_DefaultChannels.push_back(u32_Index);
            }
         }
         c_Node = c_Node.nextSibling();
      }
   }

   return C_NO_ERR;
}
