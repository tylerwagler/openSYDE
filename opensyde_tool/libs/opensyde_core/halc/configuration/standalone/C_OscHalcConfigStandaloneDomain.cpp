//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standalone HALC domain config

   Standalone HALC domain config

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include "stwerrors.hpp"

#include "C_OscHalcConfigStandaloneDomain.hpp"

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcConfigStandaloneDomain::C_OscHalcConfigStandaloneDomain(void)
    : C_OscHalcConfigDomain() {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor to use data of an instance of type C_OscHalcConfigDomain

   \param[in]   orc_ConfigDomain          Project HALC domain configuration
   \param[in]   orc_StandaloneChannels    Standalone information of all domain
   channels
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcConfigStandaloneDomain::C_OscHalcConfigStandaloneDomain(
    const C_OscHalcConfigDomain &orc_ConfigDomain,
    const QList<C_OscHalcConfigStandaloneChannel> &orc_StandaloneChannels)
    : C_OscHalcConfigDomain(orc_ConfigDomain),
      c_StandaloneChannels(orc_StandaloneChannels) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneDomain::ToQDataStream(QDataStream &ro_DataStream) const {
  // Serialize base class fields (C_OscHalcConfigDomain)
  // Note: Base class doesn't have serialization yet, so we skip it for now
  // This will need to be addressed when C_OscHalcConfigDomain is migrated
  
  // Serialize standalone channels
  ro_DataStream << static_cast<int32_t>(c_StandaloneChannels.size());
  for (const C_OscHalcConfigStandaloneChannel &c_Channel : c_StandaloneChannels) {
    c_Channel.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneDomain::FromQDataStream(QDataStream &ro_DataStream) {
  c_StandaloneChannels.clear();
  int32_t s32_Count;
  ro_DataStream >> s32_Count;
  for (int32_t i = 0; i < s32_Count; ++i) {
    C_OscHalcConfigStandaloneChannel c_Channel;
    c_Channel.FromQDataStream(ro_DataStream);
    c_StandaloneChannels.append(c_Channel);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigStandaloneDomain::ToJsonObject() const {
  QJsonObject c_Obj;
  QJsonArray c_ChannelsArray;
  for (const C_OscHalcConfigStandaloneChannel &c_Channel : c_StandaloneChannels) {
    c_ChannelsArray.append(c_Channel.ToJsonObject());
  }
  c_Obj["standalone-channels"] = c_ChannelsArray;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneDomain::FromJsonObject(const QJsonObject &orc_Object) {
  c_StandaloneChannels.clear();
  if (orc_Object.contains("standalone-channels")) {
    QJsonArray c_ChannelsArray = orc_Object["standalone-channels"].toArray();
    for (const QJsonValue &c_Value : c_ChannelsArray) {
      C_OscHalcConfigStandaloneChannel c_Channel;
      c_Channel.FromJsonObject(c_Value.toObject());
      c_StandaloneChannels.append(c_Channel);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandaloneDomain::ToQDomElement(
    QDomDocument &orc_Doc, const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  QDomElement c_ChannelsElement = orc_Doc.createElement("standalone-channels");
  for (const C_OscHalcConfigStandaloneChannel &c_Channel : c_StandaloneChannels) {
    QDomElement c_ChannelElement = c_Channel.ToQDomElement(orc_Doc, "channel");
    c_ChannelsElement.appendChild(c_ChannelElement);
  }
  c_Element.appendChild(c_ChannelsElement);
  orc_Doc.appendChild(c_Element);
  return stw::errors::C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigStandaloneDomain::FromQDomElement(const QDomElement &orc_Element) {
  c_StandaloneChannels.clear();
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "standalone-channels") {
      QDomNode c_ChannelNode = c_Elem.firstChild();
      while (!c_ChannelNode.isNull()) {
        QDomElement c_ChannelElem = c_ChannelNode.toElement();
        if (!c_ChannelElem.isNull() && c_ChannelElem.tagName() == "channel") {
          C_OscHalcConfigStandaloneChannel c_Channel;
          c_Channel.FromQDomElement(c_ChannelElem);
          c_StandaloneChannels.append(c_Channel);
        }
        c_ChannelNode = c_ChannelNode.nextSibling();
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
  return stw::errors::C_NO_ERR;
