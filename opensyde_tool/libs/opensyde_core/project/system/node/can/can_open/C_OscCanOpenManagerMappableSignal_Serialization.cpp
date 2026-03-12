//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscCanOpenManagerMappableSignal

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscCanOpenManagerMappableSignal.hpp"

using namespace stw::opensyde_core;

//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenManagerMappableSignal::C_OscCanOpenManagerMappableSignal()
   : q_AutoMinMaxUsed(false) {}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerMappableSignal::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << q_AutoMinMaxUsed;
   c_SignalData.ToQDataStream(ro_DataStream);
   c_DatapoolData.ToQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerMappableSignal::FromQDataStream(QDataStream &ro_DataStream) {
   ro_DataStream >> q_AutoMinMaxUsed;
   c_SignalData.FromQDataStream(ro_DataStream);
   c_DatapoolData.FromQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanOpenManagerMappableSignal::ToJsonObject() const {
   QJsonObject c_Obj;

   c_Obj["autoMinMaxUsed"] = q_AutoMinMaxUsed;
   c_Obj["signalData"] = c_SignalData.ToJsonObject();
   c_Obj["datapoolData"] = c_DatapoolData.ToJsonObject();

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerMappableSignal::FromJsonObject(const QJsonObject &ro_Json) {
   if (ro_Json.contains("autoMinMaxUsed")) {
      q_AutoMinMaxUsed = ro_Json["autoMinMaxUsed"].toBool();
   }
   if (ro_Json.contains("signalData")) {
      c_SignalData.FromJsonObject(ro_Json["signalData"].toObject());
   }
   if (ro_Json.contains("datapoolData")) {
      c_DatapoolData.FromJsonObject(ro_Json["datapoolData"].toObject());
   }
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscCanOpenManagerMappableSignal::ToQDomDocument(QDomDocument &ro_Doc,
                                                              const QString &orc_ElementName) const {
   QDomElement c_Element = ro_Doc.createElement(orc_ElementName);

   QDomElement c_AutoMinMaxElement = ro_Doc.createElement("autoMinMaxUsed");
   c_AutoMinMaxElement.appendChild(ro_Doc.createTextNode(QString::number(q_AutoMinMaxUsed ? 1 : 0)));
   c_Element.appendChild(c_AutoMinMaxElement);

   QDomElement c_SignalElement = c_SignalData.ToQDomDocument(ro_Doc, "signalData");
   c_Element.appendChild(c_SignalElement);

   QDomElement c_DatapoolElement = c_DatapoolData.ToQDomDocument(ro_Doc, "datapoolData");
   c_Element.appendChild(c_DatapoolElement);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerMappableSignal::FromQDomDocument(const QDomElement &ro_Element) {
   QDomNode c_Node = ro_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull()) {
         const QString c_TagName = c_Elem.tagName();

         if (c_TagName == "autoMinMaxUsed") {
            q_AutoMinMaxUsed = (c_Elem.text().toInt() == 1);
         } else if (c_TagName == "signalData") {
            c_SignalData.FromQDomDocument(c_Elem);
         } else if (c_TagName == "datapoolData") {
            c_DatapoolData.FromQDomDocument(c_Elem);
         }
      }
      c_Node = c_Node.nextSibling();
   }
}
