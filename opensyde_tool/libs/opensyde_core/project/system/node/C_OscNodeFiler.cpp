//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node reader/writer (Multi-Format Implementation)

   Load / save node data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscNodeFiler.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"

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
/*! \brief   Load node from file (auto-detect format)

   \param[out]     orc_Node         Node data
   \param[in]      orc_FilePath     File path
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadNodeFile(C_OscNode &orc_Node,
                                           const QString &orc_FilePath,
                                           const bool oq_SkipContent) {
   return mh_DetectAndLoad(orc_Node, orc_FilePath, oq_SkipContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to file (auto-detect format from extension)

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_FilePath                     File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveNodeFile(
   const C_OscNode &orc_Node, const QString &orc_FilePath,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Node, orc_FilePath, opc_CreatedFiles,
                                orc_NodeIndicesToNameMap);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Node, orc_FilePath, opc_CreatedFiles,
                              orc_NodeIndicesToNameMap);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_Node, orc_FilePath, opc_CreatedFiles,
                             orc_NodeIndicesToNameMap);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_Node, orc_FilePath, opc_CreatedFiles,
                             orc_NodeIndicesToNameMap);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from binary file

   \param[out]     orc_Node         Node data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadBinary(C_OscNode &orc_Node,
                                         const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading node definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Node.FromQDataStream(in);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to binary file

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_Path                         File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveBinary(
   const C_OscNode &orc_Node, const QString &orc_Path,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving node definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Node.ToQDataStream(out);
   file.close();

   if (s32_Result == C_NO_ERR) {
      // Track created file if requested
      if (opc_CreatedFiles != nullptr) {
         *opc_CreatedFiles << orc_Path;
      }
   } else {
      osc_write_log_error("Saving node definition",
                          "Failed to serialize node data to binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from memory (binary)

   \param[out]     orc_Node         Node data
   \param[in]      orc_Data         Binary data

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadFromMemoryBinary(C_OscNode &orc_Node,
                                                   const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Node.FromQDataStream(in);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from memory buffer.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to memory (binary)

   \param[in]      orc_Node         Node data to store

   \return
   QByteArray     Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscNodeFiler::h_SaveToMemoryBinary(const C_OscNode &orc_Node) {
   QByteArray c_Data;
   QDataStream out(&c_Data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);

   orc_Node.ToQDataStream(out);

   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from JSON file

   \param[out]     orc_Node         Node data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadJson(C_OscNode &orc_Node,
                                       const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading node definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(file.readAll(), &c_ParseError);
   file.close();

   if (c_ParseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading node definition",
                          QString("JSON parse error: %1").arg(c_ParseError.errorString()));
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      osc_write_log_error("Loading node definition",
                          "JSON root element is not an object.");
      return C_CONFIG;
   }

   int32_t s32_Result = orc_Node.FromJsonObject(c_Doc.object());

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from JSON file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to JSON file

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_Path                         File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveJson(
   const C_OscNode &orc_Node, const QString &orc_Path,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving node definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonObject c_Json = orc_Node.ToJsonObject();
   QJsonDocument c_Doc(c_Json);
   file.write(c_Doc.toJson(QJsonDocument::Indented));
   file.close();

   // Track created file if requested
   if (opc_CreatedFiles != nullptr) {
      *opc_CreatedFiles << orc_Path;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from memory (JSON)

   \param[out]     orc_Node         Node data
   \param[in]      orc_Object       JSON object

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadFromMemoryJson(C_OscNode &orc_Node,
                                                 const QJsonObject &orc_Object) {
   int32_t s32_Result = orc_Node.FromJsonObject(orc_Object);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from JSON object.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to memory (JSON)

   \param[in]      orc_Node         Node data to store

   \return
   QJsonObject    JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeFiler::h_SaveToMemoryJson(const C_OscNode &orc_Node) {
   return orc_Node.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from XML file

   \param[out]     orc_Node         Node data
   \param[in]      orc_Path         File path
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadXml(C_OscNode &orc_Node,
                                      const QString &orc_Path,
                                      const bool oq_SkipContent) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading node definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QString c_Error;
   int i_Line, i_Column;

   if (!c_Doc.setContent(file.readAll(), &c_Error, &i_Line, &i_Column)) {
      osc_write_log_error("Loading node definition",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(i_Line).arg(i_Column).arg(c_Error));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement c_Root = c_Doc.documentElement();
   int32_t s32_Result = orc_Node.FromQDomElement(c_Root);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from XML file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to XML file

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_Path                         File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveXml(
   const C_OscNode &orc_Node, const QString &orc_Path,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving node definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QDomElement c_Element = orc_Node.ToQDomDocument(c_Doc, "node");
   c_Doc.appendChild(c_Element);

   // Write with XML declaration
   file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   file.write(c_Doc.toString().toUtf8());
   file.close();

   // Track created file if requested
   if (opc_CreatedFiles != nullptr) {
      *opc_CreatedFiles << orc_Path;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node from memory (XML)

   \param[out]     orc_Node         Node data
   \param[in]      orc_Element      XML element
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadFromMemoryXml(C_OscNode &orc_Node,
                                                const QDomElement &orc_Element,
                                                const bool oq_SkipContent) {
   int32_t s32_Result = orc_Node.FromQDomElement(orc_Element);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading node definition",
                          "Failed to deserialize node data from XML element.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node to memory (XML)

   \param[in]      orc_Node         Node data to store
   \param[in,out]  orc_Doc          DOM document

   \return
   QDomElement    XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeFiler::h_SaveToMemoryXml(const C_OscNode &orc_Node,
                                                  QDomDocument &orc_Doc) {
   return orc_Node.ToQDomDocument(orc_Doc, "node");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node (legacy XML compatibility)

   \param[out]     orc_Node         Node data
   \param[in]      orc_FilePath     File path
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadNodeFile_Legacy(C_OscNode &orc_Node,
                                                  const QString &orc_FilePath,
                                                  const bool oq_SkipContent) {
   // Delegate to original implementation for backward compatibility
   return C_OscNodeFiler::h_LoadNodeFile(orc_Node, orc_FilePath, oq_SkipContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node (legacy XML compatibility)

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_FilePath                     File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveNodeFile_Legacy(
   const C_OscNode &orc_Node, const QString &orc_FilePath,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   
   // Delegate to original implementation for backward compatibility
   return C_OscNodeFiler::h_SaveNodeFile(orc_Node, orc_FilePath, opc_CreatedFiles,
                                             orc_NodeIndicesToNameMap);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node (legacy XML compatibility)

   \param[out]     orc_Node         Node data
   \param[in]      orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_LoadNode_Legacy(C_OscNode &orc_Node,
                                              C_OscXmlParserBase &orc_XmlParser,
                                              const QString &orc_BasePath,
                                              const bool oq_SkipContent) {
   Q_UNUSED(orc_Node)
   Q_UNUSED(orc_XmlParser)
   Q_UNUSED(orc_BasePath)
   Q_UNUSED(oq_SkipContent)
   // Legacy XML parser-based API is no longer supported; use h_LoadXml instead
   osc_write_log_error("Loading node definition",
                       "h_LoadNode_Legacy is no longer supported. Use h_LoadXml instead.");
   return C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node (legacy XML compatibility)

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_XmlParser                    XML parser
   \param[in]      orc_BasePath                     Base path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::h_SaveNode_Legacy(
   const C_OscNode &orc_Node, C_OscXmlParserBase &orc_XmlParser,
   const QString &orc_BasePath, QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap) {
   Q_UNUSED(orc_Node)
   Q_UNUSED(orc_XmlParser)
   Q_UNUSED(orc_BasePath)
   Q_UNUSED(opc_CreatedFiles)
   Q_UNUSED(orc_NodeIndicesToNameMap)
   // Legacy XML parser-based API is no longer supported; use h_SaveXml instead
   osc_write_log_error("Saving node definition",
                       "h_SaveNode_Legacy is no longer supported. Use h_SaveXml instead.");
   return C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get folder name for node

   \param[in]      orc_NodeName     Node name

   \return
   QString        Folder name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscNodeFiler::h_GetFolderName(const QString &orc_NodeName) {
   return "node_" +
          C_OscSystemFilerUtil::h_PrepareItemNameForFileName(orc_NodeName);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get file name for node

   \return
   QString        File name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscNodeFiler::h_GetFileName(void) {
   return "node_core.xml";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect format and load from file

   \param[out]     orc_Node         Node data
   \param[in]      orc_Path         File path
   \param[in]      oq_SkipContent   Skip content (datapools, halc, etc.)

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::mh_DetectAndLoad(C_OscNode &orc_Node,
                                             const QString &orc_Path,
                                             const bool oq_SkipContent) {
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_Node, orc_Path);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_Node, orc_Path);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_Node, orc_Path, oq_SkipContent);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      return h_LoadXml(orc_Node, orc_Path, oq_SkipContent);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node with format specification

   \param[in]      orc_Node                         Node data to store
   \param[in]      orc_Path                         File path
   \param[in,out]  opc_CreatedFiles                 Created files list
   \param[in]      orc_NodeIndicesToNameMap         Node indices to name map
   \param[in]      orc_Format                       Format string ("binary", "json", "xml")

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeFiler::mh_SaveNodeInternal(
   const C_OscNode &orc_Node, const QString &orc_Path,
   QStringList *const opc_CreatedFiles,
   const QHash<uint32_t, QString> &orc_NodeIndicesToNameMap,
   const QString &orc_Format) {
   
   if (orc_Format == "binary") {
      return h_SaveBinary(orc_Node, orc_Path, opc_CreatedFiles,
                          orc_NodeIndicesToNameMap);
   } else if (orc_Format == "json") {
      return h_SaveJson(orc_Node, orc_Path, opc_CreatedFiles,
                        orc_NodeIndicesToNameMap);
   } else if (orc_Format == "xml") {
      return h_SaveXml(orc_Node, orc_Path, opc_CreatedFiles,
                       orc_NodeIndicesToNameMap);
   } else {
      osc_write_log_error("Saving node definition",
                          QString("Unknown format \"%1\" for \"%2\".")
                          .arg(orc_Format, orc_Path));
      return C_CONFIG;
   }
}
