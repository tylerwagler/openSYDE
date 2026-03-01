//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Qt-native serialization framework supporting multiple formats
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSERIALIZER_HPP
#define C_OSCSERIALIZER_HPP

#include "stwtypes.hpp"
#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QDomDocument>

namespace stw {
namespace opensyde_core {

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Enumeration of supported serialization formats
*/
//----------------------------------------------------------------------------------------------------------------------
enum class E_SerializationFormat {
   e_BINARY,    ///< QDataStream - fast, compact, binary format
   e_JSON,      ///< QJsonDocument - human-readable JSON format
   e_XML        ///< QDomDocument - legacy XML format for compatibility
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Qt-native serialization framework supporting multiple formats

   This class provides a unified interface for serializing and deserializing
   data in multiple formats:
   - Binary (QDataStream): Fast, compact, ideal for internal caches
   - JSON (QJsonDocument): Human-readable, suitable for configuration files
   - XML (QDomDocument): Legacy compatibility with existing file formats

   \note This class is a static utility class and should not be instantiated.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscSerializer {
public:
   // --------------------------------------------------------------------------
   // Binary Serialization (QDataStream)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save data to binary format

      \param[in]  orc_Data     Data to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   Data saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveBinary(const QByteArray& orc_Data, const QString& orc_Path);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load data from binary format

      \param[out] orc_Data     Destination for loaded data
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   Data loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   Data format is invalid
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadBinary(QByteArray& orc_Data, const QString& orc_Path);

   // --------------------------------------------------------------------------
   // JSON Serialization (QJsonDocument)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save JSON object to file

      \param[in]  orc_Object   JSON object to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   JSON saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveJson(const QJsonObject& orc_Object, const QString& orc_Path);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load JSON object from file

      \param[out] orc_Object   Destination for loaded JSON object
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   JSON loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   JSON parse error
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadJson(QJsonObject& orc_Object, const QString& orc_Path);

   // --------------------------------------------------------------------------
   // XML Serialization (QDomDocument)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save QDomDocument to file

      \param[in]  orc_Doc      XML document to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   XML saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveXml(const QDomDocument& orc_Doc, const QString& orc_Path);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load QDomDocument from file

      \param[out] orc_Doc      Destination for loaded XML document
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   XML loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   XML parse error
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadXml(QDomDocument& orc_Doc, const QString& orc_Path);

   // --------------------------------------------------------------------------
   // Format Detection
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Detect serialization format from file extension

      \param[in]  orc_Path     File path to analyze

      \return Detected format based on file extension
   */
   //----------------------------------------------------------------------------------------------------------------------
   static E_SerializationFormat DetectFormat(const QString& orc_Path);

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Get file extension for a given format

      \param[in]  oq_Format    Serialization format

      \return Corresponding file extension (including dot)
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QString GetFileExtension(E_SerializationFormat oq_Format);

private:
   // Internal helper methods
   static int32_t mh_SaveToStream(const QByteArray& orc_Data, QFile& orc_File);
   static int32_t mh_LoadFromStream(QByteArray& orc_Data, QFile& orc_File);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCSERIALIZER_HPP
