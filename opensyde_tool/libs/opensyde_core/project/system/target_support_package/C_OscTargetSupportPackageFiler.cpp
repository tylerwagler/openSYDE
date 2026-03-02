//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscTargetSupportPackage with Qt-native serialization
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscTargetSupportPackageFiler.hpp"
#include "C_OscTargetSupportPackage.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscTargetSupportPackageFiler::C_OscTargetSupportPackageFiler() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from file with auto-detection of format based on extension

   \param[out] orc_Config Configuration to load into
   \param[out] orc_NodePath Node path (output)
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_LoadFile(C_OscTargetSupportPackage &orc_Config,
                                                        QString &orc_NodePath,
                                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_LoadBinary(orc_Config, orc_NodePath, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_LoadJson(orc_Config, orc_NodePath, orc_Path);
   } else {
      // Default to XML
      return h_LoadXml(orc_Config, orc_NodePath, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to file with auto-detection of format based on extension

   \param[in] orc_Config Configuration to save
   \param[in] orc_NodePath Node path
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_SaveFile(const C_OscTargetSupportPackage &orc_Config,
                                                        const QString &orc_NodePath,
                                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_SaveBinary(orc_Config, orc_NodePath, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_SaveJson(orc_Config, orc_NodePath, orc_Path);
   } else {
      // Default to XML
      return h_SaveXml(orc_Config, orc_NodePath, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from binary file

   \param[out] orc_Config Configuration to load into
   \param[out] orc_NodePath Node path (output)
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_LoadBinary(C_OscTargetSupportPackage &orc_Config,
                                                          QString &orc_NodePath,
                                                          const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Read version
   uint16_t u16_Version = 0;
   c_Stream >> u16_Version;
   
   if (u16_Version != 1) {
      return C_CONFIG;
   }
   
   // Read node path
   c_Stream >> orc_NodePath;
   
   // Read target support package data
   int32_t s32_Result = orc_Config.FromQDataStream(c_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to binary file

   \param[in] orc_Config Configuration to save
   \param[in] orc_NodePath Node path
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_SaveBinary(const C_OscTargetSupportPackage &orc_Config,
                                                          const QString &orc_NodePath,
                                                          const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Write version
   uint16_t u16_Version = 1;
   c_Stream << u16_Version;
   
   // Write node path
   c_Stream << orc_NodePath;
   
   // Write target support package data
   int32_t s32_Result = orc_Config.ToQDataStream(c_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from JSON file

   \param[out] orc_Config Configuration to load into
   \param[out] orc_NodePath Node path (output)
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_LoadJson(C_OscTargetSupportPackage &orc_Config,
                                                        QString &orc_NodePath,
                                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
   if (c_Doc.isNull()) {
      return C_CONFIG;
   }
   
   if (!c_Doc.isObject()) {
      return C_CONFIG;
   }
   
   QJsonObject c_Object = c_Doc.object();
   
   // Read node path
   if (c_Object.contains("node-path")) {
      orc_NodePath = c_Object["node-path"].toString();
   }
   
   // Read target support package data
   if (c_Object.contains("target-support-package")) {
      int32_t s32_Result = orc_Config.FromJsonObject(c_Object["target-support-package"].toObject());
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to JSON file

   \param[in] orc_Config Configuration to save
   \param[in] orc_NodePath Node path
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_SaveJson(const C_OscTargetSupportPackage &orc_Config,
                                                        const QString &orc_NodePath,
                                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   QJsonObject c_Object;
   
   c_Object["node-path"] = orc_NodePath;
   c_Object["target-support-package"] = orc_Config.ToJsonObject();
   
   QJsonDocument c_Doc(c_Object);
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   c_File.write(c_Doc.toJson(QJsonDocument::Compact));
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from XML file

   \param[out] orc_Config Configuration to load into
   \param[out] orc_NodePath Node path (output)
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_LoadXml(C_OscTargetSupportPackage &orc_Config,
                                                       QString &orc_NodePath,
                                                       const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File)) {
      return C_CONFIG;
   }
   
   QDomElement c_RootElement = c_Doc.documentElement();
   if (c_RootElement.tagName() != "target-support-packages") {
      return C_CONFIG;
   }
   
   // Read node path
   QDomNode c_NodePathNode = c_RootElement.namedItem("node-path");
   if (!c_NodePathNode.isNull()) {
      orc_NodePath = c_NodePathNode.toElement().text();
   }
   
   // Read target support package data
   QDomNode c_TspNode = c_RootElement.namedItem("target-support-package");
   if (!c_TspNode.isNull()) {
      int32_t s32_Result = orc_Config.FromQDomElement(c_TspNode.toElement());
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to XML file

   \param[in] orc_Config Configuration to save
   \param[in] orc_NodePath Node path
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler::h_SaveXml(const C_OscTargetSupportPackage &orc_Config,
                                                       const QString &orc_NodePath,
                                                       const QString &orc_Path) {
   using namespace stw::errors;
   
   QDomDocument c_Doc("target-support-packages");
   QDomElement c_RootElement = c_Doc.createElement("target-support-packages");
   c_Doc.appendChild(c_RootElement);
   
   // Add node path
   QDomElement c_NodePathElement = c_Doc.createElement("node-path");
   c_NodePathElement.appendChild(c_Doc.createTextNode(orc_NodePath));
   c_RootElement.appendChild(c_NodePathElement);
   
   // Add target support package
   QDomElement c_TspElement = orc_Config.ToQDomDocument(c_Doc, "target-support-package");
   c_RootElement.appendChild(c_TspElement);
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);
   
   return C_NO_ERR;
}
