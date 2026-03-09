//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for a list in a data pool (implementation)

   Data class for a list in a data pool

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include <QString>

#include "C_OscNodeDataPoolList.hpp"
#include <QJsonArray>
#include <QJsonValue>

#include "C_OscHashUtil.hpp"
#include "C_OscUtils.hpp"
#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const uint32_t C_OscNodeDataPoolList::hu32_DEFAULT_NVM_SIZE = 100UL;

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
C_OscNodeDataPoolList::C_OscNodeDataPoolList(void)
    : c_Name("NewList"), c_Comment(""), q_NvmCrcActive(false), u32_NvmCrc(0),
      u32_NvmStartAddress(0), u32_NvmSize(0), c_Elements(), c_DataSets() {
  c_Elements.resize(1);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out] oru32_HashValue    Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::CalcHash(uint32_t &oru32_HashValue) const {
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_Name, this->c_Comment,
                              this->u32_NvmCrc, this->u32_NvmStartAddress, this->u32_NvmSize,
                              this->c_Elements, this->c_DataSets);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Recalculate data pool list element addresses
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::RecalculateAddress(void) {
  uint32_t u32_Offset = this->u32_NvmStartAddress;

  // If there is a CRC two bytes are reserved at the start
  if (this->q_NvmCrcActive == true) {
    u32_Offset += 2;
  }

  for (uint32_t u32_ItDataElement = 0;
       u32_ItDataElement < this->c_Elements.size(); ++u32_ItDataElement) {
    C_OscNodeDataPoolListElement &rc_CurElem =
        this->c_Elements[u32_ItDataElement];
    rc_CurElem.u32_NvmStartAddress = u32_Offset;
    u32_Offset += rc_CurElem.GetSizeByte();
  }
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Move list in data pool

   \param[in] oru32_Start  Start index
   \param[in] oru32_Target Target index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::MoveElement(const uint32_t &oru32_Start,
                                        const uint32_t &oru32_Target) {
  if ((oru32_Start < this->c_Elements.size()) &&
      (oru32_Target < this->c_Elements.size())) {
    // Copy
    const C_OscNodeDataPoolListElement c_ListElementData =
        this->c_Elements[oru32_Start];
    // Erase
    this->c_Elements.erase(this->c_Elements.begin() + oru32_Start);
    // Insert
    this->c_Elements.insert(this->c_Elements.begin() + oru32_Target,
                            c_ListElementData);
    RecalculateAddress();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get number of bytes occupied by variables (including CRC space)

   This only counts the raw data bytes. So any possible alignment gaps when
   handling the data in a "struct" will not be considered. If the list is
   configured to be CRCed 2 bytes will be added to the result.

   \return
   Number of bytes occupied by variables (including CRC space)
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscNodeDataPoolList::GetNumBytesUsed(void) const {
  uint32_t u32_Retval = 0;

  if (this->q_NvmCrcActive == true) {
    u32_Retval += 2;
  }
  for (uint32_t u32_ItListElement = 0;
       u32_ItListElement < this->c_Elements.size(); ++u32_ItListElement) {
    const C_OscNodeDataPoolListElement &rc_NodeDataPoolListElement =
        this->c_Elements[u32_ItListElement];
    u32_Retval += rc_NodeDataPoolListElement.GetSizeByte();
  }
  return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get number of bytes not occupied by variables

   \return
   Number of bytes not occupied by variables
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolList::GetFreeBytes(void) const {
  return static_cast<int32_t>(static_cast<int64_t>(u32_NvmSize) -
                              static_cast<int64_t>(GetNumBytesUsed()));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check error for specified data set

   \param[in]  oru32_DataSetIndex  Node data pool list data set index
   \param[out] opq_NameConflict    Name conflict
   \param[out] opq_NameInvalid     Name not usable as variable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::CheckErrorDataSet(
    const uint32_t &oru32_DataSetIndex, bool *const opq_NameConflict,
    bool *const opq_NameInvalid) const {
  // Init
  if (oru32_DataSetIndex < this->c_DataSets.size()) {
    const C_OscNodeDataPoolDataSet &rc_CurrentElement =
        this->c_DataSets[oru32_DataSetIndex];
    // Check variable name
    if (opq_NameInvalid != NULL) {
      if (C_OscUtils::h_CheckValidCeName(rc_CurrentElement.c_Name) == false) {
        *opq_NameInvalid = true;
      } else {
        *opq_NameInvalid = false;
      }
    }
    // Name conflict
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
      for (uint32_t u32_ItDataSet = 0;
           (u32_ItDataSet < this->c_DataSets.size()) &&
           (*opq_NameConflict == false);
           ++u32_ItDataSet) {
        if (u32_ItDataSet != oru32_DataSetIndex) {
          const C_OscNodeDataPoolDataSet &rc_DataSet =
              this->c_DataSets[u32_ItDataSet];
          if (rc_CurrentElement.c_Name.toLower() ==
              rc_DataSet.c_Name.toLower()) {
            *opq_NameConflict = true;
          }
        }
      }
    }
  } else {
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
    }
    if (opq_NameInvalid != NULL) {
      *opq_NameInvalid = false;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check error for specified element

   \param[in]  oru32_ElementIndex         Node data pool list element index
   \param[out] opq_NameConflict           Name conflict
   \param[out] opq_NameInvalid            Name not usable as variable
   \param[out] opq_MinOverMax             Minimum value over maximum value
   \param[out] opq_DataSetValueInvalid    Data set value out of range
   \param[out] opc_InvalidDataSetIndices  List of indexes of troublesome data
   sets
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::CheckErrorElement(
    const uint32_t &oru32_ElementIndex, bool *const opq_NameConflict,
    bool *const opq_NameInvalid, bool *const opq_MinOverMax,
    bool *const opq_DataSetValueInvalid,
    QList<uint32_t> *const opc_InvalidDataSetIndices) const {
  // Init
  if (oru32_ElementIndex < this->c_Elements.size()) {
    const C_OscNodeDataPoolListElement &rc_CurrentElement =
        this->c_Elements[oru32_ElementIndex];
    // Check variable name
    if (opq_NameInvalid != NULL) {
      if (C_OscUtils::h_CheckValidCeName(rc_CurrentElement.c_Name) == false) {
        *opq_NameInvalid = true;
      } else {
        *opq_NameInvalid = false;
      }
    }
    // Name conflict
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
      for (uint32_t u32_ItElement = 0;
           (u32_ItElement < this->c_Elements.size()) &&
           (*opq_NameConflict == false);
           ++u32_ItElement) {
        if (u32_ItElement != oru32_ElementIndex) {
          const C_OscNodeDataPoolListElement &rc_ListElement =
              this->c_Elements[u32_ItElement];
          if (rc_CurrentElement.c_Name.toLower() ==
              rc_ListElement.c_Name.toLower()) {
            *opq_NameConflict = true;
          }
        }
      }
    }
    // Check min max
    if (opq_MinOverMax != NULL) {
      if (rc_CurrentElement.c_MinValue <= rc_CurrentElement.c_MaxValue) {
        *opq_MinOverMax = false;
      } else {
        *opq_MinOverMax = true;
      }
    }
    // Check data sets
    if (opq_DataSetValueInvalid != NULL) {
      *opq_DataSetValueInvalid = false;
      for (uint32_t u32_ItDataSet = 0;
           u32_ItDataSet < rc_CurrentElement.c_DataSetValues.size();
           ++u32_ItDataSet) {
        bool q_ValueBelowMin = false;
        bool q_ValueOverMax = false;
        this->CheckErrorDataSetValue(oru32_ElementIndex, u32_ItDataSet,
                                     &q_ValueBelowMin, &q_ValueOverMax, NULL);
        if ((q_ValueBelowMin == true) || (q_ValueOverMax == true)) {
          *opq_DataSetValueInvalid = true;
          if (opc_InvalidDataSetIndices != NULL) {
            opc_InvalidDataSetIndices->push_back(u32_ItDataSet);
          }
        }
      }
    }
  } else {
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
    }
    if (opq_NameInvalid != NULL) {
      *opq_NameInvalid = false;
    }
    if (opq_MinOverMax != NULL) {
      *opq_MinOverMax = false;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check error for specified data set value

   \param[in]  oru32_ElementIndex Node data pool list element index
   \param[in]  oru32_DataSetIndex Node data pool list data set index
   \param[out] opq_ValueBelowMin  Data set value below minimum
   \param[out] opq_ValueOverMax   Data set value over maximum
   \param[in] opu32_ArrayIndex   Optional parameter to check only a single data
   set array index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::CheckErrorDataSetValue(
    const uint32_t &oru32_ElementIndex, const uint32_t &oru32_DataSetIndex,
    bool *const opq_ValueBelowMin, bool *const opq_ValueOverMax,
    const uint32_t *const opu32_ArrayIndex) const {
  if (opq_ValueBelowMin != NULL) {
    *opq_ValueBelowMin = false;
  }
  if (opq_ValueOverMax != NULL) {
    *opq_ValueOverMax = false;
  }
  if (oru32_ElementIndex < this->c_Elements.size()) {
    const C_OscNodeDataPoolListElement &rc_CurrentElement =
        this->c_Elements[oru32_ElementIndex];
    if (oru32_DataSetIndex < rc_CurrentElement.c_DataSetValues.size()) {
      const C_OscNodeDataPoolContent &rc_DataSetValue =
          rc_CurrentElement.c_DataSetValues[oru32_DataSetIndex];
      if (opq_ValueBelowMin != NULL) {
        *opq_ValueBelowMin = false;
        if (opu32_ArrayIndex == NULL) {
          if ((rc_DataSetValue >= rc_CurrentElement.c_MinValue) == false) {
            *opq_ValueBelowMin = true;
          }
        } else {
          if (rc_DataSetValue.CompareArrayGreaterOrEqual(
                  rc_CurrentElement.c_MinValue, *opu32_ArrayIndex) == false) {
            *opq_ValueBelowMin = true;
          }
        }
      }
      if (opq_ValueOverMax != NULL) {
        *opq_ValueOverMax = false;
        if (opu32_ArrayIndex == NULL) {
          if ((rc_DataSetValue <= rc_CurrentElement.c_MaxValue) == false) {
            *opq_ValueOverMax = true;
          }
        } else {
          if (rc_DataSetValue.CompareArrayGreater(rc_CurrentElement.c_MaxValue,
                                                  *opu32_ArrayIndex) == true) {
            *opq_ValueOverMax = true;
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle name max char limit

   \param[in]      ou32_NameMaxCharLimit  Name max char limit
   \param[in,out]  opc_ChangedItems       Changed items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::HandleNameMaxCharLimit(
    const uint32_t ou32_NameMaxCharLimit,
    QList<C_OscSystemNameMaxCharLimitChangeReportItem>
        *const opc_ChangedItems) {
  C_OscSystemNameMaxCharLimitChangeReportItem::h_HandleNameMaxCharLimitItem(
      ou32_NameMaxCharLimit, "node-datapool-list-name", this->c_Name,
      opc_ChangedItems);
  for (uint32_t u32_ItEl = 0UL; u32_ItEl < this->c_Elements.size();
       ++u32_ItEl) {
    C_OscNodeDataPoolListElement &rc_El = this->c_Elements[u32_ItEl];
    C_OscSystemNameMaxCharLimitChangeReportItem::h_HandleNameMaxCharLimitItem(
        ou32_NameMaxCharLimit, "node-datapool-list-element-name", rc_El.c_Name,
        opc_ChangedItems);
  }
  for (uint32_t u32_ItDataset = 0UL; u32_ItDataset < this->c_DataSets.size();
       ++u32_ItDataset) {
    C_OscNodeDataPoolDataSet &rc_Dataset = this->c_DataSets[u32_ItDataset];
    C_OscSystemNameMaxCharLimitChangeReportItem::h_HandleNameMaxCharLimitItem(
        ou32_NameMaxCharLimit, "node-datapool-list-datset-name",
        rc_Dataset.c_Name, opc_ChangedItems);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set CRC from big endian BLOB

   Convert BLOB to CRC.
   Warning: 2 Byte CRC expected

   \param[in]     orc_Data    data to set

   \return
   C_NO_ERR   value set
   C_RANGE    size of orc_Data does not match our size
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscNodeDataPoolList::SetCrcFromBigEndianBlob(const QByteArray &orc_Data) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_Data.size() == 2) {
    this->u32_NvmCrc =
        static_cast<uint32_t>((static_cast<uint32_t>(orc_Data[0]) << 8U)) +
        orc_Data[1];
  } else {
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set CRC from little endian BLOB

   Convert BLOB to CRC.
   Warning: 2 Byte CRC expected

   \param[in]     orc_Data    data to set

   \return
   C_NO_ERR   value set
   C_RANGE    size of orc_Data does not match our size
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscNodeDataPoolList::SetCrcFromLittleEndianBlob(const QByteArray &orc_Data) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_Data.size() == 2) {
    this->u32_NvmCrc =
        static_cast<uint32_t>((static_cast<uint32_t>(orc_Data[1]) << 8U)) +
        orc_Data[0];
  } else {
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get CRC as big endian BLOB

   Convert content CRC to BLOB.
   Warning: 2 Byte CRC expected

   \param[out]     orc_Data    data to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::GetCrcAsBigEndianBlob(QByteArray &orc_Data) const {
  const uint16_t u16_Value = static_cast<uint16_t>(this->u32_NvmCrc);

  orc_Data.resize(2);
  orc_Data[0] = static_cast<uint8_t>(u16_Value >> 8U);
  orc_Data[1] = static_cast<uint8_t>(u16_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get CRC as little endian BLOB

   Convert content CRC to BLOB.
   Warning: 2 Byte CRC expected

   \param[out]     orc_Data    data to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::GetCrcAsLittleEndianBlob(
    QByteArray &orc_Data) const {
  const uint16_t u16_Value = static_cast<uint16_t>(this->u32_NvmCrc);

  orc_Data.resize(2);
  orc_Data[1] = static_cast<uint8_t>(u16_Value >> 8U);
  orc_Data[0] = static_cast<uint8_t>(u16_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_Name;
  ro_DataStream << c_Comment;
  ro_DataStream << q_NvmCrcActive;
  ro_DataStream << u32_NvmCrc;
  ro_DataStream << u32_NvmStartAddress;
  ro_DataStream << u32_NvmSize;
  
  // Serialize elements
  ro_DataStream << static_cast<qint32>(c_Elements.size());
  for (const auto &c_Element : c_Elements) {
    c_Element.ToQDataStream(ro_DataStream);
  }
  
  // Serialize data sets
  ro_DataStream << static_cast<qint32>(c_DataSets.size());
  for (const auto &c_DataSet : c_DataSets) {
    c_DataSet.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_Name;
  ro_DataStream >> c_Comment;
  ro_DataStream >> q_NvmCrcActive;
  ro_DataStream >> u32_NvmCrc;
  ro_DataStream >> u32_NvmStartAddress;
  ro_DataStream >> u32_NvmSize;
  
  // Deserialize elements
  qint32 s_ElementsSize;
  ro_DataStream >> s_ElementsSize;
  c_Elements.clear();
  for (qint32 s_I = 0; s_I < s_ElementsSize; ++s_I) {
    C_OscNodeDataPoolListElement c_Element;
    c_Element.FromQDataStream(ro_DataStream);
    c_Elements.append(c_Element);
  }
  
  // Deserialize data sets
  qint32 s_DataSetsSize;
  ro_DataStream >> s_DataSetsSize;
  c_DataSets.clear();
  for (qint32 s_I = 0; s_I < s_DataSetsSize; ++s_I) {
    C_OscNodeDataPoolDataSet c_DataSet;
    c_DataSet.FromQDataStream(ro_DataStream);
    c_DataSets.append(c_DataSet);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolList::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["name"] = c_Name;
  c_Obj["comment"] = c_Comment;
  c_Obj["nvm-crc-active"] = q_NvmCrcActive;
  c_Obj["nvm-crc"] = static_cast<qint64>(u32_NvmCrc);
  c_Obj["nvm-start-address"] = static_cast<qint64>(u32_NvmStartAddress);
  c_Obj["nvm-size"] = static_cast<qint64>(u32_NvmSize);
  
  // Serialize elements
  QJsonArray c_ElementsArray;
  for (const auto &c_Element : c_Elements) {
    c_ElementsArray.append(c_Element.ToJsonObject());
  }
  c_Obj["elements"] = c_ElementsArray;
  
  // Serialize data sets
  QJsonArray c_DataSetsArray;
  for (const auto &c_DataSet : c_DataSets) {
    c_DataSetsArray.append(c_DataSet.ToJsonObject());
  }
  c_Obj["data-sets"] = c_DataSetsArray;
  
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolList::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("comment")) c_Comment = orc_Object["comment"].toString();
  if (orc_Object.contains("nvm-crc-active")) q_NvmCrcActive = orc_Object["nvm-crc-active"].toBool();
  if (orc_Object.contains("nvm-crc")) u32_NvmCrc = static_cast<uint32_t>(orc_Object["nvm-crc"].toInt());
  if (orc_Object.contains("nvm-start-address")) u32_NvmStartAddress = static_cast<uint32_t>(orc_Object["nvm-start-address"].toInt());
  if (orc_Object.contains("nvm-size")) u32_NvmSize = static_cast<uint32_t>(orc_Object["nvm-size"].toInt());
  
  // Deserialize elements
  if (orc_Object.contains("elements")) {
    QJsonArray c_ElementsArray = orc_Object["elements"].toArray();
    c_Elements.clear();
    for (const auto &c_Value : c_ElementsArray) {
      C_OscNodeDataPoolListElement c_Element;
      c_Element.FromJsonObject(c_Value.toObject());
      c_Elements.append(c_Element);
    }
  }
  
  // Deserialize data sets
  if (orc_Object.contains("data-sets")) {
    QJsonArray c_DataSetsArray = orc_Object["data-sets"].toArray();
    c_DataSets.clear();
    for (const auto &c_Value : c_DataSetsArray) {
      C_OscNodeDataPoolDataSet c_DataSet;
      c_DataSet.FromJsonObject(c_Value.toObject());
      c_DataSets.append(c_DataSet);
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
QDomElement C_OscNodeDataPoolList::ToQDomDocument(QDomDocument &orc_Doc,
                                                   const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("name", c_Name);
  c_Element.setAttribute("comment", c_Comment);
  c_Element.setAttribute("nvm-crc-active", q_NvmCrcActive ? "true" : "false");
  c_Element.setAttribute("nvm-crc", QString::number(u32_NvmCrc));
  c_Element.setAttribute("nvm-start-address", QString::number(u32_NvmStartAddress));
  c_Element.setAttribute("nvm-size", QString::number(u32_NvmSize));
  
  // Serialize elements
  for (const auto &c_ElementItem : c_Elements) {
    QDomElement c_ElementElem = c_ElementItem.ToQDomDocument(orc_Doc, "element");
    c_Element.appendChild(c_ElementElem);
  }
  
  // Serialize data sets
  for (const auto &c_DataSet : c_DataSets) {
    QDomElement c_DataSetElem = c_DataSet.ToQDomDocument(orc_Doc, "data-set");
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
void C_OscNodeDataPoolList::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  if (orc_Element.hasAttribute("comment")) c_Comment = orc_Element.attribute("comment");
  if (orc_Element.hasAttribute("nvm-crc-active")) q_NvmCrcActive = (orc_Element.attribute("nvm-crc-active") == "true");
  if (orc_Element.hasAttribute("nvm-crc")) u32_NvmCrc = orc_Element.attribute("nvm-crc").toUInt();
  if (orc_Element.hasAttribute("nvm-start-address")) u32_NvmStartAddress = orc_Element.attribute("nvm-start-address").toUInt();
  if (orc_Element.hasAttribute("nvm-size")) u32_NvmSize = orc_Element.attribute("nvm-size").toUInt();
  
  // Deserialize elements
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_TagName = c_Elem.tagName();
      if (c_TagName == "element") {
        C_OscNodeDataPoolListElement c_ElementItem;
        c_ElementItem.FromQDomDocument(c_Elem);
        c_Elements.append(c_ElementItem);
      } else if (c_TagName == "data-set") {
        C_OscNodeDataPoolDataSet c_DataSet;
        c_DataSet.FromQDomDocument(c_Elem);
        c_DataSets.append(c_DataSet);
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
