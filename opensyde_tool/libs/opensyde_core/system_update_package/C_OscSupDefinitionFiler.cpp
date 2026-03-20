 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System update package definition filer with multi-format support

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "precomp_headers.hpp"
 #include <QFile>
 #include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
 #include <fstream>

 #include "C_OscSupDefinitionFiler.hpp"
 #include "C_OscSupNodeDefinitionFiler.hpp"
 #include "C_OscXmlParser.hpp"
 #include "C_OscUtils.hpp"
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
 const QString C_OscSupDefinitionFiler_New::hc_PACKAGE_UPDATE_DEF =
     "service_update_package.syde_supdef";
 static const uint16_t mu16_FILE_VERSION = 2U;
 static const QString mc_FILE_VERSION = "file-version";
 static const QString mc_BUS_INDEX = "bus-index-client";
 static const QString mc_ROOT_NAME = "opensyde-updatepack-definition";
 static const QString mc_NODES = "nodes";
 static const QString mc_NODE = "node";
 static const QString mc_NODE_ACTIVE_ATTR = "active";
 static const QString mc_NODE_POSITION_ATTR = "position";
 static const QString mc_NODE_UPDATE = "update_package";
 static const QString mc_NODE_FILE_ATTR = "file";

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
 /*! \brief   Creates update package definition file (auto-detects format)

    \param[in]  orc_Path             destination path
    \param[in]  orc_SupDefContent    content to write
    \param[in]  orc_Files            Files

    \return
    C_NO_ERR    success
    C_RD_WR     read/write error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_CreateUpdatePackageDefFile(
     const QString &orc_Path,
     const C_OscSupDefinition &orc_SupDefContent,
     const QStringList &orc_Files) {
     const QString c_FileName =
         stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_Path) +
         hc_PACKAGE_UPDATE_DEF;
     const QString c_Extension = QFileInfo(c_FileName).suffix().toLower();

     if (c_Extension == "bin") {
         return h_SaveBinary(c_FileName, orc_SupDefContent, orc_Files);
     } else if (c_Extension == "json") {
         return h_SaveJson(c_FileName, orc_SupDefContent, orc_Files);
     } else {
         // XML default
         return h_SaveXml(c_FileName, orc_SupDefContent, orc_Files);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load update package definition file (auto-detects format)

    \param[in]      orc_TargetUnzipPath    Target unzip path
    \param[in]      oq_IsZip               Is zip
    \param[in]      orc_PackagePath        Package path
    \param[out]     oru32_FileVersion      File version
    \param[in,out]  orc_FilePackagePath    File package path
    \param[out]     oru32_ActiveBusIndex   Active bus index
    \param[in,out]  orc_ActiveNodes        Active nodes
    \param[in,out]  orc_UpdatePosition     Update position
    \param[in,out]  orc_PackageFiles       Package files

    \return
    STW error codes
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_LoadUpdatePackageDefFile(
     const QString &orc_TargetUnzipPath,
     const bool oq_IsZip,
     const QString &orc_PackagePath,
     uint32_t &oru32_FileVersion,
     QString &orc_FilePackagePath,
     uint32_t &oru32_ActiveBusIndex,
     QByteArray &orc_ActiveNodes,
     QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_PackageFiles) {
     QString c_Path;
     if (oq_IsZip) {
         c_Path = orc_TargetUnzipPath + C_OscSupDefinitionFiler_New::hc_PACKAGE_UPDATE_DEF;
     } else {
         orc_FilePackagePath =
             stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_PackagePath);
         c_Path = orc_FilePackagePath + C_OscSupDefinitionFiler_New::hc_PACKAGE_UPDATE_DEF;
     }

     const QString c_Extension = QFileInfo(c_Path).suffix().toLower();

     if (c_Extension == "bin") {
         return h_LoadBinary(c_Path, oru32_FileVersion, orc_FilePackagePath,
                            oru32_ActiveBusIndex, orc_ActiveNodes,
                            orc_UpdatePosition, orc_PackageFiles);
     } else if (c_Extension == "json") {
         return h_LoadJson(c_Path, oru32_FileVersion, orc_FilePackagePath,
                          oru32_ActiveBusIndex, orc_ActiveNodes,
                          orc_UpdatePosition, orc_PackageFiles);
     } else {
         // XML default
         return h_LoadXml(c_Path, oru32_FileVersion, orc_FilePackagePath,
                         oru32_ActiveBusIndex, orc_ActiveNodes,
                         orc_UpdatePosition, orc_PackageFiles);
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load from binary format

    \param[in]      orc_Path               Path to file
    \param[out]     oru32_FileVersion      File version
    \param[in,out]  orc_FilePackagePath    File package path
    \param[out]     oru32_ActiveBusIndex   Active bus index
    \param[in,out]  orc_ActiveNodes        Active nodes
    \param[in,out]  orc_UpdatePosition     Update position
    \param[in,out]  orc_PackageFiles       Package files

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_LoadBinary(
     const QString &orc_Path,
     uint32_t &oru32_FileVersion,
     QString &orc_FilePackagePath,
     uint32_t &oru32_ActiveBusIndex,
     QByteArray &orc_ActiveNodes,
     QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_PackageFiles) {
     QFile c_File(orc_Path);
     if (!c_File.open(QIODevice::ReadOnly)) {
         return C_RD_WR;
     }

     QDataStream c_Stream(&c_File);
     c_Stream.setVersion(QDataStream::Qt_5_12);

     // Read file version
     c_Stream >> oru32_FileVersion;
     if (c_Stream.status() != QDataStream::Ok) {
         c_File.close();
         return C_CONFIG;
     }

     // Check version
     if (oru32_FileVersion != mu16_FILE_VERSION) {
         c_File.close();
         return C_CONFIG;
     }

     // Read bus index
     c_Stream >> oru32_ActiveBusIndex;

     // Read active nodes
     quint32 u32_NodeCount = 0;
     c_Stream >> u32_NodeCount;
     orc_ActiveNodes.resize(u32_NodeCount);
     c_Stream.readRawData(reinterpret_cast<char*>(orc_ActiveNodes.data()), u32_NodeCount);

     // Read update positions
     c_Stream >> u32_NodeCount;
     orc_UpdatePosition.clear();
     orc_UpdatePosition.reserve(u32_NodeCount);
     for (quint32 i = 0; i < u32_NodeCount; ++i) {
         uint32_t u32_Pos = 0;
         c_Stream >> u32_Pos;
         orc_UpdatePosition.append(u32_Pos);
     }

     // Read package files
     c_Stream >> u32_NodeCount;
     orc_PackageFiles.clear();
     orc_PackageFiles.reserve(u32_NodeCount);
     for (quint32 i = 0; i < u32_NodeCount; ++i) {
         QString c_FileName;
         c_Stream >> c_FileName;
         orc_PackageFiles.append(c_FileName);
     }

     c_File.close();
     return (c_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load from JSON format

    \param[in]      orc_Path               Path to file
    \param[out]     oru32_FileVersion      File version
    \param[in,out]  orc_FilePackagePath    File package path
    \param[out]     oru32_ActiveBusIndex   Active bus index
    \param[in,out]  orc_ActiveNodes        Active nodes
    \param[in,out]  orc_UpdatePosition     Update position
    \param[in,out]  orc_PackageFiles       Package files

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_LoadJson(
     const QString &orc_Path,
     uint32_t &oru32_FileVersion,
     QString &orc_FilePackagePath,
     uint32_t &oru32_ActiveBusIndex,
     QByteArray &orc_ActiveNodes,
     QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_PackageFiles) {
     QFile c_File(orc_Path);
     if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return C_RD_WR;
     }

     QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
     c_File.close();

     if (!c_Doc.isObject()) {
         return C_CONFIG;
     }

     const QJsonObject c_Object = c_Doc.object();

     // Read file version
     if (!c_Object.contains("file-version") || !c_Object["file-version"].isDouble()) {
         return C_CONFIG;
     }
     oru32_FileVersion = static_cast<uint32_t>(c_Object["file-version"].toDouble());

     // Check version
     if (oru32_FileVersion != mu16_FILE_VERSION) {
         return C_CONFIG;
     }

     // Read bus index
     if (!c_Object.contains("bus-index-client") || !c_Object["bus-index-client"].isDouble()) {
         return C_CONFIG;
     }
     oru32_ActiveBusIndex = static_cast<uint32_t>(c_Object["bus-index-client"].toDouble());

     // Read nodes
     if (!c_Object.contains("nodes") || !c_Object["nodes"].isArray()) {
         return C_CONFIG;
     }

     const QJsonArray c_Nodes = c_Object["nodes"].toArray();
     orc_ActiveNodes.clear();
     orc_UpdatePosition.clear();
     orc_PackageFiles.clear();

     for (const QJsonValue &rc_Value : c_Nodes) {
         if (!rc_Value.isObject()) {
             return C_CONFIG;
         }

         const QJsonObject c_Node = rc_Value.toObject();
         uint8_t u8_Active = static_cast<uint8_t>(c_Node["active"].toInt());
         orc_ActiveNodes.push_back(u8_Active);

         if (u8_Active == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE) {
             uint32_t u32_Pos = static_cast<uint32_t>(c_Node["position"].toDouble());
             orc_UpdatePosition.append(u32_Pos);
             orc_PackageFiles.append(c_Node["file"].toString());
         } else {
             orc_UpdatePosition.append(0);
             orc_PackageFiles.append("");
         }
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load from XML format

    \param[in]      orc_Path               Path to file
    \param[out]     oru32_FileVersion      File version
    \param[in,out]  orc_FilePackagePath    File package path
    \param[out]     oru32_ActiveBusIndex   Active bus index
    \param[in,out]  orc_ActiveNodes        Active nodes
    \param[in,out]  orc_UpdatePosition     Update position
    \param[in,out]  orc_PackageFiles       Package files

    \return
    C_NO_ERR   no error
    C_CONFIG   invalid format
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_LoadXml(
     const QString &orc_Path,
     uint32_t &oru32_FileVersion,
     QString &orc_FilePackagePath,
     uint32_t &oru32_ActiveBusIndex,
     QByteArray &orc_ActiveNodes,
     QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_PackageFiles) {
     C_OscXmlParser c_XmlParser;
     const int32_t s32_Retval = c_XmlParser.LoadFromFile(orc_Path);

     if (s32_Retval != C_NO_ERR) {
         return C_RD_WR;
     }

     if (c_XmlParser.SelectRoot() != mc_ROOT_NAME) {
         return C_CONFIG;
     }

     // File version
     if (c_XmlParser.SelectNodeChild(mc_FILE_VERSION) != mc_FILE_VERSION) {
         return C_CONFIG;
     }
     const QString c_FileVersion = c_XmlParser.GetNodeContent();
     oru32_FileVersion = static_cast<uint32_t>(c_FileVersion.toInt());
     c_XmlParser.SelectRoot();

     if (oru32_FileVersion != mu16_FILE_VERSION) {
         return C_CONFIG;
     }

     // Active bus index
     if (c_XmlParser.SelectNodeChild(mc_BUS_INDEX) != mc_BUS_INDEX) {
         return C_CONFIG;
     }
     const QString c_BusIndex = c_XmlParser.GetNodeContent();
     oru32_ActiveBusIndex = static_cast<uint32_t>(c_BusIndex.toInt());
     c_XmlParser.SelectRoot();

     // Load nodes
     mh_LoadNodes(c_XmlParser, orc_ActiveNodes, orc_UpdatePosition, orc_PackageFiles);

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to binary format

    \param[in]  orc_Path             Path to file
    \param[in]  orc_SupDefContent    Content to save
    \param[in]  orc_Files            Files

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_SaveBinary(
     const QString &orc_Path,
     const C_OscSupDefinition &orc_SupDefContent,
     const QStringList &orc_Files) {
     QFile c_File(orc_Path);
     if (!c_File.open(QIODevice::WriteOnly)) {
         return C_RD_WR;
     }

     QDataStream c_Stream(&c_File);
     c_Stream.setVersion(QDataStream::Qt_5_12);

     // Write file version
     c_Stream << static_cast<uint32_t>(mu16_FILE_VERSION);

     // Write bus index
     c_Stream << orc_SupDefContent.u32_ActiveBusIndex;

     // Write active nodes
     quint32 u32_NodeCount = static_cast<quint32>(orc_SupDefContent.c_Nodes.size());
     c_Stream << u32_NodeCount;
     for (const C_OscSupNodeDefinition &rc_Node : orc_SupDefContent.c_Nodes) {
         c_Stream << rc_Node.u8_Active;
     }

     // Write update positions
     c_Stream << u32_NodeCount;
     for (const C_OscSupNodeDefinition &rc_Node : orc_SupDefContent.c_Nodes) {
         c_Stream << rc_Node.u32_Position;
     }

     // Write package files
     c_Stream << u32_NodeCount;
     for (const QString &rc_File : orc_Files) {
         c_Stream << rc_File;
     }

     c_File.close();
     return (c_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to JSON format

    \param[in]  orc_Path             Path to file
    \param[in]  orc_SupDefContent    Content to save
    \param[in]  orc_Files            Files

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_SaveJson(
     const QString &orc_Path,
     const C_OscSupDefinition &orc_SupDefContent,
     const QStringList &orc_Files) {
     QJsonObject c_Object;

     // Write file version
     c_Object["file-version"] = static_cast<double>(mu16_FILE_VERSION);

     // Write bus index
     c_Object["bus-index-client"] = static_cast<double>(orc_SupDefContent.u32_ActiveBusIndex);

     // Write nodes
     QJsonArray c_Nodes;
     Q_ASSERT(orc_SupDefContent.c_Nodes.size() == orc_Files.size());
     for (int i = 0; i < orc_SupDefContent.c_Nodes.size(); ++i) {
         const C_OscSupNodeDefinition &rc_Node = orc_SupDefContent.c_Nodes[i];
         QJsonObject c_Node;

         c_Node["active"] = static_cast<double>(rc_Node.u8_Active);

         if (rc_Node.u8_Active == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE) {
             c_Node["position"] = static_cast<double>(rc_Node.u32_Position);
             c_Node["file"] = orc_Files[i];
         }

         c_Nodes.append(c_Node);
     }
     c_Object["nodes"] = c_Nodes;

     // Write to file
     QJsonDocument c_Doc(c_Object);
     QFile c_File(orc_Path);
     if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
         return C_RD_WR;
     }
     c_File.write(c_Doc.toJson(QJsonDocument::Indented));
     c_File.close();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save to XML format

    \param[in]  orc_Path             Path to file
    \param[in]  orc_SupDefContent    Content to save
    \param[in]  orc_Files            Files

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_SaveXml(
     const QString &orc_Path,
     const C_OscSupDefinition &orc_SupDefContent,
     const QStringList &orc_Files) {
     C_OscXmlParser c_XmlParser;

     // Root Node
     c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);

     // File version
     c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION);
     c_XmlParser.SetNodeContent(QString::number(mu16_FILE_VERSION));
     c_XmlParser.SelectNodeParent();

     // Save nodes
     mh_SaveNodes(c_XmlParser, orc_SupDefContent.c_Nodes, orc_Files);

     // Bus index
     c_XmlParser.CreateAndSelectNodeChild(mc_BUS_INDEX);
     c_XmlParser.SetNodeContent(QString::number(orc_SupDefContent.u32_ActiveBusIndex));
     c_XmlParser.SelectNodeParent();

     // Save to file
     const int32_t s32_Result = c_XmlParser.SaveToFile(orc_Path);
     return (s32_Result == C_NO_ERR) ? C_NO_ERR : C_RD_WR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save nodes to XML

    \param[in,out]  orc_XmlParser    Xml parser
    \param[in]      orc_Nodes        Nodes
    \param[in]      orc_Files        Files
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupDefinitionFiler_New::mh_SaveNodes(
     C_OscXmlParserBase &orc_XmlParser,
     const QList<C_OscSupNodeDefinition> &orc_Nodes,
     const QStringList &orc_Files) {
     Q_ASSERT(orc_Nodes.size() == orc_Files.size());

     orc_XmlParser.CreateAndSelectNodeChild(mc_NODES);

     for (uint32_t u32_Pos = 0; u32_Pos < static_cast<uint32_t>(orc_Nodes.size()); ++u32_Pos) {
         const C_OscSupNodeDefinition c_CurrentNode = orc_Nodes[u32_Pos];

         orc_XmlParser.CreateAndSelectNodeChild(mc_NODE);
         orc_XmlParser.SetAttributeUint32(mc_NODE_ACTIVE_ATTR,
                                          static_cast<uint32_t>(c_CurrentNode.u8_Active));

         if (c_CurrentNode.u8_Active == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE) {
             if ((c_CurrentNode.c_ApplicationFileNames.size() > 0) ||
                 (c_CurrentNode.c_NvmFileNames.size() > 0) ||
                 (c_CurrentNode.c_PemFile != "")) {
                 orc_XmlParser.SetAttributeUint32(mc_NODE_POSITION_ATTR,
                                                  c_CurrentNode.u32_Position);
             }

             orc_XmlParser.CreateAndSelectNodeChild(mc_NODE_UPDATE);
             orc_XmlParser.SetAttributeString(mc_NODE_FILE_ATTR, orc_Files[u32_Pos]);
             orc_XmlParser.SelectNodeParent();
         }

         orc_XmlParser.SelectNodeParent();
     }

     orc_XmlParser.SelectNodeParent();
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load nodes from XML

    \param[in,out]  orc_XmlParser       Xml parser
    \param[in,out]  orc_ActiveNodes     Active nodes
    \param[in,out]  orc_UpdatePosition  Update position
    \param[in,out]  orc_PackageFiles    Package files
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscSupDefinitionFiler_New::mh_LoadNodes(C_OscXmlParserBase &orc_XmlParser,
                                                QByteArray &orc_ActiveNodes,
                                                QList<uint32_t> &orc_UpdatePosition,
                                                QStringList &orc_PackageFiles) {
     orc_XmlParser.SelectNodeChild(mc_NODES);
     orc_XmlParser.SelectNodeChild(mc_NODE);

     QString c_SelectedNode;
     do {
         QString c_File;
         uint32_t u32_UpdatePosition = 0U;
         const uint8_t u8_NodeActive = static_cast<uint8_t>(
             orc_XmlParser.GetAttributeUint32(mc_NODE_ACTIVE_ATTR));
         orc_ActiveNodes.push_back(u8_NodeActive);

         if (u8_NodeActive == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE) {
             u32_UpdatePosition = static_cast<uint8_t>(
                 orc_XmlParser.GetAttributeUint32(mc_NODE_POSITION_ATTR));
             orc_XmlParser.SelectNodeChild(mc_NODE_UPDATE);
             c_File = orc_XmlParser.GetAttributeString(mc_NODE_FILE_ATTR);
             orc_XmlParser.SelectNodeParent();
         }

         orc_UpdatePosition.push_back(u32_UpdatePosition);
         orc_PackageFiles.push_back(c_File);

         c_SelectedNode = orc_XmlParser.SelectNodeNext(mc_NODE);
     } while (c_SelectedNode == mc_NODE);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Create update package definition file (legacy - XML only)

    \see h_CreateUpdatePackageDefFile
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_CreateUpdatePackageDefFileLegacy(
     const QString &orc_Path,
     const C_OscSupDefinition &orc_SupDefContent,
     const QStringList &orc_Files) {
     return h_SaveXml(stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_Path) +
                      hc_PACKAGE_UPDATE_DEF, orc_SupDefContent, orc_Files);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load update package definition file (legacy - XML only)

    \see h_LoadUpdatePackageDefFile
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscSupDefinitionFiler_New::h_LoadUpdatePackageDefFileLegacy(
     const QString &orc_TargetUnzipPath,
     const bool oq_IsZip,
     const QString &orc_PackagePath,
     uint32_t &oru32_FileVersion,
     QString &orc_FilePackagePath,
     uint32_t &oru32_ActiveBusIndex,
     QByteArray &orc_ActiveNodes,
     QList<uint32_t> &orc_UpdatePosition,
     QStringList &orc_PackageFiles) {
     QString c_Path;
     if (oq_IsZip) {
         c_Path = orc_TargetUnzipPath + hc_PACKAGE_UPDATE_DEF;
     } else {
         orc_FilePackagePath =
             stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_PackagePath);
         c_Path = orc_FilePackagePath + hc_PACKAGE_UPDATE_DEF;
     }
     return h_LoadXml(c_Path, oru32_FileVersion, orc_FilePackagePath,
                     oru32_ActiveBusIndex, orc_ActiveNodes,
                     orc_UpdatePosition, orc_PackageFiles);
 }
