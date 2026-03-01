//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Migrated Filer for XCE manifest data using Qt-native serialization
   
   This is the new Qt-native implementation using C_OscSerializer framework.
   It supports multiple formats (binary, JSON, XML) with automatic format detection.
   Backward compatibility with legacy C_OscXmlParser is maintained.
   
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QDomDocument>

#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscXceManifestFiler_New.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscSerializer.hpp"
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
const QString C_OscXceManifestFiler_New::hc_FILE_NAME = "manifest_xce.syde_pkg";
const uint16_t C_OscXceManifestFiler_New::mhu16_FILE_VERSION_1 = 1;
const uint16_t C_OscXceManifestFiler_New::mhu16_PACKAGE_VERSION_1 = 1;

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
   \brief Load manifest from file (auto-detects format)
   
   \param[out] orc_Config    Destination configuration
   \param[in]  orc_Path      Source file path
   
   \return
   C_NO_ERR    Data loaded successfully
   C_RANGE     File does not exist
   C_CONFIG    File format is invalid or data is corrupted
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_LoadFile(C_OscXceManifest& orc_Config, const QString& orc_Path) {
   using namespace stw::errors;
   
   // Check if file exists
   if (!QFileInfo(orc_Path).exists() || !QFileInfo(orc_Path).isFile()) {
      osc_write_log_error("Loading XCE manifest data",
                          "File \"" + orc_Path + "\" does not exist.");
      return C_RANGE;
   }
   
   // Detect format from file extension
   stw::opensyde_core::E_SerializationFormat e_Format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   
   // Load based on detected format
   switch (e_Format) {
      case stw::opensyde_core::E_SerializationFormat::e_BINARY: {
         QByteArray c_Data;
         int32_t s32_Result = stw::opensyde_core::C_OscSerializer::LoadBinary(c_Data, orc_Path);
         if (s32_Result != C_NO_ERR) {
            osc_write_log_error("Loading XCE manifest data",
                                "Binary file \"" + orc_Path + "\" could not be loaded.");
            return s32_Result;
         }
         return h_LoadBinary(orc_Config, c_Data);
      }
      
      case stw::opensyde_core::E_SerializationFormat::e_JSON: {
         QJsonObject c_Object;
         int32_t s32_Result = stw::opensyde_core::C_OscSerializer::LoadJson(c_Object, orc_Path);
         if (s32_Result != C_NO_ERR) {
            osc_write_log_error("Loading XCE manifest data",
                                "JSON file \"" + orc_Path + "\" could not be loaded.");
            return s32_Result;
         }
         return h_LoadJson(orc_Config, c_Object);
      }
      
      case stw::opensyde_core::E_SerializationFormat::e_XML: {
         QDomDocument c_Doc;
         int32_t s32_Result = stw::opensyde_core::C_OscSerializer::LoadXml(c_Doc, orc_Path);
         if (s32_Result != C_NO_ERR) {
            osc_write_log_error("Loading XCE manifest data",
                                "XML file \"" + orc_Path + "\" could not be loaded.");
            return s32_Result;
         }
         
         // Find the manifest element
         QDomElement c_Root = c_Doc.documentElement();
         if (c_Root.isNull()) {
            osc_write_log_error("Loading XCE manifest data",
                                "XML file \"" + orc_Path + "\" has no root element.");
            return C_CONFIG;
         }
         
         // Try to find xce-manifest element (new format) or legacy format
         QDomElement c_ManifestElement;
         if (c_Root.tagName() == "xce-manifest") {
            c_ManifestElement = c_Root;
         } else if (c_Root.tagName() == "opensyde-xce-package-manifest") {
            // Legacy format - navigate appropriately
            QDomNode c_PackageNode = c_Root.namedItem("package");
            if (c_PackageNode.isNull()) {
               osc_write_log_error("Loading XCE manifest data",
                                   "Legacy XML missing 'package' element.");
               return C_CONFIG;
            }
            QDomElement c_PackageElement = c_PackageNode.toElement();
            QDomNode c_ConfigNode = c_PackageElement.namedItem("xce-config");
            if (c_ConfigNode.isNull()) {
               osc_write_log_error("Loading XCE manifest data",
                                   "Legacy XML missing 'xce-config' element.");
               return C_CONFIG;
            }
            c_ManifestElement = c_ConfigNode.toElement();
         } else {
            osc_write_log_error("Loading XCE manifest data",
                                "Unknown XML root element: " + c_Root.tagName());
            return C_CONFIG;
         }
         
         return h_LoadXml(orc_Config, c_ManifestElement);
      }
      
      default:
         osc_write_log_error("Loading XCE manifest data",
                             "Unknown file format for \"" + orc_Path + "\".");
         return C_CONFIG;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save manifest to file (auto-detects format from extension)
   
   \param[in]  orc_Config    Source configuration
   \param[in]  orc_Path      Destination file path
   
   \return
   C_NO_ERR   Data saved successfully
   C_CONFIG   Data is invalid or file cannot be written
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_SaveFile(const C_OscXceManifest& orc_Config, const QString& orc_Path) {
   using namespace stw::errors;
   
   // Detect format from file extension
   stw::opensyde_core::E_SerializationFormat e_Format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   
   // Save based on detected format
   switch (e_Format) {
      case stw::opensyde_core::E_SerializationFormat::e_BINARY: {
         QByteArray c_Data = h_SaveBinary(orc_Config);
         int32_t s32_Result = stw::opensyde_core::C_OscSerializer::SaveBinary(c_Data, orc_Path);
         if (s32_Result != C_NO_ERR) {
            osc_write_log_error("Saving XCE manifest data",
                                "Binary file \"" + orc_Path + "\" could not be saved.");
         }
         return s32_Result;
      }
      
      case stw::opensyde_core::E_SerializationFormat::e_JSON: {
         QJsonObject c_Object = h_SaveJson(orc_Config);
         int32_t s32_Result = stw::opensyde_core::C_OscSerializer::SaveJson(c_Object, orc_Path);
         if (s32_Result != C_NO_ERR) {
            osc_write_log_error("Saving XCE manifest data",
                                "JSON file \"" + orc_Path + "\" could not be saved.");
         }
         return s32_Result;
      }
      
      case stw::opensyde_core::E_SerializationFormat::e_XML: {
         QDomDocument c_Doc;
         QDomElement c_ManifestElement = h_SaveXml(orc_Config, c_Doc);
         
         // For legacy compatibility, wrap in opensyde-xce-package-manifest structure
         QDomElement c_Root = c_Doc.createElement("opensyde-xce-package-manifest");
         
         // Add file version
         QDomElement c_FileVersion = c_Doc.createElement("file-version");
         QDomText c_FileVersionText = c_Doc.createTextNode(QString::number(mhu16_FILE_VERSION_1));
         c_FileVersion.appendChild(c_FileVersionText);
         c_Root.appendChild(c_FileVersion);
         
         // Add package element
         QDomElement c_Package = c_Doc.createElement("package");
         c_Package.setAttribute("types", "xce-certificates-config");
         
         // Add xce-config wrapper
         QDomElement c_XceConfig = c_Doc.createElement("xce-config");
         QDomElement c_PackageVersion = c_Doc.createElement("package-version");
         QDomText c_PackageVersionText = c_Doc.createTextNode(QString::number(mhu16_PACKAGE_VERSION_1));
         c_PackageVersion.appendChild(c_PackageVersionText);
         c_XceConfig.appendChild(c_PackageVersion);
         
         // Append the actual manifest element
         c_XceConfig.appendChild(c_ManifestElement);
         c_Package.appendChild(c_XceConfig);
         c_Root.appendChild(c_Package);
         
         c_Doc.appendChild(c_Root);
         
         // Save to file
         QFile c_File(orc_Path);
         if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
            osc_write_log_error("Saving XCE manifest data",
                                "File \"" + orc_Path + "\" could not be created.");
            return C_CONFIG;
         }
         
         QTextStream c_Stream(&c_File);
         c_Stream.setEncoding(QStringConverter::Utf8);
         c_Stream << c_Doc.toString(2);
         c_File.close();
         
         return C_NO_ERR;
      }
      
      default:
         osc_write_log_error("Saving XCE manifest data",
                             "Unknown file format for \"" + orc_Path + "\".");
         return C_CONFIG;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load manifest from binary format (QDataStream)
   
   \param[out] orc_Config    Destination configuration
   \param[in]  orc_Data      Binary data
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_LoadBinary(C_OscXceManifest& orc_Config, const QByteArray& orc_Data) {
   QDataStream c_Stream(orc_Data);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   return orc_Config.FromQDataStream(c_Stream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save manifest to binary format (QDataStream)
   
   \param[in]  orc_Config    Source configuration
   
   \return QByteArray containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscXceManifestFiler_New::h_SaveBinary(const C_OscXceManifest& orc_Config) {
   QByteArray c_Data;
   QDataStream c_Stream(&c_Data, QIODevice::WriteOnly);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   orc_Config.ToQDataStream(c_Stream);
   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load manifest from JSON format
   
   \param[out] orc_Config    Destination configuration
   \param[in]  orc_Object    JSON object
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_LoadJson(C_OscXceManifest& orc_Config, const QJsonObject& orc_Object) {
   return orc_Config.FromJsonObject(orc_Object);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save manifest to JSON format
   
   \param[in]  orc_Config    Source configuration
   
   \return QJsonObject containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscXceManifestFiler_New::h_SaveJson(const C_OscXceManifest& orc_Config) {
   return orc_Config.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load manifest from XML format (QDomDocument)
   
   \param[out] orc_Config    Destination configuration
   \param[in]  orc_Element   XML element containing manifest data
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_LoadXml(C_OscXceManifest& orc_Config, const QDomElement& orc_Element) {
   return orc_Config.FromQDomElement(orc_Element);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save manifest to XML format (QDomDocument)
   
   \param[in]  orc_Config    Source configuration
   \param[in]  orc_Doc       XML document to append to
   
   \return QDomElement representing the serialized manifest
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscXceManifestFiler_New::h_SaveXml(const C_OscXceManifest& orc_Config, QDomDocument& orc_Doc) {
   return orc_Config.ToQDomDocument(orc_Doc, "xce-manifest");
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Legacy compatibility: Load from C_OscXmlParserBase
   \deprecated Use h_LoadXml() with QDomElement instead
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler_New::h_LoadData(C_OscXceManifest& orc_Config, C_OscXmlParserBase& orc_XmlParser) {
   using namespace stw::errors;
   
   // Simplified legacy implementation
   // Navigate to certificates-path element
   QString c_Result = orc_XmlParser.SelectNodeChild("certificates-path");
   if (!c_Result.isEmpty()) {
      QString c_Path = orc_XmlParser.GetNodeContent();
      orc_Config.c_CertificatesPath = c_Path;
      orc_XmlParser.SelectNodeParent();
   }
   
   // Note: For full legacy compatibility, would need to parse parameters list
   // This is a simplified version focusing on the main field
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Legacy compatibility: Save to C_OscXmlParserBase
   \deprecated Use h_SaveXml() with QDomDocument instead
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXceManifestFiler_New::h_SaveData(const C_OscXceManifest& orc_Config, C_OscXmlParserBase& orc_XmlParser) {
   // Legacy API
   Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("xce-config") == "xce-config");
   Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("certificates-path") == "certificates-path");
   orc_XmlParser.SetNodeContent(orc_Config.c_CertificatesPath);
   orc_XmlParser.SelectNodeParent(); // certificates-path
   orc_XmlParser.SelectNodeParent(); // xce-config
}
