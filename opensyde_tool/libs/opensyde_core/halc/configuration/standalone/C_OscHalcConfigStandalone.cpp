//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Complete standalone HALC config

   Complete standalone HALC config

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include "stwerrors.hpp"

#include "C_OscHalcConfigStandalone.hpp"

#include <QDataStream>
#include <QDomDocument>
#include <QJsonObject>
#include <QJsonArray>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor/destructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcConfigStandalone::C_OscHalcConfigStandalone(void)
    : u32_DefinitionContentVersion(0UL) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandalone::ToQDataStream(QDataStream &ro_DataStream) const {
  using namespace stw::errors;
  ro_DataStream << c_DeviceType << static_cast<int32_t>(u32_DefinitionContentVersion);
  ro_DataStream << static_cast<int32_t>(c_Domains.size());
  for (const C_OscHalcConfigStandaloneDomain &c_Domain : c_Domains) {
    c_Domain.ToQDataStream(ro_DataStream);
  }
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandalone::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_DeviceType >> u32_DefinitionContentVersion;
  c_Domains.clear();
  int32_t s32_Count;
  ro_DataStream >> s32_Count;
  for (int32_t i = 0; i < s32_Count; ++i) {
    C_OscHalcConfigStandaloneDomain c_Domain;
    c_Domain.FromQDataStream(ro_DataStream);
    c_Domains.append(c_Domain);
  }
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigStandalone::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["device-type"] = c_DeviceType;
  c_Obj["definition-content-version"] = static_cast<int32_t>(u32_DefinitionContentVersion);
  QJsonArray c_DomainsArray;
  for (const C_OscHalcConfigStandaloneDomain &c_Domain : c_Domains) {
    c_DomainsArray.append(c_Domain.ToJsonObject());
  }
  c_Obj["domains"] = c_DomainsArray;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandalone::FromJsonObject(const QJsonObject &orc_Object) {
  using namespace stw::errors;
  if (orc_Object.contains("device-type")) {
    c_DeviceType = orc_Object["device-type"].toString();
  }
  if (orc_Object.contains("definition-content-version")) {
    u32_DefinitionContentVersion =
        static_cast<uint32_t>(orc_Object["definition-content-version"].toInt());
  }
  c_Domains.clear();
  if (orc_Object.contains("domains")) {
    QJsonArray c_DomainsArray = orc_Object["domains"].toArray();
    for (const QJsonValue &c_Value : c_DomainsArray) {
      C_OscHalcConfigStandaloneDomain c_Domain;
      c_Domain.FromJsonObject(c_Value.toObject());
      c_Domains.append(c_Domain);
    }
  }
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandalone::ToQDomElement(
    QDomDocument &orc_Doc, const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  
  QDomElement c_DeviceElement = orc_Doc.createElement("device-type");
  c_DeviceElement.appendChild(orc_Doc.createTextNode(c_DeviceType));
  c_Element.appendChild(c_DeviceElement);
  
  QDomElement c_VersionElement = orc_Doc.createElement("definition-content-version");
  c_VersionElement.appendChild(orc_Doc.createTextNode(QString::number(u32_DefinitionContentVersion)));
  c_Element.appendChild(c_VersionElement);
  
  QDomElement c_DomainsElement = orc_Doc.createElement("domains");
  for (const C_OscHalcConfigStandaloneDomain &c_Domain : c_Domains) {
    QDomElement c_DomainElement = c_Domain.ToQDomElement(orc_Doc, "domain");
    c_DomainsElement.appendChild(c_DomainElement);
  }
  c_Element.appendChild(c_DomainsElement);
  
  orc_Doc.appendChild(c_Element);
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandalone::FromQDomElement(const QDomElement &orc_Element) {
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_TagName = c_Elem.tagName();
      if (c_TagName == "device-type") {
        c_DeviceType = c_Elem.text();
      } else if (c_TagName == "definition-content-version") {
        u32_DefinitionContentVersion = static_cast<uint32_t>(c_Elem.text().toUInt());
      } else if (c_TagName == "domains") {
        c_Domains.clear();
        QDomNode c_DomainNode = c_Elem.firstChild();
        while (!c_DomainNode.isNull()) {
          QDomElement c_DomainElem = c_DomainNode.toElement();
          if (!c_DomainElem.isNull() && c_DomainElem.tagName() == "domain") {
            C_OscHalcConfigStandaloneDomain c_Domain;
            c_Domain.FromQDomElement(c_DomainElem);
            c_Domains.append(c_Domain);
          }
          c_DomainNode = c_DomainNode.nextSibling();
        }
      }
    }
    c_Node = c_Node.nextSibling();
  }
  return stw::errors::C_NO_ERR;
}
