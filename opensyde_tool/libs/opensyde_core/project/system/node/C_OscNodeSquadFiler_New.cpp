 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       NodeSquad reader/writer with multi-format support

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "precomp_headers.hpp"
 #include <QFileInfo>
 #include <fstream>

 #include "C_OscNodeSquadFiler_New.hpp"
 #include "C_OscLoggingHandler.hpp"
 #include "C_OscSystemFilerUtil.hpp"
 #include "C_OscUtils.hpp"
 #include "stwtypes.hpp"
 #include "stwerrors.hpp"

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
 /*! \brief  Load node groups from file (auto-detects format)

    \param[out]     orc_NodeGroups   Node groups
    \param[in]      orc_Path         Path to file

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
    C_RANGE    file does not exist
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadFile(QList<C_OscNodeSquad> &orc_NodeGroups,
                                             const QString &orc_Path) {
     int32_t s32_Retval = C_NO_ERR;

     if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
         const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

         if (c_Extension == "bin") {
             // Binary format
             std::ifstream c_File(orc_Path.toLocal8Bit().constData(), std::ios::binary);
             if (c_File.is_open()) {
                 QDataStream c_Stream(&c_File);
                 c_Stream.setVersion(QDataStream::Qt_5_12);
                 s32_Retval = h_LoadBinary(orc_NodeGroups, c_Stream);
                 c_File.close();
             } else {
                 osc_write_log_error("Loading node groups",
                                     "File \"" + orc_Path + "\" could not be opened for binary reading.");
                 s32_Retval = C_NOACT;
             }
         } else if (c_Extension == "json") {
             // JSON format
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
                 QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
                 c_File.close();
                 if (c_Doc.isObject()) {
                     s32_Retval = h_LoadJson(orc_NodeGroups, c_Doc.object());
                 } else {
                     osc_write_log_error("Loading node groups",
                                         "JSON file \"" + orc_Path + "\" does not contain a valid object.");
                     s32_Retval = C_CONFIG;
                 }
             } else {
                 osc_write_log_error("Loading node groups",
                                     "File \"" + orc_Path + "\" could not be opened for JSON reading.");
                 s32_Retval = C_NOACT;
             }
         } else {
             // XML format (default)
             C_OscXmlParserLog c_XmlParser;
             s32_Retval = c_XmlParser.LoadFromFile(orc_Path);
             if (s32_Retval == C_NO_ERR) {
                 s32_Retval = h_LoadXml(orc_NodeGroups, c_XmlParser);
             } else {
                 osc_write_log_error("Loading node groups",
                                     "File \"" + orc_Path + "\" could not be parsed as XML.");
                 s32_Retval = C_NOACT;
             }
         }
     } else {
         osc_write_log_error("Loading node groups",
                             "File \"" + orc_Path + "\" does not exist.");
         s32_Retval = C_RANGE;
     }

     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node groups to file (auto-detects format from extension)

    \param[in]      orc_NodeGroups   Node groups
    \param[in]      orc_Path         Path to file

    \return
    C_NO_ERR   data saved
    C_RD_WR    could not write to file
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_SaveFile(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                             const QString &orc_Path) {
     const QString c_Extension = QFileInfo(orc_Path).suffix().toLower();

     if (c_Extension == "bin") {
         // Binary format
         QFile c_File(orc_Path);
         if (c_File.open(QIODevice::WriteOnly)) {
             QDataStream c_Stream(&c_File);
             c_Stream.setVersion(QDataStream::Qt_5_12);
             const int32_t s32_Result = h_SaveBinary(orc_NodeGroups, c_Stream);
             c_File.close();
             return s32_Result;
         } else {
             osc_write_log_error("Saving node groups",
                                 "File \"" + orc_Path + "\" could not be opened for binary writing.");
             return C_RD_WR;
         }
     } else if (c_Extension == "json") {
         // JSON format
         QJsonObject c_Object;
         const int32_t s32_Result = h_SaveJson(orc_NodeGroups, c_Object);
         if (s32_Result == C_NO_ERR) {
             QJsonDocument c_Doc(c_Object);
             QFile c_File(orc_Path);
             if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
                 c_File.write(c_Doc.toJson(QJsonDocument::Indented));
                 c_File.close();
             } else {
                 osc_write_log_error("Saving node groups",
                                     "File \"" + orc_Path + "\" could not be opened for JSON writing.");
                 return C_RD_WR;
             }
         }
         return s32_Result;
     } else {
         // XML format (default)
         C_OscXmlParserLog c_XmlParser;
         const int32_t s32_Result = h_SaveXml(orc_NodeGroups, c_XmlParser);
         if (s32_Result == C_NO_ERR) {
             return C_OscSystemFilerUtil::h_SaveStringToFile(
                 c_XmlParser.GetContent(), orc_Path, "Saving node groups");
         }
         return s32_Result;
     }
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node groups from binary stream

    \param[out]     orc_NodeGroups   Node groups
    \param[in,out]  orc_Stream       Binary stream

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadBinary(QList<C_OscNodeSquad> &orc_NodeGroups,
                                               QDataStream &orc_Stream) {
     orc_NodeGroups.clear();

     // Read count
     uint32_t u32_Count = 0UL;
     orc_Stream >> u32_Count;

     if (orc_Stream.status() != QDataStream::Ok) {
         return C_CONFIG;
     }

     // Reserve space
     orc_NodeGroups.reserve(u32_Count);

     // Read each node group
     for (uint32_t u32_Index = 0UL; u32_Index < u32_Count; ++u32_Index) {
         C_OscNodeSquad c_NodeGroup;
         const int32_t s32_Result = c_NodeGroup.FromQDataStream(orc_Stream);
         if (s32_Result != C_NO_ERR) {
             return s32_Result;
         }
         orc_NodeGroups.append(c_NodeGroup);
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node groups from JSON object

    \param[out]     orc_NodeGroups   Node groups
    \param[in]      orc_Object       JSON object

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadJson(QList<C_OscNodeSquad> &orc_NodeGroups,
                                             const QJsonObject &orc_Object) {
     orc_NodeGroups.clear();

     // Get array from JSON
     if (!orc_Object.contains("node-groups")) {
         return C_CONFIG;
     }

     const QJsonArray c_Array = orc_Object["node-groups"].toArray();

     // Reserve space
     orc_NodeGroups.reserve(c_Array.size());

     // Read each node group
     for (const QJsonValue &rc_Value : c_Array) {
         if (!rc_Value.isObject()) {
             return C_CONFIG;
         }

         C_OscNodeSquad c_NodeGroup;
         const int32_t s32_Result = c_NodeGroup.FromJsonObject(rc_Value.toObject());
         if (s32_Result != C_NO_ERR) {
             return s32_Result;
         }
         orc_NodeGroups.append(c_NodeGroup);
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node groups from XML parser

    \param[out]     orc_NodeGroups   Node groups
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadXml(QList<C_OscNodeSquad> &orc_NodeGroups,
                                            C_OscXmlParserBase &orc_XmlParser) {
     int32_t s32_Retval = C_NO_ERR;

     orc_NodeGroups.clear();
     if (orc_XmlParser.SelectNodeChild("node-groups") == "node-groups") {
         uint32_t u32_LengthNodeGroup;

         s32_Retval =
             orc_XmlParser.GetAttributeUint32Error("length", u32_LengthNodeGroup);
         if (s32_Retval == C_NO_ERR) {
             QString c_NodeName;
             orc_NodeGroups.reserve(u32_LengthNodeGroup);
             c_NodeName = orc_XmlParser.SelectNodeChild("node-group");
             if (c_NodeName == "node-group") {
                 do {
                     C_OscNodeSquad c_NewNodeGroup;
                     s32_Retval = C_OscNodeSquadFiler_New::h_LoadNodeGroup(c_NewNodeGroup,
                                                                           orc_XmlParser);
                     if (s32_Retval == C_NO_ERR) {
                         orc_NodeGroups.push_back(c_NewNodeGroup);
                     }
                     c_NodeName = orc_XmlParser.SelectNodeNext("node-group");
                 } while ((c_NodeName == "node-group") && (s32_Retval == C_NO_ERR));
                 if (s32_Retval == C_NO_ERR) {
                     if (u32_LengthNodeGroup != orc_NodeGroups.size()) {
                         s32_Retval = C_CONFIG;
                         osc_write_log_error(
                             "Loading system definition",
                             "Expected " + QString::number(u32_LengthNodeGroup) +
                                 " node groups but received " +
                                 QString::number(orc_NodeGroups.size()) + " node groups.");
                     }
                     if (s32_Retval == C_NO_ERR) {
                         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "node-groups");
                     }
                 }
             }
         }
         if (s32_Retval == C_NO_ERR) {
             orc_XmlParser.SelectNodeParent();
         }
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node groups to binary stream

    \param[in]      orc_NodeGroups   Node groups
    \param[in,out]  orc_Stream       Binary stream

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_SaveBinary(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                               QDataStream &orc_Stream) {
     // Write count
     orc_Stream << static_cast<uint32_t>(orc_NodeGroups.size());

     // Write each node group
     for (const C_OscNodeSquad &rc_NodeGroup : orc_NodeGroups) {
         const int32_t s32_Result = rc_NodeGroup.ToQDataStream(orc_Stream);
         if (s32_Result != C_NO_ERR) {
             return s32_Result;
         }
     }

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node groups to JSON object

    \param[in]      orc_NodeGroups   Node groups
    \param[out]     orc_Object       JSON object

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_SaveJson(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                             QJsonObject &orc_Object) {
     QJsonArray c_Array;

     // Convert each node group to JSON
     for (const C_OscNodeSquad &rc_NodeGroup : orc_NodeGroups) {
         c_Array.append(rc_NodeGroup.ToJsonObject());
     }

     orc_Object["node-groups"] = c_Array;

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node groups to XML parser

    \param[in]      orc_NodeGroups   Node groups
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_SaveXml(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                            C_OscXmlParserBase &orc_XmlParser) {
     orc_XmlParser.CreateAndSelectNodeChild("node-groups");
     orc_XmlParser.SetAttributeUint32(
         "length", static_cast<uint32_t>(orc_NodeGroups.size()));
     for (uint32_t u32_ItGroup = 0UL; u32_ItGroup < orc_NodeGroups.size();
          ++u32_ItGroup) {
         const C_OscNodeSquad &rc_NodeGroup = orc_NodeGroups[u32_ItGroup];
         C_OscNodeSquadFiler_New::h_SaveNodeGroup(rc_NodeGroup, orc_XmlParser);
     }
     orc_XmlParser.SelectNodeParent();

     return C_NO_ERR;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node group from XML parser

    \param[out]     orc_NodeGroup    Node group
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadNodeGroup(C_OscNodeSquad &orc_NodeGroup,
                                                  C_OscXmlParserBase &orc_XmlParser) {
     int32_t s32_Retval = orc_XmlParser.SelectNodeChildError("base-name");

     if (s32_Retval == C_NO_ERR) {
         orc_NodeGroup.c_BaseName = orc_XmlParser.GetNodeContent();
         orc_XmlParser.SelectNodeParent();
         s32_Retval = orc_XmlParser.SelectNodeChildError("sub-node-indices");
         if (s32_Retval == C_NO_ERR) {
             uint32_t u32_LengthSubIndices;
             s32_Retval =
                 orc_XmlParser.GetAttributeUint32Error("length", u32_LengthSubIndices);
             if (s32_Retval == C_NO_ERR) {
                 orc_NodeGroup.c_SubNodeIndexes.clear();
                 orc_NodeGroup.c_SubNodeIndexes.reserve(u32_LengthSubIndices);
                 s32_Retval = orc_XmlParser.SelectNodeChildError("sub-node-index");
                 if (s32_Retval == C_NO_ERR) {
                     QString c_NodeName;
                     do {
                         uint32_t u32_Value;
                         s32_Retval =
                             orc_XmlParser.GetAttributeUint32Error("value", u32_Value);
                         if (s32_Retval == C_NO_ERR) {
                             orc_NodeGroup.c_SubNodeIndexes.push_back(u32_Value);
                         }
                         c_NodeName = orc_XmlParser.SelectNodeNext("sub-node-index");
                     } while ((c_NodeName == "sub-node-index") &&
                              (s32_Retval == C_NO_ERR));
                     if (s32_Retval == C_NO_ERR) {
                         if (u32_LengthSubIndices != orc_NodeGroup.c_SubNodeIndexes.size()) {
                             s32_Retval = C_CONFIG;
                             osc_write_log_error(
                                 "Loading system definition",
                                 "Expected " + QString::number(u32_LengthSubIndices) +
                                     " sub node indices but received " +
                                     QString::number(orc_NodeGroup.c_SubNodeIndexes.size()) +
                                     " sub node indices.");
                         }
                     }
                     if (s32_Retval == C_NO_ERR) {
                         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "sub-node-indices");
                     }
                 }
             }
             if (s32_Retval == C_NO_ERR) {
                 orc_XmlParser.SelectNodeParent();
             }
         }
     }
     return s32_Retval;
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node group to XML parser

    \param[in]      orc_NodeGroup    Node group
    \param[in,out]  orc_XmlParser    XML parser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscNodeSquadFiler_New::h_SaveNodeGroup(const C_OscNodeSquad &orc_NodeGroup,
                                               C_OscXmlParserBase &orc_XmlParser) {
     orc_XmlParser.CreateAndSelectNodeChild("node-group");
     orc_XmlParser.CreateNodeChild("base-name", orc_NodeGroup.c_BaseName);
     orc_XmlParser.CreateAndSelectNodeChild("sub-node-indices");
     orc_XmlParser.SetAttributeUint32(
         "length", static_cast<uint32_t>(orc_NodeGroup.c_SubNodeIndexes.size()));
     for (uint32_t u32_ItIndex = 0UL;
          u32_ItIndex < orc_NodeGroup.c_SubNodeIndexes.size(); ++u32_ItIndex) {
         orc_XmlParser.CreateAndSelectNodeChild("sub-node-index");
         orc_XmlParser.SetAttributeUint32(
             "value", orc_NodeGroup.c_SubNodeIndexes[u32_ItIndex]);
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "sub-node-indices");
     }
     Q_ASSERT(orc_XmlParser.SelectNodeParent() == "node-group");
     Q_ASSERT(orc_XmlParser.SelectNodeParent() == "node-groups");
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node groups (legacy compatibility)

    \param[out]     orc_NodeGroups   Node groups
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadNodeGroups(QList<C_OscNodeSquad> &orc_NodeGroups,
                                                   C_OscXmlParserBase &orc_XmlParser) {
     return h_LoadXml(orc_NodeGroups, orc_XmlParser);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Load node group (legacy compatibility)

    \param[out]     orc_NodeGroup    Node group
    \param[in,out]  orc_XmlParser    XML parser

    \return
    C_NO_ERR   no error
    C_CONFIG   content is invalid or incomplete
 */
 //----------------------------------------------------------------------------------------------------------------------
 int32_t C_OscNodeSquadFiler_New::h_LoadNodeGroup(C_OscNodeSquad &orc_NodeGroup,
                                                  C_OscXmlParserBase &orc_XmlParser) {
     return h_LoadNodeGroup(orc_NodeGroup, orc_XmlParser);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node groups (legacy compatibility)

    \param[in]      orc_NodeGroups   Node groups
    \param[in,out]  orc_XmlParser    XML parser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscNodeSquadFiler_New::h_SaveNodeGroups(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                                C_OscXmlParserBase &orc_XmlParser) {
     h_SaveXml(orc_NodeGroups, orc_XmlParser);
 }

 //----------------------------------------------------------------------------------------------------------------------
 /*! \brief  Save node group (legacy compatibility)

    \param[in]      orc_NodeGroup    Node group
    \param[in,out]  orc_XmlParser    XML parser
 */
 //----------------------------------------------------------------------------------------------------------------------
 void C_OscNodeSquadFiler_New::h_SaveNodeGroup(const C_OscNodeSquad &orc_NodeGroup,
                                               C_OscXmlParserBase &orc_XmlParser) {
     h_SaveNodeGroup(orc_NodeGroup, orc_XmlParser);
 }
