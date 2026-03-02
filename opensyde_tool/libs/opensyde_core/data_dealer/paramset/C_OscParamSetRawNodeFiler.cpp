//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for parameter set raw node data (new Qt-native implementation)

   File handler for parameter set raw node data with Qt-native serialization support (binary, JSON, XML).

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetRawNodeFiler.hpp"
#include "stwerrors.hpp"
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonParseError>

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
/*!
   \brief Load parameter set raw node from file (auto-detect format)

   \param[in] c_FilePath Path to the file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadFile(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   // Determine file format by extension
   QString c_LowerPath = c_FilePath.toLower();
   
   if (c_LowerPath.endsWith(".bin")) {
      return h_LoadBinary(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_LoadJson(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".xml")) {
      return h_LoadXml(c_FilePath, rc_Node);
   } else {
      // Default to binary if no extension or unknown extension
      return h_LoadBinary(c_FilePath, rc_Node);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to file (auto-detect format)

   \param[in] c_FilePath Path to the file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveFile(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   // Determine file format by extension
   QString c_LowerPath = c_FilePath.toLower();
   
   if (c_LowerPath.endsWith(".bin")) {
      return h_SaveBinary(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_SaveJson(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".xml")) {
      return h_SaveXml(c_FilePath, rc_Node);
   } else {
      // Default to binary if no extension or unknown extension
      return h_SaveBinary(c_FilePath, rc_Node);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from binary file

   \param[in] c_FilePath Path to the binary file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadBinary(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   int32_t i32_Result = rc_Node.FromQDataStream(c_Stream);
   
   c_File.close();
   
   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to binary file

   \param[in] c_FilePath Path to the binary file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveBinary(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   int32_t i32_Result = rc_Node.ToQDataStream(c_Stream);
   
   c_File.close();
   
   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from JSON file

   \param[in] c_FilePath Path to the JSON file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadJson(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   QString c_Content = c_Stream.readAll();
   c_File.close();

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(c_Content.toUtf8(), &c_ParseError);
   
   if (c_ParseError.error != QJsonParseError::NoError) {
      return C_CONFIG;
   }
   
   if (!c_Doc.isObject()) {
      return C_CONFIG;
   }
   
   int32_t i32_Result = rc_Node.FromJsonObject(c_Doc.object());
   
   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to JSON file

   \param[in] c_FilePath Path to the JSON file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveJson(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QJsonObject c_Object = rc_Node.ToJsonObject();
   QJsonDocument c_Doc(c_Object);
   
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Stream << c_Doc.toJson(QJsonDocument::Compact);
   c_File.close();
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from XML file

   \param[in] c_FilePath Path to the XML file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadXml(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDomDocument c_Doc;
   QString c_ErrorMsg;
   int32_t i32_LineNumber;
   
   if (!c_Doc.setContent(&c_File, &c_ErrorMsg, &i32_LineNumber)) {
      c_File.close();
      return C_CONFIG;
   }
   
   c_File.close();
   
   int32_t i32_Result = rc_Node.FromQDomElement(c_Doc.documentElement());
   
   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to XML file

   \param[in] c_FilePath Path to the XML file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveXml(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QDomDocument c_Doc("param-set-raw-node");
   
   QDomElement c_RootElement = rc_Node.ToQDomDocument(c_Doc, "param-set-raw-node");
   c_Doc.appendChild(c_RootElement);
   
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Stream << c_Doc.toString();
   c_File.close();
   
   return C_NO_ERR;
}