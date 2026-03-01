//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Migrated Filer for XCE manifest data using Qt-native serialization
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEMANIFESTFILER_NEW_HPP
#define C_OSCXCEMANIFESTFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXceManifest.hpp"
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscXceManifestFiler_New {
public:
   // --------------------------------------------------------------------------
   // Primary API: Auto-detect format and load/save
   // --------------------------------------------------------------------------
   
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
   static int32_t h_LoadFile(C_OscXceManifest& orc_Config, const QString& orc_Path);
   
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
   static int32_t h_SaveFile(const C_OscXceManifest& orc_Config, const QString& orc_Path);
   
   // --------------------------------------------------------------------------
   // Format-Specific API
   // --------------------------------------------------------------------------
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load manifest from binary format (QDataStream)
      
      \param[out] orc_Config    Destination configuration
      \param[in]  orc_Data      Binary data
      
      \return C_NO_ERR on success
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscXceManifest& orc_Config, const QByteArray& orc_Data);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save manifest to binary format (QDataStream)
      
      \param[in]  orc_Config    Source configuration
      
      \return QByteArray containing serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QByteArray h_SaveBinary(const C_OscXceManifest& orc_Config);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load manifest from JSON format
      
      \param[out] orc_Config    Destination configuration
      \param[in]  orc_Object    JSON object
      
      \return C_NO_ERR on success
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscXceManifest& orc_Config, const QJsonObject& orc_Object);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save manifest to JSON format
      
      \param[in]  orc_Config    Source configuration
      
      \return QJsonObject containing serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QJsonObject h_SaveJson(const C_OscXceManifest& orc_Config);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Load manifest from XML format (QDomDocument)
      
      \param[out] orc_Config    Destination configuration
      \param[in]  orc_Element   XML element containing manifest data
      
      \return C_NO_ERR on success
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscXceManifest& orc_Config, const QDomElement& orc_Element);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Save manifest to XML format (QDomDocument)
      
      \param[in]  orc_Config    Source configuration
      \param[in]  orc_Doc       XML document to append to
      
      \return QDomElement representing the serialized manifest
   */
   //----------------------------------------------------------------------------------------------------------------------
   static QDomElement h_SaveXml(const C_OscXceManifest& orc_Config, QDomDocument& orc_Doc);
   
   // --------------------------------------------------------------------------
   // Legacy Compatibility API (deprecated but still supported)
   // --------------------------------------------------------------------------
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Legacy compatibility: Load from C_OscXmlParserBase
      \deprecated Use h_LoadXml() with QDomElement instead
      
      \param[out] orc_Config    Destination configuration
      \param[in]  orc_XmlParser  Legacy XML parser
      
      \return C_NO_ERR on success
   */
   //----------------------------------------------------------------------------------------------------------------------
   static int32_t h_LoadData(C_OscXceManifest& orc_Config, class C_OscXmlParserBase& orc_XmlParser);
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Legacy compatibility: Save to C_OscXmlParserBase
      \deprecated Use h_SaveXml() with QDomDocument instead
      
      \param[in]  orc_Config    Source configuration
      \param[in]  orc_XmlParser  Legacy XML parser
   */
   //----------------------------------------------------------------------------------------------------------------------
   static void h_SaveData(const C_OscXceManifest& orc_Config, class C_OscXmlParserBase& orc_XmlParser);
   
   static const QString hc_FILE_NAME;

private:
   C_OscXceManifestFiler_New();
   
   static const uint16_t mhu16_FILE_VERSION_1;
   static const uint16_t mhu16_PACKAGE_VERSION_1;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif // C_OSCXCEMANIFESTFILER_NEW_HPP
