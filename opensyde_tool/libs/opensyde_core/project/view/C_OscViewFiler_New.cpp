//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       View reader/writer (Multi-Format Implementation)

   Load / save view data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscViewFiler_New.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include "C_OscLoggingHandler.hpp"

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
/*! \brief   Load view from file (auto-detect format)

   \param[out]     orc_View         View data
   \param[in]      orc_FilePath     File path
   \param[in]      orc_OscNodes     List of nodes (for reference resolution)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadViewFile(C_OscViewData &orc_View,
                                           const QString &orc_FilePath,
                                           const QList<C_OscNode> &orc_OscNodes) {
   // Note: For binary and JSON, orc_OscNodes may not be needed if references are stored as IDs
   // For XML, delegate to legacy implementation if complex reference resolution is required
   Q_UNUSED(orc_OscNodes);
   
   return mh_DetectAndLoad(orc_View, orc_FilePath, orc_OscNodes);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to file (auto-detect format from extension)

   \param[in]      orc_View         View data to store
   \param[in]      orc_FilePath     File path
   \param[in]      orc_OscNodes     List of nodes (for reference resolution)

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_SaveViewFile(const C_OscViewData &orc_View,
                                           const QString &orc_FilePath,
                                           const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_View, orc_FilePath, orc_OscNodes);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_View, orc_FilePath, orc_OscNodes);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_View, orc_FilePath, orc_OscNodes);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_View, orc_FilePath, orc_OscNodes);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from binary file

   \param[out]     orc_View         View data
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadBinary(C_OscViewData &orc_View,
                                         const QString &orc_Path,
                                         const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading view",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_View.FromQDataStream(in);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to binary file

   \param[in]      orc_View         View data to store
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_SaveBinary(const C_OscViewData &orc_View,
                                         const QString &orc_Path,
                                         const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving view",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_View.ToQDataStream(out);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Saving view",
                          "Failed to serialize view data to binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from memory (binary)

   \param[out]     orc_View         View data
   \param[in]      orc_Data         Binary data

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadFromMemoryBinary(C_OscViewData &orc_View,
                                                   const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_View.FromQDataStream(in);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from memory buffer.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to memory (binary)

   \param[in]      orc_View         View data to store

   \return
   QByteArray     Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscViewFiler_New::h_SaveToMemoryBinary(const C_OscViewData &orc_View) {
   QByteArray c_Data;
   QDataStream out(&c_Data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);

   orc_View.ToQDataStream(out);

   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from JSON file

   \param[out]     orc_View         View data
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadJson(C_OscViewData &orc_View,
                                       const QString &orc_Path,
                                       const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading view",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(file.readAll(), &c_ParseError);
   file.close();

   if (c_ParseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading view",
                          QString("JSON parse error: %1").arg(c_ParseError.errorString()));
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      osc_write_log_error("Loading view",
                          "JSON root element is not an object.");
      return C_CONFIG;
   }

   int32_t s32_Result = orc_View.FromJsonObject(c_Doc.object());

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from JSON file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to JSON file

   \param[in]      orc_View         View data to store
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_SaveJson(const C_OscViewData &orc_View,
                                       const QString &orc_Path,
                                       const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving view",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonObject c_Json = orc_View.ToJsonObject();
   QJsonDocument c_Doc(c_Json);
   file.write(c_Doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from memory (JSON)

   \param[out]     orc_View         View data
   \param[in]      orc_Object       JSON object

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadFromMemoryJson(C_OscViewData &orc_View,
                                                 const QJsonObject &orc_Object) {
   int32_t s32_Result = orc_View.FromJsonObject(orc_Object);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from JSON object.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to memory (JSON)

   \param[in]      orc_View         View data to store

   \return
   QJsonObject    JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscViewFiler_New::h_SaveToMemoryJson(const C_OscViewData &orc_View) {
   return orc_View.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from XML file

   \param[out]     orc_View         View data
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadXml(C_OscViewData &orc_View,
                                      const QString &orc_Path,
                                      const QList<C_OscNode> &orc_OscNodes) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading view",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QString c_Error;
   int i_Line, i_Column;

   if (!c_Doc.setContent(file.readAll(), &c_Error, &i_Line, &i_Column)) {
      osc_write_log_error("Loading view",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(i_Line).arg(i_Column).arg(c_Error));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement c_Root = c_Doc.documentElement();
   int32_t s32_Result = orc_View.FromQDomElement(c_Root);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from XML file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to XML file

   \param[in]      orc_View         View data to store
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_SaveXml(const C_OscViewData &orc_View,
                                      const QString &orc_Path,
                                      const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving view",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QDomElement c_Element = orc_View.ToQDomDocument(c_Doc, "view");
   c_Doc.appendChild(c_Element);

   // Write with XML declaration
   file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   file.write(c_Doc.toString().toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view from memory (XML)

   \param[out]     orc_View         View data
   \param[in]      orc_Element      XML element
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadFromMemoryXml(C_OscViewData &orc_View,
                                                const QDomElement &orc_Element,
                                                const QList<C_OscNode> &orc_OscNodes) {
   Q_UNUSED(orc_OscNodes);
   
   int32_t s32_Result = orc_View.FromQDomElement(orc_Element);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading view",
                          "Failed to deserialize view data from XML element.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view to memory (XML)

   \param[in]      orc_View         View data to store
   \param[in,out]  orc_Doc          DOM document

   \return
   QDomElement    XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscViewFiler_New::h_SaveToMemoryXml(const C_OscViewData &orc_View,
                                                  QDomDocument &orc_Doc) {
   return orc_View.ToQDomDocument(orc_Doc, "view");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view (legacy XML compatibility)

   \param[out]     orc_View         View data
   \param[in]      orc_FilePath     File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::h_LoadViewFileOsc(C_OscViewData &orc_View,
                                              const QString &orc_FilePath,
                                              const QList<C_OscNode> &orc_OscNodes) {
   // Delegate to original implementation for backward compatibility
   return C_OscViewFiler::mh_LoadViewFileOsc(orc_View, orc_FilePath, orc_OscNodes);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view (legacy XML compatibility)

   \param[in]      orc_View         View data to store
   \param[in]      orc_FilePath     File path
   \param[in]      orc_OscNodes     List of nodes
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewFiler_New::h_SaveViewFileOsc(const C_OscViewData &orc_View,
                                           const QString &orc_FilePath,
                                           const QList<C_OscNode> &orc_OscNodes) {
   // Delegate to original implementation for backward compatibility
   C_OscViewFiler::mh_SaveViewFileOsc(orc_View, orc_FilePath, orc_OscNodes);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect format and load from file

   \param[out]     orc_View         View data
   \param[in]      orc_Path         File path
   \param[in]      orc_OscNodes     List of nodes

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewFiler_New::mh_DetectAndLoad(C_OscViewData &orc_View,
                                             const QString &orc_Path,
                                             const QList<C_OscNode> &orc_OscNodes) {
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_View, orc_Path, orc_OscNodes);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_View, orc_Path, orc_OscNodes);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_View, orc_Path, orc_OscNodes);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      return h_LoadXml(orc_View, orc_Path, orc_OscNodes);
   }
}
