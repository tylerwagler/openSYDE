//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for data pool files (Multi-Format Implementation)

   Load / save data pool data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscNodeDataPoolFiler.hpp"
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
/*! \brief   Load data pool from file (auto-detect format)

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadDataPoolFile(C_OscNodeDataPool &orc_DataPool,
                                                        const QString &orc_FilePath) {
   return mh_DetectAndLoad(orc_DataPool, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to file (auto-detect format from extension)

   \param[in]      orc_DataPool         Data pool data to store
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_SaveDataPoolFile(const C_OscNodeDataPool &orc_DataPool,
                                                        const QString &orc_FilePath) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_DataPool, orc_FilePath);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_DataPool, orc_FilePath);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_DataPool, orc_FilePath);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                              .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_DataPool, orc_FilePath);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from binary file

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadBinary(C_OscNodeDataPool &orc_DataPool,
                                                  const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data pool",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   orc_DataPool.FromQDataStream(in);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to binary file

   \param[in]      orc_DataPool         Data pool data to store
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_SaveBinary(const C_OscNodeDataPool &orc_DataPool,
                                                  const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data pool",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);

   const_cast<C_OscNodeDataPool &>(orc_DataPool).ToQDataStream(out);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from memory (binary)

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_Data             Serialized data

   \return
   C_NO_ERR   data loaded
   C_CONFIG   data format is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadFromMemoryBinary(C_OscNodeDataPool &orc_DataPool,
                                                            const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   orc_DataPool.FromQDataStream(in);

   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to memory (binary)

   \param[in]      orc_DataPool         Data pool data

   \return
   QByteArray containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscNodeDataPoolFiler::h_SaveToMemoryBinary(const C_OscNodeDataPool &orc_DataPool) const {
   QByteArray data;
   QDataStream out(&data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);
   const_cast<C_OscNodeDataPool &>(orc_DataPool).ToQDataStream(out);
   return data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from JSON file

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   JSON parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadJson(C_OscNodeDataPool &orc_DataPool,
                                                const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data pool",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QJsonParseError parseError;
   QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
   file.close();

   if (parseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading data pool",
                          QString("JSON parse error: %1").arg(parseError.errorString()));
      return C_CONFIG;
   }

   orc_DataPool.FromJsonObject(doc.object());
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to JSON file

   \param[in]      orc_DataPool         Data pool data to store
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_SaveJson(const C_OscNodeDataPool &orc_DataPool,
                                                const QString &orc_FilePath) {
   QJsonObject json = const_cast<C_OscNodeDataPool &>(orc_DataPool).ToJsonObject();
   QJsonDocument doc(json);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data pool",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from memory (JSON)

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_Object           JSON object

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadFromMemoryJson(C_OscNodeDataPool &orc_DataPool,
                                                          const QJsonObject &orc_Object) {
   orc_DataPool.FromJsonObject(orc_Object);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to memory (JSON)

   \param[in]      orc_DataPool         Data pool data

   \return
   QJsonObject containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolFiler::h_SaveToMemoryJson(const C_OscNodeDataPool &orc_DataPool) const {
   return const_cast<C_OscNodeDataPool &>(orc_DataPool).ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from XML file

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   XML parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadXml(C_OscNodeDataPool &orc_DataPool,
                                               const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading data pool",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDomDocument doc;
   QString errorMessage;
   int errorLine, errorColumn;

   if (!doc.setContent(file.readAll(), &errorMessage, &errorLine, &errorColumn)) {
      osc_write_log_error("Loading data pool",
                          QString("XML parse error at line %1, column %2: %3")
                            .arg(errorLine)
                            .arg(errorColumn)
                            .arg(errorMessage));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement rootElement = doc.documentElement();
   orc_DataPool.FromQDomDocument(rootElement);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to XML file

   \param[in]      orc_DataPool         Data pool data to store
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_SaveXml(const C_OscNodeDataPool &orc_DataPool,
                                               const QString &orc_FilePath) {
   QDomDocument doc;
   QDomElement rootElement = const_cast<C_OscNodeDataPool &>(orc_DataPool).ToQDomDocument(doc, "data-pool");
   doc.appendChild(rootElement);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving data pool",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toString(2).toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool from memory (XML)

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_Element          XML element

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadFromMemoryXml(C_OscNodeDataPool &orc_DataPool,
                                                         const QDomElement &orc_Element) {
   orc_DataPool.FromQDomDocument(orc_Element);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool to memory (XML)

   \param[in]      orc_DataPool         Data pool data
   \param[in]      ro_Doc               DOM document

   \return
   QDomElement containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeDataPoolFiler::h_SaveToMemoryXml(const C_OscNodeDataPool &orc_DataPool,
                                                           QDomDocument &ro_Doc) const {
   return const_cast<C_OscNodeDataPool &>(orc_DataPool).ToQDomDocument(ro_Doc, "data-pool");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper for format detection and loading

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_FilePath         File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   file format not supported or parse error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::mh_DetectAndLoad(C_OscNodeDataPool &orc_DataPool,
                                                      const QString &orc_FilePath) {
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_DataPool, orc_FilePath);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_DataPool, orc_FilePath);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_DataPool, orc_FilePath);
   } else {
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Attempting XML format as fallback.")
                              .arg(c_Extension, orc_FilePath));
      return h_LoadXml(orc_DataPool, orc_FilePath);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load data pool (deprecated)

   \param[out]     orc_DataPool         Data pool data
   \param[in]      orc_XmlParser        XML parser

   \return
   C_NO_ERR   data loaded
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolFiler::h_LoadDataPool(C_OscNodeDataPool &orc_DataPool,
                                                    C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   return C_OscNodeDataPoolFiler::h_LoadDataPool(orc_DataPool, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save data pool (deprecated)

   \param[in]      orc_DataPool         Data pool data
   \param[in]      orc_XmlParser        XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPoolFiler::h_SaveDataPool(const C_OscNodeDataPool &orc_DataPool,
                                                 C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   C_OscNodeDataPoolFiler::h_SaveDataPool(orc_DataPool, orc_XmlParser);
}
