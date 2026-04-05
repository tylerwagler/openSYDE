 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node squad reader/writer (multi-format)
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCNODESQUADFILER_HPP
 #define C_OSCNODESQUADFILER_HPP

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "C_OscNodeSquad.hpp"
 #include "C_OscFilerUtil.hpp"
 #include "C_OscXmlParser.hpp"
 #include <QString>

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

 class C_OscNodeSquadFiler {
 public:
     // --------------------------------------------------------------------------
     // File-path-based operations (auto-detect format)
     // --------------------------------------------------------------------------
     static int32_t h_LoadFile(QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);
     static int32_t h_SaveFile(const QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);

     // --------------------------------------------------------------------------
     // File-path-based operations (format-specific)
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(QList<C_OscNodeSquad> &orc_NodeGroups,
                                 const QString &orc_Path);
     static int32_t h_SaveBinary(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                 const QString &orc_Path);
     static int32_t h_LoadJson(QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);
     static int32_t h_SaveJson(const QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);
     static int32_t h_LoadXml(QList<C_OscNodeSquad> &orc_NodeGroups,
                              const QString &orc_Path);
     static int32_t h_SaveXml(const QList<C_OscNodeSquad> &orc_NodeGroups,
                              const QString &orc_Path);

     // --------------------------------------------------------------------------
     // In-memory operations (stream/object-based)
     // --------------------------------------------------------------------------
     static int32_t h_LoadFromMemoryBinary(QList<C_OscNodeSquad> &orc_NodeGroups,
                                           QDataStream &orc_Stream);
     static int32_t h_SaveToMemoryBinary(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                         QDataStream &orc_Stream);
     static int32_t h_LoadFromMemoryJson(QList<C_OscNodeSquad> &orc_NodeGroups,
                                         const QJsonObject &orc_Object);
     static int32_t h_SaveToMemoryJson(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                       QJsonObject &orc_Object);
     static int32_t h_LoadFromMemoryXml(QList<C_OscNodeSquad> &orc_NodeGroups,
                                        C_OscXmlParserBase &orc_XmlParser);
     static int32_t h_SaveToMemoryXml(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                      C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Single item operations
     // --------------------------------------------------------------------------
     static int32_t h_LoadNodeGroup(C_OscNodeSquad &orc_NodeGroup,
                                    C_OscXmlParserBase &orc_XmlParser);
     static void h_SaveNodeGroup(const C_OscNodeSquad &orc_NodeGroup,
                                 C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Legacy compatibility methods (deprecated)
     // --------------------------------------------------------------------------
     [[deprecated("Use h_LoadFile/h_SaveFile with auto-detection instead")]]
     static int32_t h_LoadNodeGroups(QList<C_OscNodeSquad> &orc_NodeGroups,
                                     C_OscXmlParserBase &orc_XmlParser);
     [[deprecated("Use h_LoadFile/h_SaveFile with auto-detection instead")]]
     static void h_SaveNodeGroups(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                  C_OscXmlParserBase &orc_XmlParser);
 };

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
