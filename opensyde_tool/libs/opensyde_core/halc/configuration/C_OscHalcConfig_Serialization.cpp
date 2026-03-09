//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcConfig

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcConfig.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfig::ToQDataStream(QDataStream &ro_DataStream) const {
   using namespace stw::errors;

   // Serialize base class (C_OscHalcDefBase)
   C_OscHalcDefBase::ToQDataStream(ro_DataStream);

   // Serialize domains
   ro_DataStream << static_cast<int32_t>(mc_Domains.size());
   for (const auto &c_Domain : mc_Domains) {
      c_Domain.ToQDataStream(ro_DataStream);
   }
    return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfig::FromQDataStream(QDataStream &ro_DataStream) {
   using namespace stw::errors;

   // Deserialize base class
   C_OscHalcDefBase::FromQDataStream(ro_DataStream);

   // Deserialize domains
   int32_t s32_DomainCount;
   ro_DataStream >> s32_DomainCount;
   mc_Domains.clear();
   for (int32_t s32_It = 0; s32_It < s32_DomainCount; ++s32_It) {
      C_OscHalcConfigDomain c_Domain;
      c_Domain.FromQDataStream(ro_DataStream);
      mc_Domains.push_back(c_Domain);
   }
   return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfig::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize base class
   const QJsonObject c_BaseObj = C_OscHalcDefBase::ToJsonObject();
   c_Obj["base"] = c_BaseObj;

   // Serialize domains
   QJsonArray c_DomainArray;
   for (const auto &c_Domain : mc_Domains) {
      c_DomainArray.append(c_Domain.ToJsonObject());
   }
   c_Obj["domains"] = c_DomainArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfig::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize base class
   if (orc_Object.contains("base")) {
      C_OscHalcDefBase::FromJsonObject(orc_Object["base"].toObject());
   }

   // Deserialize domains
   if (orc_Object.contains("domains")) {
      mc_Domains.clear();
      const QJsonArray c_DomainArray = orc_Object["domains"].toArray();
      for (const QJsonValue &c_Value : c_DomainArray) {
         C_OscHalcConfigDomain c_Domain;
         c_Domain.FromJsonObject(c_Value.toObject());
         mc_Domains.push_back(c_Domain);
      }
   }
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcConfig::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   using namespace stw::errors;

   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize base class
   QDomElement c_BaseElem = C_OscHalcDefBase::ToQDomElement(orc_Doc, "base");
   c_Element.appendChild(c_BaseElem);

   // Serialize domains
   QDomElement c_DomainElem = orc_Doc.createElement("domains");
   for (const auto &c_Domain : mc_Domains) {
      QDomElement c_DomainItemElem = c_Domain.ToQDomElement(orc_Doc, "domain");
      c_DomainElem.appendChild(c_DomainItemElem);
   }
   c_Element.appendChild(c_DomainElem);

   orc_Doc.appendChild(c_Element);
   return stw::errors::C_NO_ERR;
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfig::FromQDomElement(const QDomElement &orc_Element) {
   using namespace stw::errors;

   // Deserialize base class
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "base") {
         C_OscHalcDefBase::FromQDomElement(c_Elem);
         break;
      }
      c_Node = c_Node.nextSibling();
   }

   // Deserialize domains
   QDomNodeList c_List = orc_Element.elementsByTagName("domains");
   if (!c_List.isEmpty()) {
      QDomElement c_DomainElem = c_List.item(0).toElement();
      mc_Domains.clear();
      QDomNode c_DomainNode = c_DomainElem.firstChild();
      while (!c_DomainNode.isNull()) {
         QDomElement c_DomainItemElem = c_DomainNode.toElement();
         if (!c_DomainItemElem.isNull() && c_DomainItemElem.tagName() == "domain") {
            C_OscHalcConfigDomain c_Domain;
            c_Domain.FromQDomElement(c_DomainItemElem);
            mc_Domains.push_back(c_Domain);
         }
         c_DomainNode = c_DomainNode.nextSibling();
      }
   }
   return stw::errors::C_NO_ERR;
  return stw::errors::C_NO_ERR;
}
