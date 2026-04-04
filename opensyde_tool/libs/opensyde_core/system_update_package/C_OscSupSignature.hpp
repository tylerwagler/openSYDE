 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature with multi-format support
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCSUPSIGNATUREFILER_HPP
 #define C_OSCSUPSIGNATUREFILER_HPP

 #include <QString>
 #include <QDataStream>
 #include <QJsonObject>
 #include <QDomDocument>
 #include <QDomElement>

 /* -- Namespace
  * -----------------------------------------------------------------------------------------------------
  */
 namespace stw {
 namespace opensyde_core {
 /* -- Global Constants
  * ----------------------------------------------------------------------------------------------
  */

 /* -- Types
  * ---------------------------------------------------------------------------------------------------------
  */

 class C_OscSupSignatureFiler {
 public:
     // --------------------------------------------------------------------------
     // Public Static Methods - Multi-format file operations
     // --------------------------------------------------------------------------
     static int32_t h_CreateSignatureFile(const QString &orc_Path,
                                          const QString &orc_Signature);
     static int32_t h_LoadSignatureFile(const QString &orc_Path,
                                        QString &orc_Signature);

     // --------------------------------------------------------------------------
     // Format-specific loading methods
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(QString &orc_Signature, QDataStream &orc_Stream);
     static int32_t h_LoadJson(QString &orc_Signature, const QJsonObject &orc_Object);
     static int32_t h_LoadXml(QString &orc_Signature, C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Format-specific saving methods
     // --------------------------------------------------------------------------
     static int32_t h_SaveBinary(const QString &orc_Signature, QDataStream &orc_Stream);
     static int32_t h_SaveJson(const QString &orc_Signature, QJsonObject &orc_Object);
     static int32_t h_SaveXml(const QString &orc_Signature, C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Helper methods
     // --------------------------------------------------------------------------
     static QString h_GetSignatureFileName(void);

     // --------------------------------------------------------------------------
     // Legacy compatibility methods (deprecated)
     // --------------------------------------------------------------------------
     [[deprecated("Use h_CreateSignatureFile/h_LoadSignatureFile with auto-detection instead")]]
     static int32_t h_CreateSignatureFileLegacy(const QString &orc_Path,
                                                const QString &orc_Signature);
     [[deprecated("Use h_CreateSignatureFile/h_LoadSignatureFile with auto-detection instead")]]
     static int32_t h_LoadSignatureFileLegacy(const QString &orc_Path,
                                              QString &orc_Signature);
 };

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
