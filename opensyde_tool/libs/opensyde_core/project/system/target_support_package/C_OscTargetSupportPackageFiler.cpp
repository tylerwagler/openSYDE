//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target support package reader/writer (Multi-Format Implementation)

   Load / save target support package data from / to binary, JSON, or XML files
   using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscTargetSupportPackageFiler.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include "C_OscLoggingHandler.hpp"

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
/*! \brief   Load target support package from file (auto-detect format)

   \param[out]     orc_Package      Package data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadFile(C_OscTargetSupportPackage &orc_Package,
                                                       const QString &orc_Path) {
   return mh_DetectAndLoad(orc_Package, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save target support package to file (auto-detect format from extension)

   \param[in]      orc_Package      Package data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_SaveFile(const C_OscTargetSupportPackage &orc_Package,
                                                       const QString &orc_Path) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Package, orc_Path);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Package, orc_Path);
   } else if (c_Extension == ".xml") {
      s32_Retval = h_SaveXml(orc_Package, orc_Path);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      s32_Retval = h_SaveXml(orc_Package, orc_Path);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from binary file

   \param[out]     orc_Package      Package data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadBinary(C_OscTargetSupportPackage &orc_Package,
                                                         const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading target support package",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Package.FromQDataStream(in);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to binary file

   \param[in]      orc_Package      Package data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_SaveBinary(const C_OscTargetSupportPackage &orc_Package,
                                                         const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving target support package",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Package.ToQDataStream(out);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Saving target support package",
                          "Failed to serialize package data to binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from memory (binary)

   \param[out]     orc_Package      Package data
   \param[in]      orc_Data         Binary data

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadFromMemoryBinary(C_OscTargetSupportPackage &orc_Package,
                                                                   const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Package.FromQDataStream(in);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from memory buffer.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to memory (binary)

   \param[in]      orc_Package      Package data to store

   \return
   QByteArray     Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscTargetSupportPackageFiler_New::h_SaveToMemoryBinary(
   const C_OscTargetSupportPackage &orc_Package) {
   QByteArray c_Data;
   QDataStream out(&c_Data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);

   orc_Package.ToQDataStream(out);

   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from JSON file

   \param[out]     orc_Package      Package data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadJson(C_OscTargetSupportPackage &orc_Package,
                                                       const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading target support package",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(file.readAll(), &c_ParseError);
   file.close();

   if (c_ParseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading target support package",
                          QString("JSON parse error: %1").arg(c_ParseError.errorString()));
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      osc_write_log_error("Loading target support package",
                          "JSON root element is not an object.");
      return C_CONFIG;
   }

   int32_t s32_Result = orc_Package.FromJsonObject(c_Doc.object());

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from JSON file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to JSON file

   \param[in]      orc_Package      Package data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_SaveJson(const C_OscTargetSupportPackage &orc_Package,
                                                       const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving target support package",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonObject c_Json = orc_Package.ToJsonObject();
   QJsonDocument c_Doc(c_Json);
   file.write(c_Doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from memory (JSON)

   \param[out]     orc_Package      Package data
   \param[in]      orc_Object       JSON object

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadFromMemoryJson(C_OscTargetSupportPackage &orc_Package,
                                                                 const QJsonObject &orc_Object) {
   int32_t s32_Result = orc_Package.FromJsonObject(orc_Object);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from JSON object.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to memory (JSON)

   \param[in]      orc_Package      Package data to store

   \return
   QJsonObject    JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscTargetSupportPackageFiler_New::h_SaveToMemoryJson(
   const C_OscTargetSupportPackage &orc_Package) {
   return orc_Package.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from XML file

   \param[out]     orc_Package      Package data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadXml(C_OscTargetSupportPackage &orc_Package,
                                                      const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading target support package",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QString c_Error;
   int i_Line, i_Column;

   if (!c_Doc.setContent(file.readAll(), &c_Error, &i_Line, &i_Column)) {
      osc_write_log_error("Loading target support package",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(i_Line).arg(i_Column).arg(c_Error));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement c_Root = c_Doc.documentElement();
   int32_t s32_Result = orc_Package.FromQDomElement(c_Root);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from XML file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to XML file

   \param[in]      orc_Package      Package data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_SaveXml(const C_OscTargetSupportPackage &orc_Package,
                                                      const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving target support package",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QDomElement c_Element = orc_Package.ToQDomDocument(c_Doc, "target-support-package");
   c_Doc.appendChild(c_Element);

   // Write with XML declaration
   file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   file.write(c_Doc.toString().toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package from memory (XML)

   \param[out]     orc_Package      Package data
   \param[in]      orc_Element      XML element

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadFromMemoryXml(C_OscTargetSupportPackage &orc_Package,
                                                                const QDomElement &orc_Element) {
   int32_t s32_Result = orc_Package.FromQDomElement(orc_Element);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading target support package",
                          "Failed to deserialize package data from XML element.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package to memory (XML)

   \param[in]      orc_Package      Package data to store
   \param[in,out]  orc_Doc          DOM document

   \return
   QDomElement    XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscTargetSupportPackageFiler_New::h_SaveToMemoryXml(
   const C_OscTargetSupportPackage &orc_Package, QDomDocument &orc_Doc) {
   return orc_Package.ToQDomDocument(orc_Doc, "target-support-package");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load package (legacy XML compatibility)

   \param[out]     orc_Package      Package data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::h_LoadPackage(C_OscTargetSupportPackage &orc_Package,
                                                          C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   return C_OscTargetSupportPackageFiler::h_LoadPackage(orc_Package, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save package (legacy XML compatibility)

   \param[in]      orc_Package      Package data to store
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageFiler_New::h_SavePackage(const C_OscTargetSupportPackage &orc_Package,
                                                       C_OscXmlParserBase &orc_XmlParser) {
   // Delegate to original implementation for backward compatibility
   C_OscTargetSupportPackageFiler::h_SavePackage(orc_Package, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect format and load from file

   \param[out]     orc_Package      Package data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackageFiler_New::mh_DetectAndLoad(C_OscTargetSupportPackage &orc_Package,
                                                             const QString &orc_Path) {
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_Package, orc_Path);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_Package, orc_Path);
   } else if (c_Extension == ".xml") {
      return h_LoadXml(orc_Package, orc_Path);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      return h_LoadXml(orc_Package, orc_Path);
   }
}
