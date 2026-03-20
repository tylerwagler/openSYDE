//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for device definition files (Multi-Format Implementation)

   Load / save device definition data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
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
/*! \brief   Load device definition from file (auto-detect format)

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadDeviceDefinitionFile(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                                    const QString &orc_FilePath) {
   return mh_DetectAndLoad(orc_DeviceDefinition, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to file (auto-detect format from extension)

   \param[in]      orc_DeviceDefinition    Device definition data to store
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_SaveDeviceDefinitionFile(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                                    const QString &orc_FilePath) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_DeviceDefinition, orc_FilePath);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_DeviceDefinition, orc_FilePath);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_DeviceDefinition, orc_FilePath);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                              .arg(c_Extension, orc_FilePath));
      s32_Retval = h_SaveXml(orc_DeviceDefinition, orc_FilePath);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from binary file

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadBinary(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                      const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading device definition",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   orc_DeviceDefinition.FromQDataStream(in);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to binary file

   \param[in]      orc_DeviceDefinition    Device definition data to store
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_SaveBinary(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                      const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving device definition",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);

   const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToQDataStream(out);
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from memory (binary)

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_Data                Serialized data

   \return
   C_NO_ERR   data loaded
   C_CONFIG   data format is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadFromMemoryBinary(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                                const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   orc_DeviceDefinition.FromQDataStream(in);

   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to memory (binary)

   \param[in]      orc_DeviceDefinition    Device definition data

   \return
   QByteArray containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscDeviceDefinitionFiler_New::h_SaveToMemoryBinary(const C_OscDeviceDefinition &orc_DeviceDefinition) const {
   QByteArray data;
   QDataStream out(&data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);
   const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToQDataStream(out);
   return data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from JSON file

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   JSON parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadJson(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                    const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading device definition",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QJsonParseError parseError;
   QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
   file.close();

   if (parseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading device definition",
                          QString("JSON parse error: %1").arg(parseError.errorString()));
      return C_CONFIG;
   }

   orc_DeviceDefinition.FromJsonObject(doc.object());
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to JSON file

   \param[in]      orc_DeviceDefinition    Device definition data to store
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_SaveJson(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                    const QString &orc_FilePath) {
   QJsonObject json = const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToJsonObject();
   QJsonDocument doc(json);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving device definition",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from memory (JSON)

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_Object              JSON object

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadFromMemoryJson(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                              const QJsonObject &orc_Object) {
   orc_DeviceDefinition.FromJsonObject(orc_Object);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to memory (JSON)

   \param[in]      orc_DeviceDefinition    Device definition data

   \return
   QJsonObject containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscDeviceDefinitionFiler_New::h_SaveToMemoryJson(const C_OscDeviceDefinition &orc_DeviceDefinition) const {
   return const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from XML file

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   XML parse error or invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadXml(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                   const QString &orc_FilePath) {
   QFile file(orc_FilePath);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading device definition",
                          QString("Could not open file \"%1\" for reading.").arg(orc_FilePath));
      return C_CONFIG;
   }

   QDomDocument doc;
   QString errorMessage;
   int errorLine, errorColumn;

   if (!doc.setContent(file.readAll(), &errorMessage, &errorLine, &errorColumn)) {
      osc_write_log_error("Loading device definition",
                          QString("XML parse error at line %1, column %2: %3")
                            .arg(errorLine)
                            .arg(errorColumn)
                            .arg(errorMessage));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement rootElement = doc.documentElement();
   orc_DeviceDefinition.FromQDomDocument(rootElement);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to XML file

   \param[in]      orc_DeviceDefinition    Device definition data to store
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_SaveXml(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                   const QString &orc_FilePath) {
   QDomDocument doc;
   QDomElement rootElement = const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToQDomDocument(doc, "device-definition");
   doc.appendChild(rootElement);

   QFile file(orc_FilePath);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving device definition",
                          QString("Could not open file \"%1\" for writing.").arg(orc_FilePath));
      return C_RD_WR;
   }

   file.write(doc.toString(2).toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load device definition from memory (XML)

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_Element             XML element

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_LoadFromMemoryXml(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                             const QDomElement &orc_Element) {
   orc_DeviceDefinition.FromQDomDocument(orc_Element);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save device definition to memory (XML)

   \param[in]      orc_DeviceDefinition    Device definition data
   \param[in]      ro_Doc                  DOM document

   \return
   QDomElement containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscDeviceDefinitionFiler_New::h_SaveToMemoryXml(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                               QDomDocument &ro_Doc) const {
   return const_cast<C_OscDeviceDefinition &>(orc_DeviceDefinition).ToQDomDocument(ro_Doc, "device-definition");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper for format detection and loading

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_FilePath            File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   file format not supported or parse error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::mh_DetectAndLoad(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                          const QString &orc_FilePath) {
   const QString c_Extension = orc_FilePath.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_DeviceDefinition, orc_FilePath);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_DeviceDefinition, orc_FilePath);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_DeviceDefinition, orc_FilePath);
   } else {
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Attempting XML format as fallback.")
                              .arg(c_Extension, orc_FilePath));
      return h_LoadXml(orc_DeviceDefinition, orc_FilePath);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load device definition (deprecated)

   \param[out]     orc_DeviceDefinition    Device definition data
   \param[in]      orc_Path                File path

   \return
   C_NO_ERR   data loaded
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_Load(C_OscDeviceDefinition &orc_DeviceDefinition,
                                                const QString &orc_Path) {
   // Delegate to original implementation for backward compatibility
   return C_OscDeviceDefinitionFiler::h_Load(orc_DeviceDefinition, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save device definition (deprecated)

   \param[in]      orc_DeviceDefinition    Device definition data
   \param[in]      orc_Path                File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceDefinitionFiler_New::h_Save(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                                const QString &orc_Path) {
   // Delegate to original implementation for backward compatibility
   return C_OscDeviceDefinitionFiler::h_Save(orc_DeviceDefinition, orc_Path);
}
