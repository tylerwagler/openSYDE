//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       osy_tsp_convert: convert V2 (.syde_tsp file-version="0x0002") Target Support Packages to V3.

   Reads a legacy V2 TSP using C_OscTargetSupportPackageV2Filer and produces a V3 TSP wrapper plus a bundled
   node-definition zip containing a synthesized C_OscNode (only TSP-level metadata + applications). Datapools,
   COM protocols and HALC are not present in V2 TSPs and are written empty in V3.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include "C_SclString.hpp"
#include "TglFile.hpp"
#include "TglUtils.hpp"

#include "C_OscLoggingHandler.hpp"
#include "C_OscNode.hpp"
#include "C_OscNodeApplication.hpp"
#include "C_OscNodeFiler.hpp"
#include "C_OscNodeOpenSydeServerSettings.hpp"
#include "C_OscNodeProperties.hpp"
#include "C_OscTargetSupportPackageV2.hpp"
#include "C_OscTargetSupportPackageV2Filer.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscZipFile.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
static const C_SclString mhc_TOOL_NAME = "osy_tsp_convert";

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Print short usage banner. */
//----------------------------------------------------------------------------------------------------------------------
static void mh_PrintUsage(void)
{
   std::cout << "Usage: " << mhc_TOOL_NAME.c_str() << " <input-v2.syde_tsp> <output-v3.syde_tsp>\n"
             << "\n"
             << "Converts a legacy openSYDE V2 Target Support Package to V3 format.\n"
             << "Writes a wrapper XML at <output-v3.syde_tsp> plus a sibling <node-name>.zip\n"
             << "node-definition bundle.\n";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Make a string usable as a C identifier / file basename.

   Replace any character outside [A-Za-z0-9_] with '_'. Used to derive a node name from a device name
   (V2 device names like "ESX-4CS-GW" become "ESX_4CS_GW").
*/
//----------------------------------------------------------------------------------------------------------------------
static C_SclString mh_Sanitize(const C_SclString & orc_Input)
{
   C_SclString c_Result;
   for (uint32_t u32_Index = 1U; u32_Index <= orc_Input.Length(); ++u32_Index)
   {
      const char_t cn_Char = orc_Input[u32_Index];
      const bool q_Ok = ((cn_Char >= 'A') && (cn_Char <= 'Z')) ||
                        ((cn_Char >= 'a') && (cn_Char <= 'z')) ||
                        ((cn_Char >= '0') && (cn_Char <= '9')) ||
                        (cn_Char == '_');
      c_Result += q_Ok ? cn_Char : '_';
   }
   if (c_Result.Length() == 0U)
   {
      c_Result = "node";
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Map a V2 application entry onto a V3 C_OscNodeApplication. */
//----------------------------------------------------------------------------------------------------------------------
static C_OscNodeApplication mh_ConvertApplication(const C_OscTspApplication & orc_V2App)
{
   C_OscNodeApplication c_App;

   if (orc_V2App.q_GeneratesPsiFiles == true)
   {
      c_App.e_Type = C_OscNodeApplication::ePARAMETER_SET_HALC;
   }
   else if (orc_V2App.q_IsProgrammable == true)
   {
      c_App.e_Type = C_OscNodeApplication::ePROGRAMMABLE_APPLICATION;
   }
   else
   {
      c_App.e_Type = C_OscNodeApplication::eBINARY;
   }

   c_App.q_Active = true;
   c_App.u8_ProcessId = orc_V2App.u8_ProcessId;
   c_App.c_Name = orc_V2App.c_Name;
   c_App.c_Comment = orc_V2App.c_Comment;
   c_App.c_ProjectPath = orc_V2App.c_ProjectFolder;
   c_App.c_IdeCall = orc_V2App.c_IdeCall;
   // V2 had a "is-standard-opensyde" flag separate from path; in V3, an empty path means
   // "use the standard SYDE Coder C". Honour the V2 flag by clearing the path in that case.
   c_App.c_CodeGeneratorPath = orc_V2App.q_IsStandardSydeCoderCe ? C_SclString("") : orc_V2App.c_CodeGeneratorPath;
   c_App.c_GeneratePath = orc_V2App.c_GeneratePath;
   c_App.u16_GenCodeVersion = orc_V2App.u16_GenCodeVersion;
   c_App.c_ResultPaths = orc_V2App.c_ResultPaths;

   return c_App;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Build a synthetic V3 C_OscNode from V2 TSP metadata. */
//----------------------------------------------------------------------------------------------------------------------
static C_OscNode mh_BuildNodeFromV2(const C_OscTargetSupportPackageV2 & orc_V2)
{
   C_OscNode c_Node;

   c_Node.Initialize();
   c_Node.c_DeviceType = orc_V2.c_DeviceName;
   c_Node.c_Properties.c_Name = mh_Sanitize(orc_V2.c_DeviceName);
   c_Node.c_Properties.c_Comment = orc_V2.c_Comment;

   // Carry V2 server settings forward; Initialize() has already filled the rest with defaults.
   c_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxParallelTransmissions = orc_V2.u8_MaxParallelTransmissions;
   c_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxMessageBufferTx = orc_V2.u16_MaxMessageBufferTx;
   c_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxRoutingMessageBufferRx = orc_V2.u16_MaxRoutingMessageBufferRx;
   c_Node.c_Properties.c_OpenSydeServerSettings.s16_DpdDataBlockIndex =
      static_cast<int16_t>(orc_V2.u8_ApplicationIndex);

   c_Node.c_Properties.c_CodeExportSettings = orc_V2.c_CodeExportSettings;

   c_Node.c_Applications.clear();
   for (uint32_t u32_Index = 0U; u32_Index < orc_V2.c_Applications.size(); ++u32_Index)
   {
      c_Node.c_Applications.push_back(mh_ConvertApplication(orc_V2.c_Applications[u32_Index]));
   }

   // Datapools, COM protocols, HALC, X-app properties: leave at the Initialize() defaults (empty).
   return c_Node;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write a minimal valid node_ui.xml for a node with no datapools, no protocols, no bus connections.

   The V3 loader requires:
     - root "opensyde-node-ui-definition"
     - "file-version" = 1
     - "node" element containing "com-protocols", "data-pools", "busconnections" (the loader insists on
       busconnections being present even if empty).

   \return C_NO_ERR on success, C_RD_WR on file write failure.
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_WriteEmptyUiNodeXml(const C_SclString & orc_FilePath)
{
   C_OscXmlParser c_XmlParser;

   c_XmlParser.CreateAndSelectNodeChild("opensyde-node-ui-definition");
   c_XmlParser.CreateNodeChild("file-version", "1");

   tgl_assert(c_XmlParser.CreateAndSelectNodeChild("node") == "node");
   {
      c_XmlParser.CreateAndSelectNodeChild("com-protocols");
      c_XmlParser.SetAttributeUint32("length", 0U);
      tgl_assert(c_XmlParser.SelectNodeParent() == "node");

      c_XmlParser.CreateAndSelectNodeChild("data-pools");
      c_XmlParser.SetAttributeUint32("length", 0U);
      tgl_assert(c_XmlParser.SelectNodeParent() == "node");

      // "box" is optional for the loader, but the GUI's BoxBase loader expects standard fields if present;
      // safer to omit entirely than to emit a partial box.

      // "busconnections" is required (load returns C_CONFIG if missing)
      c_XmlParser.CreateAndSelectNodeChild("busconnections");
      tgl_assert(c_XmlParser.SelectNodeParent() == "node");
   }

   return c_XmlParser.SaveToFile(orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write the V3 TSP wrapper XML.

   \param[in]  orc_OutputTspPath   Full path to the .syde_tsp file to create.
   \param[in]  orc_DeviceName      Device-name string for V3 <device-name>.
   \param[in]  orc_Comment         Comment string for V3 <tsp-comment>.
   \param[in]  orc_NodeZipName     Filename only of the bundled node-definition zip (sibling of the wrapper).
   \param[in]  orc_TemplatePath    Optional template archive path (e.g. ./foo.syde_tp). Pass empty to omit.
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_WriteWrapperXml(const C_SclString & orc_OutputTspPath, const C_SclString & orc_DeviceName,
                                  const C_SclString & orc_Comment, const C_SclString & orc_NodeZipName,
                                  const C_SclString & orc_TemplatePath)
{
   C_OscXmlParser c_XmlParser;

   c_XmlParser.CreateAndSelectNodeChild("opensyde-target-support-package");
   c_XmlParser.CreateNodeChild("file-version", "3");
   c_XmlParser.CreateNodeChild("device-name", orc_DeviceName);
   c_XmlParser.CreateNodeChild("tsp-comment", orc_Comment);
   c_XmlParser.CreateNodeChild("node-definition", orc_NodeZipName);

   if (orc_TemplatePath.Length() > 0U)
   {
      c_XmlParser.CreateAndSelectNodeChild("template-project");
      c_XmlParser.CreateNodeChild("template", orc_TemplatePath);
      tgl_assert(c_XmlParser.SelectNodeParent() == "opensyde-target-support-package");
   }

   return c_XmlParser.SaveToFile(orc_OutputTspPath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Recursively delete a directory tree. Best-effort, errors are logged but ignored. */
//----------------------------------------------------------------------------------------------------------------------
static void mh_RemoveTree(const C_SclString & orc_Path)
{
   if (TglDirectoryExists(orc_Path))
   {
      const C_SclString c_Cmd = "rm -rf '" + orc_Path + "'";
      const int x_Rc = std::system(c_Cmd.c_str());
      (void)x_Rc;
   }
}

//----------------------------------------------------------------------------------------------------------------------
int main(const int argc, char_t * const opacn_Argv[])
{
   if (argc != 3)
   {
      mh_PrintUsage();
      return 1;
   }

   const C_SclString c_InputPath = opacn_Argv[1];
   const C_SclString c_OutputPath = opacn_Argv[2];

   // Send core log messages to stdout so failures surface in CI / interactive runs.
   C_OscLoggingHandler::h_SetWriteToConsoleActive(true);

   if (!TglFileExists(c_InputPath))
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": input file not found: " << c_InputPath.c_str() << "\n";
      return 2;
   }

   // --- 1) Load V2 TSP --------------------------------------------------------------------------------------------
   C_OscTargetSupportPackageV2 c_V2;
   int32_t s32_Retval = C_OscTargetSupportPackageV2Filer::h_Load(c_V2, c_InputPath);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": failed to load V2 TSP (rc=" << s32_Retval << ")\n";
      return 3;
   }

   // --- 2) Synthesize V3 node from V2 metadata --------------------------------------------------------------------
   const C_OscNode c_Node = mh_BuildNodeFromV2(c_V2);
   const C_SclString c_NodeBaseName = mh_Sanitize(c_V2.c_DeviceName);

   // --- 3) Stage node_core.xml + node_ui.xml in a temp directory --------------------------------------------------
   const C_SclString c_OutputDir = TglExtractFilePath(c_OutputPath);
   const C_SclString c_StageDir = TglFileIncludeTrailingDelimiter(c_OutputDir) +
                                  c_NodeBaseName + "_v3_stage";
   mh_RemoveTree(c_StageDir);
   if (TglCreateDirectory(c_StageDir) != 0)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": could not create staging dir " << c_StageDir.c_str() << "\n";
      return 4;
   }

   const C_SclString c_OscNodePath = TglFileIncludeTrailingDelimiter(c_StageDir) + C_OscNodeFiler::h_GetFileName();
   const C_SclString c_UiNodePath = TglFileIncludeTrailingDelimiter(c_StageDir) + "node_ui.xml";

   std::vector<C_SclString> c_CreatedFiles;
   const std::map<uint32_t, C_SclString> c_EmptyNodeMap;
   s32_Retval = C_OscNodeFiler::h_SaveNodeFile(c_Node, c_OscNodePath, &c_CreatedFiles, c_EmptyNodeMap);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": h_SaveNodeFile failed (rc=" << s32_Retval << ")\n";
      mh_RemoveTree(c_StageDir);
      return 5;
   }

   s32_Retval = mh_WriteEmptyUiNodeXml(c_UiNodePath);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": writing node_ui.xml failed (rc=" << s32_Retval << ")\n";
      mh_RemoveTree(c_StageDir);
      return 6;
   }

   // --- 4) Pack stage dir into the node-definition zip ------------------------------------------------------------
   const C_SclString c_NodeZipName = c_NodeBaseName + ".zip";
   const C_SclString c_NodeZipPath = TglFileIncludeTrailingDelimiter(c_OutputDir) + c_NodeZipName;

   // h_CreateZipFile expects ENTRIES relative to orc_SourcePath (which itself needs a trailing delimiter).
   // c_CreatedFiles from h_SaveNodeFile is relative to the node-core file's directory, which is c_StageDir.
   const C_SclString c_StageDirWithSep = TglFileIncludeTrailingDelimiter(c_StageDir);
   std::set<C_SclString> c_FilesToZip;
   c_FilesToZip.insert(C_OscNodeFiler::h_GetFileName());
   c_FilesToZip.insert("node_ui.xml");
   for (uint32_t u32_Index = 0U; u32_Index < c_CreatedFiles.size(); ++u32_Index)
   {
      c_FilesToZip.insert(c_CreatedFiles[u32_Index]);
   }

   C_SclString c_ZipError;
   s32_Retval = C_OscZipFile::h_CreateZipFile(c_StageDirWithSep, c_FilesToZip, c_NodeZipPath, &c_ZipError);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": zip creation failed (rc=" << s32_Retval << "): "
                << c_ZipError.c_str() << "\n";
      mh_RemoveTree(c_StageDir);
      return 7;
   }

   // --- 5) Write the V3 wrapper XML -------------------------------------------------------------------------------
   s32_Retval = mh_WriteWrapperXml(c_OutputPath, c_V2.c_DeviceName, c_V2.c_Comment,
                                   c_NodeZipName, c_V2.c_TemplatePath);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": writing V3 TSP failed (rc=" << s32_Retval << ")\n";
      mh_RemoveTree(c_StageDir);
      return 8;
   }

   mh_RemoveTree(c_StageDir);

   std::cout << mhc_TOOL_NAME.c_str() << ": wrote V3 TSP " << c_OutputPath.c_str() << "\n"
             << "                   node bundle " << c_NodeZipPath.c_str() << "\n";
   return 0;
}
