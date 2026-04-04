//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bus reader/writer (Multi-Format Implementation)

   Load / save bus data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscSystemBusFiler.hpp"
#include "C_OscSystemBusFilerV2.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"

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
/*! \brief   Load bus from file (auto-detect format)

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadFile(C_OscSystemBus &orc_Bus,
                                            const QString &orc_Path) {
   return mh_DetectAndLoad(orc_Bus, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to file (auto-detect format from extension)

   \param[in]      orc_Bus          Bus data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_SaveFile(const C_OscSystemBus &orc_Bus,
                                            const QString &orc_Path) {
   int32_t s32_Retval = C_NO_ERR;

   // Detect format from file extension
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      s32_Retval = h_SaveBinary(orc_Bus, orc_Path);
   } else if (c_Extension == ".json") {
      s32_Retval = h_SaveJson(orc_Bus, orc_Path);
   } else if (c_Extension == ".xml" || c_Extension == ".dbc") {
      s32_Retval = h_SaveXml(orc_Bus, orc_Path);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      s32_Retval = h_SaveXml(orc_Bus, orc_Path);
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from binary file

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadBinary(C_OscSystemBus &orc_Bus,
                                              const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading bus definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream in(&file);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Bus.FromQDataStream(in);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to binary file

   \param[in]      orc_Bus          Bus data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_SaveBinary(const C_OscSystemBus &orc_Bus,
                                              const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving bus definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDataStream out(&file);
   out.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Bus.ToQDataStream(out);
   file.close();

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Saving bus definition",
                          "Failed to serialize bus data to binary file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from memory (binary)

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Data         Binary data

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadFromMemoryBinary(C_OscSystemBus &orc_Bus,
                                                        const QByteArray &orc_Data) {
   QDataStream in(orc_Data);
   in.setVersion(QDataStream::Qt_6_0);

   int32_t s32_Result = orc_Bus.FromQDataStream(in);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from memory buffer.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to memory (binary)

   \param[in]      orc_Bus          Bus data to store

   \return
   QByteArray    Binary data
*/
//----------------------------------------------------------------------------------------------------------------------
QByteArray C_OscSystemBusFiler::h_SaveToMemoryBinary(
   const C_OscSystemBus &orc_Bus) {
   QByteArray c_Data;
   QDataStream out(&c_Data, QIODevice::WriteOnly);
   out.setVersion(QDataStream::Qt_6_0);

   orc_Bus.ToQDataStream(out);

   return c_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from JSON file

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadJson(C_OscSystemBus &orc_Bus,
                                            const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading bus definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(file.readAll(), &c_ParseError);
   file.close();

   if (c_ParseError.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading bus definition",
                          QString("JSON parse error: %1").arg(c_ParseError.errorString()));
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      osc_write_log_error("Loading bus definition",
                          "JSON root element is not an object.");
      return C_CONFIG;
   }

   int32_t s32_Result = orc_Bus.FromJsonObject(c_Doc.object());

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from JSON file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to JSON file

   \param[in]      orc_Bus          Bus data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_SaveJson(const C_OscSystemBus &orc_Bus,
                                            const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving bus definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QJsonObject c_Json = orc_Bus.ToJsonObject();
   QJsonDocument c_Doc(c_Json);
   file.write(c_Doc.toJson(QJsonDocument::Indented));
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from memory (JSON)

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Object       JSON object

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadFromMemoryJson(C_OscSystemBus &orc_Bus,
                                                      const QJsonObject &orc_Object) {
   int32_t s32_Result = orc_Bus.FromJsonObject(orc_Object);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from JSON object.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to memory (JSON)

   \param[in]      orc_Bus          Bus data to store

   \return
   QJsonObject    JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscSystemBusFiler::h_SaveToMemoryJson(
   const C_OscSystemBus &orc_Bus) {
   return orc_Bus.ToJsonObject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from XML file

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadXml(C_OscSystemBus &orc_Bus,
                                           const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading bus definition",
                          QString("Could not open file \"%1\" for reading.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QString c_Error;
   int i_Line, i_Column;

   if (!c_Doc.setContent(file.readAll(), &c_Error, &i_Line, &i_Column)) {
      osc_write_log_error("Loading bus definition",
                          QString("XML parse error at line %1, column %2: %3")
                          .arg(i_Line).arg(i_Column).arg(c_Error));
      file.close();
      return C_CONFIG;
   }
   file.close();

   QDomElement c_Root = c_Doc.documentElement();
   int32_t s32_Result = orc_Bus.FromQDomElement(c_Root);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from XML file.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to XML file

   \param[in]      orc_Bus          Bus data to store
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data saved
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_SaveXml(const C_OscSystemBus &orc_Bus,
                                           const QString &orc_Path) {
   QFile file(orc_Path);
   if (!file.open(QIODevice::WriteOnly)) {
      osc_write_log_error("Saving bus definition",
                          QString("Could not open file \"%1\" for writing.")
                          .arg(orc_Path));
      return C_CONFIG;
   }

   QDomDocument c_Doc;
   QDomElement c_Element = orc_Bus.ToQDomDocument(c_Doc, "bus");
   c_Doc.appendChild(c_Element);

   // Write with XML declaration
   file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   file.write(c_Doc.toString().toUtf8());
   file.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from memory (XML)

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Element      XML element

   \return
   C_NO_ERR   data read
   C_CONFIG   content is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadFromMemoryXml(C_OscSystemBus &orc_Bus,
                                                     const QDomElement &orc_Element) {
   int32_t s32_Result = orc_Bus.FromQDomElement(orc_Element);

   if (s32_Result != C_NO_ERR) {
      osc_write_log_error("Loading bus definition",
                          "Failed to deserialize bus data from XML element.");
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to memory (XML)

   \param[in]      orc_Bus          Bus data to store
   \param[in,out]  orc_Doc          DOM document

   \return
   QDomElement    XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscSystemBusFiler::h_SaveToMemoryXml(
   const C_OscSystemBus &orc_Bus, QDomDocument &orc_Doc) {
   return orc_Bus.ToQDomDocument(orc_Doc, "bus");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus from legacy XML parser

   Delegates to C_OscSystemBusFilerV2 for parsing bus data from a C_OscXmlParserBase context.
   This is used by C_OscSystemDefinitionFiler where buses are embedded inline in the system definition XML.

   pre-condition: the passed XML parser has the active node set to "bus"
   post-condition: the passed XML parser has the active node set to the same "bus"

   \param[out]     orc_Bus          Bus data
   \param[in,out]  orc_XmlParser    XML parser with "bus" node active

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::h_LoadBus(C_OscSystemBus &orc_Bus,
                                       C_OscXmlParserBase &orc_XmlParser)
{
   return C_OscSystemBusFilerV2::h_LoadBus(orc_Bus, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus to legacy XML parser

   Delegates to C_OscSystemBusFilerV2 for writing bus data into a C_OscXmlParserBase context.
   This is used by C_OscSystemDefinitionFiler where buses are embedded inline in the system definition XML.

   pre-condition: the passed XML parser has the active node set to "bus"
   post-condition: the passed XML parser has the active node set to the same "bus"

   \param[in]      orc_Bus          Bus data to store
   \param[in,out]  orc_XmlParser    XML parser with "bus" node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemBusFiler::h_SaveBus(const C_OscSystemBus &orc_Bus,
                                    C_OscXmlParserBase &orc_XmlParser)
{
   C_OscSystemBusFilerV2::h_SaveBus(orc_Bus, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect format and load from file

   \param[out]     orc_Bus          Bus data
   \param[in]      orc_Path         File path

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBusFiler::mh_DetectAndLoad(C_OscSystemBus &orc_Bus,
                                                  const QString &orc_Path) {
   const QString c_Extension = orc_Path.right(4).toLower();

   if (c_Extension == ".bin") {
      return h_LoadBinary(orc_Bus, orc_Path);
   } else if (c_Extension == ".json") {
      return h_LoadJson(orc_Bus, orc_Path);
   } else if (c_Extension == ".xml" || c_Extension == ".dbc") {
      return h_LoadXml(orc_Bus, orc_Path);
   } else {
      // Default to XML for backward compatibility
      osc_write_log_warning("File I/O",
                            QString("Unknown file extension \"%1\" for \"%2\". "
                                    "Defaulting to XML format.")
                            .arg(c_Extension, orc_Path));
      return h_LoadXml(orc_Bus, orc_Path);
   }
}
