//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job reader/writer (Multi-Format Implementation)

   Load / save data logger job data from / to binary, JSON, or XML files using
   the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include "C_OscDataLoggerJobFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>
#include <QDomDocument>

using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::scl;

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
/*! \brief   Load data logger job from file (auto-detect format)

   \param[out]     orc_Job          Job data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadFile(C_OscDataLoggerJob &orc_Job,
                                                const QString &orc_Path) {
   return mh_DetectAndLoad(orc_Job, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data logger job to file (auto-detect format from extension)

   \param[in]      orc_Job          Job data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveFile(const C_OscDataLoggerJob &orc_Job,
                                                const QString &orc_Path) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Job, orc_Path);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Job, orc_Path);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_Job, orc_Path);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      s32_Retval = h_SaveXml(orc_Job, orc_Path);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from binary file

   \param[out]     orc_Job          Job data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadBinary(C_OscDataLoggerJob &orc_Job,
                                                  const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data logger job",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Job.FromQDataStream(in);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to binary file

   \param[in]      orc_Job          Job data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveBinary(const C_OscDataLoggerJob &orc_Job,
                                                  const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data logger job",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Job.ToQDataStream(out);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Saving data logger job",
                          "Failed to serialize job data to binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from memory (binary)

   \param[out]     orc_Job          Job data
   \param[in]      orc_Data         Binary data

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadFromMemoryBinary(C_OscDataLoggerJob &orc_Job,
                                                            const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Job.FromQDataStream(in);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from memory buffer.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to memory (binary)

   \param[in]      orc_Job          Job data to store

   \return
   QByteArray     Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscDataLoggerJobFiler_New::h_SaveToMemoryBinary(
   const C_OscDataLoggerJob &orc_Job) {
   QByteArray c_Data;
   QDataStream out(&c_Data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);

   orc_Job.ToQDataStream(out);

   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from JSON file

   \param[out]     orc_Job          Job data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadJson(C_OscDataLoggerJob &orc_Job,
                                                const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data logger job",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(file.readAll(), &c_ParseError);
   file.close();

   if (c_ParseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading data logger job",
                          QString("JSON parse error: %1").arg(c_ParseError.errorString()));
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      osc_write_log_error("Loading data logger job",
                          "JSON root element is not an object.");
      return C_CONFIG;
   }

   int32_t s32_Result = orc_Job.FromJsonObject(c_Doc.object());

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from JSON file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to JSON file

   \param[in]      orc_Job          Job data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveJson(const C_OscDataLoggerJob &orc_Job,
                                                const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data logger job",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonObject c_Json = orc_Job.ToJsonObject();
   QJsonDocument c_Doc(c_Json);
   file.write(c_Doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from memory (JSON)

   \param[out]     orc_Job          Job data
   \param[in]      orc_Object       JSON object

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadFromMemoryJson(C_OscDataLoggerJob &orc_Job,
                                                          const QJsonObject &orc_Object) {
   int32_t s32_Result = orc_Job.FromJsonObject(orc_Object);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from JSON object.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to memory (JSON)

   \param[in]      orc_Job          Job data to store

   \return
   QJsonObject    JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscDataLoggerJobFiler_New::h_SaveToMemoryJson(
   const C_OscDataLoggerJob &orc_Job) {
   return orc_Job.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from XML file

   \param[out]     orc_Job          Job data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadXml(C_OscDataLoggerJob &orc_Job,
                                               const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data logger job",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QString c_Error;
   int i_Line, i_Column;

   if (!c_Doc.setContent(file.readAll(), &c_Error, &i_Line, &i_Column)) {
      osc_write_log_error("Loading data logger job",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(i_Line).arg(i_Column).arg(c_Error));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement c_Root = c_Doc.documentElement();
   int32_t s32_Result = orc_Job.FromQDomElement(c_Root);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from XML file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to XML file

   \param[in]      orc_Job          Job data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_SaveXml(const C_OscDataLoggerJob &orc_Job,
                                               const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data logger job",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QDomElement c_Element = orc_Job.ToQDomDocument(c_Doc, "job");
   c_Doc.appendChild(c_Element);

   // Write with XML declaration
   file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   file.write(c_Doc.toString().toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job from memory (XML)

   \param[out]     orc_Job          Job data
   \param[in]      orc_Element      XML element

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadFromMemoryXml(C_OscDataLoggerJob &orc_Job,
                                                         const QDomElement &orc_Element) {
   int32_t s32_Result = orc_Job.FromQDomElement(orc_Element);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading data logger job",
                          "Failed to deserialize job data from XML element.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job to memory (XML)

   \param[in]      orc_Job          Job data to store
   \param[in,out]  orc_Doc          DOM document

   \return
   QDomElement    XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscDataLoggerJobFiler_New::h_SaveToMemoryXml(
   const C_OscDataLoggerJob &orc_Job, QDomDocument &orc_Doc) {
   return orc_Job.ToQDomDocument(orc_Doc, "job");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load job (legacy XML compatibility)

   \param[out]     orc_Job          Job data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler_New::h_LoadJob(C_OscDataLoggerJob &orc_Job,
                                               C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   return C_OscDataLoggerJobFiler::h_LoadJob(orc_Job, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save job (legacy XML compatibility)

   \param[in]      orc_Job          Job data to store
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler_New::h_SaveJob(const C_OscDataLoggerJob &orc_Job,
                                            C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   C_OscDataLoggerJobFiler::h_SaveJob(orc_Job, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect format and load from file

   \param[out]     orc_Job          Job data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
  //----------------------------------------------------------------------------------------------------------------------
  int32_t C_OscDataLoggerJobFiler_New::mh_DetectAndLoad(C_OscDataLoggerJob &orc_Job,
                                                        const QString &orc_Path) {
     const QString c_Extension = orc_Path.right(4).toLower();

     if (c_Extension == ".bin") {
        return h_LoadBinary(orc_Job, orc_Path);
     } else if (c_Extension == ".json") {
        return h_LoadJson(orc_Job, orc_Path);
     } else if (c_Extension == ".xml") {
        return h_LoadXml(orc_Job, orc_Path);
     } else {
        // Default to XML for backward compatibility
        osc_write_log_warning("File I/O",
                              QString("Unknown file extension \"%1\" for \"%2\". "
                                      "Defaulting to XML format.")
                              .arg(c_Extension, orc_Path));
        return h_LoadXml(orc_Job, orc_Path);
     }
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Load data logger jobs from XML parser (clipboard support)

     \param[out]     orc_Config       Job configuration list
     \param[in]      orc_XmlParser    XML parser

     \return
     C_NO_ERR   data read
     C_CONFIG   content of file is invalid
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t C_OscDataLoggerJobFiler_New::h_LoadData(QList<C_OscDataLoggerJob> &orc_Config,
                                                  C_OscXmlParserBase &orc_XmlParser) {
     // Implementation copied from legacy filer
     int32_t s32_Retval = C_NO_ERR;
     uint16_t u16_Version;
     
     s32_Retval = C_OscSystemFilerUtil::h_CheckVersion(orc_XmlParser, 1U, "file-version",
                                                        "Loading data loggers data");
     if (s32_Retval == C_NO_ERR) {
        if (orc_XmlParser.SelectNodeChild("data-loggers") == "data-loggers") {
           uint32_t u32_JobCount = orc_XmlParser.GetNumChildren();
           for (uint32_t u32_Job = 0U; u32_Job < u32_JobCount; u32_Job++) {
              C_OscDataLoggerJob c_Job;
              if (h_LoadJob(c_Job, orc_XmlParser) == C_NO_ERR) {
                 orc_Config.append(c_Job);
              }
           }
           orc_XmlParser.SelectNodeParent();
        } else {
           s32_Retval = C_CONFIG;
        }
     }
     return s32_Retval;
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Save data logger jobs to XML parser (clipboard support)

     \param[in]      orc_Config       Job configuration list
     \param[in]      orc_XmlParser    XML parser
  */
  //----------------------------------------------------------------------------------------------------------------------
  void C_OscDataLoggerJobFiler_New::h_SaveData(const QList<C_OscDataLoggerJob> &orc_Config,
                                               C_OscXmlParserBase &orc_XmlParser) {
     // Implementation copied from legacy filer
     Q_ASSERT(orc_XmlParser.CreateAndNodeChild("file-version") == "file-version");
     orc_XmlParser.SetAttribute("version", "1");
     orc_XmlParser.SelectNodeParent();
     
     Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("data-loggers") == "data-loggers");
     for (QList<C_OscDataLoggerJob>::ConstIterator c_It = orc_Config.constBegin();
          c_It != orc_Config.constEnd(); ++c_It) {
        h_SaveJob(*c_It, orc_XmlParser);
     }
     orc_XmlParser.SelectNodeParent();
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Load data element ID from XML parser

     \param[out]     orc_Config       Data element ID
     \param[in]      orc_XmlParser    XML parser

     \return
     C_NO_ERR   data read
     C_CONFIG   content of file is invalid
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t C_OscDataLoggerJobFiler_New::h_LoadDataElementId(C_OscNodeDataPoolListElementId &orc_Config,
                                                           C_OscXmlParserBase &orc_XmlParser) {
     int32_t s32_Retval = C_NO_ERR;
     
     if (orc_XmlParser.SelectNodeChild("data-pool") == "data-pool") {
        orc_Config.c_NodeName = orc_XmlParser.GetNodeContent();
        orc_XmlParser.SelectNodeParent();
     }
     
     if (orc_XmlParser.SelectNodeChild("data-pool-list") == "data-pool-list") {
        orc_Config.c_ListName = orc_XmlParser.GetNodeContent();
        orc_XmlParser.SelectNodeParent();
     }
     
     if (orc_XmlParser.SelectNodeChild("data-pool-list-element") == "data-pool-list-element") {
        orc_Config.c_ElementName = orc_XmlParser.GetNodeContent();
        orc_XmlParser.SelectNodeParent();
     }
     
     return s32_Retval;
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Save data element ID to XML parser

     \param[in]      orc_Config       Data element ID
     \param[in]      orc_XmlParser    XML parser
  */
  //----------------------------------------------------------------------------------------------------------------------
  void C_OscDataLoggerJobFiler_New::h_SaveDataElementId(
      const C_OscNodeDataPoolListElementId &orc_Config, C_OscXmlParserBase &orc_XmlParser) {
     Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("data-pool") == "data-pool");
     orc_XmlParser.SetNodeContent(orc_Config.c_NodeName);
     orc_XmlParser.SelectNodeParent();
     
     Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("data-pool-list") == "data-pool-list");
     orc_XmlParser.SetNodeContent(orc_Config.c_ListName);
     orc_XmlParser.SelectNodeParent();
     
     Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("data-pool-list-element") == "data-pool-list-element");
     orc_XmlParser.SetNodeContent(orc_Config.c_ElementName);
     orc_XmlParser.SelectNodeParent();
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Load data element optional array ID from XML parser

     \param[out]     orc_Config       Data element optional array ID
     \param[in]      orc_XmlParser    XML parser

     \return
     C_NO_ERR   data read
     C_CONFIG   content of file is invalid
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t C_OscDataLoggerJobFiler_New::h_LoadDataElementOptArrayId(
      C_OscNodeDataPoolListElementOptArrayId &orc_Config, C_OscXmlParserBase &orc_XmlParser) {
     int32_t s32_Retval = h_LoadDataElementId(orc_Config.c_Id, orc_XmlParser);
     
     if (s32_Retval == C_NO_ERR) {
        if (orc_XmlParser.SelectNodeChild("array-id") == "array-id") {
           orc_Config.c_ArrayId = static_cast<uint32_t>(orc_XmlParser.GetAttributeContent("index").toUInt());
           orc_XmlParser.SelectNodeParent();
        }
     }
     
     return s32_Retval;
  }

  //----------------------------------------------------------------------------------------------------------------------
  /*! \brief   Save data element optional array ID to XML parser

     \param[in]      orc_Config       Data element optional array ID
     \param[in]      orc_XmlParser    XML parser
  */
  //----------------------------------------------------------------------------------------------------------------------
  void C_OscDataLoggerJobFiler_New::h_SaveDataElementOptArrayId(
      const C_OscNodeDataPoolListElementOptArrayId &orc_Config, C_OscXmlParserBase &orc_XmlParser) {
     h_SaveDataElementId(orc_Config.c_Id, orc_XmlParser);
     
     Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("array-id") == "array-id");
     orc_XmlParser.SetAttribute("index", QString::number(orc_Config.c_ArrayId));
     orc_XmlParser.SelectNodeParent();
  }
