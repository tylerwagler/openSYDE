//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standalone HALC channel config with parameter Ids

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscHalcConfigStandaloneChannel.hpp"

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
C_OscHalcConfigStandaloneChannel::C_OscHalcConfigStandaloneChannel(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneChannel::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_ParameterIds;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneChannel::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_ParameterIds;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigStandaloneChannel::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["parameter-ids"] = QJsonValue::fromVariant(c_ParameterIds);
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneChannel::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("parameter-ids")) {
    QJsonArray c_Array = orc_Object["parameter-ids"].toArray();
    c_ParameterIds.clear();
    for (const QJsonValue &c_Value : c_Array) {
      c_ParameterIds.append(c_Value.toString());
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
QDomElement C_OscHalcConfigStandaloneChannel::ToQDomDocument(
    QDomDocument &orc_Doc, const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  QDomElement c_ParamsElement = orc_Doc.createElement("parameter-ids");
  for (const QString &c_Id : c_ParameterIds) {
    QDomElement c_IdElement = orc_Doc.createElement("id");
    c_IdElement.appendChild(orc_Doc.createTextNode(c_Id));
    c_ParamsElement.appendChild(c_IdElement);
  }
  c_Element.appendChild(c_ParamsElement);
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigStandaloneChannel::FromQDomDocument(const QDomElement &orc_Element) {
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "parameter-ids") {
      QDomNode c_IdNode = c_Elem.firstChild();
      while (!c_IdNode.isNull()) {
        QDomElement c_IdElem = c_IdNode.toElement();
        if (!c_IdElem.isNull() && c_IdElem.tagName() == "id") {
          c_ParameterIds.append(c_IdElem.text());
        }
        c_IdNode = c_IdNode.nextSibling();
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
