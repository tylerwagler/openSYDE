//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Implementation of Qt-native serialization framework
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscSerializer.hpp"
#include "C_OscLoggingHandler.hpp"
#include "precomp_headers.hpp"
#include "stwerrors.hpp"

using namespace stw::errors;

namespace stw {
namespace opensyde_core {

//----------------------------------------------------------------------------------------------------------------------
// Binary Serialization Implementation
//----------------------------------------------------------------------------------------------------------------------

int32_t C_OscSerializer::SaveBinary(const QByteArray& orc_Data, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Binary serialization",
                         QString("Failed to open file for writing: %1").arg(orc_Path));
      return C_RD_WR;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);
   out.setByteOrder(QDataStream::LittleEndian);

   // Write data size header for validation
   out << static_cast<qint64>(orc_Data.size());
   out << orc_Data;

   if (out.status() != QDataStream::Ok) {
      osc_write_log_error("Binary serialization",
                         QString("Failed to write binary data to: %1").arg(orc_Path));
      file.close();
      return C_RD_WR;
   }

   file.close();
   osc_write_log_info("Binary serialization",
                     QString("Successfully saved %1 bytes to: %2")
                         .arg(orc_Data.size())
                         .arg(orc_Path));

   return C_NO_ERR;
}

int32_t C_OscSerializer::LoadBinary(QByteArray& orc_Data, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Binary serialization",
                         QString("Failed to open file for reading: %1").arg(orc_Path));
      return C_RD_WR;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);
   in.setByteOrder(QDataStream::LittleEndian);

   // Read and validate size header
   qint64 expectedSize = 0;
   in >> expectedSize;

   if (in.status() != QDataStream::Ok) {
      osc_write_log_error("Binary serialization",
                         "Failed to read binary data size header");
      file.close();
      return C_CONFIG;
   }

   // Sanity check on size
   if (expectedSize < 0 || expectedSize > 100 * 1024 * 1024) { // 100MB limit
      osc_write_log_error("Binary serialization",
                         QString("Invalid data size in file: %1 bytes").arg(expectedSize));
      file.close();
      return C_CONFIG;
   }

   // Read actual data
   orc_Data.resize(static_cast<int>(expectedSize));
   in >> orc_Data;

   if (in.status() != QDataStream::Ok) {
      osc_write_log_error("Binary serialization",
                         QString("Failed to read binary data from: %1").arg(orc_Path));
      file.close();
      return C_CONFIG;
   }

   // Validate size match
   if (orc_Data.size() != expectedSize) {
      osc_write_log_warning("Binary serialization",
                           QString("Size mismatch: expected %1 bytes, read %2 bytes")
                               .arg(expectedSize)
                               .arg(orc_Data.size()));
   }

   file.close();
   osc_write_log_info("Binary serialization",
                     QString("Successfully loaded %1 bytes from: %2")
                         .arg(orc_Data.size())
                         .arg(orc_Path));

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
// JSON Serialization Implementation
//----------------------------------------------------------------------------------------------------------------------

int32_t C_OscSerializer::SaveJson(const QJsonObject& orc_Object, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      osc_write_log_error("JSON serialization",
                         QString("Failed to open file for writing: %1").arg(orc_Path));
      return C_RD_WR;
   }

   QJsonDocument doc(orc_Object);
   QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

   qint64 written = file.write(jsonData);
   if (written == -1) {
      osc_write_log_error("JSON serialization",
                         QString("Failed to write JSON data to: %1").arg(orc_Path));
      file.close();
      return C_RD_WR;
   }

   file.close();
   osc_write_log_info("JSON serialization",
                     QString("Successfully saved JSON (%1 bytes) to: %2")
                         .arg(written)
                         .arg(orc_Path));

   return C_NO_ERR;
}

int32_t C_OscSerializer::LoadJson(QJsonObject& orc_Object, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      osc_write_log_error("JSON serialization",
                         QString("Failed to open file for reading: %1").arg(orc_Path));
      return C_RD_WR;
   }

   QByteArray jsonData = file.readAll();
   file.close();

   if (jsonData.isEmpty()) {
      osc_write_log_error("JSON serialization",
                         QString("Empty JSON file: %1").arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError parseError;
   QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

   if (parseError.error != QJsonParseError::NoError) {
      osc_write_log_error("JSON serialization",
                         QString("JSON parse error in %1 at offset %2: %3")
                             .arg(orc_Path)
                             .arg(parseError.offset)
                             .arg(parseError.errorString()));
      return C_CONFIG;
   }

   if (!doc.isObject()) {
      osc_write_log_error("JSON serialization",
                         QString("JSON root is not an object in file: %1").arg(orc_Path));
      return C_CONFIG;
   }

   orc_Object = doc.object();
   osc_write_log_info("JSON serialization",
                     QString("Successfully loaded JSON (%1 bytes) from: %2")
                         .arg(jsonData.size())
                         .arg(orc_Path));

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
// XML Serialization Implementation
//----------------------------------------------------------------------------------------------------------------------

int32_t C_OscSerializer::SaveXml(const QDomDocument& orc_Doc, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      osc_write_log_error("XML serialization",
                         QString("Failed to open file for writing: %1").arg(orc_Path));
      return C_RD_WR;
   }

   // Write document with formatting
   QString xmlContent = orc_Doc.toString(2); // 2-space indentation

   qint64 written = file.write(xmlContent.toUtf8());
   if (written == -1) {
      osc_write_log_error("XML serialization",
                         QString("Failed to write XML data to: %1").arg(orc_Path));
      file.close();
      return C_RD_WR;
   }

   file.close();
   osc_write_log_info("XML serialization",
                     QString("Successfully saved XML (%1 bytes) to: %2")
                         .arg(written)
                         .arg(orc_Path));

   return C_NO_ERR;
}

int32_t C_OscSerializer::LoadXml(QDomDocument& orc_Doc, const QString& orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      osc_write_log_error("XML serialization",
                         QString("Failed to open file for reading: %1").arg(orc_Path));
      return C_RD_WR;
   }

   QByteArray xmlData = file.readAll();
   file.close();

   if (xmlData.isEmpty()) {
      osc_write_log_error("XML serialization",
                         QString("Empty XML file: %1").arg(orc_Path));
      return C_CONFIG;
   }

   QString errorMessage;
   int errorLine, errorColumn;

   if (!orc_Doc.setContent(xmlData, &errorMessage, &errorLine, &errorColumn)) {
      osc_write_log_error("XML serialization",
                         QString("XML parse error in %1 at line %2, column %3: %4")
                             .arg(orc_Path)
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorMessage));
      return C_CONFIG;
   }

   osc_write_log_info("XML serialization",
                     QString("Successfully loaded XML (%1 bytes) from: %2")
                         .arg(xmlData.size())
                         .arg(orc_Path));

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
// Format Detection Implementation
//----------------------------------------------------------------------------------------------------------------------

E_SerializationFormat C_OscSerializer::DetectFormat(const QString& orc_Path) {
   QString lowerPath = orc_Path.toLower();

   if (lowerPath.endsWith(".bin")) {
      return E_SerializationFormat::e_BINARY;
   } else if (lowerPath.endsWith(".json")) {
      return E_SerializationFormat::e_JSON;
   } else if (lowerPath.endsWith(".xml") ||
              lowerPath.endsWith(".syde") ||
              lowerPath.endsWith(".ods") ||
              lowerPath.endsWith(".xco") ||
              lowerPath.endsWith(".xce")) {
      // Default to XML for backward compatibility with existing formats
      return E_SerializationFormat::e_XML;
   }

   // Default to XML for unknown extensions
   return E_SerializationFormat::e_XML;
}

QString C_OscSerializer::GetFileExtension(E_SerializationFormat oq_Format) {
   switch (oq_Format) {
      case E_SerializationFormat::e_BINARY:
         return ".bin";
      case E_SerializationFormat::e_JSON:
         return ".json";
      case E_SerializationFormat::e_XML:
      default:
         return ".xml";
   }
}

//----------------------------------------------------------------------------------------------------------------------
// Private Helper Methods
//----------------------------------------------------------------------------------------------------------------------

int32_t C_OscSerializer::mh_SaveToStream(const QByteArray& orc_Data, QFile& orc_File) {
   QDataStream out(&orc_File);
   out.setVersion(QDataStream::Qt_6_0);
   out << orc_Data;
   return (out.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
}

int32_t C_OscSerializer::mh_LoadFromStream(QByteArray& orc_Data, QFile& orc_File) {
   QDataStream in(&orc_File);
   in.setVersion(QDataStream::Qt_6_0);
   in >> orc_Data;
   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

} // namespace opensyde_core
} // namespace stw
