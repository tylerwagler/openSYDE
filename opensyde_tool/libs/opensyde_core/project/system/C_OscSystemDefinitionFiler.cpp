 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load/save system definition data from/to file with multi-format support (V3)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "precomp_headers.hpp"
 #include <QDir>
 #include <QFile>
 #include <QFileInfo>
 #include <fstream>

 #include "C_OscSystemDefinitionFiler.hpp"
 #include "C_OscSystemFilerUtil.hpp"
 #include "C_OscLoggingHandler.hpp"
 #include "C_OscUtils.hpp"
 #include "C_OscNodeSquadFiler.hpp"
 #include "C_OscNodeFiler.hpp"
 #include "C_OscSystemBusFiler.hpp"
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
 /*! \brief   Load system definition from file (auto-detects format)

    \param[out]     orc_SystemDefinition            Pointer to storage
    \param[in]      orc_PathSystemDefinition        Path to system definition
    \param[in]      orc_PathDeviceDefinitions       Path to device definitions
    \param[in]      oq_UseDeviceDefinitions         Flag for using device definitions
    \param[in,out]  opu16_ReadFileVersion           Optional storage for read file version

    \return
    C_NO_ERR    data read
    C_RANGE     specified system definition file does not exist
    C_NOACT     specified file is present but structure is invalid
    C_CONFIG    system definition file content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_LoadFile(
     C_OscSystemDefinition &orc_SystemDefinition,
     const QString &orc_PathSystemDefinition,
     const QString &orc_PathDeviceDefinitions,
     const bool oq_UseDeviceDefinitions, uint16_t *const opu16_ReadFileVersion) {
     int32_t s32_Retval = C_NO_ERR;

     if (QFileInfo(orc_PathSystemDefinition).exists() &&
         QFileInfo(orc_PathSystemDefinition).isFile()) {
         const QString c_Extension = QFileInfo(orc_PathSystemDefinition).suffix().toLower();

         if (c_Extension == "bin") {
             // Binary format
             std::ifstream c_File(orc_PathSystemDefinition.toLocal8Bit().constData(), std::ios::binary);
             if (c_File.is_open()) {
                 QDataStream c_Stream(&c_File);
                 c_Stream.setVersion(QDataStream::Qt_5_12);
                 s32_Retval = h_LoadBinary(orc_SystemDefinition, c_Stream);
                 c_File.close();
             } else {
                 osc_write_log_error("Loading System Definition",
                                     "File \"" + orc_PathSystemDefinition + "\" could not be opened for binary reading.");
                 s32_Retval = C_NOACT;
             }
         } else if (c_Extension == "json") {
             // JSON format
             QFile c_File(orc_PathSystemDefinition);
             if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
                 QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
                 c_File.close();
                 if (c_Doc.isObject()) {
                     s32_Retval = h_LoadJson(orc_SystemDefinition, c_Doc.object());
                 } else {
                     osc_write_log_error("Loading System Definition",
                                         "JSON file does not contain a valid object.");
                     s32_Retval = C_CONFIG;
                 }
             } else {
                 osc_write_log_error("Loading System Definition",
                                     "File \"" + orc_PathSystemDefinition + "\" could not be opened for JSON reading.");
                 s32_Retval = C_NOACT;
             }
         } else {
             // XML format (default)
             C_OscXmlParserLog c_XmlParser;
             c_XmlParser.SetLogHeading("Loading System Definition");
             s32_Retval = c_XmlParser.LoadFromFile(orc_PathSystemDefinition);
             if (s32_Retval == C_NO_ERR) {
                 s32_Retval = h_LoadXml(orc_SystemDefinition, c_XmlParser);
             } else {
                 osc_write_log_error("Loading System Definition",
                                     "File \"" + orc_PathSystemDefinition + "\" could not be parsed as XML.");
                 s32_Retval = C_NOACT;
             }
         }

         // Save file string for reference
         if (s32_Retval == C_NO_ERR) {
             C_OscUtils::h_FileToString(orc_PathSystemDefinition, orc_SystemDefinition.c_FileString);
         }
     } else {
         osc_write_log_error("Loading System Definition",
                             "File \"" + orc_PathSystemDefinition + "\" does not exist.");
         s32_Retval = C_RANGE;
     }

     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save system definition to file (auto-detects format from extension)

    \param[in]      orc_SystemDefinition   Pointer to storage
    \param[in]      orc_Path               Path of system definition
    \param[in,out]  opc_CreatedFiles       Optional storage for history of created files

    \return
    C_NO_ERR   data saved
    C_RD_WR    could not write to file
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_SaveFile(
     const C_OscSystemDefinition &orc_SystemDefinition,
     const QString &orc_Path,
     QStringList *const opc_CreatedFiles) {
     const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

     if (c_Extension == "bin") {
         // Binary format
         QFile c_File(orc_Path);
         if (c_File.open(QIODevice::WriteOnly)) {
             QDataStream c_Stream(&c_File);
             c_Stream.setVersion(QDataStream::Qt_5_12);
             const int32_t s32_Result = h_SaveBinary(orc_SystemDefinition, c_Stream);
             c_File.close();
             return s32_Result;
         } else {
             osc_write_log_error("Saving System Definition",
                                 "File \"" + orc_Path + "\" could not be opened for binary writing.");
             return C_RD_WR;
         }
     } else if (c_Extension == "json") {
         // JSON format
         QJsonObject c_Object;
         const int32_t s32_Result = h_SaveJson(orc_SystemDefinition, c_Object);
         if (s32_Result == C_NO_ERR) {
             QJsonDocument c_Doc(c_Object);
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
                 c_File.write(c_Doc.toJson(QJsonDocument::Indented));
                 c_File.close();
                 if (opc_CreatedFiles) {
                     opc_CreatedFiles->append(orc_Path);
                 }
             } else {
                 osc_write_log_error("Saving System Definition",
                                     "File \"" + orc_Path + "\" could not be opened for JSON writing.");
                 return C_RD_WR;
             }
         }
         return s32_Result;
     } else {
         // XML format (default)
         C_OscXmlParserLog c_XmlParser;
         const int32_t s32_Result = h_SaveXml(orc_SystemDefinition, c_XmlParser);
         if (s32_Result == C_NO_ERR) {
             const int32_t s32_SaveResult = C_OscSystemFilerUtil::h_SaveStringToFile(
                 c_XmlParser.GetContent(), orc_Path, "Saving System Definition");
             if (s32_SaveResult == C_NO_ERR && opc_CreatedFiles) {
                 opc_CreatedFiles->append(orc_Path);
             }
             return s32_SaveResult;
         }
         return s32_Result;
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load system definition from binary stream

    \param[out]     orc_SystemDefinition   Pointer to storage
    \param[in,out]  orc_Stream             Binary stream

    \return
    C_NO_ERR    no error
    C_CONFIG    content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_LoadBinary(
     C_OscSystemDefinition &orc_SystemDefinition,
     QDataStream &orc_Stream) {
     return orc_SystemDefinition.FromQDataStream(orc_Stream);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load system definition from JSON object

    \param[out]     orc_SystemDefinition   Pointer to storage
    \param[in]      orc_Object             JSON object

    \return
    C_NO_ERR    no error
    C_CONFIG    content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_LoadJson(
     C_OscSystemDefinition &orc_SystemDefinition,
     const QJsonObject &orc_Object) {
     return orc_SystemDefinition.FromJsonObject(orc_Object);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load system definition from XML parser

    \param[out]     orc_SystemDefinition   Pointer to storage
    \param[in,out]  orc_XmlParser          XML parser

    \return
    C_NO_ERR    no error
    C_CONFIG    content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_LoadXml(
     C_OscSystemDefinition &orc_SystemDefinition,
     C_OscXmlParserBase &orc_XmlParser) {
     return orc_SystemDefinition.FromQDomElement(
         orc_XmlParser.GetRootElement());
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save system definition to binary stream

    \param[in]      orc_SystemDefinition   System definition to save
    \param[in,out]  orc_Stream             Binary stream

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_SaveBinary(
     const C_OscSystemDefinition &orc_SystemDefinition,
     QDataStream &orc_Stream) {
     return orc_SystemDefinition.ToQDataStream(orc_Stream);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save system definition to JSON object

    \param[in]      orc_SystemDefinition   System definition to save
    \param[out]     orc_Object             JSON object

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_SaveJson(
     const C_OscSystemDefinition &orc_SystemDefinition,
     QJsonObject &orc_Object) {
     orc_Object = orc_SystemDefinition.ToJsonObject();
     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save system definition to XML parser

    \param[in]      orc_SystemDefinition   System definition to save
    \param[in,out]  orc_XmlParser          XML parser

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSystemDefinitionFiler_New::h_SaveXml(
     const C_OscSystemDefinition &orc_SystemDefinition,
     C_OscXmlParserBase &orc_XmlParser) {
     QDomElement c_Element = orc_SystemDefinition.ToQDomDocument(
         orc_XmlParser.GetDocument(), "opensyde-system-definition");

     // Replace root element if needed
     QDomDocument &rc_Doc = orc_XmlParser.GetDocument();
     if (rc_Doc.documentElement().isNull()) {
         rc_Doc.appendChild(c_Element);
     } else {
         rc_Doc.replaceChild(c_Element, rc_Doc.documentElement());
     }

     return C_NO_ERR;
 }



//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Split device type string into main and sub type

   \param[in]      orc_CompleteType   Complete device type string
   \param[out]     orc_MainType       Main type (output)
   \param[out]     orc_SubType        Sub type (output)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemDefinitionFiler_New::h_SplitDeviceType(const QString &orc_CompleteType,
                                                       QString &orc_MainType,
                                                       QString &orc_SubType) {
    const int32_t s32_Pos = orc_CompleteType.indexOf(C_OscNodeSquad::hc_SEPARATOR);

    if (s32_Pos != -1) {
        orc_MainType = orc_CompleteType.left(s32_Pos);
        orc_SubType = orc_CompleteType.mid(s32_Pos + C_OscNodeSquad::hc_SEPARATOR.length());
    } else {
        orc_MainType = "";
        orc_SubType = orc_CompleteType;
    }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load nodes from XML parser (clipboard support)

   \param[out]     orc_Nodes                Node list
   \param[in]      orc_XmlParser            XML parser
   \param[in]      orc_DeviceDefinitions    Device definitions
   \param[in]      orc_BasePath             Base path
   \param[in]      oq_UseDeviceDefinitions  Use device definitions
   \param[in]      oq_UseFileInterface      Use file interface
   \param[in]      opc_NodesToLoad          Nodes to load
   \param[in]      oq_SkipContent           Skip content
   \param[in]      opc_ExpectedNodeName     Expected node name
   \param[out]     opc_ErrorDetailsMissingDevices  Error details

   \return
   C_NO_ERR   data read
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemDefinitionFiler_New::h_LoadNodes(QList<C_OscNode> &orc_Nodes,
                                                    C_OscXmlParserBase &orc_XmlParser,
                                                    const C_OscDeviceManager &orc_DeviceDefinitions,
                                                    const QString &orc_BasePath,
                                                    const bool oq_UseDeviceDefinitions,
                                                    const bool oq_UseFileInterface,
                                                    const QByteArray *const opc_NodesToLoad,
                                                    const bool oq_SkipContent,
                                                    const QString *const opc_ExpectedNodeName,
                                                    QStringList *const opc_ErrorDetailsMissingDevices) {
    // Delegate to legacy filer for clipboard operations
    return C_OscSystemDefinitionFiler::h_LoadNodes(orc_Nodes, orc_XmlParser,
                                                    orc_DeviceDefinitions, orc_BasePath,
                                                    oq_UseDeviceDefinitions, oq_UseFileInterface,
                                                    opc_NodesToLoad, oq_SkipContent,
                                                    opc_ExpectedNodeName, opc_ErrorDetailsMissingDevices);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load buses from XML parser (clipboard support)

   \param[out]     orc_Buses      Bus list
   \param[in]      orc_XmlParser  XML parser

   \return
   C_NO_ERR   data read
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemDefinitionFiler_New::h_LoadBuses(QList<C_OscSystemBus> &orc_Buses,
                                                    C_OscXmlParserBase &orc_XmlParser) {
    // Delegate to legacy filer for clipboard operations
    return C_OscSystemDefinitionFiler::h_LoadBuses(orc_Buses, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save nodes to XML parser (clipboard support)

   \param[in]      orc_Nodes          Node list
   \param[in]      orc_XmlParser      XML parser
   \param[in]      orc_BasePath       Base path
   \param[out]     opc_CreatedFiles   Created files
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemDefinitionFiler_New::h_SaveNodes(const QList<C_OscNode> &orc_Nodes,
                                                    C_OscXmlParserBase &orc_XmlParser,
                                                    const QString &orc_BasePath,
                                                    QStringList *const opc_CreatedFiles) {
    // Delegate to legacy filer for clipboard operations
    C_OscSystemDefinitionFiler::h_SaveNodes(orc_Nodes, orc_XmlParser, orc_BasePath, opc_CreatedFiles);
    return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save buses to XML parser (clipboard support)

   \param[in]      orc_Buses      Bus list
   \param[in]      orc_XmlParser  XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemDefinitionFiler_New::h_SaveBuses(const QList<C_OscSystemBus> &orc_Buses,
                                                 C_OscXmlParserBase &orc_XmlParser) {
    // Delegate to legacy filer for clipboard operations
    C_OscSystemDefinitionFiler::h_SaveBuses(orc_Buses, orc_XmlParser);
}
