//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcConfigDomain

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcConfigDomain.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigDomain::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize base class data (C_OscHalcDefDomain)
   // Note: C_OscHalcDefDomain serialization would need to be added separately
   ro_DataStream << c_DomainConfig.c_Name;
   ro_DataStream << c_DomainConfig.c_Comment;
   
   // Serialize channel configs
   ro_DataStream << static_cast<qint32>(c_ChannelConfigs.size());
   for (const auto &c_Channel : c_ChannelConfigs) {
      c_Channel.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigDomain::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> c_DomainConfig.c_Name;
   ro_DataStream >> c_DomainConfig.c_Comment;
   
   qint32 s_Size;
   ro_DataStream >> s_Size;
   c_ChannelConfigs.clear();
   for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
      C_OscHalcConfigChannel c_Channel;
      c_Channel.FromQDataStream(ro_DataStream);
      c_ChannelConfigs.append(c_Channel);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigDomain::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["name"] = c_DomainConfig.c_Name;
   c_Obj["comment"] = c_DomainConfig.c_Comment;
   
   QJsonArray c_ChannelsArray;
   for (const auto &c_Channel : c_ChannelConfigs) {
      c_ChannelsArray.append(c_Channel.ToJsonObject());
   }
   c_Obj["channels"] = c_ChannelsArray;
   
   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigDomain::FromJsonObject(const QJsonObject &orc_Object) {
   if (orc_Object.contains("name")) c_DomainConfig.c_Name = orc_Object["name"].toString();
   if (orc_Object.contains("comment")) c_DomainConfig.c_Comment = orc_Object["comment"].toString();
   
   if (orc_Object.contains("channels")) {
      QJsonArray c_ChannelsArray = orc_Object["channels"].toArray();
      c_ChannelConfigs.clear();
      for (const auto &c_Value : c_ChannelsArray) {
         C_OscHalcConfigChannel c_Channel;
         c_Channel.FromJsonObject(c_Value.toObject());
         c_ChannelConfigs.append(c_Channel);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcConfigDomain::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
   c_Element.setAttribute("name", c_DomainConfig.c_Name);
   c_Element.setAttribute("comment", c_DomainConfig.c_Comment);
   
   for (const auto &c_Channel : c_ChannelConfigs) {
      QDomElement c_ChannelElem = c_Channel.ToQDomElement(orc_Doc, "channel");
      c_Element.appendChild(c_ChannelElem);
   }
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigDomain::FromQDomElement(const QDomElement &orc_Element) {
   if (orc_Element.hasAttribute("name")) c_DomainConfig.c_Name = orc_Element.attribute("name");
   if (orc_Element.hasAttribute("comment")) c_DomainConfig.c_Comment = orc_Element.attribute("comment");
   
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "channel") {
         C_OscHalcConfigChannel c_Channel;
         c_Channel.FromQDomElement(c_Elem);
         c_ChannelConfigs.append(c_Channel);
      }
      c_Node = c_Node.nextSibling();
   }
}
