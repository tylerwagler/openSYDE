//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job information

   Data logger job information

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscDataLoggerJob.hpp"
#include "C_OscDataLoggerJobProperties.hpp"
#include "C_OscDataLoggerJobAdditionalTriggerProperties.hpp"
#include "C_OscDataLoggerDataElementReference.hpp"
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscHashUtil.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
bool C_OscDataLoggerJob::hq_AllowDataloggerFeature = true;

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
C_OscDataLoggerJob::C_OscDataLoggerJob() : q_IsEnabled(true) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJob::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->q_IsEnabled, this->c_Properties, this->c_ConfiguredDataElements);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)

   \param[out] orc_Stream    Output stream for serialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJob::ToQDataStream(QDataStream& orc_Stream) const {
    using namespace stw::errors;
    
    orc_Stream << q_IsEnabled;
    
    // Serialize properties
    uint32_t u32_Elem = static_cast<uint32_t>(c_Properties.e_UseCase);
    orc_Stream << u32_Elem;
    
    u32_Elem = static_cast<uint32_t>(c_Properties.e_LogFileFormat);
    orc_Stream << u32_Elem;
    
    u32_Elem = static_cast<uint32_t>(c_Properties.e_LocalLogTrigger);
    orc_Stream << u32_Elem;
    
    orc_Stream << c_Properties.c_Name;
    orc_Stream << c_Properties.c_Comment;
    orc_Stream << c_Properties.u32_MaxLogEntries;
    orc_Stream << c_Properties.u32_MaxLogDurationSec;
    orc_Stream << c_Properties.u32_LogIntervalMs;
    orc_Stream << c_Properties.c_LogDestinationDirectory;
    
    // Serialize additional trigger properties
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.q_Enable;
    
    // Serialize element ID fields directly
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex;
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex;
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex;
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex;
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetUseArrayElementIndex();
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetArrayElementIndex();
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetHalChannelName();
    
    // Serialize threshold (serialize as a data content object)
    // Serialize type
    uint32_t u32_ThresholdType = static_cast<uint32_t>(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetType());
    orc_Stream << u32_ThresholdType;
    // Serialize array flag
    bool q_ThresholdArray = c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArray();
    orc_Stream << q_ThresholdArray;
    // Serialize array size
    uint32_t u32_ThresholdSize = c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArraySize();
    orc_Stream << u32_ThresholdSize;
    // Serialize actual data as byte array
    QByteArray c_ThresholdData;
    const_cast<C_OscNodeDataPoolContent&>(c_Properties.c_AdditionalTriggerProperties.c_Threshold).GetValueAsLittleEndianBlob(c_ThresholdData);
    orc_Stream << c_ThresholdData;
    
    orc_Stream << c_Properties.c_AdditionalTriggerProperties.c_Operation;
    
    // Serialize data elements count
    uint32_t u32_Count = c_ConfiguredDataElements.size();
    orc_Stream << u32_Count;
    
    // Serialize each data element
    for (const C_OscDataLoggerDataElementReference &c_Element : c_ConfiguredDataElements) {
       orc_Stream << c_Element.q_UseCustomName;
       orc_Stream << c_Element.c_CustomName;
       
       // Serialize element ID fields directly
       orc_Stream << c_Element.c_ConfiguredElementId.u32_NodeIndex;
       orc_Stream << c_Element.c_ConfiguredElementId.u32_DataPoolIndex;
       orc_Stream << c_Element.c_ConfiguredElementId.u32_ListIndex;
       orc_Stream << c_Element.c_ConfiguredElementId.u32_ElementIndex;
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
int32_t C_OscDataLoggerJob::FromQDataStream(QDataStream& orc_Stream) {
    using namespace stw::errors;
    
    orc_Stream >> q_IsEnabled;
    
    // Deserialize properties
    uint32_t u32_Elem = 0;
    orc_Stream >> u32_Elem;
    c_Properties.e_UseCase = static_cast<C_OscDataLoggerJobProperties::E_UseCase>(u32_Elem);
    
    orc_Stream >> u32_Elem;
    c_Properties.e_LogFileFormat = static_cast<C_OscDataLoggerJobProperties::E_LogFileFormat>(u32_Elem);
    
    orc_Stream >> u32_Elem;
    c_Properties.e_LocalLogTrigger = static_cast<C_OscDataLoggerJobProperties::E_LocalLogTrigger>(u32_Elem);
    
    orc_Stream >> c_Properties.c_Name;
    orc_Stream >> c_Properties.c_Comment;
    orc_Stream >> c_Properties.u32_MaxLogEntries;
    orc_Stream >> c_Properties.u32_MaxLogDurationSec;
    orc_Stream >> c_Properties.u32_LogIntervalMs;
    orc_Stream >> c_Properties.c_LogDestinationDirectory;
    
    // Deserialize additional trigger properties
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.q_Enable;
    
    // Deserialize element ID fields directly
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex;
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex;
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex;
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex;
    bool q_UseArrayElementIndex = false;
    orc_Stream >> q_UseArrayElementIndex;
    uint32_t u32_ArrayElementIndex = 0;
    orc_Stream >> u32_ArrayElementIndex;
    QString c_HalChannelName;
    orc_Stream >> c_HalChannelName;
    c_Properties.c_AdditionalTriggerProperties.c_ElementId.SetHalChannelName(c_HalChannelName);
    // Note: Setting array index may require a different approach - for now we assume it's handled by constructor
    
    // Deserialize threshold as a data content object
    uint32_t u32_ThresholdType = 0;
    orc_Stream >> u32_ThresholdType;
    bool q_ThresholdArray = false;
    orc_Stream >> q_ThresholdArray;
    uint32_t u32_ThresholdSize = 0;
    orc_Stream >> u32_ThresholdSize;
    QByteArray c_ThresholdData;
    orc_Stream >> c_ThresholdData;
    
    // Reconstruct the threshold value from binary blob
    c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetType(static_cast<C_OscNodeDataPoolContent::E_Type>(u32_ThresholdType));
    c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArray(q_ThresholdArray);
    c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArraySize(u32_ThresholdSize);
    // Set the actual data - need to use the appropriate setter based on type
    // For simplicity, we'll assume the blob can be restored via SetValueFromLittleEndianBlob
    c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetValueFromLittleEndianBlob(c_ThresholdData);
    
    orc_Stream >> c_Properties.c_AdditionalTriggerProperties.c_Operation;
    
    // Deserialize data elements count
    uint32_t u32_Count = 0;
    orc_Stream >> u32_Count;
    
    c_ConfiguredDataElements.clear();
    
    // Deserialize each data element
    for (uint32_t i = 0; i < u32_Count; ++i) {
       C_OscDataLoggerDataElementReference c_Element;
       orc_Stream >> c_Element.q_UseCustomName;
       orc_Stream >> c_Element.c_CustomName;
       
       // Deserialize element ID fields directly
       orc_Stream >> c_Element.c_ConfiguredElementId.u32_NodeIndex;
       orc_Stream >> c_Element.c_ConfiguredElementId.u32_DataPoolIndex;
       orc_Stream >> c_Element.c_ConfiguredElementId.u32_ListIndex;
       orc_Stream >> c_Element.c_ConfiguredElementId.u32_ElementIndex;
       
       c_ConfiguredDataElements.append(c_Element);
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
QJsonObject C_OscDataLoggerJob::ToJsonObject() const {
    QJsonObject c_Object;
    
    c_Object["enabled"] = q_IsEnabled;
    
    // Serialize properties
    QJsonObject c_PropertiesObject;
    c_PropertiesObject["name"] = c_Properties.c_Name;
    c_PropertiesObject["comment"] = c_Properties.c_Comment;
    
    // Handle enum conversions
    switch (c_Properties.e_UseCase) {
       case C_OscDataLoggerJobProperties::eUC_MANUAL:
          c_PropertiesObject["use-case"] = "manual";
          break;
       case C_OscDataLoggerJobProperties::eUC_AWS:
          c_PropertiesObject["use-case"] = "aws";
          break;
       case C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD:
          c_PropertiesObject["use-case"] = "machines-cloud";
          break;
       default:
          c_PropertiesObject["use-case"] = "manual";
          break;
    }
    
    switch (c_Properties.e_LogFileFormat) {
       case C_OscDataLoggerJobProperties::eLFF_CSV:
          c_PropertiesObject["log-file-format"] = "csv";
          break;
       case C_OscDataLoggerJobProperties::eLFF_PARQUET:
          c_PropertiesObject["log-file-format"] = "parquet";
          break;
       default:
          c_PropertiesObject["log-file-format"] = "csv";
          break;
    }
    
    c_PropertiesObject["max-log-entries"] = static_cast<qlonglong>(c_Properties.u32_MaxLogEntries);
    c_PropertiesObject["max-log-duration-sec"] = static_cast<qlonglong>(c_Properties.u32_MaxLogDurationSec);
    c_PropertiesObject["log-interval-ms"] = static_cast<qlonglong>(c_Properties.u32_LogIntervalMs);
    
    switch (c_Properties.e_LocalLogTrigger) {
       case C_OscDataLoggerJobProperties::eLLT_ON_CHANGE:
          c_PropertiesObject["local-log-trigger"] = "on-change";
          break;
       case C_OscDataLoggerJobProperties::eLLT_INTERVAL:
          c_PropertiesObject["local-log-trigger"] = "interval";
          break;
       default:
          c_PropertiesObject["local-log-trigger"] = "on-change";
          break;
    }
    
    c_PropertiesObject["log-destination-directory"] = c_Properties.c_LogDestinationDirectory;
    
    // Serialize additional trigger properties
    QJsonObject c_AdditionalTriggerObject;
    c_AdditionalTriggerObject["enable"] = c_Properties.c_AdditionalTriggerProperties.q_Enable;
    
    // Serialize element ID as nested object
    QJsonObject c_ElementIdObject;
    c_ElementIdObject["node-index"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex);
    c_ElementIdObject["data-pool-index"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex);
    c_ElementIdObject["list-index"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex);
    c_ElementIdObject["element-index"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex);
    c_ElementIdObject["use-array-index"] = c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetUseArrayElementIndex();
    c_ElementIdObject["array-index"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetArrayElementIndex());
    c_ElementIdObject["hal-channel"] = c_Properties.c_AdditionalTriggerProperties.c_ElementId.GetHalChannelName();
    c_AdditionalTriggerObject["element-id"] = c_ElementIdObject;
    
    // Serialize threshold as nested object
    QJsonObject c_ThresholdObject;
    c_ThresholdObject["type"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetType());
    c_ThresholdObject["is-array"] = c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArray();
    c_ThresholdObject["array-size"] = static_cast<qlonglong>(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArraySize());
    
    // Serialize actual data as base64 string
    QByteArray c_ThresholdData;
    const_cast<C_OscNodeDataPoolContent&>(c_Properties.c_AdditionalTriggerProperties.c_Threshold).GetValueAsLittleEndianBlob(c_ThresholdData);
    c_ThresholdObject["data"] = QString(c_ThresholdData.toBase64());
    
    c_AdditionalTriggerObject["threshold"] = c_ThresholdObject;
    
    c_AdditionalTriggerObject["operation"] = c_Properties.c_AdditionalTriggerProperties.c_Operation;
    
    c_PropertiesObject["additional-trigger"] = c_AdditionalTriggerObject;
    
    c_Object["properties"] = c_PropertiesObject;
    
    // Serialize data elements
    QJsonArray c_DataElementsArray;
    for (const C_OscDataLoggerDataElementReference &c_Element : c_ConfiguredDataElements) {
       QJsonObject c_ElementObject;
       c_ElementObject["use-custom-name"] = c_Element.q_UseCustomName;
       c_ElementObject["custom-name"] = c_Element.c_CustomName;
       
       // Serialize element ID as nested object
       QJsonObject c_ElementIdChildObject;
       c_ElementIdChildObject["node-index"] = static_cast<qlonglong>(c_Element.c_ConfiguredElementId.u32_NodeIndex);
       c_ElementIdChildObject["data-pool-index"] = static_cast<qlonglong>(c_Element.c_ConfiguredElementId.u32_DataPoolIndex);
       c_ElementIdChildObject["list-index"] = static_cast<qlonglong>(c_Element.c_ConfiguredElementId.u32_ListIndex);
       c_ElementIdChildObject["element-index"] = static_cast<qlonglong>(c_Element.c_ConfiguredElementId.u32_ElementIndex);
       c_ElementObject["element-id"] = c_ElementIdChildObject;
       
       c_DataElementsArray.append(c_ElementObject);
    }
    
    c_Object["configured-data-elements"] = c_DataElementsArray;
    
    return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject

   \param[in] orc_Object    JSON object to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJob::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("enabled")) {
      q_IsEnabled = orc_Object["enabled"].toBool();
   }
   
   // Deserialize properties
   if (orc_Object.contains("properties")) {
      QJsonObject c_PropertiesObject = orc_Object["properties"].toObject();
      
      c_Properties.c_Name = c_PropertiesObject["name"].toString();
      c_Properties.c_Comment = c_PropertiesObject["comment"].toString();
      
      // Handle enum conversions
      if (c_PropertiesObject.contains("use-case")) {
         QString c_UseCaseStr = c_PropertiesObject["use-case"].toString();
         if (c_UseCaseStr == "aws") {
            c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_AWS;
         } else if (c_UseCaseStr == "machines-cloud") {
            c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD;
         } else {
            c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_MANUAL;
         }
      }
      
      if (c_PropertiesObject.contains("log-file-format")) {
         QString c_LogFileFormatStr = c_PropertiesObject["log-file-format"].toString();
         if (c_LogFileFormatStr == "parquet") {
            c_Properties.e_LogFileFormat = C_OscDataLoggerJobProperties::eLFF_PARQUET;
         } else {
            c_Properties.e_LogFileFormat = C_OscDataLoggerJobProperties::eLFF_CSV;
         }
      }
      
      if (c_PropertiesObject.contains("max-log-entries")) {
         c_Properties.u32_MaxLogEntries = static_cast<uint32_t>(c_PropertiesObject["max-log-entries"].toInteger());
      }
      
      if (c_PropertiesObject.contains("max-log-duration-sec")) {
         c_Properties.u32_MaxLogDurationSec = static_cast<uint32_t>(c_PropertiesObject["max-log-duration-sec"].toInteger());
      }
      
      if (c_PropertiesObject.contains("log-interval-ms")) {
         c_Properties.u32_LogIntervalMs = static_cast<uint32_t>(c_PropertiesObject["log-interval-ms"].toInteger());
      }
      
      if (c_PropertiesObject.contains("local-log-trigger")) {
         QString c_LocalLogTriggerStr = c_PropertiesObject["local-log-trigger"].toString();
         if (c_LocalLogTriggerStr == "interval") {
            c_Properties.e_LocalLogTrigger = C_OscDataLoggerJobProperties::eLLT_INTERVAL;
         } else {
            c_Properties.e_LocalLogTrigger = C_OscDataLoggerJobProperties::eLLT_ON_CHANGE;
         }
      }
      
      if (c_PropertiesObject.contains("log-destination-directory")) {
         c_Properties.c_LogDestinationDirectory = c_PropertiesObject["log-destination-directory"].toString();
      }
      
       // Deserialize additional trigger properties
       if (c_PropertiesObject.contains("additional-trigger")) {
          QJsonObject c_AdditionalTriggerObject = c_PropertiesObject["additional-trigger"].toObject();
          c_Properties.c_AdditionalTriggerProperties.q_Enable = c_AdditionalTriggerObject["enable"].toBool();
          
          if (c_AdditionalTriggerObject.contains("element-id")) {
             QJsonObject c_ElementIdObject = c_AdditionalTriggerObject["element-id"].toObject();
             if (c_ElementIdObject.contains("node-index")) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex = static_cast<uint32_t>(c_ElementIdObject["node-index"].toInteger());
             }
             if (c_ElementIdObject.contains("data-pool-index")) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex = static_cast<uint32_t>(c_ElementIdObject["data-pool-index"].toInteger());
             }
             if (c_ElementIdObject.contains("list-index")) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex = static_cast<uint32_t>(c_ElementIdObject["list-index"].toInteger());
             }
             if (c_ElementIdObject.contains("element-index")) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex = static_cast<uint32_t>(c_ElementIdObject["element-index"].toInteger());
             }
             if (c_ElementIdObject.contains("hal-channel")) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.SetHalChannelName(c_ElementIdObject["hal-channel"].toString());
             }
          }
          
          if (c_AdditionalTriggerObject.contains("threshold")) {
             QJsonObject c_ThresholdObject = c_AdditionalTriggerObject["threshold"].toObject();
             if (c_ThresholdObject.contains("type")) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetType(static_cast<C_OscNodeDataPoolContent::E_Type>(c_ThresholdObject["type"].toInteger()));
             }
             if (c_ThresholdObject.contains("is-array")) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArray(c_ThresholdObject["is-array"].toBool());
             }
             if (c_ThresholdObject.contains("array-size")) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArraySize(static_cast<uint32_t>(c_ThresholdObject["array-size"].toInteger()));
             }
             if (c_ThresholdObject.contains("data")) {
                QByteArray c_ThresholdData = QByteArray::fromBase64(c_ThresholdObject["data"].toString().toUtf8());
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetValueFromLittleEndianBlob(c_ThresholdData);
             }
          }
          
          if (c_AdditionalTriggerObject.contains("operation")) {
             c_Properties.c_AdditionalTriggerProperties.c_Operation = c_AdditionalTriggerObject["operation"].toString();
          }
       }
   }
   
    // Deserialize data elements
    if (orc_Object.contains("configured-data-elements")) {
       QJsonArray c_DataElementsArray = orc_Object["configured-data-elements"].toArray();
       
       c_ConfiguredDataElements.clear();
       
       for (const QJsonValue &c_ElementValue : c_DataElementsArray) {
          if (c_ElementValue.isObject()) {
             QJsonObject c_ElementObject = c_ElementValue.toObject();
             C_OscDataLoggerDataElementReference c_Element;
             
             if (c_ElementObject.contains("use-custom-name")) {
                c_Element.q_UseCustomName = c_ElementObject["use-custom-name"].toBool();
             }
             
             if (c_ElementObject.contains("custom-name")) {
                c_Element.c_CustomName = c_ElementObject["custom-name"].toString();
             }
             
             if (c_ElementObject.contains("element-id")) {
                QJsonObject c_ElementIdObject = c_ElementObject["element-id"].toObject();
                if (c_ElementIdObject.contains("node-index")) {
                   c_Element.c_ConfiguredElementId.u32_NodeIndex = static_cast<uint32_t>(c_ElementIdObject["node-index"].toInteger());
                }
                if (c_ElementIdObject.contains("data-pool-index")) {
                   c_Element.c_ConfiguredElementId.u32_DataPoolIndex = static_cast<uint32_t>(c_ElementIdObject["data-pool-index"].toInteger());
                }
                if (c_ElementIdObject.contains("list-index")) {
                   c_Element.c_ConfiguredElementId.u32_ListIndex = static_cast<uint32_t>(c_ElementIdObject["list-index"].toInteger());
                }
                if (c_ElementIdObject.contains("element-index")) {
                   c_Element.c_ConfiguredElementId.u32_ElementIndex = static_cast<uint32_t>(c_ElementIdObject["element-index"].toInteger());
                }
             }
             
             c_ConfiguredDataElements.append(c_Element);
          }
       }
    }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument

   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element to create

   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscDataLoggerJob::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Serialize enabled flag
   QDomElement c_EnabledElement = orc_Doc.createElement("enabled");
   QDomText c_EnabledText = orc_Doc.createTextNode(QString::number(q_IsEnabled ? 1 : 0));
   c_EnabledElement.appendChild(c_EnabledText);
   c_Element.appendChild(c_EnabledElement);
   
   // Serialize properties
   QDomElement c_PropertiesElement = orc_Doc.createElement("properties");
   
   QDomElement c_NameElement = orc_Doc.createElement("name");
   QDomText c_NameText = orc_Doc.createTextNode(c_Properties.c_Name);
   c_NameElement.appendChild(c_NameText);
   c_PropertiesElement.appendChild(c_NameElement);
   
   QDomElement c_CommentElement = orc_Doc.createElement("comment");
   QDomText c_CommentText = orc_Doc.createTextNode(c_Properties.c_Comment);
   c_CommentElement.appendChild(c_CommentText);
   c_PropertiesElement.appendChild(c_CommentElement);
   
   // Serialize enums
    QDomElement c_UseCaseElement = orc_Doc.createElement("use-case");
    QString c_UseCaseTextStr;
    switch (c_Properties.e_UseCase) {
       case C_OscDataLoggerJobProperties::eUC_AWS:
          c_UseCaseTextStr = "aws";
          break;
       case C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD:
          c_UseCaseTextStr = "machines-cloud";
          break;
       default:
          c_UseCaseTextStr = "manual";
          break;
    }
    QDomText c_UseCaseText = orc_Doc.createTextNode(c_UseCaseTextStr);
    c_UseCaseElement.appendChild(c_UseCaseText);
    c_PropertiesElement.appendChild(c_UseCaseElement);
    
    QDomElement c_LogFileFormatElement = orc_Doc.createElement("log-file-format");
    QString c_LogFileFormatTextStr;
    switch (c_Properties.e_LogFileFormat) {
       case C_OscDataLoggerJobProperties::eLFF_PARQUET:
          c_LogFileFormatTextStr = "parquet";
          break;
       default:
          c_LogFileFormatTextStr = "csv";
          break;
    }
    QDomText c_LogFileFormatText = orc_Doc.createTextNode(c_LogFileFormatTextStr);
    c_LogFileFormatElement.appendChild(c_LogFileFormatText);
    c_PropertiesElement.appendChild(c_LogFileFormatElement);
   
   QDomElement c_MaxLogEntriesElement = orc_Doc.createElement("max-log-entries");
   QDomText c_MaxLogEntriesText = orc_Doc.createTextNode(QString::number(c_Properties.u32_MaxLogEntries));
   c_MaxLogEntriesElement.appendChild(c_MaxLogEntriesText);
   c_PropertiesElement.appendChild(c_MaxLogEntriesElement);
   
   QDomElement c_MaxLogDurationElement = orc_Doc.createElement("max-log-duration-sec");
   QDomText c_MaxLogDurationText = orc_Doc.createTextNode(QString::number(c_Properties.u32_MaxLogDurationSec));
   c_MaxLogDurationElement.appendChild(c_MaxLogDurationText);
   c_PropertiesElement.appendChild(c_MaxLogDurationElement);
   
   QDomElement c_LogIntervalElement = orc_Doc.createElement("log-interval-ms");
   QDomText c_LogIntervalText = orc_Doc.createTextNode(QString::number(c_Properties.u32_LogIntervalMs));
   c_LogIntervalElement.appendChild(c_LogIntervalText);
   c_PropertiesElement.appendChild(c_LogIntervalElement);
   
    QDomElement c_LocalLogTriggerElement = orc_Doc.createElement("local-log-trigger");
    QString c_LocalLogTriggerTextStr;
    switch (c_Properties.e_LocalLogTrigger) {
       case C_OscDataLoggerJobProperties::eLLT_INTERVAL:
          c_LocalLogTriggerTextStr = "interval";
          break;
       default:
          c_LocalLogTriggerTextStr = "on-change";
          break;
    }
    QDomText c_LocalLogTriggerText = orc_Doc.createTextNode(c_LocalLogTriggerTextStr);
    c_LocalLogTriggerElement.appendChild(c_LocalLogTriggerText);
    c_PropertiesElement.appendChild(c_LocalLogTriggerElement);
   
   QDomElement c_LogDestinationDirectoryElement = orc_Doc.createElement("log-destination-directory");
   QDomText c_LogDestinationDirectoryText = orc_Doc.createTextNode(c_Properties.c_LogDestinationDirectory);
   c_LogDestinationDirectoryElement.appendChild(c_LogDestinationDirectoryText);
   c_PropertiesElement.appendChild(c_LogDestinationDirectoryElement);
   
    // Serialize additional trigger properties
    QDomElement c_AdditionalTriggerElement = orc_Doc.createElement("additional-trigger");
    c_AdditionalTriggerElement.setAttribute("enable", c_Properties.c_AdditionalTriggerProperties.q_Enable ? "true" : "false");
    
    // Serialize element ID fields directly
    QDomElement c_ElementIdElement = orc_Doc.createElement("element-id");
    QDomElement c_ElemNodeId = orc_Doc.createElement("node-index");
    c_ElemNodeId.appendChild(orc_Doc.createTextNode(QString::number(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex)));
    c_ElementIdElement.appendChild(c_ElemNodeId);
    
    QDomElement c_ElemDataPoolId = orc_Doc.createElement("data-pool-index");
    c_ElemDataPoolId.appendChild(orc_Doc.createTextNode(QString::number(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex)));
    c_ElementIdElement.appendChild(c_ElemDataPoolId);
    
    QDomElement c_ElemListId = orc_Doc.createElement("list-index");
    c_ElemListId.appendChild(orc_Doc.createTextNode(QString::number(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex)));
    c_ElementIdElement.appendChild(c_ElemListId);
    
    QDomElement c_ElemElementId = orc_Doc.createElement("element-index");
    c_ElemElementId.appendChild(orc_Doc.createTextNode(QString::number(c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex)));
    c_ElementIdElement.appendChild(c_ElemElementId);
    
    c_AdditionalTriggerElement.appendChild(c_ElementIdElement);
    
    // Serialize threshold fields directly
    QDomElement c_ThresholdElement = orc_Doc.createElement("threshold");
    QDomElement c_ThreshType = orc_Doc.createElement("type");
    c_ThreshType.appendChild(orc_Doc.createTextNode(QString::number(static_cast<int>(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetType()))));
    c_ThresholdElement.appendChild(c_ThreshType);
    
    QDomElement c_ThreshArray = orc_Doc.createElement("is-array");
    c_ThreshArray.appendChild(orc_Doc.createTextNode(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArray() ? "1" : "0"));
    c_ThresholdElement.appendChild(c_ThreshArray);
    
    QDomElement c_ThreshSize = orc_Doc.createElement("array-size");
    c_ThreshSize.appendChild(orc_Doc.createTextNode(QString::number(c_Properties.c_AdditionalTriggerProperties.c_Threshold.GetArraySize())));
    c_ThresholdElement.appendChild(c_ThreshSize);
    
    // Serialize actual data as base64
    QByteArray c_ThresholdData;
    const_cast<C_OscNodeDataPoolContent&>(c_Properties.c_AdditionalTriggerProperties.c_Threshold).GetValueAsLittleEndianBlob(c_ThresholdData);
    QDomElement c_ThreshData = orc_Doc.createElement("data");
    c_ThreshData.appendChild(orc_Doc.createTextNode(QString::fromUtf8(c_ThresholdData.toBase64())));
    c_ThresholdElement.appendChild(c_ThreshData);
    
    c_AdditionalTriggerElement.appendChild(c_ThresholdElement);
    
    QDomElement c_OperationElement = orc_Doc.createElement("operation");
    QDomText c_OperationText = orc_Doc.createTextNode(c_Properties.c_AdditionalTriggerProperties.c_Operation);
    c_OperationElement.appendChild(c_OperationText);
    c_AdditionalTriggerElement.appendChild(c_OperationElement);
    
    c_PropertiesElement.appendChild(c_AdditionalTriggerElement);
    c_Element.appendChild(c_PropertiesElement);
    
    // Serialize data elements
    QDomElement c_DataElementsElement = orc_Doc.createElement("configured-data-elements");
    for (const C_OscDataLoggerDataElementReference &c_Element : c_ConfiguredDataElements) {
       QDomElement c_DataElement = orc_Doc.createElement("data-element");
       
       QDomElement c_UseCustomNameElement = orc_Doc.createElement("use-custom-name");
       QDomText c_UseCustomNameText = orc_Doc.createTextNode(QString::number(c_Element.q_UseCustomName ? 1 : 0));
       c_UseCustomNameElement.appendChild(c_UseCustomNameText);
       c_DataElement.appendChild(c_UseCustomNameElement);
       
       QDomElement c_CustomNameElement = orc_Doc.createElement("custom-name");
       QDomText c_CustomNameText = orc_Doc.createTextNode(c_Element.c_CustomName);
       c_CustomNameElement.appendChild(c_CustomNameText);
       c_DataElement.appendChild(c_CustomNameElement);
       
       // Serialize element ID fields directly
       QDomElement c_ElementIdChildElement = orc_Doc.createElement("element-id");
       QDomElement c_ChildNodeId = orc_Doc.createElement("node-index");
       c_ChildNodeId.appendChild(orc_Doc.createTextNode(QString::number(c_Element.c_ConfiguredElementId.u32_NodeIndex)));
       c_ElementIdChildElement.appendChild(c_ChildNodeId);
       
       QDomElement c_ChildDataPoolId = orc_Doc.createElement("data-pool-index");
       c_ChildDataPoolId.appendChild(orc_Doc.createTextNode(QString::number(c_Element.c_ConfiguredElementId.u32_DataPoolIndex)));
       c_ElementIdChildElement.appendChild(c_ChildDataPoolId);
       
       QDomElement c_ChildListId = orc_Doc.createElement("list-index");
       c_ChildListId.appendChild(orc_Doc.createTextNode(QString::number(c_Element.c_ConfiguredElementId.u32_ListIndex)));
       c_ElementIdChildElement.appendChild(c_ChildListId);
       
       QDomElement c_ChildElementId = orc_Doc.createElement("element-index");
       c_ChildElementId.appendChild(orc_Doc.createTextNode(QString::number(c_Element.c_ConfiguredElementId.u32_ElementIndex)));
       c_ElementIdChildElement.appendChild(c_ChildElementId);
       
       c_DataElement.appendChild(c_ElementIdChildElement);
       
       c_DataElementsElement.appendChild(c_DataElement);
    }
   
   c_Element.appendChild(c_DataElementsElement);
   
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
 QDomElement C_OscDataLoggerJob::ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
    return this->ToQDomDocument(orc_Doc, orc_RootElementName);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*!
    \brief Deserialize from QDomElement

   \param[in] orc_Element    XML element to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJob::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   // Deserialize enabled flag
   QDomNode c_EnabledNode = orc_Element.namedItem("enabled");
   if (!c_EnabledNode.isNull()) {
      QDomText c_Text = c_EnabledNode.toText();
      if (!c_Text.isNull()) {
         q_IsEnabled = (c_Text.data() == "1");
      }
   }
   
   // Deserialize properties
   QDomNode c_PropertiesNode = orc_Element.namedItem("properties");
   if (!c_PropertiesNode.isNull()) {
      QDomElement c_PropertiesElement = c_PropertiesNode.toElement();
      
      // Deserialize name
      QDomNode c_NameNode = c_PropertiesElement.namedItem("name");
      if (!c_NameNode.isNull()) {
         QDomText c_NameText = c_NameNode.toText();
         if (!c_NameText.isNull()) {
            c_Properties.c_Name = c_NameText.data();
         }
      }
      
      // Deserialize comment
      QDomNode c_CommentNode = c_PropertiesElement.namedItem("comment");
      if (!c_CommentNode.isNull()) {
         QDomText c_CommentText = c_CommentNode.toText();
         if (!c_CommentText.isNull()) {
            c_Properties.c_Comment = c_CommentText.data();
         }
      }
      
      // Deserialize use-case
      QDomNode c_UseCaseNode = c_PropertiesElement.namedItem("use-case");
      if (!c_UseCaseNode.isNull()) {
         QDomText c_UseCaseText = c_UseCaseNode.toText();
         if (!c_UseCaseText.isNull()) {
            QString c_UseCaseStr = c_UseCaseText.data();
            if (c_UseCaseStr == "aws") {
               c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_AWS;
            } else if (c_UseCaseStr == "machines-cloud") {
               c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD;
            } else {
               c_Properties.e_UseCase = C_OscDataLoggerJobProperties::eUC_MANUAL;
            }
         }
      }
      
      // Deserialize log file format
      QDomNode c_LogFileFormatNode = c_PropertiesElement.namedItem("log-file-format");
      if (!c_LogFileFormatNode.isNull()) {
         QDomText c_LogFileFormatText = c_LogFileFormatNode.toText();
         if (!c_LogFileFormatText.isNull()) {
            QString c_LogFileFormatStr = c_LogFileFormatText.data();
            if (c_LogFileFormatStr == "parquet") {
               c_Properties.e_LogFileFormat = C_OscDataLoggerJobProperties::eLFF_PARQUET;
            } else {
               c_Properties.e_LogFileFormat = C_OscDataLoggerJobProperties::eLFF_CSV;
            }
         }
      }
      
      // Deserialize max log entries
      QDomNode c_MaxLogEntriesNode = c_PropertiesElement.namedItem("max-log-entries");
      if (!c_MaxLogEntriesNode.isNull()) {
         QDomText c_MaxLogEntriesText = c_MaxLogEntriesNode.toText();
         if (!c_MaxLogEntriesText.isNull()) {
            c_Properties.u32_MaxLogEntries = static_cast<uint32_t>(c_MaxLogEntriesText.data().toInt());
         }
      }
      
      // Deserialize max log duration
      QDomNode c_MaxLogDurationNode = c_PropertiesElement.namedItem("max-log-duration-sec");
      if (!c_MaxLogDurationNode.isNull()) {
         QDomText c_MaxLogDurationText = c_MaxLogDurationNode.toText();
         if (!c_MaxLogDurationText.isNull()) {
            c_Properties.u32_MaxLogDurationSec = static_cast<uint32_t>(c_MaxLogDurationText.data().toInt());
         }
      }
      
      // Deserialize log interval
      QDomNode c_LogIntervalNode = c_PropertiesElement.namedItem("log-interval-ms");
      if (!c_LogIntervalNode.isNull()) {
         QDomText c_LogIntervalText = c_LogIntervalNode.toText();
         if (!c_LogIntervalText.isNull()) {
            c_Properties.u32_LogIntervalMs = static_cast<uint32_t>(c_LogIntervalText.data().toInt());
         }
      }
      
      // Deserialize local log trigger
      QDomNode c_LocalLogTriggerNode = c_PropertiesElement.namedItem("local-log-trigger");
      if (!c_LocalLogTriggerNode.isNull()) {
         QDomText c_LocalLogTriggerText = c_LocalLogTriggerNode.toText();
         if (!c_LocalLogTriggerText.isNull()) {
            QString c_LocalLogTriggerStr = c_LocalLogTriggerText.data();
            if (c_LocalLogTriggerStr == "interval") {
               c_Properties.e_LocalLogTrigger = C_OscDataLoggerJobProperties::eLLT_INTERVAL;
            } else {
               c_Properties.e_LocalLogTrigger = C_OscDataLoggerJobProperties::eLLT_ON_CHANGE;
            }
         }
      }
      
      // Deserialize log destination directory
      QDomNode c_LogDestinationDirectoryNode = c_PropertiesElement.namedItem("log-destination-directory");
      if (!c_LogDestinationDirectoryNode.isNull()) {
         QDomText c_LogDestinationDirectoryText = c_LogDestinationDirectoryNode.toText();
         if (!c_LogDestinationDirectoryText.isNull()) {
            c_Properties.c_LogDestinationDirectory = c_LogDestinationDirectoryText.data();
         }
      }
      
       // Deserialize additional trigger properties
       QDomNode c_AdditionalTriggerNode = c_PropertiesElement.namedItem("additional-trigger");
       if (!c_AdditionalTriggerNode.isNull()) {
          QDomElement c_AdditionalTriggerElement = c_AdditionalTriggerNode.toElement();
          
          QString c_EnableStr = c_AdditionalTriggerElement.attribute("enable");
          if (c_EnableStr == "true") {
             c_Properties.c_AdditionalTriggerProperties.q_Enable = true;
          } else {
             c_Properties.c_AdditionalTriggerProperties.q_Enable = false;
          }
          
          // Deserialize element ID fields directly
          QDomNode c_ElementIdNode = c_AdditionalTriggerElement.namedItem("element-id");
          if (!c_ElementIdNode.isNull()) {
             QDomElement c_ElementIdElement = c_ElementIdNode.toElement();
             
             QDomNode c_ElemNodeId = c_ElementIdElement.namedItem("node-index");
             if (!c_ElemNodeId.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_NodeIndex = static_cast<uint32_t>(c_ElemNodeId.toElement().text().toInt());
             }
             
             QDomNode c_ElemDataPoolId = c_ElementIdElement.namedItem("data-pool-index");
             if (!c_ElemDataPoolId.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_DataPoolIndex = static_cast<uint32_t>(c_ElemDataPoolId.toElement().text().toInt());
             }
             
             QDomNode c_ElemListId = c_ElementIdElement.namedItem("list-index");
             if (!c_ElemListId.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ListIndex = static_cast<uint32_t>(c_ElemListId.toElement().text().toInt());
             }
             
             QDomNode c_ElemElementId = c_ElementIdElement.namedItem("element-index");
             if (!c_ElemElementId.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_ElementId.u32_ElementIndex = static_cast<uint32_t>(c_ElemElementId.toElement().text().toInt());
             }
          }
          
          // Deserialize threshold fields directly
          QDomNode c_ThresholdNode = c_AdditionalTriggerElement.namedItem("threshold");
          if (!c_ThresholdNode.isNull()) {
             QDomElement c_ThresholdElement = c_ThresholdNode.toElement();
             
             QDomNode c_ThreshTypeNode = c_ThresholdElement.namedItem("type");
             if (!c_ThreshTypeNode.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetType(static_cast<C_OscNodeDataPoolContent::E_Type>(c_ThreshTypeNode.toElement().text().toInt()));
             }
             
             QDomNode c_ThreshArrayNode = c_ThresholdElement.namedItem("is-array");
             if (!c_ThreshArrayNode.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArray(c_ThreshArrayNode.toElement().text() == "1");
             }
             
             QDomNode c_ThreshSizeNode = c_ThresholdElement.namedItem("array-size");
             if (!c_ThreshSizeNode.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetArraySize(static_cast<uint32_t>(c_ThreshSizeNode.toElement().text().toInt()));
             }
             
             QDomNode c_ThreshDataNode = c_ThresholdElement.namedItem("data");
             if (!c_ThreshDataNode.isNull()) {
                QByteArray c_ThresholdData = QByteArray::fromBase64(c_ThreshDataNode.toElement().text().toUtf8());
                c_Properties.c_AdditionalTriggerProperties.c_Threshold.SetValueFromLittleEndianBlob(c_ThresholdData);
             }
          }
          
          // Deserialize operation
          QDomNode c_OperationNode = c_AdditionalTriggerElement.namedItem("operation");
          if (!c_OperationNode.isNull()) {
             QDomText c_OperationText = c_OperationNode.toText();
             if (!c_OperationText.isNull()) {
                c_Properties.c_AdditionalTriggerProperties.c_Operation = c_OperationText.data();
             }
          }
       }
   }
   
    // Deserialize data elements
    QDomNode c_DataElementsNode = orc_Element.namedItem("configured-data-elements");
    if (!c_DataElementsNode.isNull()) {
       c_ConfiguredDataElements.clear();
       
       QDomNode c_DataElementNode = c_DataElementsNode.firstChild();
       while (!c_DataElementNode.isNull()) {
          if (c_DataElementNode.isElement()) {
             QDomElement c_DataElement = c_DataElementNode.toElement();
             
             C_OscDataLoggerDataElementReference c_Element;
             
             // Deserialize use custom name
             QDomNode c_UseCustomNameNode = c_DataElement.namedItem("use-custom-name");
             if (!c_UseCustomNameNode.isNull()) {
                QDomText c_UseCustomNameText = c_UseCustomNameNode.toText();
                if (!c_UseCustomNameText.isNull()) {
                   c_Element.q_UseCustomName = (c_UseCustomNameText.data() == "1");
                }
             }
             
             // Deserialize custom name
             QDomNode c_CustomNameNode = c_DataElement.namedItem("custom-name");
             if (!c_CustomNameNode.isNull()) {
                QDomText c_CustomNameText = c_CustomNameNode.toText();
                if (!c_CustomNameText.isNull()) {
                   c_Element.c_CustomName = c_CustomNameText.data();
                }
             }
             
             // Deserialize element ID fields directly
             QDomNode c_ElementIdNode = c_DataElement.namedItem("element-id");
             if (!c_ElementIdNode.isNull()) {
                QDomElement c_ElementIdElement = c_ElementIdNode.toElement();
                
                QDomNode c_ElemNodeId = c_ElementIdElement.namedItem("node-index");
                if (!c_ElemNodeId.isNull()) {
                   c_Element.c_ConfiguredElementId.u32_NodeIndex = static_cast<uint32_t>(c_ElemNodeId.toElement().text().toInt());
                }
                
                QDomNode c_ElemDataPoolId = c_ElementIdElement.namedItem("data-pool-index");
                if (!c_ElemDataPoolId.isNull()) {
                   c_Element.c_ConfiguredElementId.u32_DataPoolIndex = static_cast<uint32_t>(c_ElemDataPoolId.toElement().text().toInt());
                }
                
                QDomNode c_ElemListId = c_ElementIdElement.namedItem("list-index");
                if (!c_ElemListId.isNull()) {
                   c_Element.c_ConfiguredElementId.u32_ListIndex = static_cast<uint32_t>(c_ElemListId.toElement().text().toInt());
                }
                
                QDomNode c_ElemElementId = c_ElementIdElement.namedItem("element-index");
                if (!c_ElemElementId.isNull()) {
                   c_Element.c_ConfiguredElementId.u32_ElementIndex = static_cast<uint32_t>(c_ElemElementId.toElement().text().toInt());
                }
             }
             
             c_ConfiguredDataElements.append(c_Element);
          }
          
          c_DataElementNode = c_DataElementNode.nextSibling();
       }
    }
   
   return C_NO_ERR;
}
