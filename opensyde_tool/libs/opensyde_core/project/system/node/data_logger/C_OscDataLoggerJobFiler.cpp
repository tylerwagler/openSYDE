//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscDataLoggerJob with Qt-native serialization
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscDataLoggerJobFiler_New.hpp"
#include "C_OscDataLoggerJob.hpp"
#include "C_OscDataLoggerJobProperties.hpp"
#include "C_OscDataLoggerJobAdditionalTriggerProperties.hpp"
#include "C_OscDataLoggerDataElementReference.hpp"
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscDataLoggerJobFiler_New::C_OscDataLoggerJobFiler_New() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from file with auto-detection of format based on extension

   \param[out] orc_Config Path to file to load from
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadFile(QList<C_OscDataLoggerJob> &orc_Config,
                                                const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_LoadBinary(orc_Config, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_LoadJson(orc_Config, orc_Path);
   } else {
      // Default to XML
      return h_LoadXml(orc_Config, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to file with auto-detection of format based on extension

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveFile(const QList<C_OscDataLoggerJob> &orc_Config,
                                                const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_SaveBinary(orc_Config, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_SaveJson(orc_Config, orc_Path);
   } else {
      // Default to XML
      return h_SaveXml(orc_Config, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from binary file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadBinary(QList<C_OscDataLoggerJob> &orc_Config,
                                                  const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Read version
   uint16_t u16_Version = 0;
   c_Stream >> u16_Version;
   
   if (u16_Version != 1) {
      return C_CONFIG;
   }
   
   // Read count of jobs
   uint32_t u32_Count = 0;
   c_Stream >> u32_Count;
   
   orc_Config.clear();
   
   // Read jobs
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscDataLoggerJob c_Job;
      
      int32_t s32_Result = mh_LoadJobData(c_Job, c_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
      
      orc_Config.append(c_Job);
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to binary file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveBinary(const QList<C_OscDataLoggerJob> &orc_Config,
                                                   const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Write version
   uint16_t u16_Version = 1;
   c_Stream << u16_Version;
   
   // Write count of jobs
   uint32_t u32_Count = orc_Config.size();
   c_Stream << u32_Count;
   
   // Write jobs
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      int32_t s32_Result = mh_SaveJobData(c_Job, c_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from JSON file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadJson(QList<C_OscDataLoggerJob> &orc_Config,
                                                const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
   if (c_Doc.isNull()) {
      return C_CONFIG;
   }
   
   if (!c_Doc.isObject()) {
      return C_CONFIG;
   }
   
   QJsonObject c_Object = c_Doc.object();
   
   // Look for jobs array
   if (!c_Object.contains("jobs")) {
      return C_CONFIG;
   }
   
   QJsonArray c_JobsArray = c_Object["jobs"].toArray();
   
   orc_Config.clear();
   
   for (const QJsonValue &c_JobValue : c_JobsArray) {
      if (!c_JobValue.isObject()) {
         return C_CONFIG;
      }
      
      C_OscDataLoggerJob c_Job;
      
      int32_t s32_Result = c_Job.FromJsonObject(c_JobValue.toObject());
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
      
      orc_Config.append(c_Job);
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to JSON file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveJson(const QList<C_OscDataLoggerJob> &orc_Config,
                                                 const QString &orc_Path) {
   using namespace stw::errors;
   
   QJsonObject c_Object;
   
   // Create jobs array
   QJsonArray c_JobsArray;
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      c_JobsArray.append(c_Job.ToJsonObject());
   }
   
   c_Object["jobs"] = c_JobsArray;
   
   QJsonDocument c_Doc(c_Object);
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   c_File.write(c_Doc.toJson(QJsonDocument::Compact));
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from XML file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadXml(QList<C_OscDataLoggerJob> &orc_Config,
                                               const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File)) {
      return C_CONFIG;
   }
   
   QDomElement c_RootElement = c_Doc.documentElement();
   if (c_RootElement.tagName() != "data-logger-jobs") {
      return C_CONFIG;
   }
   
   orc_Config.clear();
   
   // Process each job element
   QDomNode c_Node = c_RootElement.firstChild();
   while (!c_Node.isNull()) {
      if (c_Node.isElement()) {
         QDomElement c_Element = c_Node.toElement();
         
         if (c_Element.tagName() == "job") {
            C_OscDataLoggerJob c_Job;
            
            int32_t s32_Result = c_Job.FromQDomElement(c_Element);
            if (s32_Result != C_NO_ERR) {
               return s32_Result;
            }
            
            orc_Config.append(c_Job);
         }
      }
      
      c_Node = c_Node.nextSibling();
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to XML file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveXml(const QList<C_OscDataLoggerJob> &orc_Config,
                                                const QString &orc_Path) {
   using namespace stw::errors;
   
   QDomDocument c_Doc("data-logger-jobs");
   QDomElement c_RootElement = c_Doc.createElement("data-logger-jobs");
   c_Doc.appendChild(c_RootElement);
   
   // Add each job
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      QDomElement c_JobElement = c_Job.ToQDomDocument(c_Doc, "job");
      c_RootElement.appendChild(c_JobElement);
   }
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load job data from QDataStream

   \param[out] orc_Config Job configuration to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_LoadJobData(C_OscDataLoggerJob &orc_Config,
                                                     QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Load enabled flag
   orc_Stream >> orc_Config.q_IsEnabled;
   
   // Load properties
   int32_t s32_Result = mh_LoadJobProperties(orc_Config.c_Properties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Load data elements
   s32_Result = mh_LoadConfiguredDataElements(orc_Config.c_ConfiguredDataElements, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save job data to QDataStream

   \param[in] orc_Config Job configuration to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_SaveJobData(const C_OscDataLoggerJob &orc_Config,
                                                     QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Save enabled flag
   orc_Stream << orc_Config.q_IsEnabled;
   
   // Save properties
   int32_t s32_Result = mh_SaveJobProperties(orc_Config.c_Properties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Save data elements
   s32_Result = mh_SaveConfiguredDataElements(orc_Config.c_ConfiguredDataElements, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load job properties from QDataStream

   \param[out] orc_Config Properties to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_LoadJobProperties(C_OscDataLoggerJobProperties &orc_Config,
                                                           QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Load basic properties
   orc_Stream >> orc_Config.c_Name;
   orc_Stream >> orc_Config.c_Comment;
   
   // Load enums as integer values
   uint32_t u32_Elem = 0;
   orc_Stream >> u32_Elem;
   orc_Config.e_UseCase = static_cast<C_OscDataLoggerJobProperties::E_UseCase>(u32_Elem);
   
   orc_Stream >> u32_Elem;
   orc_Config.e_LogFileFormat = static_cast<C_OscDataLoggerJobProperties::E_LogFileFormat>(u32_Elem);
   
   orc_Stream >> orc_Config.u32_MaxLogEntries;
   orc_Stream >> orc_Config.u32_MaxLogDurationSec;
   orc_Stream >> orc_Config.u32_LogIntervalMs;
   
   orc_Stream >> u32_Elem;
   orc_Config.e_LocalLogTrigger = static_cast<C_OscDataLoggerJobProperties::E_LocalLogTrigger>(u32_Elem);
   
   orc_Stream >> orc_Config.c_LogDestinationDirectory;
   
   // Load additional trigger properties
   int32_t s32_Result = mh_LoadJobAdditionalTriggerProperties(orc_Config.c_AdditionalTriggerProperties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save job properties to QDataStream

   \param[in] orc_Config Properties to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_SaveJobProperties(const C_OscDataLoggerJobProperties &orc_Config,
                                                           QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Save basic properties
   orc_Stream << orc_Config.c_Name;
   orc_Stream << orc_Config.c_Comment;
   
   // Save enums as integer values
   orc_Stream << static_cast<uint32_t>(orc_Config.e_UseCase);
   orc_Stream << static_cast<uint32_t>(orc_Config.e_LogFileFormat);
   orc_Stream << orc_Config.u32_MaxLogEntries;
   orc_Stream << orc_Config.u32_MaxLogDurationSec;
   orc_Stream << orc_Config.u32_LogIntervalMs;
   
   orc_Stream << static_cast<uint32_t>(orc_Config.e_LocalLogTrigger);
   
   orc_Stream << orc_Config.c_LogDestinationDirectory;
   
   // Save additional trigger properties
   int32_t s32_Result = mh_SaveJobAdditionalTriggerProperties(orc_Config.c_AdditionalTriggerProperties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load additional trigger properties from QDataStream

   \param[out] orc_Config Properties to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_LoadJobAdditionalTriggerProperties(
    C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Load flag
   orc_Stream >> orc_Config.q_Enable;
   
   // Load element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_LoadFromStream(orc_Config.c_ElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Load threshold
   s32_Result = C_OscNodeDataPoolContent::h_LoadFromStream(orc_Config.c_Threshold, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Load operation string
   orc_Stream >> orc_Config.c_Operation;
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save additional trigger properties to QDataStream

   \param[in] orc_Config Properties to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_SaveJobAdditionalTriggerProperties(
    const C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Save flag
   orc_Stream << orc_Config.q_Enable;
   
   // Save element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_SaveToStream(orc_Config.c_ElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Save threshold
   s32_Result = C_OscNodeDataPoolContent::h_SaveToStream(orc_Config.c_Threshold, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   // Save operation string
   orc_Stream << orc_Config.c_Operation;
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load configured data elements from QDataStream

   \param[out] orc_Config Elements to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_LoadConfiguredDataElements(
    QList<C_OscDataLoggerDataElementReference> &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Load count
   uint32_t u32_Count = 0;
   orc_Stream >> u32_Count;
   
   orc_Config.clear();
   
   // Load elements
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscDataLoggerDataElementReference c_Element;
      
      int32_t s32_Result = mh_LoadConfiguredDataElement(c_Element, orc_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
      
      orc_Config.append(c_Element);
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save configured data elements to QDataStream

   \param[in] orc_Config Elements to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_SaveConfiguredDataElements(
    const QList<C_OscDataLoggerDataElementReference> &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Save count
   uint32_t u32_Count = orc_Config.size();
   orc_Stream << u32_Count;
   
   // Save elements
   for (const C_OscDataLoggerDataElementReference &c_Element : orc_Config) {
      int32_t s32_Result = mh_SaveConfiguredDataElement(c_Element, orc_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load configured data element from QDataStream

   \param[out] orc_Config Element to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_LoadConfiguredDataElement(
    C_OscDataLoggerDataElementReference &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Load flags
   orc_Stream >> orc_Config.q_UseCustomName;
   
   // Load custom name
   orc_Stream >> orc_Config.c_CustomName;
   
   // Load element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_LoadFromStream(orc_Config.c_ConfiguredElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save configured data element to QDataStream

   \param[in] orc_Config Element to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
int32_t C_OscDataLoggerJobFiler_New::mh_SaveConfiguredDataElement(
    const C_OscDataLoggerDataElementReference &orc_Config,
    QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Save flags
   orc_Stream << orc_Config.q_UseCustomName;
   
   // Save custom name
   orc_Stream << orc_Config.c_CustomName;
   
   // Save element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_SaveToStream(orc_Config.c_ConfiguredElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}