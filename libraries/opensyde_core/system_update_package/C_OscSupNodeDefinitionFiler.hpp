//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update package definition filer
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPNODEDEFINITIONFILER_HPP
#define C_OSCSUPNODEDEFINITIONFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <map>

#include <string>
#include "C_OscXmlParser.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSupNodeDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSupNodeDefinitionFiler
{
public:
   static int32_t h_SaveNodes(const std::vector<std::string> & orc_Files,
                              const std::vector<C_OscSupNodeDefinition> & orc_Nodes, const bool oq_UseMinorVersion1);
   static int32_t h_LoadNodes(const std::vector<std::string> & orc_Files,
                              const std::vector<std::string> & orc_NodeFoldersAbs,
                              const std::vector<uint8_t> & orc_ActiveNodes,
                              std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::map<uint32_t,
                                                                                                           uint32_t> & orc_UpdateOrderByNodes, const std::vector<uint32_t> & orc_UpdatePosition, std::vector<std::string> & orc_Signatures, const bool oq_UseMinorVersion1);

   static const uint8_t hu8_ACTIVE_NODE;

private:
   static int32_t mh_SaveNode(const std::string & orc_File, const C_OscSupNodeDefinition & orc_Node,
                              const bool oq_UseMinorVersion1);
   static void mh_LoadFilesSection(std::vector<std::string> & orc_Files, const uint32_t ou32_NodeCounter,
                                   const uint32_t ou32_UpdatePos, std::map<uint32_t,
                                                                           uint32_t> & orc_PositionMap,
                                   const std::string & orc_NodeFolderAbs, C_OscXmlParserBase & orc_XmlParser,
                                   const std::string & orc_BaseNodeName,
                                   const std::string & orc_ElementNodeName);
   static void mh_LoadPemConfigSectionVersion1(C_OscSuSequences::C_DoFlash & orc_DoFlash,
                                               const uint32_t ou32_NodeCounter, const uint32_t ou32_UpdatePos,
                                               std::map<uint32_t,
                                                        uint32_t> & orc_PositionMap,
                                               const std::string & orc_NodeFolderAbs,
                                               C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadPemConfigSectionVersion1Minor1(C_OscSuSequences::C_DoFlash & orc_DoFlash,
                                                        const uint32_t ou32_NodeCounter, const uint32_t ou32_UpdatePos,
                                                        std::map<uint32_t,
                                                                 uint32_t> & orc_PositionMap,
                                                        const std::string & orc_NodeFolderAbs,
                                                        C_OscXmlParserBase & orc_XmlParser);
   static void mh_DecideNodeRequiresFlashForSecurity(const C_OscSuSequences::C_DoFlash & orc_DoFlash,
                                                     const uint32_t ou32_NodeCounter, const uint32_t ou32_UpdatePos,
                                                     std::map<uint32_t,
                                                              uint32_t> & orc_PositionMap);
   static int32_t mh_LoadPemConfigOption(const std::string & orc_NodeName, bool & orq_SendOption,
                                         bool & orq_EnabledOption, const bool oq_AddFileOption,
                                         C_OscXmlParserBase & orc_XmlParser,
                                         const std::string & orc_NodeFolderAbs,
                                         std::string * const opc_FileName = nullptr);
   static void mh_SaveFiles(const std::vector<std::string> & orc_Files, C_OscXmlParserBase & orc_XmlParser,
                            const std::string & orc_BaseNodeName,
                            const std::string & orc_ElementNodeName);
   static void mh_SavePemConfigVersion1(const C_OscSupNodeDefinition & orc_CurrentNode,
                                        C_OscXmlParserBase & orc_XmlParser);
   static void mh_SavePemConfigVersion1Minor1(const C_OscSupNodeDefinition & orc_CurrentNode,
                                              C_OscXmlParserBase & orc_XmlParser);
   static void mh_SavePemConfigOption(const std::string & orc_NodeName, const bool oq_SendOption,
                                      const bool oq_EnabledOption, const bool oq_AddFileOption,
                                      C_OscXmlParserBase & orc_XmlParser, const std::string oc_FileName = "");
   static void mh_SaveSignatureFile(const C_OscSupNodeDefinition & orc_Node, C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadSignatureFile(const std::string & orc_NodeFolderAbs,
                                       std::string & orc_Signature, C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_CheckFileVersion(const bool oq_UseMinorVersion1, C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
