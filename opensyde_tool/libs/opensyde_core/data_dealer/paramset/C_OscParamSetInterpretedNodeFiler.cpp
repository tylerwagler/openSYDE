//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for interpreted parameter set node files (Multi-Format Implementation)

   Load / save interpreted parameter set node data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetInterpretedNodeFiler.hpp"
#include "C_OscParamSetInterpretedNodeFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------------------------------------
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
/*! \brief   Load interpreted node from file (auto-detect format)

   \param[out]     orc_Node              Node data
   \param[in]      orc_FilePath          File path
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadInterpretedNodeFile(
  C_OscParamSetInterpretedNode &orc_Node, const QString &orc_FilePath, bool &orq_MissingOptionalContent) {
   orq_MissingOptionalContent = false;
   return mh_DetectAndLoad(orc_Node, orc_FilePath, orq_MissingOptionalContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to file (auto-detect format from extension)

   \param[in]      orc_Node              Node data to store
   \param[in]      orc_FilePath          File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_SaveInterpretedNodeFile(const C_OscParamSetInterpretedNode &orc_Node,
                                                                          const QString &orc_FilePath) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Node, orc_FilePath);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Node, orc_FilePath);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_Node, orc_FilePath);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                              .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_Node, orc_FilePath);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from binary file

   \param[out]     orc_Node              Node data
   \param[in]      orc_FilePath          File path
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadBinary(C_OscParamSetInterpretedNode &orc_Node,
                                                             const QString &orc_FilePath,
                                                             bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading interpreted parameter set node",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   orc_Node.FromQDataStream(in);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to binary file

   \param[in]      orc_Node              Node data to store
   \param[in]      orc_FilePath          File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_SaveBinary(const C_OscParamSetInterpretedNode &orc_Node,
                                                             const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving interpreted parameter set node",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);

   const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToQDataStream(out);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from memory (binary)

   \param[out]     orc_Node              Node data
   \param[in]      orc_Data              Serialized data
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   data format is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadFromMemoryBinary(C_OscParamSetInterpretedNode &orc_Node,
                                                                       const QByteArray &orc_Data,
                                                                       bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   orc_Node.FromQDataStream(in);

   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to memory (binary)

   \param[in]      orc_Node              Node data
   \return
   QByteArray containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscParamSetInterpretedNodeFiler_New::h_SaveToMemoryBinary(const C_OscParamSetInterpretedNode &orc_Node)
  const {
   QByteArray data;
   QDataStream out(&data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);
   const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToQDataStream(out);
   return data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from JSON file

   \param[out]     orc_Node              Node data
   \param[in]      orc_FilePath          File path
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   JSON parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadJson(C_OscParamSetInterpretedNode &orc_Node,
                                                           const QString &orc_FilePath,
                                                           bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading interpreted parameter set node",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QJsonParseError parseError;
   QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
   file.close();

   if (parseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading interpreted parameter set node",
                          QString("JSON parse error: %1").arg(parseError.errorString()));
      return C_CONFIG;
   }

   orc_Node.FromJsonObject(doc.object());
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to JSON file

   \param[in]      orc_Node              Node data to store
   \param[in]      orc_FilePath          File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_SaveJson(const C_OscParamSetInterpretedNode &orc_Node,
                                                           const QString &orc_FilePath) {
   QJsonObject json = const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToJsonObject();
   QJsonDocument doc(json);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving interpreted parameter set node",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from memory (JSON)

   \param[out]     orc_Node              Node data
   \param[in]      orc_Object            JSON object
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadFromMemoryJson(C_OscParamSetInterpretedNode &orc_Node,
                                                                     const QJsonObject &orc_Object,
                                                                     bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   orc_Node.FromJsonObject(orc_Object);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to memory (JSON)

   \param[in]      orc_Node              Node data
   \return
   QJsonObject containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetInterpretedNodeFiler_New::h_SaveToMemoryJson(const C_OscParamSetInterpretedNode &orc_Node)
  const {
   return const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from XML file

   \param[out]     orc_Node              Node data
   \param[in]      orc_FilePath          File path
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   XML parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadXml(C_OscParamSetInterpretedNode &orc_Node,
                                                          const QString &orc_FilePath,
                                                          bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading interpreted parameter set node",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDomDocument doc;
   QString errorMessage;
   int errorLine, errorColumn;

   if (!doc.setContent(file.readAll(), &errorMessage, &errorLine, &errorColumn)) {
      osc_write_log_error("Loading interpreted parameter set node",
                          QString("XML parse error at line %1, column %2: %3")
                            .arg(errorLine)
                            .arg(errorColumn)
                            .arg(errorMessage));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement rootElement = doc.documentElement();
   orc_Node.FromQDomDocument(rootElement);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to XML file

   \param[in]      orc_Node              Node data to store
   \param[in]      orc_FilePath          File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_SaveXml(const C_OscParamSetInterpretedNode &orc_Node,
                                                          const QString &orc_FilePath) {
   QDomDocument doc;
   QDomElement rootElement = const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToQDomDocument(doc, "node");
   doc.appendChild(rootElement);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving interpreted parameter set node",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toString(2).toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load interpreted node from memory (XML)

   \param[out]     orc_Node              Node data
   \param[in]      orc_Element           XML element
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadFromMemoryXml(C_OscParamSetInterpretedNode &orc_Node,
                                                                    const QDomElement &orc_Element,
                                                                    bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);

   orc_Node.FromQDomDocument(orc_Element);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save interpreted node to memory (XML)

   \param[in]      orc_Node              Node data
   \param[in]      ro_Doc                DOM document
   \return
   QDomElement containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscParamSetInterpretedNodeFiler_New::h_SaveToMemoryXml(const C_OscParamSetInterpretedNode &orc_Node,
                                                                      QDomDocument &ro_Doc) const {
   return const_cast<C_OscParamSetInterpretedNode &>(orc_Node).ToQDomDocument(ro_Doc, "node");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper for format detection and loading

   \param[out]     orc_Node              Node data
   \param[in]      orc_FilePath          File path
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   file format not supported or parse error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::mh_DetectAndLoad(C_OscParamSetInterpretedNode &orc_Node,
                                                                 const QString &orc_FilePath,
                                                                 bool &orq_MissingOptionalContent) {
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_Node, orc_FilePath, orq_MissingOptionalContent);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_Node, orc_FilePath, orq_MissingOptionalContent);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_Node, orc_FilePath, orq_MissingOptionalContent);
   } else {
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Attempting XML format as fallback.")
                              .arg(c_Extension, orc_FilePath));
      return h_LoadXml(orc_Node, orc_FilePath, orq_MissingOptionalContent);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load interpreted node (deprecated)

   \param[out]     orc_Node              Node data
   \param[in]      orc_XmlParser         XML parser
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetInterpretedNodeFiler_New::h_LoadInterpretedNode(C_OscParamSetInterpretedNode &orc_Node,
                                                                      C_OscXmlParserBase &orc_XmlParser,
                                                                      bool &orq_MissingOptionalContent) {
   // Delegate to original implementation for backward compatibility
   return C_OscParamSetInterpretedNodeFiler::h_LoadInterpretedNode(orc_Node, orc_XmlParser, orq_MissingOptionalContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save interpreted node (deprecated)

   \param[in]      orc_Node              Node data
   \param[in]      orc_XmlParser         XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedNodeFiler_New::h_SaveInterpretedNode(const C_OscParamSetInterpretedNode &orc_Node,
                                                                   C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   C_OscParamSetInterpretedNodeFiler::h_SaveInterpretedNode(orc_Node, orc_XmlParser);
}
