 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       NodeSquad reader/writer with multi-format support
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCNODESQUADFILER_NEW_HPP
 #define C_OSCNODESQUADFILER_NEW_HPP

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "C_OscNodeSquad.hpp"
 #include "C_OscFilerHelper.hpp"
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

 class C_OscNodeSquadFiler_New {
 public:
     // --------------------------------------------------------------------------
     // Public Static Methods - Multi-format file operations
     // --------------------------------------------------------------------------
     static int32_t h_LoadFile(QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);
     static int32_t h_SaveFile(const QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QString &orc_Path);

     // --------------------------------------------------------------------------
     // Format-specific loading methods
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(QList<C_OscNodeSquad> &orc_NodeGroups,
                                 QDataStream &orc_Stream);
     static int32_t h_LoadJson(QList<C_OscNodeSquad> &orc_NodeGroups,
                               const QJsonObject &orc_Object);
     static int32_t h_LoadXml(QList<C_OscNodeSquad> &orc_NodeGroups,
                              C_OscXmlParserBase &orc_XmlParser);

     // --------------------------------------------------------------------------
     // Format-specific saving methods
     // --------------------------------------------------------------------------
     static int32_t h_SaveBinary(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                 QDataStream &orc_Stream);
     static int32_t h_SaveJson(const QList<C_OscNodeSquad> &orc_NodeGroups,
                               QJsonObject &orc_Object);
     static int32_t h_SaveXml(const QList<C_OscNodeSquad> &orc_NodeGroups,
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
     static int32_t h_LoadNodeGroup(C_OscNodeSquad &orc_NodeGroup,
                                    C_OscXmlParserBase &orc_XmlParser);
     [[deprecated("Use h_LoadFile/h_SaveFile with auto-detection instead")]]
     static void h_SaveNodeGroups(const QList<C_OscNodeSquad> &orc_NodeGroups,
                                  C_OscXmlParserBase &orc_XmlParser);
     [[deprecated("Use h_LoadFile/h_SaveFile with auto-detection instead")]]
     static void h_SaveNodeGroup(const C_OscNodeSquad &orc_NodeGroup,
                                 C_OscXmlParserBase &orc_XmlParser);
 };

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
