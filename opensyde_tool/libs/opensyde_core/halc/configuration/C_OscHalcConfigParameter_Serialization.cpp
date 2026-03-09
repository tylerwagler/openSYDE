//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcConfigParameter

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcConfigParameter.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameter::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize the value (C_OscHalcDefContent)
   c_Value.ToQDataStream(ro_DataStream);
   
   // Serialize comment
   ro_DataStream << c_Comment;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameter::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize the value
   c_Value.FromQDataStream(ro_DataStream);
   
   // Deserialize comment
   ro_DataStream >> c_Comment;
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcConfigParameter::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["value"] = c_Value.ToJsonObject();
   c_Obj["comment"] = c_Comment;
   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameter::FromJsonObject(const QJsonObject &orc_Object) {
   if (orc_Object.contains("value")) {
      c_Value.FromJsonObject(orc_Object["value"].toObject());
   }
   if (orc_Object.contains("comment")) {
      c_Comment = orc_Object["comment"].toString();
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcConfigParameter::ToQDomDocument(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
   
   // Serialize value
   QDomElement c_ValueElem = c_Value.ToQDomDocument(orc_Doc, "value");
   c_Element.appendChild(c_ValueElem);
   
   // Add comment as attribute if present
   if (!c_Comment.isEmpty()) {
      c_Element.setAttribute("comment", c_Comment);
   }
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcConfigParameter::FromQDomDocument(const QDomElement &orc_Element) {
   // Deserialize value
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "value") {
         c_Value.FromQDomDocument(c_Elem);
         break;
      }
      c_Node = c_Node.nextSibling();
   }
   
   // Deserialize comment
   if (orc_Element.hasAttribute("comment")) {
      c_Comment = orc_Element.attribute("comment");
   }
}
