//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefDomain

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefDomain.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefDomain::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << c_Id;
   ro_DataStream << c_Name;
   ro_DataStream << c_SingularName;
   ro_DataStream << c_Comment;
   ro_DataStream << static_cast<int32_t>(e_Category);

   // Serialize channels
   ro_DataStream << static_cast<int32_t>(c_Channels.size());
   for (const auto &c_Channel : c_Channels) {
      c_Channel.ToQDataStream(ro_DataStream);
   }

   // Serialize channel use cases
   ro_DataStream << static_cast<int32_t>(c_ChannelUseCases.size());
   for (const auto &c_UseCase : c_ChannelUseCases) {
      c_UseCase.ToQDataStream(ro_DataStream);
   }

   // Serialize domain values
   c_DomainValues.ToQDataStream(ro_DataStream);

   // Serialize channel values
   c_ChannelValues.ToQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefDomain::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> c_Id;
   ro_DataStream >> c_Name;
   ro_DataStream >> c_SingularName;
   ro_DataStream >> c_Comment;

   int32_t s32_Category;
   ro_DataStream >> s32_Category;
   e_Category = static_cast<E_Category>(s32_Category);

   // Deserialize channels
   int32_t s32_ChannelCount;
   ro_DataStream >> s32_ChannelCount;
   c_Channels.clear();
   for (int32_t s32_It = 0; s32_It < s32_ChannelCount; ++s32_It) {
      C_OscHalcDefChannelDef c_Channel;
      c_Channel.FromQDataStream(ro_DataStream);
      c_Channels.push_back(c_Channel);
   }

   // Deserialize channel use cases
   int32_t s32_UseCaseCount;
   ro_DataStream >> s32_UseCaseCount;
   c_ChannelUseCases.clear();
   for (int32_t s32_It = 0; s32_It < s32_UseCaseCount; ++s32_It) {
      C_OscHalcDefChannelUseCase c_UseCase;
      c_UseCase.FromQDataStream(ro_DataStream);
      c_ChannelUseCases.push_back(c_UseCase);
   }

   // Deserialize domain values
   c_DomainValues.FromQDataStream(ro_DataStream);

   // Deserialize channel values
   c_ChannelValues.FromQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefDomain::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize basic fields
   c_Obj["id"] = c_Id;
   c_Obj["name"] = c_Name;
   c_Obj["singularName"] = c_SingularName;
   c_Obj["comment"] = c_Comment;
   c_Obj["category"] = static_cast<int32_t>(e_Category);

   // Serialize channels
   QJsonArray c_ChannelArray;
   for (const auto &c_Channel : c_Channels) {
      c_ChannelArray.append(c_Channel.ToJsonObject());
   }
   c_Obj["channels"] = c_ChannelArray;

   // Serialize channel use cases
   QJsonArray c_UseCaseArray;
   for (const auto &c_UseCase : c_ChannelUseCases) {
      c_UseCaseArray.append(c_UseCase.ToJsonObject());
   }
   c_Obj["channelUseCases"] = c_UseCaseArray;

   // Serialize domain values
   c_Obj["domainValues"] = c_DomainValues.ToJsonObject();

   // Serialize channel values
   c_Obj["channelValues"] = c_ChannelValues.ToJsonObject();

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefDomain::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize basic fields
   if (orc_Object.contains("id")) {
      c_Id = orc_Object["id"].toString();
   }
   if (orc_Object.contains("name")) {
      c_Name = orc_Object["name"].toString();
   }
   if (orc_Object.contains("singularName")) {
      c_SingularName = orc_Object["singularName"].toString();
   }
   if (orc_Object.contains("comment")) {
      c_Comment = orc_Object["comment"].toString();
   }
   if (orc_Object.contains("category")) {
      e_Category = static_cast<E_Category>(orc_Object["category"].toInt());
   }

   // Deserialize channels
   if (orc_Object.contains("channels")) {
      c_Channels.clear();
      const QJsonArray c_ChannelArray = orc_Object["channels"].toArray();
      for (const QJsonValue &c_Value : c_ChannelArray) {
         C_OscHalcDefChannelDef c_Channel;
         c_Channel.FromJsonObject(c_Value.toObject());
         c_Channels.push_back(c_Channel);
      }
   }

   // Deserialize channel use cases
   if (orc_Object.contains("channelUseCases")) {
      c_ChannelUseCases.clear();
      const QJsonArray c_UseCaseArray = orc_Object["channelUseCases"].toArray();
      for (const QJsonValue &c_Value : c_UseCaseArray) {
         C_OscHalcDefChannelUseCase c_UseCase;
         c_UseCase.FromJsonObject(c_Value.toObject());
         c_ChannelUseCases.push_back(c_UseCase);
      }
   }

   // Deserialize domain values
   if (orc_Object.contains("domainValues")) {
      c_DomainValues.FromJsonObject(orc_Object["domainValues"].toObject());
   }

   // Deserialize channel values
   if (orc_Object.contains("channelValues")) {
      c_ChannelValues.FromJsonObject(orc_Object["channelValues"].toObject());
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefDomain::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("id", c_Id);
   c_Element.setAttribute("name", c_Name);
   c_Element.setAttribute("singularName", c_SingularName);
   if (!c_Comment.isEmpty()) {
      c_Element.setAttribute("comment", c_Comment);
   }
   c_Element.setAttribute("category", static_cast<int32_t>(e_Category));

   // Serialize channels
   QDomElement c_ChannelsElem = orc_Doc.createElement("channels");
   for (const auto &c_Channel : c_Channels) {
      QDomElement c_ChannelElem = c_Channel.ToQDomElement(orc_Doc, "channel");
      c_ChannelsElem.appendChild(c_ChannelElem);
   }
   c_Element.appendChild(c_ChannelsElem);

   // Serialize channel use cases
   QDomElement c_UseCasesElem = orc_Doc.createElement("channelUseCases");
   for (const auto &c_UseCase : c_ChannelUseCases) {
      QDomElement c_UseCaseElem = c_UseCase.ToQDomElement(orc_Doc, "useCase");
      c_UseCasesElem.appendChild(c_UseCaseElem);
   }
   c_Element.appendChild(c_UseCasesElem);

   // Serialize domain values
   QDomElement c_DomainValuesElem = c_DomainValues.ToQDomElement(orc_Doc, "domainValues");
   c_Element.appendChild(c_DomainValuesElem);

   // Serialize channel values
   QDomElement c_ChannelValuesElem = c_ChannelValues.ToQDomElement(orc_Doc, "channelValues");
   c_Element.appendChild(c_ChannelValuesElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefDomain::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize basic fields from attributes
   if (orc_Element.hasAttribute("id")) {
      c_Id = orc_Element.attribute("id");
   }
   if (orc_Element.hasAttribute("name")) {
      c_Name = orc_Element.attribute("name");
   }
   if (orc_Element.hasAttribute("singularName")) {
      c_SingularName = orc_Element.attribute("singularName");
   }
   if (orc_Element.hasAttribute("comment")) {
      c_Comment = orc_Element.attribute("comment");
   }
   if (orc_Element.hasAttribute("category")) {
      e_Category = static_cast<E_Category>(orc_Element.attribute("category").toInt());
   }

   // Deserialize channels
   QDomNodeList c_List = orc_Element.elementsByTagName("channels");
   if (!c_List.isEmpty()) {
      QDomElement c_ChannelsElem = c_List.item(0).toElement();
      c_Channels.clear();
      QDomNode c_Node = c_ChannelsElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_Elem = c_Node.toElement();
         if (!c_Elem.isNull() && c_Elem.tagName() == "channel") {
            C_OscHalcDefChannelDef c_Channel;
            c_Channel.FromQDomElement(c_Elem);
            c_Channels.push_back(c_Channel);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize channel use cases
   c_List = orc_Element.elementsByTagName("channelUseCases");
   if (!c_List.isEmpty()) {
      QDomElement c_UseCasesElem = c_List.item(0).toElement();
      c_ChannelUseCases.clear();
      QDomNode c_Node = c_UseCasesElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_Elem = c_Node.toElement();
         if (!c_Elem.isNull() && c_Elem.tagName() == "useCase") {
            C_OscHalcDefChannelUseCase c_UseCase;
            c_UseCase.FromQDomElement(c_Elem);
            c_ChannelUseCases.push_back(c_UseCase);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize domain values
   c_List = orc_Element.elementsByTagName("domainValues");
   if (!c_List.isEmpty()) {
      c_DomainValues.FromQDomElement(c_List.item(0).toElement());
   }

   // Deserialize channel values
   c_List = orc_Element.elementsByTagName("channelValues");
   if (!c_List.isEmpty()) {
      c_ChannelValues.FromQDomElement(c_List.item(0).toElement());
   }

   return C_NO_ERR;
}
