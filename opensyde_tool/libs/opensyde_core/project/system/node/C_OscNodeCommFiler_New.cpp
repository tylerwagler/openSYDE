//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for comm description files (Multi-Format Implementation)

   Load / save CAN communication protocol data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscNodeCommFiler_New.hpp"
#include "C_OscNodeCommFiler.hpp"
#include "C_OscSystemFilerUtil.hpp"
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
/*! \brief   Load protocol from file (auto-detect format)

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_FilePath         File path
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadProtocolFile(C_OscCanProtocol &orc_Protocol,
                                                    const QString &orc_FilePath,
                                                    const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   return mh_DetectAndLoad(orc_Protocol, orc_FilePath, orc_NodeDataPools);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to file (auto-detect format from extension)

   \param[in]      orc_Protocol         Protocol data to store
   \param[in]      orc_FilePath         File path
   \param[in]      orc_DatapoolName     Data pool name

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_SaveProtocolFile(const C_OscCanProtocol &orc_Protocol,
                                                    const QString &orc_FilePath,
                                                    const QString &orc_DatapoolName) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Protocol, orc_FilePath, orc_DatapoolName);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Protocol, orc_FilePath, orc_DatapoolName);
   } else if (c_Extension == ".xml" || c_Extension == ".dbc") {
      s32_Retval = h_SaveXml(orc_Protocol, orc_FilePath, orc_DatapoolName);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_Protocol, orc_FilePath, orc_DatapoolName);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from binary file

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_FilePath         File path
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadBinary(C_OscCanProtocol &orc_Protocol,
                                              const QString &orc_FilePath,
                                              const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading protocol definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_FilePath));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   orc_Protocol.FromQDataStream(in);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to binary file

   \param[in]      orc_Protocol         Protocol data to store
   \param[in]      orc_FilePath         File path
   \param[in]      orc_DatapoolName     Data pool name

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_SaveBinary(const C_OscCanProtocol &orc_Protocol,
                                              const QString &orc_FilePath,
                                              const QString &orc_DatapoolName) {
   Q_UNUSED(orc_DatapoolName);
   
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving protocol definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_FilePath));
      return C_RD_WR;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);

   orc_Protocol.ToQDataStream(out);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from memory (binary)

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_Data             Serialized data
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   data format is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadFromMemoryBinary(C_OscCanProtocol &orc_Protocol,
                                                        const QByteArray &orc_Data,
                                                        const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   orc_Protocol.FromQDataStream(in);

   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to memory (binary)

   \param[in]      orc_Protocol         Protocol data
   \param[in]      orc_DatapoolName     Data pool name

   \return
   QByteArray containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscNodeCommFiler_New::h_SaveToMemoryBinary(const C_OscCanProtocol &orc_Protocol,
                                                         const QString &orc_DatapoolName) const {
   Q_UNUSED(orc_DatapoolName);
   
   QByteArray data;
   QDataStream out(&data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);
   const_cast<C_OscCanProtocol&>(orc_Protocol).ToQDataStream(out);
   return data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from JSON file

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_FilePath         File path
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   JSON parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadJson(C_OscCanProtocol &orc_Protocol,
                                            const QString &orc_FilePath,
                                            const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading protocol definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_FilePath));
      return C_CONFIG;
   }

   QJsonParseError parseError;
   QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
   file.close();

   if (parseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading protocol definition",
                          QString("JSON parse error: %1").arg(parseError.errorString()));
      return C_CONFIG;
   }

   orc_Protocol.FromJsonObject(doc.object());
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to JSON file

   \param[in]      orc_Protocol         Protocol data to store
   \param[in]      orc_FilePath         File path
   \param[in]      orc_DatapoolName     Data pool name

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_SaveJson(const C_OscCanProtocol &orc_Protocol,
                                            const QString &orc_FilePath,
                                            const QString &orc_DatapoolName) {
   Q_UNUSED(orc_DatapoolName);
   
   QJsonObject json = const_cast<C_OscCanProtocol&>(orc_Protocol).ToJsonObject();
   QJsonDocument doc(json);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving protocol definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from memory (JSON)

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_Object           JSON object
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadFromMemoryJson(C_OscCanProtocol &orc_Protocol,
                                                      const QJsonObject &orc_Object,
                                                      const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   orc_Protocol.FromJsonObject(orc_Object);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to memory (JSON)

   \param[in]      orc_Protocol         Protocol data
   \param[in]      orc_DatapoolName     Data pool name

   \return
   QJsonObject containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeCommFiler_New::h_SaveToMemoryJson(const C_OscCanProtocol &orc_Protocol,
                                                        const QString &orc_DatapoolName) const {
   Q_UNUSED(orc_DatapoolName);
   
   return const_cast<C_OscCanProtocol&>(orc_Protocol).ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from XML file

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_FilePath         File path
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   XML parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadXml(C_OscCanProtocol &orc_Protocol,
                                           const QString &orc_FilePath,
                                           const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading protocol definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_FilePath));
      return C_CONFIG;
   }

   QDomDocument doc;
   QString errorMessage;
   int errorLine, errorColumn;

   if (!doc.setContent(file.readAll(), &errorMessage, &errorLine, &errorColumn)) {
      osc_write_log_error("Loading protocol definition",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(errorLine).arg(errorColumn).arg(errorMessage));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement rootElement = doc.documentElement();
   orc_Protocol.FromQDomDocument(rootElement);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to XML file

   \param[in]      orc_Protocol         Protocol data to store
   \param[in]      orc_FilePath         File path
   \param[in]      orc_DatapoolName     Data pool name

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_SaveXml(const C_OscCanProtocol &orc_Protocol,
                                           const QString &orc_FilePath,
                                           const QString &orc_DatapoolName) {
   QDomDocument doc;
   QDomElement rootElement = const_cast<C_OscCanProtocol&>(orc_Protocol).ToQDomDocument(doc, "protocol");
   doc.appendChild(rootElement);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving protocol definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toString(2).toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load protocol from memory (XML)

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_Element          XML element
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadFromMemoryXml(C_OscCanProtocol &orc_Protocol,
                                                     const QDomElement &orc_Element,
                                                     const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   Q_UNUSED(orc_NodeDataPools);
   
   orc_Protocol.FromQDomDocument(orc_Element);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save protocol to memory (XML)

   \param[in]      orc_Protocol         Protocol data
   \param[in]      ro_Doc               DOM document
   \param[in]      orc_DatapoolName     Data pool name

   \return
   QDomElement containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeCommFiler_New::h_SaveToMemoryXml(const C_OscCanProtocol &orc_Protocol,
                                                       QDomDocument &ro_Doc,
                                                       const QString &orc_DatapoolName) const {
   Q_UNUSED(orc_DatapoolName);
   
   return const_cast<C_OscCanProtocol&>(orc_Protocol).ToQDomDocument(ro_Doc, "protocol");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper for format detection and loading

   \param[out]     orc_Protocol         Protocol data
   \param[in]      orc_FilePath         File path
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   file format not supported or parse error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::mh_DetectAndLoad(C_OscCanProtocol &orc_Protocol,
                                                  const QString &orc_FilePath,
                                                  const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_Protocol, orc_FilePath, orc_NodeDataPools);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_Protocol, orc_FilePath, orc_NodeDataPools);
   } else if (c_Extension == ".xml" || c_Extension == ".dbc") {
      return h_LoadXml(orc_Protocol, orc_FilePath, orc_NodeDataPools);
   } else {
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Attempting XML format as fallback.")
                            .arg(c_Extension, orc_FilePath));
      return h_LoadXml(orc_Protocol, orc_FilePath, orc_NodeDataPools);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load node communication protocol (deprecated)

   \param[out]     orc_NodeComProtocol  Protocol data
   \param[in]      orc_XmlParser        XML parser
   \param[in]      orc_NodeDataPools    Data pools for reference

   \return
   C_NO_ERR   data loaded
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeCommFiler_New::h_LoadNodeComProtocol(C_OscCanProtocol &orc_NodeComProtocol,
                                                       C_OscXmlParserBase &orc_XmlParser,
                                                       const QList<C_OscNodeDataPool> &orc_NodeDataPools) {
   // Delegate to original implementation for backward compatibility
   return C_OscNodeCommFiler::h_LoadNodeComProtocol(orc_NodeComProtocol, orc_XmlParser, orc_NodeDataPools);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save node communication protocol (deprecated)

   \param[in]      orc_NodeComProtocol  Protocol data
   \param[in]      orc_XmlParser        XML parser
   \param[in]      orc_DatapoolName     Data pool name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCommFiler_New::h_SaveNodeComProtocol(const C_OscCanProtocol &orc_NodeComProtocol,
                                                    C_OscXmlParserBase &orc_XmlParser,
                                                    const QString &orc_DatapoolName) {
   // Delegate to original implementation for backward compatibility
   C_OscNodeCommFiler::h_SaveNodeComProtocol(orc_NodeComProtocol, orc_XmlParser, orc_DatapoolName);
}
