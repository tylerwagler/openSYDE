//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for manifest data

   Filer for manifest data

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "TglFile.hpp"
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscXcoManifestFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscXcoManifestFiler::hc_FILE_NAME = "manifest.syde_pkg";
const uint16_t C_OscXcoManifestFiler::mhu16_FILE_VERSION_1 = 1;
const uint16_t C_OscXcoManifestFiler::mhu16_PACKAGE_VERSION_1 = 1;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manifest

   \param[in,out]  orc_Config    Config
   \param[in]      orc_Path      Path

   \return
   Errc::success  data read
   Errc::range    specified manifest file does not exist
   Errc::noact    specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config   manifest file content is invalid or incomplete
                  manifest file could not be loaded
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscXcoManifestFiler::h_LoadFile(C_OscXcoManifest & orc_Config, const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_XmlParser.SetLogHeading("Loading manifest data");
      //the XML parser still reports the STW int32_t error convention
      c_Retval = make_error_code_from_stw(c_XmlParser.LoadFromFile(orc_Path));
      if (!c_Retval)
      {
         if (c_XmlParser.SelectRoot() == "opensyde-update-package-manifest")
         {
            c_Retval = h_LoadData(orc_Config, c_XmlParser);
         }
         else
         {
            osc_write_log_error("Loading manifest data",
                                "Could not find \"opensyde-update-package-manifest\" node.");
            c_Retval = Errc::config;
         }
      }
      else
      {
         osc_write_log_error("Loading manifest data", "File \"" + orc_Path + "\" could not be opened.");
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading manifest data", "File \"" + orc_Path + "\" does not exist.");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data loggers

   \param[in]  orc_Config  Config
   \param[in]  orc_Path    Path

   \return
   Errc::success  data saved
   Errc::config   data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscXcoManifestFiler::h_SaveFile(const C_OscXcoManifest & orc_Config, const std::string & orc_Path)
{
   C_OscXmlParser c_XmlParser;
   std::error_code c_Retval = make_error_code_from_stw(
      C_OscSystemFilerUtil::h_GetParserForNewFile(c_XmlParser, orc_Path, "opensyde-update-package-manifest"));

   if (!c_Retval)
   {
      //node
      C_OscXcoManifestFiler::h_SaveData(orc_Config, c_XmlParser);
      //Don't forget to save!
      if (c_XmlParser.SaveToFile(orc_Path) != C_NO_ERR)
      {
         osc_write_log_error("Saving manifest data", "Could not create file.");
         c_Retval = Errc::config;
      }
   }
   else
   {
      //More details are in log
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manifest

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success  data read
   Errc::config   manifest file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscXcoManifestFiler::h_LoadData(C_OscXcoManifest & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   std::string c_Types;
   //the XML parser and the filer utilities still report the STW int32_t error convention
   std::error_code c_Retval = make_error_code_from_stw(
      C_OscSystemFilerUtil::h_CheckVersion(orc_XmlParser, mhu16_FILE_VERSION_1, "file-version",
                                           "Loading manifest data"));

   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("package"));
   }
   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeStringError("types", c_Types));
      if (!c_Retval)
      {
         if (c_Types != "x-app-config")
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("types",
                                                                               "expecting content to be \"x-app-config\"");
            c_Retval = Errc::config;
         }
         orc_XmlParser.SelectNodeParent();
      }
   }
   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("x-app-config"));
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(
            C_OscSystemFilerUtil::h_CheckVersion(orc_XmlParser, mhu16_PACKAGE_VERSION_1, "package-version",
                                                 "Loading manifest data"));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("x-app-node"));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeStringError("name", orc_Config.c_NodeName));
         orc_XmlParser.SelectNodeParent();
      }
      orc_XmlParser.SelectNodeParent();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manifest

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXcoManifestFiler::h_SaveData(const C_OscXcoManifest & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   //File version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("file-version") == "file-version");
   orc_XmlParser.SetNodeContent(std::to_string(mhu16_FILE_VERSION_1));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Package
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("package") == "package");
   orc_XmlParser.SetAttributeString("types", "x-app-config");
   //Return
   orc_XmlParser.SelectNodeParent();
   //Config
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("x-app-config") == "x-app-config");
   orc_XmlParser.CreateNodeChild("package-version", std::to_string(mhu16_PACKAGE_VERSION_1));
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("x-app-node") == "x-app-node");
   orc_XmlParser.SetAttributeString("name", orc_Config.c_NodeName);
   //Return
   orc_XmlParser.SelectNodeParent();
   //Return
   orc_XmlParser.SelectNodeParent();

   //Return
   orc_XmlParser.SelectNodeParent();
}
