 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle project save and load with multi-format support

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
#include "precomp_headers.hpp"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>

 #include "C_OscProjectFiler.hpp"
 #include "C_OscLoggingHandler.hpp"
 #include "C_OscUtils.hpp"
 #include "C_OscXmlParser.hpp"
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
 /*! \brief   Save project to file (auto-detects format)

    Save project data to file.
    The target path must exists.
    A potentially pre-existing file will be erased.

    Before saving the:
    - Author will be overwritten with the name of the user logged into the PC
    system
    - ModificationTime will be overwritten with the current time

    \param[in,out]  orc_Project         Project data to save (Author and
    ModificationTime will be updated) \param[in]      orc_Path            Path of
    project \param[in]      orc_OpenSydeVersion Current openSYDE version

    \return
    C_NO_ERR   data saved
    C_RD_WR    problems accessing file system
    C_RANGE    orc_Path is empty
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_Save(C_OscProject &orc_Project,
                                       const QString &orc_Path,
                                       const QString &orc_OpenSydeVersion) {
     int32_t s32_Retval = C_NO_ERR;

     // Check if file was specified
     if (orc_Path == "") {
         s32_Retval = C_RANGE;
     } else {
         bool q_NewFile = true;
         // erase file if it already exists:
         const QFileInfo c_FileInfo(orc_Path);
         if (c_FileInfo.exists() && c_FileInfo.isFile()) {
             // erase it:
             int x_Return;
             x_Return = std::remove(orc_Path.toLocal8Bit().constData());
             if (x_Return != 0) {
                 osc_write_log_error("Saving project file",
                                     "Could not erase pre-existing file \"" + orc_Path +
                                         "\".");
                 s32_Retval = C_RD_WR;
             }
             q_NewFile = false;
         }

         // Determine format by extension
         if (s32_Retval == C_NO_ERR) {
             const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

             if (c_Extension == "bin") {
                 // Binary format
                 QFile c_File(orc_Path);
                 if (c_File.open(QIODevice::WriteOnly)) {
                     QDataStream c_Stream(&c_File);
                     c_Stream.setVersion(QDataStream::Qt_5_12);
                     s32_Retval = h_SaveBinary(orc_Project, orc_Path,
                                               orc_OpenSydeVersion, c_Stream);
                     c_File.close();
                 } else {
                     osc_write_log_error("Saving project file",
                                         "File \"" + orc_Path +
                                             "\" could not be opened for binary writing.");
                     s32_Retval = C_RD_WR;
                 }
             } else if (c_Extension == "json") {
                 // JSON format
                 QJsonObject c_Object;
                 const int32_t s32_Result = h_SaveJson(orc_Project, orc_Path,
                                                       orc_OpenSydeVersion, c_Object);
                 if (s32_Result == C_NO_ERR) {
                     QJsonDocument c_Doc(c_Object);
                     QFile c_File(orc_Path);
                     if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
                         c_File.write(c_Doc.toJson(QJsonDocument::Indented));
                         c_File.close();
                     } else {
                         osc_write_log_error("Saving project file",
                                             "File \"" + orc_Path +
                                                 "\" could not be opened for JSON writing.");
                         s32_Retval = C_RD_WR;
                     }
                 } else {
                     s32_Retval = s32_Result;
                 }
             } else {
                 // XML format (default)
                 C_OscXmlParser c_Xml;
                 s32_Retval = h_SaveXml(orc_Project, orc_Path, orc_OpenSydeVersion, c_Xml);
                 if (s32_Retval == C_NO_ERR) {
                     s32_Retval = c_Xml.SaveToFile(orc_Path);
                 }
             }

             if (s32_Retval != C_NO_ERR) {
                 osc_write_log_error("Saving project file",
                                     "Could not write to file \"" + orc_Path + "\".");
                 s32_Retval = C_RD_WR;
             }
         }
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load project from file (auto-detects format)

    Load project data from file.

    \param[in,out] orc_Project Project data
    \param[in]     orc_Path    Path of project

    \return
    C_NO_ERR   data read
    C_RANGE    specified file does not exist
    C_NOACT    specified file is present but structure is invalid
    C_CONFIG   content of file is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_Load(C_OscProject &orc_Project,
                                       const QString &orc_Path) {
     int32_t s32_Retval;

     const QFileInfo c_FileInfo(orc_Path);
     if (c_FileInfo.exists() && c_FileInfo.isFile()) {
         const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

         if (c_Extension == "bin") {
             // Binary format
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::ReadOnly)) {
                 QDataStream c_Stream(&c_File);
                 c_Stream.setVersion(QDataStream::Qt_5_12);
                 s32_Retval = h_LoadBinary(orc_Project, c_Stream);
                 c_File.close();
             } else {
                 osc_write_log_error("Loading project file",
                                     "File \"" + orc_Path +
                                         "\" could not be opened for binary reading.");
                 s32_Retval = C_NOACT;
             }
         } else if (c_Extension == "json") {
             // JSON format
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
                 QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
                 c_File.close();
                 if (c_Doc.isObject()) {
                     s32_Retval = h_LoadJson(orc_Project, c_Doc.object());
                 } else {
                     osc_write_log_error("Loading project file",
                                         "JSON file does not contain a valid object.");
                     s32_Retval = C_CONFIG;
                 }
             } else {
                 osc_write_log_error("Loading project file",
                                     "File \"" + orc_Path +
                                         "\" could not be opened for JSON reading.");
                 s32_Retval = C_NOACT;
             }
         } else {
             // XML format (default)
             C_OscXmlParser c_Xml;
             s32_Retval = c_Xml.LoadFromFile(orc_Path);
             if (s32_Retval == C_NO_ERR) {
                 s32_Retval = h_LoadXml(orc_Project, c_Xml);
             } else {
                 osc_write_log_error("Loading project file",
                                     "File \"" + orc_Path +
                                         "\" could not be parsed as XML.");
                 s32_Retval = C_NOACT;
             }
         }
     } else {
         osc_write_log_error("Loading project file",
                             "File does not exist \"" + orc_Path + "\".");
         s32_Retval = C_RANGE;
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load project from binary stream

    \param[in,out]  orc_Project    Project data
    \param[in,out]  orc_Stream     Binary stream

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_LoadBinary(C_OscProject &orc_Project,
                                             QDataStream &orc_Stream) {
     // Read all fields
     orc_Stream >> orc_Project.c_Author;
     orc_Stream >> orc_Project.c_Editor;
     orc_Stream >> orc_Project.c_Version;
     orc_Stream >> orc_Project.c_OpenSydeVersion;
     orc_Stream >> orc_Project.c_Template;

     // Read timestamps (stored as string representations)
     QString c_CreationTimeStr;
     QString c_ModificationTimeStr;
     orc_Stream >> c_CreationTimeStr;
     orc_Stream >> c_ModificationTimeStr;

     orc_Project.c_CreationTime = C_OscProject::h_GetTimeOfString(c_CreationTimeStr);
     orc_Project.c_ModificationTime = C_OscProject::h_GetTimeOfString(c_ModificationTimeStr);

     return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load project from JSON object

    \param[in,out]  orc_Project    Project data
    \param[in]      orc_Object     JSON object

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_LoadJson(C_OscProject &orc_Project,
                                           const QJsonObject &orc_Object) {
     if (!orc_Object.contains("author") || !orc_Object.contains("editor") ||
         !orc_Object.contains("version") || !orc_Object.contains("openSYDE_version")) {
         return C_CONFIG;
     }

     orc_Project.c_Author = orc_Object["author"].toString();
     orc_Project.c_Editor = orc_Object["editor"].toString();
     orc_Project.c_Version = orc_Object["version"].toString();
     orc_Project.c_OpenSydeVersion = orc_Object["openSYDE_version"].toString();
     orc_Project.c_Template = orc_Object["template"].toString();

     // Parse timestamps
     if (orc_Object.contains("creation_time")) {
         orc_Project.c_CreationTime = C_OscProject::h_GetTimeOfString(
             orc_Object["creation_time"].toString());
     }
     if (orc_Object.contains("modification_time")) {
         orc_Project.c_ModificationTime = C_OscProject::h_GetTimeOfString(
             orc_Object["modification_time"].toString());
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load project from XML parser

    \param[in,out]  orc_Project    Project data
    \param[in,out]  orc_XmlParser  XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_LoadXml(C_OscProject &orc_Project,
                                          C_OscXmlParserBase &orc_XmlParser) {
     QString c_Tmp;

     // Check if file and root node exists
     if (orc_XmlParser.SelectRoot() != "Project") {
         osc_write_log_error("Loading project file",
                             "XML node \"Project\" not found.");
         return C_CONFIG;
     }

     // Author & Editor
     c_Tmp = orc_XmlParser.GetAttributeString("author");
     if (c_Tmp == "") {
         stw::opensyde_core::C_OscUtils::h_GetSystemUserName(c_Tmp);
     }
     orc_Project.c_Author = c_Tmp;

     c_Tmp = orc_XmlParser.GetAttributeString("editor");
     if (c_Tmp == "") {
         // use author if last editor is empty
         c_Tmp = orc_Project.c_Author;
     }
     orc_Project.c_Editor = c_Tmp;

     // Time
     orc_Project.c_CreationTime = C_OscProject::h_GetTimeOfString(
         orc_XmlParser.GetAttributeString("creation_time"));

     orc_Project.c_ModificationTime = C_OscProject::h_GetTimeOfString(
         orc_XmlParser.GetAttributeString("modification_time"));

     orc_Project.c_OpenSydeVersion = orc_XmlParser.GetAttributeString("openSYDE_version");
     orc_Project.c_Template = orc_XmlParser.GetAttributeString("template");

     // Check Version
     if (orc_XmlParser.SelectNodeChild("Version") == "Version") {
         orc_Project.c_Version = orc_XmlParser.GetNodeContent();
         orc_XmlParser.SelectNodeParent();
     } else {
         osc_write_log_error("Loading project file",
                             "XML node \"Version\" not found.");
         return C_CONFIG;
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save project to binary stream

    \param[in,out]  orc_Project        Project data
    \param[in]      orc_Path           Path (not used for binary, but kept for API consistency)
    \param[in]      orc_OpenSydeVersion OpenSYDE version
    \param[in,out]  orc_Stream         Binary stream

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_SaveBinary(C_OscProject &orc_Project,
                                             const QString &orc_Path,
                                             const QString &orc_OpenSydeVersion,
                                             QDataStream &orc_Stream) {
     Q_UNUSED(orc_Path);

     // Update metadata
     stw::opensyde_core::C_OscUtils::h_GetSystemUserName(orc_Project.c_Editor);
     orc_Project.c_ModificationTime = QDateTime::currentDateTime();
     orc_Project.c_OpenSydeVersion = orc_OpenSydeVersion;

     // Write all fields
     orc_Stream << orc_Project.c_Author;
     orc_Stream << orc_Project.c_Editor;
     orc_Stream << orc_Project.c_Version;
     orc_Stream << orc_Project.c_OpenSydeVersion;
     orc_Stream << orc_Project.c_Template;

     // Write timestamps as strings
     orc_Stream << C_OscProject::h_GetTimeFormatted(orc_Project.c_CreationTime);
     orc_Stream << C_OscProject::h_GetTimeFormatted(orc_Project.c_ModificationTime);

     return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save project to JSON object

    \param[in,out]  orc_Project        Project data
    \param[in]      orc_Path           Path (not used for JSON, but kept for API consistency)
    \param[in]      orc_OpenSydeVersion OpenSYDE version
    \param[out]     orc_Object         JSON object

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_SaveJson(C_OscProject &orc_Project,
                                           const QString &orc_Path,
                                           const QString &orc_OpenSydeVersion,
                                           QJsonObject &orc_Object) {
     Q_UNUSED(orc_Path);

     // Update metadata
     stw::opensyde_core::C_OscUtils::h_GetSystemUserName(orc_Project.c_Editor);
     orc_Project.c_ModificationTime = QDateTime::currentDateTime();
     orc_Project.c_OpenSydeVersion = orc_OpenSydeVersion;

     // Convert to JSON
     orc_Object["author"] = orc_Project.c_Author;
     orc_Object["editor"] = orc_Project.c_Editor;
     orc_Object["version"] = orc_Project.c_Version;
     orc_Object["openSYDE_version"] = orc_Project.c_OpenSydeVersion;
     orc_Object["template"] = orc_Project.c_Template;
     orc_Object["creation_time"] = C_OscProject::h_GetTimeFormatted(orc_Project.c_CreationTime);
     orc_Object["modification_time"] = C_OscProject::h_GetTimeFormatted(orc_Project.c_ModificationTime);

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save project to XML parser

    \param[in,out]  orc_Project        Project data
    \param[in]      orc_Path           Path (not used for XML, but kept for API consistency)
    \param[in]      orc_OpenSydeVersion OpenSYDE version
    \param[in,out]  orc_XmlParser      XML parser

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_SaveXml(C_OscProject &orc_Project,
                                          const QString &orc_Path,
                                          const QString &orc_OpenSydeVersion,
                                          C_OscXmlParserBase &orc_XmlParser) {
     Q_UNUSED(orc_Path);

     // Update metadata
     stw::opensyde_core::C_OscUtils::h_GetSystemUserName(orc_Project.c_Editor);
     orc_Project.c_ModificationTime = QDateTime::currentDateTime();
     orc_Project.c_OpenSydeVersion = orc_OpenSydeVersion;

     // create root:
     orc_XmlParser.CreateNodeChild("Project");
     orc_XmlParser.SelectRoot();

     // author
     orc_XmlParser.SetAttributeString("author", orc_Project.c_Author);

     // editor
     orc_XmlParser.SetAttributeString("editor", orc_Project.c_Editor);

     // Creation
     orc_XmlParser.SetAttributeString("creation_time",
                                      C_OscProject::h_GetTimeFormatted(orc_Project.c_CreationTime));

     // modification
     orc_XmlParser.SetAttributeString("modification_time",
                                      C_OscProject::h_GetTimeFormatted(orc_Project.c_ModificationTime));

     // openSYDE version
     orc_XmlParser.SetAttributeString("openSYDE_version", orc_Project.c_OpenSydeVersion);

     // Template
     orc_XmlParser.SetAttributeString("template", orc_Project.c_Template);

     // update version
     orc_XmlParser.CreateAndSelectNodeChild("Version");
     orc_XmlParser.SetNodeContent(orc_Project.c_Version);
     orc_XmlParser.SelectNodeParent();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save project (legacy compatibility - XML only)

    \see h_Save
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_SaveLegacy(C_OscProject &orc_Project,
                                             const QString &orc_Path,
                                             const QString &orc_OpenSydeVersion) {
     return mh_SaveInternal(orc_Project, orc_Path, orc_OpenSydeVersion, false);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Load project (legacy compatibility - XML only)

    \see h_Load
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::h_LoadLegacy(C_OscProject &orc_Project,
                                             const QString &orc_Path) {
     C_OscXmlParser c_Xml;
     const int32_t s32_Result = c_Xml.LoadFromFile(orc_Path);
     if (s32_Result == C_NO_ERR) {
         return h_LoadXml(orc_Project, c_Xml);
     }
     return s32_Result;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief   Save project internal

    Warning: no error handling (job of caller)

    \param[in,out]  orc_Project         Project data to save
    \param[in]      orc_Path            Path of project
    \param[in]      orc_OpenSydeVersion Current openSYDE version
    \param[in]      oq_New              Flag if file is new

    \return
    C_NO_ERR   data was written
    C_NOACT    could not write to file
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscProjectFiler::mh_SaveInternal(C_OscProject &orc_Project,
                                                const QString &orc_Path,
                                                const QString &orc_OpenSydeVersion,
                                                const bool oq_New) {
     QString c_Tmp;
     // Open file
     C_OscXmlParser c_Xml;
     int32_t s32_Return;

     // create root:
     c_Xml.CreateNodeChild("Project");
     c_Xml.SelectRoot();

     // author
     if (oq_New == true) {
         stw::opensyde_core::C_OscUtils::h_GetSystemUserName(c_Tmp);
         orc_Project.c_Author = c_Tmp;
     }
     c_Xml.SetAttributeString("author", orc_Project.c_Author);

     // editor
     stw::opensyde_core::C_OscUtils::h_GetSystemUserName(c_Tmp);
     orc_Project.c_Editor = c_Tmp;
     c_Xml.SetAttributeString("editor", orc_Project.c_Editor);

     // Creation
     if (oq_New == true) {
         orc_Project.c_CreationTime = QDateTime::currentDateTime();
     }
     c_Xml.SetAttributeString("creation_time", C_OscProject::h_GetTimeFormatted(
                                                   orc_Project.c_CreationTime));

     // modification
     orc_Project.c_ModificationTime = QDateTime::currentDateTime();
     c_Xml.SetAttributeString(
         "modification_time",
         C_OscProject::h_GetTimeFormatted(orc_Project.c_ModificationTime));

     // openSYDE version
     orc_Project.c_OpenSydeVersion = orc_OpenSydeVersion;
     c_Xml.SetAttributeString("openSYDE_version", orc_Project.c_OpenSydeVersion);

     // Template
     c_Xml.SetAttributeString("template", orc_Project.c_Template);

     // update version
     c_Xml.CreateAndSelectNodeChild("Version");
     c_Xml.SetNodeContent(orc_Project.c_Version);
     c_Xml.SelectNodeParent();

     s32_Return = c_Xml.SaveToFile(orc_Path);
     return s32_Return;
 }
