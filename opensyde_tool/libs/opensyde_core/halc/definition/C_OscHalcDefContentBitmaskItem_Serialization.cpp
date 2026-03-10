//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefContentBitmaskItem

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefContentBitmaskItem.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContentBitmaskItem::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << c_Display;
   ro_DataStream << c_Comment;
   ro_DataStream << q_ApplyValueSetting;
   ro_DataStream << u64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefContentBitmaskItem::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> c_Display;
   ro_DataStream >> c_Comment;
   ro_DataStream >> q_ApplyValueSetting;
   ro_DataStream >> u64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefContentBitmaskItem::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize basic fields
   c_Obj["display"] = c_Display;
   c_Obj["comment"] = c_Comment;
   c_Obj["applyValueSetting"] = q_ApplyValueSetting;
   c_Obj["value"] = static_cast<double>(u64_Value);

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentBitmaskItem::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize basic fields
   if (orc_Object.contains("display")) {
      c_Display = orc_Object["display"].toString();
   }
   if (orc_Object.contains("comment")) {
      c_Comment = orc_Object["comment"].toString();
   }
   if (orc_Object.contains("applyValueSetting")) {
      q_ApplyValueSetting = orc_Object["applyValueSetting"].toBool();
   }
   if (orc_Object.contains("value")) {
      u64_Value = static_cast<uint64_t>(orc_Object["value"].toDouble());
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefContentBitmaskItem::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("display", c_Display);
   if (!c_Comment.isEmpty()) {
      c_Element.setAttribute("comment", c_Comment);
   }
   c_Element.setAttribute("applyValueSetting", q_ApplyValueSetting ? "true" : "false");
   c_Element.setAttribute("value", QString::number(u64_Value));

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefContentBitmaskItem::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize basic fields from attributes
   if (orc_Element.hasAttribute("display")) {
      c_Display = orc_Element.attribute("display");
   }
   if (orc_Element.hasAttribute("comment")) {
      c_Comment = orc_Element.attribute("comment");
   }
   if (orc_Element.hasAttribute("applyValueSetting")) {
      q_ApplyValueSetting = (orc_Element.attribute("applyValueSetting") == "true");
   }
   if (orc_Element.hasAttribute("value")) {
      bool q_Valid;
      u64_Value = static_cast<uint64_t>(orc_Element.attribute("value").toULongLong(&q_Valid));
      Q_ASSERT(q_Valid);
   }

   return C_NO_ERR;
}
