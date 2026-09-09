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

#include <system_error>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "stwerrors.hpp"
#include "stwtypes.hpp"

#include <string>
#include "TglFile.hpp"
#include "TglUtils.hpp"

#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceManager.hpp"
#include "C_OscSubDeviceDefinition.hpp"
#include "C_OscSystemBus.hpp"
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
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::tgl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
static const std::string mhc_TOOL_NAME = "osy_tsp_convert";
// V2 TSPs encode the standard SYDE Coder C as q_IsStandardSydeCoderCe=true with empty c_CodeGeneratorPath.
// V3 nodes carry an explicit path string (the V2 importer in the GUI resolved it the same way).
// Mirror C_ImpUtil::h_GetSydeCoderCePath() — Windows uses .exe, every other platform doesn't.
#ifdef _WIN32
static const std::string mhc_STANDARD_SYDE_CODER_C_PATH = "../connectors/syde_coder_c/osy_syde_coder_c.exe";
#else
static const std::string mhc_STANDARD_SYDE_CODER_C_PATH = "../connectors/syde_coder_c/osy_syde_coder_c";
#endif

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Print short usage banner. */
//----------------------------------------------------------------------------------------------------------------------
static void mh_PrintUsage(void)
{
   std::cout << "Usage: " << mhc_TOOL_NAME.c_str()
             << " [--device-library <dir>] <input-v2.syde_tsp> <output-v3.syde_tsp>\n"
             << "\n"
             << "Converts a legacy openSYDE V2 Target Support Package to V3 format.\n"
             << "Writes a wrapper XML at <output-v3.syde_tsp> plus a sibling <node-name>.zip\n"
             << "node-definition bundle.\n"
             << "\n"
             << "  --device-library <dir>  Root of the openSYDE device library used to look up the\n"
             << "                          target device's communication-interface count\n"
             << "                          (default: $HOME/.local/opt/openSYDE/devices). Without a\n"
             << "                          matching device.syd, the V3 import will lose bus wiring\n"
             << "                          on every import because the interface count won't match.\n";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Make a string usable as a C identifier / file basename.

   Replace any character outside [A-Za-z0-9_] with '_'. Used to derive a node name from a device name
   (V2 device names like "ESX-4CS-GW" become "ESX_4CS_GW").
*/
//----------------------------------------------------------------------------------------------------------------------
static std::string mh_Sanitize(const std::string & orc_Input)
{
   std::string c_Result;
   for (uint32_t u32_Index = 1U; u32_Index <= orc_Input.length(); ++u32_Index)
   {
      const char_t cn_Char = orc_Input[u32_Index];
      const bool q_Ok = ((cn_Char >= 'A') && (cn_Char <= 'Z')) ||
                        ((cn_Char >= 'a') && (cn_Char <= 'z')) ||
                        ((cn_Char >= '0') && (cn_Char <= '9')) ||
                        (cn_Char == '_');
      c_Result += q_Ok ? cn_Char : '_';
   }
   if (c_Result.length() == 0U)
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
   // V2 had a "is-standard-opensyde" flag separate from path; the GUI's V2 importer resolved that
   // flag to an explicit path (C_ImpUtil::h_GetSydeCoderCePath()) when stamping the V3 node.
   c_App.c_CodeGeneratorPath = orc_V2App.q_IsStandardSydeCoderCe ? mhc_STANDARD_SYDE_CODER_C_PATH
                               : orc_V2App.c_CodeGeneratorPath;
   c_App.c_GeneratePath = orc_V2App.c_GeneratePath;
   c_App.u16_GenCodeVersion = orc_V2App.u16_GenCodeVersion;
   c_App.c_ResultPaths = orc_V2App.c_ResultPaths;

   return c_App;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Build a synthetic V3 C_OscNode from V2 TSP metadata.

   \param[in]  orc_V2          Loaded V2 TSP struct
   \param[in]  opc_DeviceDef   Device definition for orc_V2.c_DeviceName, or NULL if unavailable.
                               When non-NULL, populates c_ComInterfaces matching the device's CAN+ETH
                               count via CreateComInterfaces (so V3 import's mh_KeepTspProperties
                               size-check passes and bus wiring is preserved).
*/
//----------------------------------------------------------------------------------------------------------------------
static C_OscNode mh_BuildNodeFromV2(const C_OscTargetSupportPackageV2 & orc_V2,
                                    const C_OscDeviceDefinition * const opc_DeviceDef)
{
   C_OscNode c_Node;

   c_Node.Initialize();
   c_Node.c_DeviceType = orc_V2.c_DeviceName;
   c_Node.c_Properties.c_Name = mh_Sanitize(orc_V2.c_DeviceName);
   c_Node.c_Properties.c_Comment = orc_V2.c_Comment;

   // V2 TSPs targeted openSYDE devices exclusively, and the GUI's V2 importer left the existing node's
   // protocol selection alone (which was open-syde for any new ESX-class node). Match that.
   c_Node.c_Properties.e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
   c_Node.c_Properties.e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;

   // Pad c_ComInterfaces to match the device's CAN+ETH count (default IPs/node-IDs).
   // mh_KeepTspProperties asserts size equality before copying bus connections; without this the
   // V3 importer drops every existing bus wiring on import.
   if (opc_DeviceDef != nullptr)
   {
      c_Node.c_Properties.CreateComInterfaces(*opc_DeviceDef, 0U);

      // Mirror what the GUI's m_InitNodeComIfSettings does after CreateComInterfaces: stamp the per-interface
      // protocol-capability flags from the device definition. CreateComInterfaces leaves them all false,
      // which would surface in the GUI as a node that "can't be flashed/diagnosed" until the user toggles
      // each interface manually.
      if (!opc_DeviceDef->c_SubDevices.empty())
      {
         const C_OscSubDeviceDefinition & rc_SubDevice = opc_DeviceDef->c_SubDevices[0];
         const bool q_DiagOpenSyde = (c_Node.c_Properties.e_DiagnosticServer == C_OscNodeProperties::eDS_OPEN_SYDE);
         for (uint32_t u32_Index = 0U; u32_Index < c_Node.c_Properties.c_ComInterfaces.size(); ++u32_Index)
         {
            C_OscNodeComInterfaceSettings & rc_Intf = c_Node.c_Properties.c_ComInterfaces[u32_Index];
            rc_Intf.q_IsUpdateEnabled = rc_SubDevice.IsUpdateAvailable(rc_Intf.e_InterfaceType);
            // Routing and diagnosis are gated on diagnostic server being openSYDE (mirrors C_OscNode helpers).
            rc_Intf.q_IsRoutingEnabled = q_DiagOpenSyde && rc_SubDevice.IsRoutingAvailable(rc_Intf.e_InterfaceType);
            rc_Intf.q_IsDiagnosisEnabled = q_DiagOpenSyde &&
                                           rc_SubDevice.IsDiagnosisAvailable(rc_Intf.e_InterfaceType);
         }
      }
   }

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
static int32_t mh_WriteEmptyUiNodeXml(const std::string & orc_FilePath)
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

   //the XML parser reports std::error_code now; this class keeps the STW int32_t convention
   return c_XmlParser.SaveToFile(orc_FilePath).value();
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
static int32_t mh_WriteWrapperXml(const std::string & orc_OutputTspPath, const std::string & orc_DeviceName,
                                  const std::string & orc_Comment, const std::string & orc_NodeZipName,
                                  const std::string & orc_TemplatePath)
{
   C_OscXmlParser c_XmlParser;

   c_XmlParser.CreateAndSelectNodeChild("opensyde-target-support-package");
   c_XmlParser.CreateNodeChild("file-version", "3");
   c_XmlParser.CreateNodeChild("device-name", orc_DeviceName);
   c_XmlParser.CreateNodeChild("tsp-comment", orc_Comment);
   c_XmlParser.CreateNodeChild("node-definition", orc_NodeZipName);

   if (orc_TemplatePath.length() > 0U)
   {
      c_XmlParser.CreateAndSelectNodeChild("template-project");
      c_XmlParser.CreateNodeChild("template", orc_TemplatePath);
      tgl_assert(c_XmlParser.SelectNodeParent() == "opensyde-target-support-package");
   }

   //the XML parser reports std::error_code now; this class keeps the STW int32_t convention
   return c_XmlParser.SaveToFile(orc_OutputTspPath).value();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Recursively delete a directory tree. Best-effort, errors are logged but ignored. */
//----------------------------------------------------------------------------------------------------------------------
static void mh_RemoveTree(const std::string & orc_Path)
{
   if (TglDirectoryExists(orc_Path))
   {
      const std::string c_Cmd = "rm -rf '" + orc_Path + "'";
      const int x_Rc = std::system(c_Cmd.c_str());
      (void)x_Rc;
   }
}

//----------------------------------------------------------------------------------------------------------------------
int main(const int argc, char_t * const opacn_Argv[])
{
   // Parse args: optional `--device-library <dir>` followed by two positional paths.
   std::string c_InputPath;
   std::string c_OutputPath;
   std::string c_DeviceLibPath;
   {
      const char_t * const pcn_Home = std::getenv("HOME");
      if (pcn_Home != nullptr)
      {
         c_DeviceLibPath = std::string(pcn_Home) + "/.local/opt/openSYDE/devices";
      }
   }

   std::vector<std::string> c_Positional;
   for (int32_t s32_Index = 1; s32_Index < argc; ++s32_Index)
   {
      const std::string c_Arg = opacn_Argv[s32_Index];
      if (c_Arg == "--device-library")
      {
         if ((s32_Index + 1) >= argc)
         {
            std::cerr << mhc_TOOL_NAME.c_str() << ": --device-library requires an argument\n";
            return 1;
         }
         c_DeviceLibPath = opacn_Argv[s32_Index + 1];
         ++s32_Index;
      }
      else if ((c_Arg == "-h") || (c_Arg == "--help"))
      {
         mh_PrintUsage();
         return 0;
      }
      else
      {
         c_Positional.push_back(c_Arg);
      }
   }

   if (c_Positional.size() != 2U)
   {
      mh_PrintUsage();
      return 1;
   }
   c_InputPath = c_Positional[0];
   c_OutputPath = c_Positional[1];

   // Send core log messages to stdout so failures surface in CI / interactive runs.
   C_OscLoggingHandler::h_SetWriteToConsoleActive(true);

   if (!TglFileExists(c_InputPath))
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": input file not found: " << c_InputPath.c_str() << "\n";
      return 2;
   }

   // --- 1) Load V2 TSP --------------------------------------------------------------------------------------------
   C_OscTargetSupportPackageV2 c_V2;
   //the filer reports std::error_code now; this tool still runs on the STW int32_t convention
   int32_t s32_Retval = C_OscTargetSupportPackageV2Filer::h_Load(c_V2, c_InputPath).value();
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": failed to load V2 TSP (rc=" << s32_Retval << ")\n";
      return 3;
   }

   // --- 2) Look up the device definition (best-effort) ------------------------------------------------------------
   // Without it the synthesized node ships zero com-interfaces, which makes the V3 importer drop bus wiring.
   C_OscDeviceManager c_DeviceManager;
   const C_OscDeviceDefinition * pc_DeviceDef = nullptr;
   uint32_t u32_SubDeviceIndex = 0U;
   if (TglDirectoryExists(c_DeviceLibPath))
   {
      std::vector<std::string> c_Roots;
      c_Roots.push_back(c_DeviceLibPath);
      //the core class reports std::error_code now; this class keeps the STW int32_t convention
      const int32_t s32_LoadRc = c_DeviceManager.LoadFromPaths(c_Roots).value();
      if (s32_LoadRc == C_NO_ERR)
      {
         pc_DeviceDef = c_DeviceManager.LookForDevice(c_V2.c_DeviceName, "", u32_SubDeviceIndex);
      }
      if (pc_DeviceDef == nullptr)
      {
         std::cerr << mhc_TOOL_NAME.c_str() << ": warning: device \"" << c_V2.c_DeviceName.c_str()
                   << "\" not found under " << c_DeviceLibPath.c_str()
                   << "; output will ship zero communication-interfaces (V3 import will drop bus wiring)\n";
      }
   }
   else
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": warning: device library not found at "
                << c_DeviceLibPath.c_str()
                << "; output will ship zero communication-interfaces (V3 import will drop bus wiring)\n";
   }

   // --- 3) Synthesize V3 node from V2 metadata --------------------------------------------------------------------
   const C_OscNode c_Node = mh_BuildNodeFromV2(c_V2, pc_DeviceDef);
   const std::string c_NodeBaseName = mh_Sanitize(c_V2.c_DeviceName);

   // --- 3) Stage node_core.xml + node_ui.xml in a temp directory --------------------------------------------------
   const std::string c_OutputDir = TglExtractFilePath(c_OutputPath);
   const std::string c_StageDir = TglFileIncludeTrailingDelimiter(c_OutputDir) +
                                  c_NodeBaseName + "_v3_stage";
   mh_RemoveTree(c_StageDir);
   if (TglCreateDirectory(c_StageDir) != 0)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": could not create staging dir " << c_StageDir.c_str() << "\n";
      return 4;
   }

   const std::string c_OscNodePath = TglFileIncludeTrailingDelimiter(c_StageDir) + C_OscNodeFiler::h_GetFileName();
   const std::string c_UiNodePath = TglFileIncludeTrailingDelimiter(c_StageDir) + "node_ui.xml";

   std::vector<std::string> c_CreatedFiles;
   const std::map<uint32_t, std::string> c_EmptyNodeMap;
   //likewise: convert at the boundary, the local stays on the STW int32_t convention
   s32_Retval = C_OscNodeFiler::h_SaveNodeFile(c_Node, c_OscNodePath, &c_CreatedFiles, c_EmptyNodeMap).value();
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
   const std::string c_NodeZipName = c_NodeBaseName + ".zip";
   const std::string c_NodeZipPath = TglFileIncludeTrailingDelimiter(c_OutputDir) + c_NodeZipName;

   // h_CreateZipFile expects ENTRIES relative to orc_SourcePath (which itself needs a trailing delimiter).
   // c_CreatedFiles from h_SaveNodeFile is relative to the node-core file's directory, which is c_StageDir.
   const std::string c_StageDirWithSep = TglFileIncludeTrailingDelimiter(c_StageDir);
   std::set<std::string> c_FilesToZip;
   c_FilesToZip.insert(C_OscNodeFiler::h_GetFileName());
   c_FilesToZip.insert("node_ui.xml");
   for (uint32_t u32_Index = 0U; u32_Index < c_CreatedFiles.size(); ++u32_Index)
   {
      c_FilesToZip.insert(c_CreatedFiles[u32_Index]);
   }

   std::string c_ZipError;
   //C_OscZipFile now reports std::error_code; this local is shared with unmigrated calls
   s32_Retval = C_OscZipFile::h_CreateZipFile(c_StageDirWithSep, c_FilesToZip, c_NodeZipPath,
                                              &c_ZipError).value();
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": zip creation failed (rc=" << s32_Retval << "): "
                << c_ZipError.c_str() << "\n";
      mh_RemoveTree(c_StageDir);
      return 7;
   }

   // --- 5) Copy the optional template archive (.syde_tp) next to the V3 wrapper -----------------------------------
   // The V2 wrapper carries a path like "./foo.syde_tp" relative to the V2 TSP's directory. The GUI's V3
   // importer resolves the same string relative to the V3 wrapper, so the archive has to be co-located.
   if (c_V2.c_TemplatePath.length() > 0U)
   {
      const std::string c_InputDir = TglFileIncludeTrailingDelimiter(TglExtractFilePath(c_InputPath));
      const std::string c_TemplateSrc = c_InputDir + c_V2.c_TemplatePath;
      const std::string c_TemplateBaseName = TglExtractFileName(c_V2.c_TemplatePath);
      const std::string c_TemplateDst = TglFileIncludeTrailingDelimiter(c_OutputDir) + c_TemplateBaseName;
      if (TglFileExists(c_TemplateSrc))
      {
         std::ifstream c_Src(c_TemplateSrc.c_str(), std::ios::binary);
         std::ofstream c_Dst(c_TemplateDst.c_str(), std::ios::binary);
         c_Dst << c_Src.rdbuf();
         if (!c_Dst)
         {
            std::cerr << mhc_TOOL_NAME.c_str() << ": failed to copy template archive to "
                      << c_TemplateDst.c_str() << "\n";
            mh_RemoveTree(c_StageDir);
            return 8;
         }
      }
      else
      {
         std::cerr << mhc_TOOL_NAME.c_str() << ": warning: template archive not found at "
                   << c_TemplateSrc.c_str() << " (V3 import will fail to extract template)\n";
      }
   }

   // --- 6) Write the V3 wrapper XML -------------------------------------------------------------------------------
   // Rewrite the template path to a sibling-of-wrapper reference (basename only) so the GUI finds it next to us.
   const std::string c_TemplateForWrapper = (c_V2.c_TemplatePath.length() > 0U)
                                            ? std::string("./") + TglExtractFileName(c_V2.c_TemplatePath)
                                            : std::string("");
   s32_Retval = mh_WriteWrapperXml(c_OutputPath, c_V2.c_DeviceName, c_V2.c_Comment,
                                   c_NodeZipName, c_TemplateForWrapper);
   if (s32_Retval != C_NO_ERR)
   {
      std::cerr << mhc_TOOL_NAME.c_str() << ": writing V3 TSP failed (rc=" << s32_Retval << ")\n";
      mh_RemoveTree(c_StageDir);
      return 9;
   }

   mh_RemoveTree(c_StageDir);

   std::cout << mhc_TOOL_NAME.c_str() << ": wrote V3 TSP " << c_OutputPath.c_str() << "\n"
             << "                   node bundle " << c_NodeZipPath.c_str() << "\n";
   return 0;
}
