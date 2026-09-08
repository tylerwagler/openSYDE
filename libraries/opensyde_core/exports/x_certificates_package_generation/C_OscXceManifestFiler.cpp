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
#include "C_SclStringCompat.hpp"

#include "TglFile.hpp"
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscXceManifestFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscXceManifestFiler::hc_FILE_NAME = "manifest.syde_pkg";
const uint16_t C_OscXceManifestFiler::mhu16_FILE_VERSION_1 = 1;
const uint16_t C_OscXceManifestFiler::mhu16_PACKAGE_VERSION_1 = 1;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

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
std::error_code C_OscXceManifestFiler::h_LoadFile(C_OscXceManifest & orc_Config, const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_XmlParser.SetLogHeading("Loading manifest data");
      //the XML parser still reports the STW int32_t error convention
      c_Retval = c_XmlParser.LoadFromFile(orc_Path);
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
std::error_code C_OscXceManifestFiler::h_SaveFile(const C_OscXceManifest & orc_Config, const std::string & orc_Path)
{
   C_OscXmlParser c_XmlParser;
   std::error_code c_Retval = C_OscSystemFilerUtil::h_GetParserForNewFile(c_XmlParser, orc_Path,
                                                                          "opensyde-update-package-manifest");

   if (!c_Retval)
   {
      //node
      C_OscXceManifestFiler::h_SaveData(orc_Config, c_XmlParser);
      //Don't forget to save!
      if (c_XmlParser.SaveToFile(orc_Path))
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
std::error_code C_OscXceManifestFiler::h_LoadData(C_OscXceManifest & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   std::string c_Types;
   //the XML parser and the filer utilities still report the STW int32_t error convention
   std::error_code c_Retval = C_OscSystemFilerUtil::h_CheckVersion(orc_XmlParser, mhu16_FILE_VERSION_1, "file-version",
                                                                   "Loading manifest data");

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("package");
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.GetAttributeStringError("types", c_Types);
      if (!c_Retval)
      {
         if (c_Types != "x-app-security-certificates")
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("types",
                                                                               "expecting content to be \"x-app-security-certificates\"");
            c_Retval = Errc::config;
         }
         orc_XmlParser.SelectNodeParent();
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("x-app-security-certificates");
      if (!c_Retval)
      {
         c_Retval = C_OscSystemFilerUtil::h_CheckVersion(orc_XmlParser, mhu16_PACKAGE_VERSION_1, "package-version",
                                                         "Loading manifest data");
      }
      if (!c_Retval)
      {
         c_Retval = orc_XmlParser.SelectNodeChildError("secure-authentication");
      }
      if (!c_Retval)
      {
         c_Retval = orc_XmlParser.GetAttributeStringError("certificates-path", orc_Config.c_CertificatesPath);
         orc_XmlParser.SelectNodeParent();
      }
      if (!c_Retval)
      {
         c_Retval = C_OscXceManifestFiler::mh_LoadUpdatePackageParameters(orc_Config.c_UpdatePackageParameters,
                                                                          orc_XmlParser);
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
void C_OscXceManifestFiler::h_SaveData(const C_OscXceManifest & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   //File version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("file-version") == "file-version");
   orc_XmlParser.SetNodeContent(std::to_string(mhu16_FILE_VERSION_1));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Package
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("package") == "package");
   orc_XmlParser.SetAttributeString("types", "x-app-security-certificates");
   //Return
   orc_XmlParser.SelectNodeParent();
   //Config
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("x-app-security-certificates") == "x-app-security-certificates");
   orc_XmlParser.CreateNodeChild("package-version", std::to_string(mhu16_PACKAGE_VERSION_1));
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("secure-authentication") == "secure-authentication");
   orc_XmlParser.SetAttributeString("certificates-path", orc_Config.c_CertificatesPath);
   //Return
   orc_XmlParser.SelectNodeParent();
   C_OscXceManifestFiler::mh_SaveUpdatePackageParameters(orc_Config.c_UpdatePackageParameters, orc_XmlParser);
   //Return
   orc_XmlParser.SelectNodeParent();

   //Return
   orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load update package parameters

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   Errc::success  data read
   Errc::config   manifest file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscXceManifestFiler::mh_LoadUpdatePackageParameters(
   std::vector<C_OscXceUpdatePackageParameters> & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   //the XML parser still reports the STW int32_t error convention
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("secure-update");

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("update-package-parameters-list");
      if (!c_Retval)
      {
         uint32_t u32_ExpectedCount;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("num-parameters", u32_ExpectedCount);
         if (!c_Retval)
         {
            uint32_t u32_ActualCount = 0UL;
            std::string c_NodeUpdatePackageParameters = orc_XmlParser.SelectNodeChild(
               "update-package-parameters");
            //Clear any existing configuration
            orc_Config.clear();
            if (c_NodeUpdatePackageParameters == "update-package-parameters")
            {
               do
               {
                  C_OscXceUpdatePackageParameters c_UpdatePackageParameters;
                  c_Retval = mh_LoadUpdatePackageParameter(c_UpdatePackageParameters, orc_XmlParser);
                  if (!c_Retval)
                  {
                     orc_Config.push_back(c_UpdatePackageParameters);
                     //Count
                     ++u32_ActualCount;
                     //Iterate
                     c_NodeUpdatePackageParameters = orc_XmlParser.SelectNodeNext("update-package-parameters");
                  }
               }
               while ((c_NodeUpdatePackageParameters == "update-package-parameters") && (!c_Retval));
               if (!c_Retval)
               {
                  //Return
                  tgl_assert(orc_XmlParser.SelectNodeParent() == "update-package-parameters-list");
               }
            }
            if (u32_ExpectedCount != u32_ActualCount)
            {
               std::string c_Tmp;
               c_Tmp = PrintFormattedCompat("Unexpected update package parameters count, expected: %u, got %u",
                                    u32_ExpectedCount,
                                    u32_ActualCount);
               orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
            }
         }
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      //Return
      orc_XmlParser.SelectNodeParent();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save update package parameters

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXceManifestFiler::mh_SaveUpdatePackageParameters(
   const std::vector<C_OscXceUpdatePackageParameters> & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("secure-update") == "secure-update");
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(
                 "update-package-parameters-list") == "update-package-parameters-list");
   orc_XmlParser.SetAttributeUint32("num-parameters", static_cast<uint32_t>(orc_Config.size()));
   for (uint32_t u32_It = 0UL; u32_It < orc_Config.size(); ++u32_It)
   {
      C_OscXceManifestFiler::mh_SaveUpdatePackageParameter(orc_Config[u32_It], orc_XmlParser);
   }
   //Return
   orc_XmlParser.SelectNodeParent();
   //Return
   orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load update package parameter

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   Errc::success  data read
   Errc::config   manifest file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscXceManifestFiler::mh_LoadUpdatePackageParameter(C_OscXceUpdatePackageParameters & orc_Config,
                                                                     const C_OscXmlParserBase & orc_XmlParser)
{
   //the XML parser still reports the STW int32_t error convention
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError("password", orc_Config.c_Password);

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.GetAttributeStringError("authentication_key", orc_Config.c_AuthenticationKeyPath);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save update package parameter

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXceManifestFiler::mh_SaveUpdatePackageParameter(const C_OscXceUpdatePackageParameters & orc_Config,
                                                          C_OscXmlParserBase & orc_XmlParser)
{
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("update-package-parameters") == "update-package-parameters");
   orc_XmlParser.SetAttributeString("password", orc_Config.c_Password);
   orc_XmlParser.SetAttributeString("authentication_key", orc_Config.c_AuthenticationKeyPath);
   //Return
   orc_XmlParser.SelectNodeParent();
}
