//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update package definition filer

   Node update package definition filer

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <system_error>
#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSupSignatureFiler.hpp"
#include "C_OscSupNodeDefinitionFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
static const uint16_t mu16_FILE_VERSION_1 = 1U;
static const std::string mc_FILE_VERSION_1_1 = "0x000101";
const uint8_t C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE = 1U;
// XML node names of service update package definition
static const std::string mc_ROOT_NAME = "opensyde-secure-update-collection-definition"; // xml root node
static const std::string mc_FILE_VERSION = "file-version";                              // xml node
static const std::string mc_FILES = "files";                                            // xml node
static const std::string mc_FILE = "file";                                              // xml node
static const std::string mc_PARAM_FILES = "param-files";                                // xml node
static const std::string mc_PARAM_FILE = "param-file";                                  // xml node
static const std::string mc_PEM_FILE_CONFIG = "pem-file-config";                        // xml node
static const std::string mc_PEM_FILE = "pem-file";                                      // xml node
static const std::string mc_PEM_FILE_CONFIG_SEC_ENAB_ATTR = "security-enabled";         // xml node
                                                                                                  // attribute
static const std::string mc_PEM_FILE_CONFIG_SEC_SEND_ATTR = "security-send";            // xml node
                                                                                                  // attribute
static const std::string mc_PEM_FILE_CONFIG_DEB_ENAB_ATTR = "debugger-enabled";         // xml node attribute
static const std::string mc_PEM_FILE_CONFIG_DEB_SEND_ATTR = "debugger-send";            // xml node attribute
static const std::string mc_SECURITY = "security-config";                               // xml node
static const std::string mc_SECURITY_AUTHENTICATION = "secure-authentication";          // xml node
static const std::string mc_SECURITY_TRAFFIC_ENCRYPTION = "traffic-encryption-config";  // xml node
static const std::string mc_SECURITY_DEBUGGER_CONFIG = "debugger-config";               // xml node
static const std::string mc_SECURITY_ENAB_ATTR = "enabled";                             // xml node attribute
static const std::string mc_SECURITY_SEND_ATTR = "send";                                // xml node attribute
static const std::string mc_FILE_NAME_ATTR = "name";                                    // xml node attribute
static const std::string mc_SIG_FILE = "secure-signature-file";                         // xml node
static const std::string mc_SIG_FILE_ATTR = "name";                                     // xml node
                                                                                                  // attribute

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save nodes

   \param[in]  orc_Files            Files
   \param[in]  orc_Nodes            Nodes
   \param[in]  oq_UseMinorVersion1  Use minor version 1

   \return
   std::error_code

   \retval   Errc::success   No error
   \retval   Errc::rd_wr     File(s) could not be created
   \retval   Errc::config    Input invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::h_SaveNodes(const std::vector<std::string> & orc_Files,
                                                         const std::vector<C_OscSupNodeDefinition> & orc_Nodes,
                                                         const bool oq_UseMinorVersion1)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Nodes.size() == orc_Files.size())
   {
      for (uint32_t u32_Pos = 0; (u32_Pos < orc_Nodes.size()) && (!c_Retval); u32_Pos++)
      {
         if (orc_Nodes[u32_Pos].u8_Active == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE)
         {
            c_Retval = C_OscSupNodeDefinitionFiler::mh_SaveNode(orc_Files[u32_Pos], orc_Nodes[u32_Pos],
                                                                oq_UseMinorVersion1);
         }
      }
   }
   else
   {
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load nodes

   \param[in]      orc_Files                 Files
   \param[in]      orc_NodeFoldersAbs        Target unzip path
   \param[in,out]  orc_ActiveNodes           Active nodes
   \param[in,out]  orc_ApplicationsToWrite   Applications to write
   \param[in,out]  orc_UpdateOrderByNodes    Update order by nodes
   \param[in]      orc_UpdatePosition        Update position
   \param[in,out]  orc_Signatures            Signatures
   \param[in]      oq_UseMinorVersion1       Use minor version 1

   \return
   std::error_code

   \retval   Errc::success   success
   \retval   Errc::rd_wr     read/write error (see log file)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::h_LoadNodes(const std::vector<std::string> & orc_Files,
                                                         const std::vector<std::string> & orc_NodeFoldersAbs,
                                                         const std::vector<uint8_t> & orc_ActiveNodes,
                                                         std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite, std::map<uint32_t,
                                                                                                                                     uint32_t> & orc_UpdateOrderByNodes, const std::vector<uint32_t> & orc_UpdatePosition, std::vector<std::string> & orc_Signatures,
                                                         const bool oq_UseMinorVersion1)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert((orc_Files.size() == orc_NodeFoldersAbs.size()) &&
              ((orc_Files.size() == orc_ActiveNodes.size()) && (orc_Files.size() == orc_UpdatePosition.size())));
   if ((orc_Files.size() == orc_NodeFoldersAbs.size()) &&
       ((orc_Files.size() == orc_ActiveNodes.size()) && (orc_Files.size() == orc_UpdatePosition.size())))
   {
      // go through all nodes
      for (uint32_t u32_NodeCounter = 0; (u32_NodeCounter < orc_ActiveNodes.size()) && (!c_Retval);
           ++u32_NodeCounter)
      {
         std::string c_Signature;
         C_OscSuSequences::C_DoFlash c_DoFlash;
         if (orc_ActiveNodes[u32_NodeCounter] == C_OscSupNodeDefinitionFiler::hu8_ACTIVE_NODE)
         {
            C_OscXmlParserLog c_XmlParser;
            c_XmlParser.SetLogHeading("Loading secure update collection definition");
            c_Retval = make_error_code_from_stw(c_XmlParser.LoadFromFile(orc_Files[u32_NodeCounter]));
            if (!c_Retval)
            {
               // get update position
               const uint32_t u32_UpdatePosition = orc_UpdatePosition[u32_NodeCounter];
               tgl_assert(c_XmlParser.SelectRoot() == mc_ROOT_NAME); // we shall have a valid and
               c_Retval = mh_CheckFileVersion(oq_UseMinorVersion1, c_XmlParser);
               if (!c_Retval)
               {
                  // compatible update package
                  C_OscSupNodeDefinitionFiler::mh_LoadFilesSection(c_DoFlash.c_FilesToFlash, u32_NodeCounter,
                                                                   u32_UpdatePosition, orc_UpdateOrderByNodes,
                                                                   orc_NodeFoldersAbs[u32_NodeCounter],
                                                                   c_XmlParser, mc_FILES, mc_FILE);
                  C_OscSupNodeDefinitionFiler::mh_LoadFilesSection(c_DoFlash.c_FilesToWriteToNvm, u32_NodeCounter,
                                                                   u32_UpdatePosition, orc_UpdateOrderByNodes,
                                                                   orc_NodeFoldersAbs[u32_NodeCounter],
                                                                   c_XmlParser, mc_PARAM_FILES, mc_PARAM_FILE);
                  if (oq_UseMinorVersion1)
                  {
                     c_Retval = C_OscSupNodeDefinitionFiler::mh_LoadPemConfigSectionVersion1Minor1(c_DoFlash,
                                                                                                   u32_NodeCounter,
                                                                                                   u32_UpdatePosition,
                                                                                                   orc_UpdateOrderByNodes,
                                                                                                   orc_NodeFoldersAbs[
                                                                                                      u32_NodeCounter],
                                                                                                   c_XmlParser);
                  }
                  else
                  {
                     C_OscSupNodeDefinitionFiler::mh_LoadPemConfigSectionVersion1(c_DoFlash, u32_NodeCounter,
                                                                                  u32_UpdatePosition,
                                                                                  orc_UpdateOrderByNodes,
                                                                                  orc_NodeFoldersAbs[u32_NodeCounter],
                                                                                  c_XmlParser);
                  }
                  if (!c_Retval)
                  {
                     c_Retval = mh_LoadSignatureFile(orc_NodeFoldersAbs[u32_NodeCounter], c_Signature, c_XmlParser);
                  }
               }
            }
         }
         orc_ApplicationsToWrite.push_back(c_DoFlash); // push back in any case even if we have no applications
                                                       // because of an inactive device
         orc_Signatures.push_back(c_Signature);        // same for signature
      }
   }
   if (c_Retval)
   {
      c_Retval = Errc::rd_wr;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save node

   \param[in]  orc_File             File
   \param[in]  orc_Node             Node
   \param[in]  oq_UseMinorVersion1  Use minor version 1

   \return
   std::error_code

   \retval   Errc::success   No error
   \retval   Errc::rd_wr     File(s) could not be created
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::mh_SaveNode(const std::string & orc_File,
                                                         const C_OscSupNodeDefinition & orc_Node,
                                                         const bool oq_UseMinorVersion1)
{
   std::error_code c_Result = Errc::success;
   C_OscXmlParser c_XmlParser;

   //Root Node
   c_XmlParser.CreateAndSelectNodeChild(mc_ROOT_NAME);
   //File version
   tgl_assert(c_XmlParser.CreateAndSelectNodeChild(mc_FILE_VERSION) == mc_FILE_VERSION);
   if (oq_UseMinorVersion1)
   {
      c_XmlParser.SetNodeContent(mc_FILE_VERSION_1_1);
   }
   else
   {
      c_XmlParser.SetNodeContent(std::to_string(mu16_FILE_VERSION_1));
   }
   tgl_assert(c_XmlParser.SelectNodeParent() == mc_ROOT_NAME);

   mh_SaveFiles(orc_Node.c_ApplicationFileNames, c_XmlParser, mc_FILES, mc_FILE);
   mh_SaveFiles(orc_Node.c_NvmFileNames, c_XmlParser, mc_PARAM_FILES, mc_PARAM_FILE);
   if (oq_UseMinorVersion1)
   {
      mh_SavePemConfigVersion1Minor1(orc_Node, c_XmlParser);
   }
   else
   {
      mh_SavePemConfigVersion1(orc_Node, c_XmlParser);
   }
   mh_SaveSignatureFile(orc_Node, c_XmlParser);

   // save update package definition file
   c_Result = make_error_code_from_stw(c_XmlParser.SaveToFile(orc_File));
   if (c_Result)
   {
      c_Result = Errc::rd_wr;
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load files section from service update package definition file

   \param[in,out]  orc_Files              Files vector to append to
   \param[in]      ou32_NodeCounter       Current node index
   \param[in]      ou32_UpdatePos         Current update position
   \param[in,out]  orc_PositionMap        Map for node indices and update positions
   \param[in]      orc_NodeFolderAbs      Path where all the files will be unzipped to
   \param[in,out]  orc_XmlParser          XMLParser for service update package definition file
   \param[in]      orc_BaseNodeName       XML node name to use on base level
   \param[in]      orc_ElementNodeName    XML node name to use on item level
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_LoadFilesSection(std::vector<std::string> & orc_Files,
                                                      const uint32_t ou32_NodeCounter, const uint32_t ou32_UpdatePos,
                                                      std::map<uint32_t, uint32_t> & orc_PositionMap,
                                                      const std::string & orc_NodeFolderAbs,
                                                      C_OscXmlParserBase & orc_XmlParser,
                                                      const std::string & orc_BaseNodeName,
                                                      const std::string & orc_ElementNodeName)
{
   if (orc_XmlParser.SelectNodeChild(orc_BaseNodeName) == orc_BaseNodeName)
   {
      std::string c_SelectedNode;
      // node has applications to update
      orc_PositionMap.insert(std::pair<uint32_t, uint32_t>(ou32_NodeCounter, ou32_UpdatePos));
      // get update application paths
      tgl_assert(orc_XmlParser.SelectNodeChild(orc_ElementNodeName) == orc_ElementNodeName);

      // go through all files
      do
      {
         // we have to take care of OS dependent path delimiters for windows '\\'
         const std::string c_XmlAttr = orc_XmlParser.GetAttributeString(mc_FILE_NAME_ATTR);
         const std::string c_FilePath = TglFileIncludeTrailingDelimiter(
            orc_NodeFolderAbs) + c_XmlAttr;
         orc_Files.push_back(c_FilePath);
         c_SelectedNode = orc_XmlParser.SelectNodeNext(orc_ElementNodeName);
      }
      while (c_SelectedNode == orc_ElementNodeName);
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_BaseNodeName);
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load PEM configuration section from service update package definition file version 1

   \param[in,out]  orc_DoFlash         Node configuration with PEM configuration
   \param[in]      ou32_NodeCounter    Current node index
   \param[in]      ou32_UpdatePos      Current update position
   \param[in,out]  orc_PositionMap     Map for node indices and update positions
   \param[in]      orc_NodeFolderAbs   Path where all the files will be unzipped to
   \param[in,out]  orc_XmlParser       XMLParser for service update package definition file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_LoadPemConfigSectionVersion1(C_OscSuSequences::C_DoFlash & orc_DoFlash,
                                                                  const uint32_t ou32_NodeCounter,
                                                                  const uint32_t ou32_UpdatePos, std::map<uint32_t,
                                                                                                          uint32_t> & orc_PositionMap, const std::string & orc_NodeFolderAbs,
                                                                  C_OscXmlParserBase & orc_XmlParser)
{
   if (orc_XmlParser.SelectNodeChild(mc_PEM_FILE_CONFIG) == mc_PEM_FILE_CONFIG)
   {
      // get PEM file path
      if (orc_XmlParser.SelectNodeChild(mc_PEM_FILE) == mc_PEM_FILE)
      {
         // we have to take care of OS dependent path delimiters for windows '\\'
         const std::string c_XmlAttr = orc_XmlParser.GetAttributeString(mc_FILE_NAME_ATTR);
         if (c_XmlAttr != "")
         {
            const std::string c_FilePath = TglFileIncludeTrailingDelimiter(
               orc_NodeFolderAbs) + c_XmlAttr;
            orc_DoFlash.c_PemFile = c_FilePath;
         }

         tgl_assert(orc_XmlParser.SelectNodeParent() == mc_PEM_FILE_CONFIG);
      }
      orc_DoFlash.q_SendSecureAuthenticationEnabledState =
         orc_XmlParser.GetAttributeBool(mc_PEM_FILE_CONFIG_SEC_SEND_ATTR, false);
      orc_DoFlash.q_SecureAuthenticationEnabled = orc_XmlParser.GetAttributeBool(mc_PEM_FILE_CONFIG_SEC_ENAB_ATTR,
                                                                                 false);

      orc_DoFlash.q_SendDebuggerEnabledState =
         orc_XmlParser.GetAttributeBool(mc_PEM_FILE_CONFIG_DEB_SEND_ATTR, false);
      orc_DoFlash.q_DebuggerEnabled = orc_XmlParser.GetAttributeBool(mc_PEM_FILE_CONFIG_DEB_ENAB_ATTR, false);

      mh_DecideNodeRequiresFlashForSecurity(orc_DoFlash, ou32_NodeCounter, ou32_UpdatePos, orc_PositionMap);

      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load PEM configuration section from service update package definition file version 1 minor 1

   \param[in,out]  orc_DoFlash         Node configuration with PEM configuration
   \param[in]      ou32_NodeCounter    Current node index
   \param[in]      ou32_UpdatePos      Current update position
   \param[in,out]  orc_PositionMap     Map for node indices and update positions
   \param[in]      orc_NodeFolderAbs   Path where all the files will be unzipped to
   \param[in,out]  orc_XmlParser       XMLParser for service update package definition file

   \return
   std::error_code

   \retval   Errc::success   No error
   \retval   Errc::config    Input invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::mh_LoadPemConfigSectionVersion1Minor1(
   C_OscSuSequences::C_DoFlash & orc_DoFlash, const uint32_t ou32_NodeCounter, const uint32_t ou32_UpdatePos,
   std::map<uint32_t, uint32_t> & orc_PositionMap, const std::string & orc_NodeFolderAbs,
   C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError(mc_SECURITY));

   if (!c_Retval)
   {
      c_Retval = mh_LoadPemConfigOption(mc_SECURITY_AUTHENTICATION,
                                        orc_DoFlash.q_SendSecureAuthenticationEnabledState,
                                        orc_DoFlash.q_SecureAuthenticationEnabled, true, orc_XmlParser,
                                        orc_NodeFolderAbs,
                                        &orc_DoFlash.c_PemFile);
   }
   if (!c_Retval)
   {
      c_Retval = mh_LoadPemConfigOption(mc_SECURITY_TRAFFIC_ENCRYPTION,
                                        orc_DoFlash.q_SendTrafficEncryptionEnabledState,
                                        orc_DoFlash.q_TrafficEncryptionEnabled, false, orc_XmlParser,
                                        orc_NodeFolderAbs);
   }
   if (!c_Retval)
   {
      c_Retval = mh_LoadPemConfigOption(mc_SECURITY_DEBUGGER_CONFIG, orc_DoFlash.q_SendDebuggerEnabledState,
                                        orc_DoFlash.q_DebuggerEnabled, false, orc_XmlParser, orc_NodeFolderAbs);
   }
   if (!c_Retval)
   {
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
   if (!c_Retval)
   {
      mh_DecideNodeRequiresFlashForSecurity(orc_DoFlash, ou32_NodeCounter, ou32_UpdatePos, orc_PositionMap);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decide node requires flash for security

   \param[in]      orc_DoFlash         Node configuration with PEM configuration
   \param[in]      ou32_NodeCounter    Current node index
   \param[in]      ou32_UpdatePos      Current update position
   \param[in,out]  orc_PositionMap     Map for node indices and update positions
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_DecideNodeRequiresFlashForSecurity(const C_OscSuSequences::C_DoFlash & orc_DoFlash,
                                                                        const uint32_t ou32_NodeCounter,
                                                                        const uint32_t ou32_UpdatePos,
                                                                        std::map<uint32_t, uint32_t> & orc_PositionMap)
{
   if ((orc_DoFlash.c_PemFile != "") || (orc_DoFlash.q_SendSecureAuthenticationEnabledState) ||
       (orc_DoFlash.q_SendTrafficEncryptionEnabledState) || (orc_DoFlash.q_SendDebuggerEnabledState))
   {
      // node has applications to update
      orc_PositionMap.insert(std::pair<uint32_t, uint32_t>(ou32_NodeCounter, ou32_UpdatePos));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load pem config option

   \param[in]      orc_NodeName        Node name
   \param[out]     orq_SendOption      Send option
   \param[out]     orq_EnabledOption   Enabled option
   \param[in]      oq_AddFileOption    Add file option
   \param[in,out]  orc_XmlParser       Xml parser
   \param[in]      orc_NodeFolderAbs   Node folder abs
   \param[in,out]  opc_FileName        File name

   \return
   std::error_code

   \retval   Errc::success   No error
   \retval   Errc::config    Input invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::mh_LoadPemConfigOption(const std::string & orc_NodeName,
                                                                    bool & orq_SendOption, bool & orq_EnabledOption,
                                                                    const bool oq_AddFileOption,
                                                                    C_OscXmlParserBase & orc_XmlParser,
                                                                    const std::string & orc_NodeFolderAbs,
                                                                    std::string * const opc_FileName)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError(orc_NodeName));

   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(mc_SECURITY_SEND_ATTR, orq_SendOption));
   }
   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(mc_SECURITY_ENAB_ATTR, orq_EnabledOption));
   }
   if (oq_AddFileOption)
   {
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError(mc_PEM_FILE));
      }
      if (!c_Retval)
      {
         tgl_assert(opc_FileName != nullptr);
         if (opc_FileName != nullptr)
         {
            // we have to take care of OS dependent path delimiters for windows '\\'
            const std::string c_XmlAttr = orc_XmlParser.GetAttributeString(mc_FILE_NAME_ATTR);
            if (c_XmlAttr != "")
            {
               const std::string c_FilePath = TglFileIncludeTrailingDelimiter(orc_NodeFolderAbs) + c_XmlAttr;
               *opc_FileName = c_FilePath;
            }
            else
            {
               *opc_FileName = "";
            }
         }
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
      }
   }
   if (!c_Retval)
   {
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_SECURITY);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save files in XML

   \param[in,out]  orc_Files              Files to save
   \param[in,out]  orc_XmlParser          XMLParser for service update package definition file
   \param[in]      orc_BaseNodeName       XML node name to use on base level
   \param[in]      orc_ElementNodeName    XML node name to use on item level
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_SaveFiles(const std::vector<std::string> & orc_Files,
                                               C_OscXmlParserBase & orc_XmlParser,
                                               const std::string & orc_BaseNodeName,
                                               const std::string & orc_ElementNodeName)
{
   if (orc_Files.size() > 0)
   {
      //Files
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_BaseNodeName) == orc_BaseNodeName);
      for (uint32_t u32_PosFile = 0; u32_PosFile < orc_Files.size(); u32_PosFile++)
      {
         //File
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_ElementNodeName) == orc_ElementNodeName);
         orc_XmlParser.SetAttributeString(mc_FILE_NAME_ATTR, orc_Files[u32_PosFile]);
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_BaseNodeName);
      }
      //Return for next node
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save PEM file configuration in XML version 1

   \param[in]      orc_CurrentNode  Current node configuration with PEM file configuration
   \param[in,out]  orc_XmlParser    XMLParser for service update package definition file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_SavePemConfigVersion1(const C_OscSupNodeDefinition & orc_CurrentNode,
                                                           C_OscXmlParserBase & orc_XmlParser)
{
   if ((orc_CurrentNode.c_PemFile != "") || (orc_CurrentNode.q_SendSecureAuthenticationEnabledState) ||
       (orc_CurrentNode.q_SendDebuggerEnabledState))
   {
      //Files
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(mc_PEM_FILE_CONFIG) == mc_PEM_FILE_CONFIG);
      orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_SEC_SEND_ATTR,
                                     orc_CurrentNode.q_SendSecureAuthenticationEnabledState);
      orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_SEC_ENAB_ATTR, orc_CurrentNode.q_SecureAuthenticationEnabled);
      orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_DEB_SEND_ATTR, orc_CurrentNode.q_SendDebuggerEnabledState);
      orc_XmlParser.SetAttributeBool(mc_PEM_FILE_CONFIG_DEB_ENAB_ATTR, orc_CurrentNode.q_DebuggerEnabled);

      //File
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(mc_PEM_FILE) == mc_PEM_FILE);
      orc_XmlParser.SetAttributeString(mc_FILE_NAME_ATTR, orc_CurrentNode.c_PemFile);
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_PEM_FILE_CONFIG);

      //Return for next node
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save PEM file configuration in XML version 1 minor 1

   \param[in]      orc_CurrentNode  Current node configuration with PEM file configuration
   \param[in,out]  orc_XmlParser    XMLParser for service update package definition file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_SavePemConfigVersion1Minor1(const C_OscSupNodeDefinition & orc_CurrentNode,
                                                                 C_OscXmlParserBase & orc_XmlParser)
{
   //Files
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(mc_SECURITY) == mc_SECURITY);
   mh_SavePemConfigOption(mc_SECURITY_AUTHENTICATION, orc_CurrentNode.q_SendSecureAuthenticationEnabledState,
                          orc_CurrentNode.q_SecureAuthenticationEnabled, true, orc_XmlParser,
                          orc_CurrentNode.c_PemFile);
   mh_SavePemConfigOption(mc_SECURITY_TRAFFIC_ENCRYPTION, orc_CurrentNode.q_SendTrafficEncryptionEnabledState,
                          orc_CurrentNode.q_TrafficEncryptionEnabled, false, orc_XmlParser);
   mh_SavePemConfigOption(mc_SECURITY_DEBUGGER_CONFIG, orc_CurrentNode.q_SendDebuggerEnabledState,
                          orc_CurrentNode.q_DebuggerEnabled, false, orc_XmlParser);

   //Return for next node
   tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save PEM file configuration option

   \param[in]      orc_NodeName        Node name
   \param[in]      oq_SendOption       Send option
   \param[in]      oq_EnabledOption    Enabled option
   \param[in]      oq_AddFileOption    Add file option
   \param[in,out]  orc_XmlParser       Xml parser
   \param[in]      oc_FileName         File name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_SavePemConfigOption(const std::string & orc_NodeName,
                                                         const bool oq_SendOption, const bool oq_EnabledOption,
                                                         const bool oq_AddFileOption,
                                                         C_OscXmlParserBase & orc_XmlParser,
                                                         const std::string oc_FileName)
{
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_NodeName) == orc_NodeName);
   orc_XmlParser.SetAttributeBool(mc_SECURITY_SEND_ATTR, oq_SendOption);
   orc_XmlParser.SetAttributeBool(mc_SECURITY_ENAB_ATTR, oq_EnabledOption);
   if (oq_AddFileOption)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(mc_PEM_FILE) == mc_PEM_FILE);
      orc_XmlParser.SetAttributeString(mc_FILE_NAME_ATTR, oc_FileName);
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
   }
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == mc_SECURITY);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save signature file

   \param[in]      orc_Node         Node
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupNodeDefinitionFiler::mh_SaveSignatureFile(const C_OscSupNodeDefinition & orc_Node,
                                                       C_OscXmlParserBase & orc_XmlParser)
{
   if (orc_Node.u8_SignaturePresent == hu8_ACTIVE_NODE)
   {
      //File
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(mc_SIG_FILE) == mc_SIG_FILE);
      orc_XmlParser.SetAttributeString(mc_SIG_FILE_ATTR, orc_Node.c_SignatureFile);
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load signature file

   \param[in]      orc_NodeFolderAbs   Node folder abs
   \param[in,out]  orc_Signature       Signature
   \param[in,out]  orc_XmlParser       Xml parser

   \return
   std::error_code

   \retval   Errc::success   File read
   \retval   Errc::rd_wr     File not read
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::mh_LoadSignatureFile(const std::string & orc_NodeFolderAbs,
                                                                  std::string & orc_Signature,
                                                                  C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild(mc_SIG_FILE) == mc_SIG_FILE)
   {
      const std::string c_XmlAttr = orc_XmlParser.GetAttributeString(mc_SIG_FILE_ATTR);
      const std::string c_PackagePathTmp = TglFileIncludeTrailingDelimiter(orc_NodeFolderAbs) +
                                                     TglExtractFileName(c_XmlAttr);
      c_Retval = C_OscSupSignatureFiler::h_LoadSignatureFile(c_PackagePathTmp, orc_Signature);

      tgl_assert(orc_XmlParser.SelectNodeParent() == mc_ROOT_NAME);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check file version

   \param[in]      oq_UseMinorVersion1    Use minor version 1
   \param[in,out]  orc_XmlParser          Xml parser

   \return
   std::error_code

   \retval   Errc::success   No error
   \retval   Errc::config    Input invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupNodeDefinitionFiler::mh_CheckFileVersion(const bool oq_UseMinorVersion1,
                                                                 C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild("file-version") == "file-version")
   {
      uint16_t u16_FileVersion = 0U;
      try
      {
          u16_FileVersion = static_cast<uint16_t>(std::stoi(orc_XmlParser.GetNodeContent()));
      }
      catch (...)
      {
         osc_write_log_error("Loading secure update collection definition",
                             "\"file-version\" could not be converted to a number.");
         c_Retval = Errc::config;
      }

      //is the file version one we know ?
      if (!c_Retval)
      {
         osc_write_log_info("Loading secure update collection definition", "Value of \"file-version\": " +
                            std::to_string(u16_FileVersion));
         //Check file version
         if (((oq_UseMinorVersion1 == false) && (u16_FileVersion != 1U)) ||
             ((oq_UseMinorVersion1 == true) && (u16_FileVersion != 0x101U)))
         {
            osc_write_log_error("Loading secure update collection definition",
                                "Version defined by \"file-version\" is not supported.");
            c_Retval = Errc::config;
         }
      }

      //Return
      orc_XmlParser.SelectNodeParent();
   }
   else
   {
      osc_write_log_error("Loading secure update collection definition", "Could not find \"file-version\" node.");
      c_Retval = Errc::config;
   }
   return c_Retval;
}
