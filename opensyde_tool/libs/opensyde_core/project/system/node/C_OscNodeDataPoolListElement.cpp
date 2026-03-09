//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for an element in a list in a data pool
   (implementation)

   Data class for an element in a list in a data pool

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QMutexLocker>
#include <QJsonArray>

#include "C_OscNodeDataPoolListElement.hpp"
#include "C_OscHashUtil.hpp"
#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeDataPoolListElement::C_OscNodeDataPoolListElement(void)
    : c_Name("NewDataElement"), c_Comment(""), c_MinValue(), c_MaxValue(),
      f64_Factor(1.0), f64_Offset(0.0), c_Unit(""), e_Access(eACCESS_RW),
      q_InterpretAsString(false), q_DiagEventCall(false), c_DataSetValues(),
      c_Value(), c_NvmValue(), q_NvmValueChanged(false),
      q_NvmValueIsValid(false), u32_NvmStartAddress(0) {
  this->SetType(C_OscNodeDataPoolContent::eUINT8);
  this->SetArray(false);
  this->c_MinValue.SetValueU8(0U);
  this->c_MaxValue.SetValueU8(255U);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::CalcHash(uint32_t &oru32_HashValue) const {
  hash_util::CalcHashMembers(oru32_HashValue, this->c_Name, this->c_Comment);
  this->c_MinValue.CalcHash(oru32_HashValue);
  this->c_MaxValue.CalcHash(oru32_HashValue);
  hash_util::CalcHashMembers(oru32_HashValue,
                             this->f64_Factor, this->f64_Offset, this->c_Unit, this->e_Access);
  // Only relevant in this case
  if ((this->GetArray() == true) &&
      (this->GetType() == C_OscNodeDataPoolContent::eSINT8)) {
    hash_util::CalcHashMembers(oru32_HashValue, this->q_InterpretAsString);
  }
  hash_util::CalcHashMembers(oru32_HashValue, this->q_DiagEventCall);

  for (uint32_t u32_Counter = 0U; u32_Counter < this->c_DataSetValues.size();
       ++u32_Counter) {
    this->c_DataSetValues[u32_Counter].CalcHash(oru32_HashValue);
  }
  // Do not calculate the value to the CRC
  hash_util::CalcHashMembers(oru32_HashValue, this->u32_NvmStartAddress);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over one element

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value \param[in]      ou32_Index       Index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::CalcHashElement(
    uint32_t &oru32_HashValue, const uint32_t ou32_Index) const {
  hash_util::CalcHashMembers(oru32_HashValue, this->c_Name, this->c_Comment);
  this->c_MinValue.CalcHashElement(oru32_HashValue, ou32_Index);
  this->c_MaxValue.CalcHashElement(oru32_HashValue, ou32_Index);
  hash_util::CalcHashMembers(oru32_HashValue,
                             this->f64_Factor, this->f64_Offset, this->c_Unit,
                             this->e_Access, this->q_DiagEventCall);

  for (uint32_t u32_Counter = 0U; u32_Counter < this->c_DataSetValues.size();
       ++u32_Counter) {
    this->c_DataSetValues[u32_Counter].CalcHashElement(oru32_HashValue,
                                                       ou32_Index);
  }
  // Do not calculate the value to the CRC
  hash_util::CalcHashMembers(oru32_HashValue, this->u32_NvmStartAddress);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over structure only

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::CalcHashStructure(
    uint32_t &oru32_HashValue) const {
  hash_util::CalcHashMembers(oru32_HashValue, this->c_Name, this->c_Comment);
  this->c_MinValue.CalcHashStructure(oru32_HashValue);
  this->c_MaxValue.CalcHashStructure(oru32_HashValue);
  hash_util::CalcHashMembers(oru32_HashValue,
                             this->f64_Factor, this->f64_Offset, this->c_Unit,
                             this->e_Access, this->q_DiagEventCall);

  for (uint32_t u32_Counter = 0U; u32_Counter < this->c_DataSetValues.size();
       ++u32_Counter) {
    this->c_DataSetValues[u32_Counter].CalcHashStructure(oru32_HashValue);
  }
  // Do not calculate the value to the CRC
  hash_util::CalcHashMembers(oru32_HashValue, this->u32_NvmStartAddress);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get size in bytes

   \return
   size of element in bytes
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscNodeDataPoolListElement::GetSizeByte(void) const {
  return this->c_Value.GetSizeByte();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Change internal type

   Change type of all variables which are of variable type

   \param[in]  ore_Value   New type value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::SetType(
    const C_OscNodeDataPoolContent::E_Type &ore_Value) {
  c_MinValue.SetType(ore_Value);
  c_MaxValue.SetType(ore_Value);
  c_Value.SetType(ore_Value);
  c_NvmValue.SetType(ore_Value);
  for (uint32_t u32_It = 0; u32_It < c_DataSetValues.size(); ++u32_It) {
    c_DataSetValues[u32_It].SetType(ore_Value);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Change array status

   \param[in]  oq_Array    New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::SetArray(const bool oq_Array) {
  c_MinValue.SetArray(oq_Array);
  c_MaxValue.SetArray(oq_Array);
  c_Value.SetArray(oq_Array);
  c_NvmValue.SetArray(oq_Array);
  for (uint32_t u32_It = 0; u32_It < c_DataSetValues.size(); ++u32_It) {
    c_DataSetValues[u32_It].SetArray(oq_Array);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resize current array

   \param[in]  oru32_Size  New size value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::SetArraySize(const uint32_t &oru32_Size) {
  c_MinValue.SetArraySize(oru32_Size);
  c_MaxValue.SetArraySize(oru32_Size);
  c_Value.SetArraySize(oru32_Size);
  c_NvmValue.SetArraySize(oru32_Size);
  for (uint32_t u32_It = 0; u32_It < c_DataSetValues.size(); ++u32_It) {
    c_DataSetValues[u32_It].SetArraySize(oru32_Size);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get internal type

   \return
   Internal type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeDataPoolContent::E_Type
C_OscNodeDataPoolListElement::GetType(void) const {
  return this->c_Value.GetType();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get array status

   \return
   Array status
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscNodeDataPoolListElement::GetArray(void) const {
  return this->c_Value.GetArray();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current array size

   \return
   Current array size
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscNodeDataPoolListElement::GetArraySize(void) const {
  return this->c_Value.GetArraySize();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks the current value for its range

   The function checks c_Value against the minimum (c_MinValue) and maximum
   (c_MaxValue)

   \return
   C_NO_ERR   Current value is valid
   C_RANGE    Current value is invalid
   C_CONFIG   Wrong types are set in minimum or maximum
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolListElement::CheckValueRange(void) const {
  int32_t s32_Return = C_RANGE;

  try {
    if ((this->c_Value.GetArray() != this->c_MaxValue.GetArray()) ||
        (this->c_Value.GetArray() != this->c_MinValue.GetArray()) ||
        (this->c_Value.GetType() != this->c_MaxValue.GetType()) ||
        (this->c_Value.GetType() != this->c_MinValue.GetType())) {
      s32_Return = C_CONFIG;
    } else {
      const bool q_Valid = (((this->c_Value >= this->c_MinValue) &&
                             (this->c_Value <= this->c_MaxValue))
                                ? true
                                : false);

      if (q_Valid == true) {
        s32_Return = C_NO_ERR;
      }
    }
  } catch (...) {
    s32_Return = C_CONFIG;
  }

  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks the current NVM value for its range

   The function checks c_NvmValue against the minimum (c_MinValue) and maximum
   (c_MaxValue)

   \return
   C_NO_ERR   Current value is valid
   C_RANGE    Current value is invalid
   C_CONFIG   Wrong types are set in minimum or maximum
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolListElement::CheckNvmValueRange(void) const {
  int32_t s32_Return = C_RANGE;

  try {
    if ((this->c_NvmValue.GetArray() != this->c_MaxValue.GetArray()) ||
        (this->c_NvmValue.GetArray() != this->c_MinValue.GetArray()) ||
        (this->c_NvmValue.GetType() != this->c_MaxValue.GetType()) ||
        (this->c_NvmValue.GetType() != this->c_MinValue.GetType())) {
      s32_Return = C_CONFIG;
    } else {
      const bool q_Valid = (((this->c_NvmValue >= this->c_MinValue) &&
                             (this->c_NvmValue <= this->c_MaxValue))
                                ? true
                                : false);

      if (q_Valid == true) {
        s32_Return = C_NO_ERR;
      }
    }
  } catch (...) {
    s32_Return = C_CONFIG;
  }

  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_Name << c_Comment << f64_Factor << f64_Offset << c_Unit
                << static_cast<int32_t>(e_Access) << q_InterpretAsString
                << q_DiagEventCall << u32_NvmStartAddress << q_NvmValueChanged
                << q_NvmValueIsValid;
  
  // Serialize C_OscNodeDataPoolContent objects
  c_MinValue.ToQDataStream(ro_DataStream);
  c_MaxValue.ToQDataStream(ro_DataStream);
  c_Value.ToQDataStream(ro_DataStream);
  c_NvmValue.ToQDataStream(ro_DataStream);
  
  // Serialize dataSetValues list
  ro_DataStream << static_cast<int32_t>(c_DataSetValues.size());
  for (const C_OscNodeDataPoolContent &c_Value : c_DataSetValues) {
    c_Value.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::FromQDataStream(QDataStream &ro_DataStream) {
  int32_t s32_Access, s32_Count;
  ro_DataStream >> c_Name >> c_Comment >> f64_Factor >> f64_Offset >> c_Unit
      >> s32_Access >> q_InterpretAsString >> q_DiagEventCall >> u32_NvmStartAddress
      >> q_NvmValueChanged >> q_NvmValueIsValid;
  e_Access = static_cast<E_Access>(s32_Access);
  
  // Deserialize C_OscNodeDataPoolContent objects
  c_MinValue.FromQDataStream(ro_DataStream);
  c_MaxValue.FromQDataStream(ro_DataStream);
  c_Value.FromQDataStream(ro_DataStream);
  c_NvmValue.FromQDataStream(ro_DataStream);
  
  // Deserialize dataSetValues list
  c_DataSetValues.clear();
  ro_DataStream >> s32_Count;
  for (int32_t i = 0; i < s32_Count; ++i) {
    C_OscNodeDataPoolContent c_Value;
    c_Value.FromQDataStream(ro_DataStream);
    c_DataSetValues.append(c_Value);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolListElement::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["name"] = c_Name;
  c_Obj["comment"] = c_Comment;
  c_Obj["factor"] = f64_Factor;
  c_Obj["offset"] = f64_Offset;
  c_Obj["unit"] = c_Unit;
  c_Obj["access"] = (e_Access == eACCESS_RW) ? "read-write" : "read-only";
  c_Obj["interpret-as-string"] = q_InterpretAsString;
  c_Obj["diag-event-call"] = q_DiagEventCall;
  c_Obj["nvm-start-address"] = static_cast<int32_t>(u32_NvmStartAddress);
  c_Obj["nvm-value-changed"] = q_NvmValueChanged;
  c_Obj["nvm-value-is-valid"] = q_NvmValueIsValid;
  
  c_Obj["min-value"] = c_MinValue.ToJsonObject();
  c_Obj["max-value"] = c_MaxValue.ToJsonObject();
  c_Obj["value"] = c_Value.ToJsonObject();
  c_Obj["nvm-value"] = c_NvmValue.ToJsonObject();
  
  QJsonArray c_DataSetArray;
  for (const C_OscNodeDataPoolContent &c_Value : c_DataSetValues) {
    c_DataSetArray.append(c_Value.ToJsonObject());
  }
  c_Obj["dataset-values"] = c_DataSetArray;
  
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("comment")) c_Comment = orc_Object["comment"].toString();
  if (orc_Object.contains("factor")) f64_Factor = orc_Object["factor"].toDouble();
  if (orc_Object.contains("offset")) f64_Offset = orc_Object["offset"].toDouble();
  if (orc_Object.contains("unit")) c_Unit = orc_Object["unit"].toString();
  if (orc_Object.contains("access")) {
    e_Access = (orc_Object["access"].toString() == "read-write") ? eACCESS_RW : eACCESS_RO;
  }
  if (orc_Object.contains("interpret-as-string")) q_InterpretAsString = orc_Object["interpret-as-string"].toBool();
  if (orc_Object.contains("diag-event-call")) q_DiagEventCall = orc_Object["diag-event-call"].toBool();
  if (orc_Object.contains("nvm-start-address")) u32_NvmStartAddress = static_cast<uint32_t>(orc_Object["nvm-start-address"].toInt());
  if (orc_Object.contains("nvm-value-changed")) q_NvmValueChanged = orc_Object["nvm-value-changed"].toBool();
  if (orc_Object.contains("nvm-value-is-valid")) q_NvmValueIsValid = orc_Object["nvm-value-is-valid"].toBool();
  
  if (orc_Object.contains("min-value")) c_MinValue.FromJsonObject(orc_Object["min-value"].toObject());
  if (orc_Object.contains("max-value")) c_MaxValue.FromJsonObject(orc_Object["max-value"].toObject());
  if (orc_Object.contains("value")) c_Value.FromJsonObject(orc_Object["value"].toObject());
  if (orc_Object.contains("nvm-value")) c_NvmValue.FromJsonObject(orc_Object["nvm-value"].toObject());
  
  if (orc_Object.contains("dataset-values")) {
    c_DataSetValues.clear();
    QJsonArray c_Array = orc_Object["dataset-values"].toArray();
    for (const QJsonValue &c_Value : c_Array) {
      C_OscNodeDataPoolContent c_Content;
      c_Content.FromJsonObject(c_Value.toObject());
      c_DataSetValues.append(c_Content);
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
QDomElement C_OscNodeDataPoolListElement::ToQDomDocument(QDomDocument &orc_Doc,
                                                         const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("name", c_Name);
  c_Element.setAttribute("factor", f64_Factor);
  c_Element.setAttribute("offset", f64_Offset);
  c_Element.setAttribute("access", (e_Access == eACCESS_RW) ? "read-write" : "read-only");
  c_Element.setAttribute("interpret-as-string", q_InterpretAsString);
  c_Element.setAttribute("diag-event-call", q_DiagEventCall);
  c_Element.setAttribute("nvm-start-address", static_cast<int32_t>(u32_NvmStartAddress));
  c_Element.setAttribute("nvm-value-changed", q_NvmValueChanged);
  c_Element.setAttribute("nvm-value-is-valid", q_NvmValueIsValid);
  
  if (!c_Comment.isEmpty()) {
    QDomElement c_CommentElem = orc_Doc.createElement("comment");
    c_CommentElem.appendChild(orc_Doc.createTextNode(c_Comment));
    c_Element.appendChild(c_CommentElem);
  }
  if (!c_Unit.isEmpty()) {
    QDomElement c_UnitElem = orc_Doc.createElement("unit");
    c_UnitElem.appendChild(orc_Doc.createTextNode(c_Unit));
    c_Element.appendChild(c_UnitElem);
  }
  
  QDomElement c_MinElem = c_MinValue.ToQDomDocument(orc_Doc, "min-value");
  c_Element.appendChild(c_MinElem);
  QDomElement c_MaxElem = c_MaxValue.ToQDomDocument(orc_Doc, "max-value");
  c_Element.appendChild(c_MaxElem);
  QDomElement c_ValueElem = c_Value.ToQDomDocument(orc_Doc, "value");
  c_Element.appendChild(c_ValueElem);
  QDomElement c_NvmValueElem = c_NvmValue.ToQDomDocument(orc_Doc, "nvm-value");
  c_Element.appendChild(c_NvmValueElem);
  
  if (!c_DataSetValues.isEmpty()) {
    QDomElement c_DataSetElem = orc_Doc.createElement("dataset-values");
    for (const C_OscNodeDataPoolContent &c_Value : c_DataSetValues) {
      QDomElement c_DataValueElem = c_Value.ToQDomDocument(orc_Doc, "dataset-value");
      c_DataSetElem.appendChild(c_DataValueElem);
    }
    c_Element.appendChild(c_DataSetElem);
  }
  
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolListElement::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  if (orc_Element.hasAttribute("factor")) f64_Factor = orc_Element.attribute("factor").toDouble();
  if (orc_Element.hasAttribute("offset")) f64_Offset = orc_Element.attribute("offset").toDouble();
  if (orc_Element.hasAttribute("access")) {
    e_Access = (orc_Element.attribute("access") == "read-write") ? eACCESS_RW : eACCESS_RO;
  }
  if (orc_Element.hasAttribute("interpret-as-string")) q_InterpretAsString = orc_Element.attribute("interpret-as-string").toInt();
  if (orc_Element.hasAttribute("diag-event-call")) q_DiagEventCall = orc_Element.attribute("diag-event-call").toInt();
  if (orc_Element.hasAttribute("nvm-start-address")) u32_NvmStartAddress = orc_Element.attribute("nvm-start-address").toUInt();
  if (orc_Element.hasAttribute("nvm-value-changed")) q_NvmValueChanged = orc_Element.attribute("nvm-value-changed").toInt();
  if (orc_Element.hasAttribute("nvm-value-is-valid")) q_NvmValueIsValid = orc_Element.attribute("nvm-value-is-valid").toInt();
  
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_Tag = c_Elem.tagName();
      if (c_Tag == "comment") c_Comment = c_Elem.text();
      else if (c_Tag == "unit") c_Unit = c_Elem.text();
      else if (c_Tag == "min-value") c_MinValue.FromQDomDocument(c_Elem);
      else if (c_Tag == "max-value") c_MaxValue.FromQDomDocument(c_Elem);
      else if (c_Tag == "value") c_Value.FromQDomDocument(c_Elem);
      else if (c_Tag == "nvm-value") c_NvmValue.FromQDomDocument(c_Elem);
      else if (c_Tag == "dataset-values") {
        c_DataSetValues.clear();
        QDomNode c_DataNode = c_Elem.firstChild();
        while (!c_DataNode.isNull()) {
          QDomElement c_DataElem = c_DataNode.toElement();
          if (!c_DataElem.isNull() && c_DataElem.tagName() == "dataset-value") {
            C_OscNodeDataPoolContent c_Content;
            c_Content.FromQDomDocument(c_DataElem);
            c_DataSetValues.append(c_Content);
          }
          c_DataNode = c_DataNode.nextSibling();
        }
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
