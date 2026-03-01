//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief   Generic template-based filer helper for Qt-native serialization
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCFILERHELPER_HPP
#define C_OSCFILERHELPER_HPP

#include "C_OscSerializer.hpp"
#include "stwtypes.hpp"
#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>

namespace stw {
namespace opensyde_core {

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Template-based filer helper for Qt types

   This template class provides a generic interface for serializing and
   deserializing Qt-based data structures in multiple formats.

   \tparam T The type to serialize/deserialize

   \note
   To use this helper, the type T must implement:
   - ToQDataStream(QDataStream&) const - for binary serialization
   - FromQDataStream(QDataStream&) - for binary deserialization
   - ToJsonObject() const - for JSON serialization
   - FromJsonObject(const QJsonObject&) - for JSON deserialization

   Example usage:
   \code
   class C_MyData {
   public:
      void ToQDataStream(QDataStream& out) const {
         out << mName << mId << mValues;
      }

      void FromQDataStream(QDataStream& in) {
         in >> mName >> mId >> mValues;
      }

      QJsonObject ToJsonObject() const {
         QJsonObject obj;
         obj["name"] = mName;
         obj["id"] = mId;
         obj["values"] = mValues;
         return obj;
      }

      void FromJsonObject(const QJsonObject& json) {
         mName = json["name"].toString();
         mId = json["id"].toInt();
         mValues = json["values"].toArray();
      }
   };

   // Usage:
   C_MyData data;
   C_OscFilerHelper<C_MyData>::SaveBinary(data, "data.bin");
   C_OscFilerHelper<C_MyData>::LoadBinary(data, "data.bin");
   \endcode
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename T>
class C_OscFilerHelper {
public:
   // --------------------------------------------------------------------------
   // Binary Serialization (QDataStream)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save object to binary format

      \param[in]  orc_Object   Object to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   Data saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveBinary(const T& orc_Object, const QString& orc_Path) {
      // Serialize object to QByteArray
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_6_0);
      out.setByteOrder(QDataStream::LittleEndian);

      // Call object's custom serialization
      const_cast<T&>(orc_Object).ToQDataStream(out);

      if (out.status() != QDataStream::Ok) {
         return C_RD_WR;
      }

      // Save to file using base serializer
      return C_OscSerializer::SaveBinary(data, orc_Path);
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load object from binary format

      \param[out] orc_Object   Destination for loaded object
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   Data loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   Data format is invalid
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadBinary(T& orc_Object, const QString& orc_Path) {
      // Load data from file
      QByteArray data;
      int32_t s32_Result = C_OscSerializer::LoadBinary(data, orc_Path);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      // Deserialize from QByteArray
      QDataStream in(data);
      in.setVersion(QDataStream::Qt_6_0);
      in.setByteOrder(QDataStream::LittleEndian);

      // Call object's custom deserialization
      orc_Object.FromQDataStream(in);

      if (in.status() != QDataStream::Ok) {
         return C_CONFIG;
      }

      return C_NO_ERR;
   }

   // --------------------------------------------------------------------------
   // JSON Serialization (QJsonDocument)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save object to JSON format

      \param[in]  orc_Object   Object to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   JSON saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveJson(const T& orc_Object, const QString& orc_Path) {
      QJsonObject json = orc_Object.ToJsonObject();
      return C_OscSerializer::SaveJson(json, orc_Path);
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load object from JSON format

      \param[out] orc_Object   Destination for loaded object
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   JSON loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   JSON parse error or invalid format
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadJson(T& orc_Object, const QString& orc_Path) {
      QJsonObject json;
      int32_t s32_Result = C_OscSerializer::LoadJson(json, orc_Path);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      orc_Object.FromJsonObject(json);
      return C_NO_ERR;
   }

   // --------------------------------------------------------------------------
   // XML Serialization (QDomDocument)
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save object to XML format

      \param[in]  orc_Object   Object to save
      \param[in]  orc_Path     Destination file path

      \return
      C_NO_ERR   XML saved successfully
      C_RD_WR    File could not be opened or written
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t SaveXml(const T& orc_Object, const QString& orc_Path) {
      QDomDocument doc;
      orc_Object.ToQDomDocument(doc);
      return C_OscSerializer::SaveXml(doc, orc_Path);
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load object from XML format

      \param[out] orc_Object   Destination for loaded object
      \param[in]  orc_Path     Source file path

      \return
      C_NO_ERR   XML loaded successfully
      C_RD_WR    File could not be opened or read
      C_CONFIG   XML parse error or invalid format
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadXml(T& orc_Object, const QString& orc_Path) {
      QDomDocument doc;
      int32_t s32_Result = C_OscSerializer::LoadXml(doc, orc_Path);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      orc_Object.FromQDomDocument(doc);
      return C_NO_ERR;
   }

   // --------------------------------------------------------------------------
   // In-Memory Serialization
   // --------------------------------------------------------------------------

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save object to QByteArray (binary format)

      \param[in]  orc_Object   Object to save

      \return QByteArray containing serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QByteArray SaveToMemoryBinary(const T& orc_Object) {
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_6_0);
      out.setByteOrder(QDataStream::LittleEndian);
      const_cast<T&>(orc_Object).ToQDataStream(out);
      return data;
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load object from QByteArray (binary format)

      \param[out] orc_Object   Destination for loaded object
      \param[in]  orc_Data     Serialized data

      \return
      C_NO_ERR   Data loaded successfully
      C_CONFIG   Data format is invalid
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadFromMemoryBinary(T& orc_Object, const QByteArray& orc_Data) {
      QDataStream in(orc_Data);
      in.setVersion(QDataStream::Qt_6_0);
      in.setByteOrder(QDataStream::LittleEndian);
      orc_Object.FromQDataStream(in);
      return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save object to QString (JSON format)

      \param[in]  orc_Object   Object to save

      \return QString containing JSON representation
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QString SaveToMemoryJson(const T& orc_Object) {
      QJsonObject json = orc_Object.ToJsonObject();
      QJsonDocument doc(json);
      return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
   }

   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load object from QString (JSON format)

      \param[out] orc_Object   Destination for loaded object
      \param[in]  orc_String   JSON string

      \return
      C_NO_ERR   Data loaded successfully
      C_CONFIG   JSON parse error
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t LoadFromMemoryJson(T& orc_Object, const QString& orc_String) {
      QJsonParseError parseError;
      QJsonDocument doc = QJsonDocument::fromJson(orc_String.toUtf8(), &parseError);

      if (parseError.error != QJsonParseError::NoError) {
         return C_CONFIG;
      }

      orc_Object.FromJsonObject(doc.object());
      return C_NO_ERR;
   }
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCFILERHELPER_HPP
