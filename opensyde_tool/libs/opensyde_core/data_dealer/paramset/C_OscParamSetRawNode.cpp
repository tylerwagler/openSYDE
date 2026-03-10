//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing all raw entries for one node in a
   parameter set file (implementation)

   Data class, containing all raw entries for one node in a parameter set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetRawNode.hpp"
#include "C_OscParamSetRawEntry.hpp"
#include "C_OscParamSetDataPoolInfo.hpp"
#include "stwerrors.hpp"
#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;
using namespace stw::errors;

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
C_OscParamSetRawNode::C_OscParamSetRawNode(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNode::ToQDataStream(QDataStream& orc_Stream) const {
   // Serialize name
   orc_Stream << c_Name;
   
   // Serialize entries count
   qint32 i32_EntryCount = static_cast<qint32>(c_Entries.size());
   orc_Stream << i32_EntryCount;
   
   // Serialize each entry
   for (const C_OscParamSetRawEntry& rc_Entry : c_Entries) {
      int32_t i32_Result = rc_Entry.ToQDataStream(orc_Stream);
      if (i32_Result != C_NO_ERR) {
         return i32_Result;
      }
   }
   
   // Serialize data pools count
   qint32 i32_DataPoolCount = static_cast<qint32>(c_DataPools.size());
   orc_Stream << i32_DataPoolCount;
   
   // Serialize each data pool info
   for (const C_OscParamSetDataPoolInfo& rc_DataPool : c_DataPools) {
      int32_t i32_Result = rc_DataPool.ToQDataStream(orc_Stream);
      if (i32_Result != C_NO_ERR) {
         return i32_Result;
      }
   }
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDataStream (binary format)
   
   \param[in,out] orc_Stream    Input stream for deserialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNode::FromQDataStream(QDataStream& orc_Stream) {
   // Deserialize name
   orc_Stream >> c_Name;
   
   // Deserialize entries count
   qint32 i32_EntryCount;
   orc_Stream >> i32_EntryCount;
   
   // Clear existing entries and reserve space
   c_Entries.clear();
   c_Entries.reserve(i32_EntryCount);
   
   // Deserialize each entry
   for (qint32 i32_Index = 0; i32_Index < i32_EntryCount; ++i32_Index) {
      C_OscParamSetRawEntry c_Entry;
      int32_t i32_Result = c_Entry.FromQDataStream(orc_Stream);
      if (i32_Result != C_NO_ERR) {
         return i32_Result;
      }
      c_Entries.append(c_Entry);
   }
   
   // Deserialize data pools count
   qint32 i32_DataPoolCount;
   orc_Stream >> i32_DataPoolCount;
   
   // Clear existing data pools and reserve space
   c_DataPools.clear();
   c_DataPools.reserve(i32_DataPoolCount);
   
   // Deserialize each data pool info
   for (qint32 i32_Index = 0; i32_Index < i32_DataPoolCount; ++i32_Index) {
      C_OscParamSetDataPoolInfo c_DataPool;
      int32_t i32_Result = c_DataPool.FromQDataStream(orc_Stream);
      if (i32_Result != C_NO_ERR) {
         return i32_Result;
      }
      c_DataPools.append(c_DataPool);
   }
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QJsonObject
   
   \return JSON object containing all data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetRawNode::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["name"] = c_Name;
   
   // Serialize entries
   QJsonArray c_EntriesArray;
   for (const C_OscParamSetRawEntry& rc_Entry : c_Entries) {
      c_EntriesArray.append(rc_Entry.ToJsonObject());
   }
   c_Object["entries"] = c_EntriesArray;
   
   // Serialize data pools
   QJsonArray c_DataPoolsArray;
   for (const C_OscParamSetDataPoolInfo& rc_DataPool : c_DataPools) {
      c_DataPoolsArray.append(rc_DataPool.ToJsonObject());
   }
   c_Object["data-pools"] = c_DataPoolsArray;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNode::FromJsonObject(const QJsonObject& orc_Object) {
   if (orc_Object.contains("name") && 
       orc_Object.contains("entries") && 
       orc_Object.contains("data-pools")) {
      
      c_Name = orc_Object["name"].toString();
      
      // Deserialize entries
      QJsonArray c_EntriesArray = orc_Object["entries"].toArray();
      c_Entries.clear();
      c_Entries.reserve(c_EntriesArray.size());
      
      for (const QJsonValue& rc_EntryValue : c_EntriesArray) {
         if (rc_EntryValue.isObject()) {
            C_OscParamSetRawEntry c_Entry;
            int32_t i32_Result = c_Entry.FromJsonObject(rc_EntryValue.toObject());
            if (i32_Result != C_NO_ERR) {
               return i32_Result;
            }
            c_Entries.append(c_Entry);
         }
      }
      
      // Deserialize data pools
      QJsonArray c_DataPoolsArray = orc_Object["data-pools"].toArray();
      c_DataPools.clear();
      c_DataPools.reserve(c_DataPoolsArray.size());
      
      for (const QJsonValue& rc_DataPoolValue : c_DataPoolsArray) {
         if (rc_DataPoolValue.isObject()) {
            C_OscParamSetDataPoolInfo c_DataPool;
            int32_t i32_Result = c_DataPool.FromJsonObject(rc_DataPoolValue.toObject());
            if (i32_Result != C_NO_ERR) {
               return i32_Result;
            }
            c_DataPools.append(c_DataPool);
         }
      }
      
      return C_NO_ERR;
   } else {
      return C_CONFIG;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument
   
   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element to create
   
   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscParamSetRawNode::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Add name
   QDomElement c_NameElement = orc_Doc.createElement("name");
   QDomText c_NameText = orc_Doc.createTextNode(c_Name);
   c_NameElement.appendChild(c_NameText);
   c_Element.appendChild(c_NameElement);
   
   // Add entries
   QDomElement c_EntriesElement = orc_Doc.createElement("entries");
   for (const C_OscParamSetRawEntry& rc_Entry : c_Entries) {
      QDomElement c_EntryElement = rc_Entry.ToQDomDocument(orc_Doc, "entry");
      c_EntriesElement.appendChild(c_EntryElement);
   }
   c_Element.appendChild(c_EntriesElement);
   
   // Add data pools
   QDomElement c_DataPoolsElement = orc_Doc.createElement("data-pools");
   for (const C_OscParamSetDataPoolInfo& rc_DataPool : c_DataPools) {
      QDomElement c_DataPoolElement = rc_DataPool.ToQDomDocument(orc_Doc, "data-pool");
      c_DataPoolsElement.appendChild(c_DataPoolElement);
   }
   c_Element.appendChild(c_DataPoolsElement);
   
    return c_Element;
}

 //----------------------------------------------------------------------------------------------------------------------
 /*!
    \brief Serialize to QDomElement (wrapper for ToQDomDocument)
 
    \param[in] orc_Doc            DOM document to create element in
    \param[in] orc_RootElementName    Name of the root element to create
 
    \return QDomElement representing the serialized data
 */
 //----------------------------------------------------------------------------------------------------------------------
 QDomElement C_OscParamSetRawNode::ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
    return this->ToQDomDocument(orc_Doc, orc_RootElementName);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*!
    \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNode::FromQDomElement(const QDomElement& orc_Element) {
   QDomNode c_NameNode = orc_Element.namedItem("name");
   QDomNode c_EntriesNode = orc_Element.namedItem("entries");
   QDomNode c_DataPoolsNode = orc_Element.namedItem("data-pools");
   
   if (!c_NameNode.isNull() && 
       !c_EntriesNode.isNull() && 
       !c_DataPoolsNode.isNull()) {
      
      QDomText c_NameText = c_NameNode.toText();
      if (!c_NameText.isNull()) {
         c_Name = c_NameText.data();
      }
      
      // Process entries
      c_Entries.clear();
      QDomNode c_CurrentEntryNode = c_EntriesNode.firstChild();
      while (!c_CurrentEntryNode.isNull()) {
         if (c_CurrentEntryNode.isElement()) {
            QDomElement c_EntryElement = c_CurrentEntryNode.toElement();
            C_OscParamSetRawEntry c_Entry;
            int32_t i32_Result = c_Entry.FromQDomElement(c_EntryElement);
            if (i32_Result != C_NO_ERR) {
               return i32_Result;
            }
            c_Entries.append(c_Entry);
         }
         c_CurrentEntryNode = c_CurrentEntryNode.nextSibling();
      }
      
      // Process data pools
      c_DataPools.clear();
      QDomNode c_CurrentDataPoolNode = c_DataPoolsNode.firstChild();
      while (!c_CurrentDataPoolNode.isNull()) {
         if (c_CurrentDataPoolNode.isElement()) {
            QDomElement c_DataPoolElement = c_CurrentDataPoolNode.toElement();
            C_OscParamSetDataPoolInfo c_DataPool;
            int32_t i32_Result = c_DataPool.FromQDomElement(c_DataPoolElement);
            if (i32_Result != C_NO_ERR) {
               return i32_Result;
            }
            c_DataPools.append(c_DataPool);
         }
         c_CurrentDataPoolNode = c_CurrentDataPoolNode.nextSibling();
      }
      
      return C_NO_ERR;
   }
   
   return C_CONFIG;
}
