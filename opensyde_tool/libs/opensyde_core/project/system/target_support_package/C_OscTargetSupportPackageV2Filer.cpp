//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target support package V2 reader/writer (Multi-Format)

   Load / save target support package V2 data from / to binary, JSON, or XML files
   using the Qt-native serialization framework.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QJsonDocument>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "C_OscLoggingHandler.hpp"
#include "C_OscTargetSupportPackageV2Filer.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

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
   \brief   Load target support package V2 from file (auto-detect format)

   \param[out]  orc_Package  Target support package V2 information read from file
   \param[in]   orc_Path     Path to file

   \return
   C_NO_ERR    data read
   C_RANGE     specified file does not exist
   C_NOACT     specified file is present but structure is invalid
   C_CONFIG    XML/JSON node or attribute missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadFile(
   C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   return mh_DetectAndLoad(orc_Package, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to file (auto-detect format from extension)

   \param[in]  orc_Package  Target support package V2 information to write to file
   \param[in]  orc_Path     Path to file

   \return
   C_NO_ERR    data written
   C_RD_WR     error writing to file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_SaveFile(
   const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QString c_LowerPath = orc_Path.toLower();

   if (c_LowerPath.endsWith(".bin")) {
      s32_Result = h_SaveBinary(orc_Package, orc_Path);
   }
   else if (c_LowerPath.endsWith(".json")) {
      s32_Result = h_SaveJson(orc_Package, orc_Path);
   }
   else if (c_LowerPath.endsWith(".xml") || c_LowerPath.endsWith(".syde_tsp")) {
      s32_Result = h_SaveXml(orc_Package, orc_Path);
   }
   else {
      // Default to XML for unknown extensions (legacy compatibility)
      osc_write_log_warning("Saving target support package V2",
         "Unknown file extension, using XML format");
      s32_Result = h_SaveXml(orc_Package, orc_Path);
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from binary file

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Path     Path to binary file

   \return
   C_NO_ERR    data read
   C_RANGE     file does not exist
   C_RD_WR     error reading file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadBinary(
   C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Package.FromQDataStream(c_Stream);
      c_File.close();
   }
   else {
      osc_write_log_error("Loading target support package V2 (binary)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to binary file

   \param[in]  orc_Package  Target support package V2 information
   \param[in]  orc_Path     Path to binary file

   \return
   C_NO_ERR    data written
   C_RD_WR     error writing file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_SaveBinary(
   const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Package.ToQDataStream(c_Stream);
      c_File.close();
   }
   else {
      osc_write_log_error("Saving target support package V2 (binary)",
         "File \"" + orc_Path + "\" could not be opened for writing.");
      s32_Result = C_RD_WR;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from binary data in memory

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Data     Binary data

   \return
   C_NO_ERR    data deserialized
   C_CONFIG    invalid data format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadFromMemoryBinary(
   C_OscTargetSupportPackageV2 &orc_Package, const QByteArray &orc_Data)
{
   int32_t s32_Result = C_NO_ERR;
   QDataStream c_Stream(orc_Data);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   orc_Package.FromQDataStream(c_Stream);
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to binary data in memory

   \param[in]  orc_Package  Target support package V2 information

   \return   Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscTargetSupportPackageV2Filer::h_SaveToMemoryBinary(
   const C_OscTargetSupportPackageV2 &orc_Package)
{
   QByteArray c_Data;
   QDataStream c_Stream(&c_Data, QIODevice::WriteOnly);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   orc_Package.ToQDataStream(c_Stream);
   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from JSON file

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Path     Path to JSON file

   \return
   C_NO_ERR    data read
   C_RANGE     file does not exist
   C_NOACT     file is not valid JSON
   C_CONFIG    JSON structure invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadJson(
   C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QJsonParseError c_ParseError;
      QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_ParseError);
      c_File.close();

      if (c_ParseError.error == QJsonParseError::NoError) {
         if (c_Doc.isObject()) {
            orc_Package.FromJsonObject(c_Doc.object());
         }
         else {
            osc_write_log_error("Loading target support package V2 (JSON)",
               "JSON root is not an object");
            s32_Result = C_CONFIG;
         }
      }
      else {
         osc_write_log_error("Loading target support package V2 (JSON)",
            "JSON parse error: " + c_ParseError.errorString());
         s32_Result = C_NOACT;
      }
   }
   else {
      osc_write_log_error("Loading target support package V2 (JSON)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to JSON file

   \param[in]  orc_Package  Target support package V2 information
   \param[in]  orc_Path     Path to JSON file

   \return
   C_NO_ERR    data written
   C_RD_WR     error writing file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_SaveJson(
   const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QJsonObject c_Object = orc_Package.ToJsonObject();
      QJsonDocument c_Doc(c_Object);
      c_File.write(c_Doc.toJson(QJsonDocument::Indented));
      c_File.close();
   }
   else {
      osc_write_log_error("Saving target support package V2 (JSON)",
         "File \"" + orc_Path + "\" could not be opened for writing.");
      s32_Result = C_RD_WR;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from JSON object in memory

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Object   JSON object

   \return
   C_NO_ERR    data deserialized
   C_CONFIG    invalid JSON structure
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadFromMemoryJson(
   C_OscTargetSupportPackageV2 &orc_Package, const QJsonObject &orc_Object)
{
   orc_Package.FromJsonObject(orc_Object); return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to JSON object in memory

   \param[in]  orc_Package  Target support package V2 information

   \return   JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscTargetSupportPackageV2Filer::h_SaveToMemoryJson(
   const C_OscTargetSupportPackageV2 &orc_Package)
{
   return orc_Package.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from XML file

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Path     Path to XML file

   \return
   C_NO_ERR    data read
   C_RANGE     file does not exist
   C_NOACT     file is not valid XML
   C_CONFIG    XML structure invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadXml(
   C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QString c_ErrorMsg;
      int i_ErrorLine;
      int i_ErrorColumn;

      if (c_Doc.setContent(c_File.readAll(), &c_ErrorMsg, &i_ErrorLine, &i_ErrorColumn)) {
         QDomElement c_Root = c_Doc.documentElement();
         orc_Package.FromQDomDocument(c_Root);
      }
      else {
         osc_write_log_error("Loading target support package V2 (XML)",
            "XML parse error at line " + QString::number(i_ErrorLine) +
            ", column " + QString::number(i_ErrorColumn) + ": " + c_ErrorMsg);
         s32_Result = C_NOACT;
      }
      c_File.close();
   }
   else {
      osc_write_log_error("Loading target support package V2 (XML)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to XML file

   \param[in]  orc_Package  Target support package V2 information
   \param[in]  orc_Path     Path to XML file

   \return
   C_NO_ERR    data written
   C_RD_WR     error writing file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_SaveXml(
   const C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QDomElement c_Element = orc_Package.ToQDomDocument(c_Doc, "opensyde-target-support-package");
      c_Doc.appendChild(c_Element);
      
      // Add XML declaration
      QString c_Content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      c_Content += c_Doc.toString(3);
      c_File.write(c_Content.toUtf8());
      c_File.close();
   }
   else {
      osc_write_log_error("Saving target support package V2 (XML)",
         "File \"" + orc_Path + "\" could not be opened for writing.");
      s32_Result = C_RD_WR;
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from XML element in memory

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Element  XML element

   \return
   C_NO_ERR    data deserialized
   C_CONFIG    invalid XML structure
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::h_LoadFromMemoryXml(
   C_OscTargetSupportPackageV2 &orc_Package, const QDomElement &orc_Element)
{
   orc_Package.FromQDomDocument(orc_Element);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Save target support package V2 to XML element in memory

   \param[in]  orc_Package  Target support package V2 information
   \param[in]  orc_Doc      XML document

   \return   XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscTargetSupportPackageV2Filer::h_SaveToMemoryXml(
   const C_OscTargetSupportPackageV2 &orc_Package, QDomDocument &orc_Doc)
{
   return orc_Package.ToQDomDocument(orc_Doc, "opensyde-target-support-package");
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Load target support package V2 from XML parser (legacy compatibility)

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_XmlParser  XML parser

   \return
   C_NO_ERR    data read
   C_CONFIG    XML structure invalid
*/
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Detect file format and load accordingly

   \param[out]  orc_Package  Target support package V2 information
   \param[in]   orc_Path     Path to file

   \return
   C_NO_ERR    data read
   C_RANGE     file does not exist
   C_NOACT     unsupported format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageV2Filer::mh_DetectAndLoad(
   C_OscTargetSupportPackageV2 &orc_Package, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QString c_LowerPath = orc_Path.toLower();

   if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
      if (c_LowerPath.endsWith(".bin")) {
         s32_Result = h_LoadBinary(orc_Package, orc_Path);
      }
      else if (c_LowerPath.endsWith(".json")) {
         s32_Result = h_LoadJson(orc_Package, orc_Path);
      }
      else if (c_LowerPath.endsWith(".xml") || c_LowerPath.endsWith(".syde_tsp")) {
         s32_Result = h_LoadXml(orc_Package, orc_Path);
      }
      else {
         // Default to XML for unknown extensions (legacy compatibility)
         osc_write_log_warning("Loading target support package V2",
            "Unknown file extension, trying XML format");
         s32_Result = h_LoadXml(orc_Package, orc_Path);
      }
   }
   else {
      osc_write_log_error("Loading target support package V2",
         "File \"" + orc_Path + "\" does not exist.");
      s32_Result = C_RANGE;
   }

   return s32_Result;
}
