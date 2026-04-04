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
int32_t C_OscDataLoggerJobFiler::h_LoadFile(C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_SaveFile(const C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_LoadBinary(C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_SaveBinary(const C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_LoadFromMemoryBinary(C_OscDataLoggerJob &orc_Job,
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
QByteArray C_OscDataLoggerJobFiler::h_SaveToMemoryBinary(
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
int32_t C_OscDataLoggerJobFiler::h_LoadJson(C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_SaveJson(const C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_LoadFromMemoryJson(C_OscDataLoggerJob &orc_Job,
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
QJsonObject C_OscDataLoggerJobFiler::h_SaveToMemoryJson(
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
int32_t C_OscDataLoggerJobFiler::h_LoadXml(C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_SaveXml(const C_OscDataLoggerJob &orc_Job,
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
int32_t C_OscDataLoggerJobFiler::h_LoadFromMemoryXml(C_OscDataLoggerJob &orc_Job,
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
QDomElement C_OscDataLoggerJobFiler::h_SaveToMemoryXml(
   const C_OscDataLoggerJob &orc_Job, QDomDocument &orc_Doc) {
   return orc_Job.ToQDomDocument(orc_Doc, "job");
}

