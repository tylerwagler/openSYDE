//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefChannelAvailability

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefChannelAvailability.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelAvailability::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize basic fields
   ro_DataStream << u32_ValueIndex;

   // Serialize dependent values
   ro_DataStream << static_cast<int32_t>(c_DependentValues.size());
   for (const auto &c_Value : c_DependentValues) {
      ro_DataStream << c_Value;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelAvailability::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize basic fields
   ro_DataStream >> u32_ValueIndex;

   // Deserialize dependent values
   int32_t s32_Count;
   ro_DataStream >> s32_Count;
   c_DependentValues.clear();
   for (int32_t s32_It = 0; s32_It < s32_Count; ++s32_It) {
      uint32_t u32_Value;
      ro_DataStream >> u32_Value;
      c_DependentValues.push_back(u32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelAvailability::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize basic fields
   c_Obj["valueIndex"] = static_cast<double>(u32_ValueIndex);

   // Serialize dependent values
   QJsonArray c_DependentArray;
   for (const auto &c_Value : c_DependentValues) {
      c_DependentArray.append(static_cast<double>(c_Value));
   }
   c_Obj["dependentValues"] = c_DependentArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelAvailability::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize basic fields
   if (orc_Object.contains("valueIndex")) {
      u32_ValueIndex = static_cast<uint32_t>(orc_Object["valueIndex"].toDouble());
   }

   // Deserialize dependent values
   if (orc_Object.contains("dependentValues")) {
      c_DependentValues.clear();
      const QJsonArray c_Array = orc_Object["dependentValues"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         c_DependentValues.push_back(static_cast<uint32_t>(c_Value.toDouble()));
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefChannelAvailability::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize basic fields as attributes
   c_Element.setAttribute("valueIndex", QString::number(u32_ValueIndex));

   // Serialize dependent values
   QDomElement c_DependentElem = orc_Doc.createElement("dependentValues");
   for (const auto &c_Value : c_DependentValues) {
      QDomElement c_ValueElem = orc_Doc.createElement("value");
      c_ValueElem.setAttribute("index", QString::number(c_Value));
      c_DependentElem.appendChild(c_ValueElem);
   }
   c_Element.appendChild(c_DependentElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelAvailability::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize basic fields from attributes
   if (orc_Element.hasAttribute("valueIndex")) {
      bool q_Valid;
      u32_ValueIndex = static_cast<uint32_t>(orc_Element.attribute("valueIndex").toUInt(&q_Valid));
      Q_ASSERT(q_Valid);
   }

   // Deserialize dependent values
   QDomNodeList c_List = orc_Element.elementsByTagName("dependentValues");
   if (!c_List.isEmpty()) {
      QDomElement c_DependentElem = c_List.item(0).toElement();
      c_DependentValues.clear();
      QDomNode c_Node = c_DependentElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ValueElem = c_Node.toElement();
         if (!c_ValueElem.isNull()) {
            bool q_Valid;
            uint32_t u32_Value = static_cast<uint32_t>(c_ValueElem.attribute("index").toUInt(&q_Valid));
            if (q_Valid) {
               c_DependentValues.push_back(u32_Value);
            }
         }
         c_Node = c_Node.nextSibling();
      }
   }

   return C_NO_ERR;
}
