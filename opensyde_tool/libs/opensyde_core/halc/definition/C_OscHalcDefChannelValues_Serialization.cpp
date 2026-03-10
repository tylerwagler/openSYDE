//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Qt-native serialization methods for C_OscHalcDefChannelValues

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "stwerrors.hpp"
#include "C_OscHalcDefChannelValues.hpp"
#include <QJsonArray>
#include <QJsonValue>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelValues::ToQDataStream(QDataStream &ro_DataStream) const {
   // Serialize parameters
   ro_DataStream << static_cast<int32_t>(c_Parameters.size());
   for (const auto &c_Param : c_Parameters) {
      c_Param.ToQDataStream(ro_DataStream);
   }

   // Serialize input values
   ro_DataStream << static_cast<int32_t>(c_InputValues.size());
   for (const auto &c_Input : c_InputValues) {
      c_Input.ToQDataStream(ro_DataStream);
   }

   // Serialize output values
   ro_DataStream << static_cast<int32_t>(c_OutputValues.size());
   for (const auto &c_Output : c_OutputValues) {
      c_Output.ToQDataStream(ro_DataStream);
   }

   // Serialize status values
   ro_DataStream << static_cast<int32_t>(c_StatusValues.size());
   for (const auto &c_Status : c_StatusValues) {
      c_Status.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefChannelValues::FromQDataStream(QDataStream &ro_DataStream) {
   // Deserialize parameters
   int32_t s32_ParamCount;
   ro_DataStream >> s32_ParamCount;
   c_Parameters.clear();
   for (int32_t s32_It = 0; s32_It < s32_ParamCount; ++s32_It) {
      C_OscHalcDefStruct c_Param;
      c_Param.FromQDataStream(ro_DataStream);
      c_Parameters.push_back(c_Param);
   }

   // Deserialize input values
   int32_t s32_InputCount;
   ro_DataStream >> s32_InputCount;
   c_InputValues.clear();
   for (int32_t s32_It = 0; s32_It < s32_InputCount; ++s32_It) {
      C_OscHalcDefStruct c_Input;
      c_Input.FromQDataStream(ro_DataStream);
      c_InputValues.push_back(c_Input);
   }

   // Deserialize output values
   int32_t s32_OutputCount;
   ro_DataStream >> s32_OutputCount;
   c_OutputValues.clear();
   for (int32_t s32_It = 0; s32_It < s32_OutputCount; ++s32_It) {
      C_OscHalcDefStruct c_Output;
      c_Output.FromQDataStream(ro_DataStream);
      c_OutputValues.push_back(c_Output);
   }

   // Deserialize status values
   int32_t s32_StatusCount;
   ro_DataStream >> s32_StatusCount;
   c_StatusValues.clear();
   for (int32_t s32_It = 0; s32_It < s32_StatusCount; ++s32_It) {
      C_OscHalcDefStruct c_Status;
      c_Status.FromQDataStream(ro_DataStream);
      c_StatusValues.push_back(c_Status);
   }
}

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelValues::ToJsonObject() const {
   QJsonObject c_Obj;

   // Serialize parameters
   QJsonArray c_ParamArray;
   for (const auto &c_Param : c_Parameters) {
      c_ParamArray.append(c_Param.ToJsonObject());
   }
   c_Obj["parameters"] = c_ParamArray;

   // Serialize input values
   QJsonArray c_InputArray;
   for (const auto &c_Input : c_InputValues) {
      c_InputArray.append(c_Input.ToJsonObject());
   }
   c_Obj["inputValues"] = c_InputArray;

   // Serialize output values
   QJsonArray c_OutputArray;
   for (const auto &c_Output : c_OutputValues) {
      c_OutputArray.append(c_Output.ToJsonObject());
   }
   c_Obj["outputValues"] = c_OutputArray;

   // Serialize status values
   QJsonArray c_StatusArray;
   for (const auto &c_Status : c_StatusValues) {
      c_StatusArray.append(c_Status.ToJsonObject());
   }
   c_Obj["statusValues"] = c_StatusArray;

   return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelValues::FromJsonObject(const QJsonObject &orc_Object) {
   // Deserialize parameters
   if (orc_Object.contains("parameters")) {
      c_Parameters.clear();
      const QJsonArray c_Array = orc_Object["parameters"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefStruct c_Param;
         c_Param.FromJsonObject(c_Value.toObject());
         c_Parameters.push_back(c_Param);
      }
   }

   // Deserialize input values
   if (orc_Object.contains("inputValues")) {
      c_InputValues.clear();
      const QJsonArray c_Array = orc_Object["inputValues"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefStruct c_Input;
         c_Input.FromJsonObject(c_Value.toObject());
         c_InputValues.push_back(c_Input);
      }
   }

   // Deserialize output values
   if (orc_Object.contains("outputValues")) {
      c_OutputValues.clear();
      const QJsonArray c_Array = orc_Object["outputValues"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefStruct c_Output;
         c_Output.FromJsonObject(c_Value.toObject());
         c_OutputValues.push_back(c_Output);
      }
   }

   // Deserialize status values
   if (orc_Object.contains("statusValues")) {
      c_StatusValues.clear();
      const QJsonArray c_Array = orc_Object["statusValues"].toArray();
      for (const QJsonValue &c_Value : c_Array) {
         C_OscHalcDefStruct c_Status;
         c_Status.FromJsonObject(c_Value.toObject());
         c_StatusValues.push_back(c_Status);
      }
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscHalcDefChannelValues::ToQDomElement(QDomDocument &orc_Doc, const QString &orc_ElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

   // Serialize parameters
   QDomElement c_ParamElem = orc_Doc.createElement("parameters");
   for (const auto &c_Param : c_Parameters) {
      QDomElement c_ParamItemElem = c_Param.ToQDomElement(orc_Doc, "parameter");
      c_ParamElem.appendChild(c_ParamItemElem);
   }
   c_Element.appendChild(c_ParamElem);

   // Serialize input values
   QDomElement c_InputElem = orc_Doc.createElement("inputValues");
   for (const auto &c_Input : c_InputValues) {
      QDomElement c_InputItemElem = c_Input.ToQDomElement(orc_Doc, "input");
      c_InputElem.appendChild(c_InputItemElem);
   }
   c_Element.appendChild(c_InputElem);

   // Serialize output values
   QDomElement c_OutputElem = orc_Doc.createElement("outputValues");
   for (const auto &c_Output : c_OutputValues) {
      QDomElement c_OutputItemElem = c_Output.ToQDomElement(orc_Doc, "output");
      c_OutputElem.appendChild(c_OutputItemElem);
   }
   c_Element.appendChild(c_OutputElem);

   // Serialize status values
   QDomElement c_StatusElem = orc_Doc.createElement("statusValues");
   for (const auto &c_Status : c_StatusValues) {
      QDomElement c_StatusItemElem = c_Status.ToQDomElement(orc_Doc, "status");
      c_StatusElem.appendChild(c_StatusItemElem);
   }
   c_Element.appendChild(c_StatusElem);

   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelValues::FromQDomElement(const QDomElement &orc_Element) {
   // Deserialize parameters
   QDomNodeList c_List = orc_Element.elementsByTagName("parameters");
   if (!c_List.isEmpty()) {
      QDomElement c_ParamElem = c_List.item(0).toElement();
      c_Parameters.clear();
      QDomNode c_Node = c_ParamElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ItemElem = c_Node.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "parameter") {
            C_OscHalcDefStruct c_Param;
            c_Param.FromQDomElement(c_ItemElem);
            c_Parameters.push_back(c_Param);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize input values
   c_List = orc_Element.elementsByTagName("inputValues");
   if (!c_List.isEmpty()) {
      QDomElement c_InputElem = c_List.item(0).toElement();
      c_InputValues.clear();
      QDomNode c_Node = c_InputElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ItemElem = c_Node.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "input") {
            C_OscHalcDefStruct c_Input;
            c_Input.FromQDomElement(c_ItemElem);
            c_InputValues.push_back(c_Input);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize output values
   c_List = orc_Element.elementsByTagName("outputValues");
   if (!c_List.isEmpty()) {
      QDomElement c_OutputElem = c_List.item(0).toElement();
      c_OutputValues.clear();
      QDomNode c_Node = c_OutputElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ItemElem = c_Node.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "output") {
            C_OscHalcDefStruct c_Output;
            c_Output.FromQDomElement(c_ItemElem);
            c_OutputValues.push_back(c_Output);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   // Deserialize status values
   c_List = orc_Element.elementsByTagName("statusValues");
   if (!c_List.isEmpty()) {
      QDomElement c_StatusElem = c_List.item(0).toElement();
      c_StatusValues.clear();
      QDomNode c_Node = c_StatusElem.firstChild();
      while (!c_Node.isNull()) {
         QDomElement c_ItemElem = c_Node.toElement();
         if (!c_ItemElem.isNull() && c_ItemElem.tagName() == "status") {
            C_OscHalcDefStruct c_Status;
            c_Status.FromQDomElement(c_ItemElem);
            c_StatusValues.push_back(c_Status);
         }
         c_Node = c_Node.nextSibling();
      }
   }

   return C_NO_ERR;
}
