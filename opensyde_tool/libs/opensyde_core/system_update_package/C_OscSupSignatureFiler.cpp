 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature with multi-format support

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "precomp_headers.hpp"
 #include <QFileInfo>
 #include <fstream>

 #include "C_OscSupSignatureFiler.hpp"
 #include "C_OscXmlParser.hpp"
 #include "C_OscSystemFilerUtil.hpp"
 #include "stwerrors.hpp"

 /* -- Used Namespaces
  * -----------------------------------------------------------------------------------------------
  */

 using namespace stw::errors;
 using namespace stw::opensyde_core;

 /* -- Module Global Constants
  * ---------------------------------------------------------------------------------------
  */
 static const uint16_t mu16_FILE_VERSION = 1U;
 static const QString mc_SIG_TYPE = "ECDSA secp256r1";
 static const QString mc_ROOT_NAME = "opensyde-secure-update-collection-signature";
 static const QString mc_FILE_VERSION = "file-version";
 static const QString mc_NODE_SIG = "signature";
 static const QString mc_NODE_SIG_TYPE_ATTR = "type";
 static const QString mc_NODE_SIG_VALUE_ATTR = "value";

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
 /*! \brief  Create signature file (auto-detects format)

    \param[in]  orc_Path       Path
    \param[in]  orc_Signature  Signature

    \return
    STW error codes

    \retval   C_NO_ERR   File created
    \retval   C_RD_WR    File not created
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_CreateSignatureFile(const QString &orc_Path,
                                                           const QString &orc_Signature) {
     const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

     if (c_Extension == "bin") {
         // Binary format
         QFile c_File(orc_Path);
         if (c_File.open(QIODevice::WriteOnly)) {
             QDataStream c_Stream(&c_File);
             c_Stream.setVersion(QDataStream::Qt_5_12);
             const int32_t s32_Result = h_SaveBinary(orc_Signature, c_Stream);
             c_File.close();
             return s32_Result;
         } else {
             return C_RD_WR;
         }
     } else if (c_Extension == "json") {
         // JSON format
         QJsonObject c_Object;
         const int32_t s32_Result = h_SaveJson(orc_Signature, c_Object);
         if (s32_Result == C_NO_ERR) {
             QJsonDocument c_Doc(c_Object);
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
                 c_File.write(c_Doc.toJson(QJsonDocument::Indented));
                 c_File.close();
                 return C_NO_ERR;
             } else {
                 return C_RD_WR;
             }
         }
         return s32_Result;
     } else {
         // XML format (default)
         C_OscXmlParser c_XmlParser;
         const int32_t s32_Result = h_SaveXml(orc_Signature, c_XmlParser);
         if (s32_Result == C_NO_ERR) {
             return c_XmlParser.SaveToFile(orc_Path);
         }
         return s32_Result;
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature file (auto-detects format)

    \param[in]      orc_Path         Path
    \param[in,out]  orc_Signature    Signature

    \return
    STW error codes

    \retval   C_NO_ERR   File read
    \retval   C_RD_WR    File not read
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_LoadSignatureFile(const QString &orc_Path,
                                                         QString &orc_Signature) {
     const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

     if (c_Extension == "bin") {
         // Binary format
         QFile c_File(orc_Path);
         if (c_File.open(QIODevice::ReadOnly)) {
             QDataStream c_Stream(&c_File);
             c_Stream.setVersion(QDataStream::Qt_5_12);
             const int32_t s32_Result = h_LoadBinary(orc_Signature, c_Stream);
             c_File.close();
             return s32_Result;
         } else {
             return C_RD_WR;
         }
     } else if (c_Extension == "json") {
         // JSON format
         QFile c_File(orc_Path);
         if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
             QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
             c_File.close();
             if (c_Doc.isObject()) {
                 return h_LoadJson(orc_Signature, c_Doc.object());
             } else {
                 return C_NOACT;
             }
         } else {
             return C_RD_WR;
         }
     } else {
         // XML format (default)
         C_OscXmlParser c_XmlParser;
         const int32_t s32_Result = c_XmlParser.LoadFromFile(orc_Path);
         if (s32_Result == C_NO_ERR) {
             return h_LoadXml(orc_Signature, c_XmlParser);
         }
         return s32_Result;
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature from binary stream

    \param[out]     orc_Signature    Signature
    \param[in,out]  orc_Stream       Binary stream

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_LoadBinary(QString &orc_Signature,
                                                  QDataStream &orc_Stream) {
     // Read signature string
     orc_Stream >> orc_Signature;
     return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature from JSON object

    \param[out]     orc_Signature    Signature
    \param[in]      orc_Object       JSON object

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_LoadJson(QString &orc_Signature,
                                                const QJsonObject &orc_Object) {
     if (!orc_Object.contains("signature") || !orc_Object["signature"].isString()) {
         return C_CONFIG;
     }
     orc_Signature = orc_Object["signature"].toString();
     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature from XML parser

    \param[out]     orc_Signature    Signature
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_LoadXml(QString &orc_Signature,
                                               C_OscXmlParserBase &orc_XmlParser) {
     // Verify root element
     if (orc_XmlParser.SelectRoot() != mc_ROOT_NAME) {
         return C_CONFIG;
     }

     // File version
     if (orc_XmlParser.SelectNodeChild(mc_FILE_VERSION) != mc_FILE_VERSION) {
         return C_CONFIG;
     }
     const QString c_FileVersion = orc_XmlParser.GetNodeContent();
     const uint32_t u32_FileVersion = c_FileVersion.toUInt();
     orc_XmlParser.SelectRoot();

     if (u32_FileVersion != mu16_FILE_VERSION) {
         return C_CONFIG;
     }

     // Signature
     if (orc_XmlParser.SelectNodeChild(mc_NODE_SIG) != mc_NODE_SIG) {
         return C_CONFIG;
     }
     const QString c_SigType = orc_XmlParser.GetAttributeString(mc_NODE_SIG_TYPE_ATTR);
     if (c_SigType != mc_SIG_TYPE) {
         return C_CONFIG;
     }
     orc_Signature = orc_XmlParser.GetAttributeString(mc_NODE_SIG_VALUE_ATTR);
     orc_XmlParser.SelectRoot();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save signature to binary stream

    \param[in]      orc_Signature    Signature
    \param[in,out]  orc_Stream       Binary stream

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_SaveBinary(const QString &orc_Signature,
                                                  QDataStream &orc_Stream) {
     orc_Stream << orc_Signature;
     return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save signature to JSON object

    \param[in]      orc_Signature    Signature
    \param[out]     orc_Object       JSON object

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_SaveJson(const QString &orc_Signature,
                                                QJsonObject &orc_Object) {
     QJsonObject c_SigObject;
     c_SigObject["type"] = mc_SIG_TYPE;
     c_SigObject["value"] = orc_Signature;
     orc_Object["signature"] = c_SigObject;
     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save signature to XML parser

    \param[in]      orc_Signature    Signature
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_SaveXml(const QString &orc_Signature,
                                               C_OscXmlParserBase &orc_XmlParser) {
     // Root Node
     orc_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);

     // File version
     orc_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION);
     orc_XmlParser.SetNodeContent(QString::number(mu16_FILE_VERSION));
     orc_XmlParser.SelectNodeParent();

     // Signature
     orc_XmlParser.CreateAndSelectNodeChild(mc_NODE_SIG);
     orc_XmlParser.SetAttributeString(mc_NODE_SIG_TYPE_ATTR, mc_SIG_TYPE);
     orc_XmlParser.SetAttributeString(mc_NODE_SIG_VALUE_ATTR, orc_Signature);
     orc_XmlParser.SelectNodeParent();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Get signature file name

    \return
    Signature file name
 */
 //----------------------------------------------------------------------------------------------------------------------
 QString C_OscSupSignatureFiler::h_GetSignatureFileName() {
     return "signature.syde_sucsig";
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Create signature file (legacy compatibility - XML only)

    \see h_CreateSignatureFile
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_CreateSignatureFileLegacy(const QString &orc_Path,
                                                                 const QString &orc_Signature) {
     C_OscXmlParser c_XmlParser;
     c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);
     c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION);
     c_XmlParser.SetNodeContent(QString::number(mu16_FILE_VERSION));
     c_XmlParser.SelectNodeParent();
     c_XmlParser.CreateAndSelectNodeChild(mc_NODE_SIG);
     c_XmlParser.SetAttributeString(mc_NODE_SIG_TYPE_ATTR, mc_SIG_TYPE);
     c_XmlParser.SetAttributeString(mc_NODE_SIG_VALUE_ATTR, orc_Signature);
     c_XmlParser.SelectNodeParent();

     const int32_t s32_Result = c_XmlParser.SaveToFile(orc_Path);
     return (s32_Result == C_NO_ERR) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load signature file (legacy compatibility - XML only)

    \see h_LoadSignatureFile
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupSignatureFiler::h_LoadSignatureFileLegacy(const QString &orc_Path,
                                                               QString &orc_Signature) {
     C_OscXmlParser c_XmlParser;
     const int32_t s32_Result = c_XmlParser.LoadFromFile(orc_Path);
     if (s32_Result == C_NO_ERR) {
         if (c_XmlParser.SelectRoot() == mc_ROOT_NAME) {
             if (c_XmlParser.SelectNodeChild(mc_FILE_VERSION) == mc_FILE_VERSION) {
                 const QString c_FileVersion = c_XmlParser.GetNodeContent();
                 const uint32_t u32_FileVersion = c_FileVersion.toUInt();
                 c_XmlParser.SelectRoot();

                 if (u32_FileVersion == mu16_FILE_VERSION) {
                     if (c_XmlParser.SelectNodeChild(mc_NODE_SIG) == mc_NODE_SIG) {
                         const QString c_SigType =
                             c_XmlParser.GetAttributeString(mc_NODE_SIG_TYPE_ATTR);
                         if (c_SigType == mc_SIG_TYPE) {
                             orc_Signature = c_XmlParser.GetAttributeString(mc_NODE_SIG_VALUE_ATTR);
                             c_XmlParser.SelectRoot();
                             return C_NO_ERR;
                         }
                     }
                 }
             }
         }
     }
     return s32_Result;
 }
