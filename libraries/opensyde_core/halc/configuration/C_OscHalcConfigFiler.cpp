//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC configuration data

   Load HALC configuration data

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <sstream>
#include <system_error>

#include "TglFile.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscHalcDefFiler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscHalcConfigFiler.hpp"
#include "C_OscHalcDefStructFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO description

   \param[out]  orc_IoData    Storage
   \param[in]   orc_Path      Path to IO description
   \param[in]   orc_BasePath  Base path

   \return
   Errc::success     data read
   Errc::range       specified system definition file does not exist
   Errc::noact       specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config      system definition file content is invalid or incomplete
                     device definition file could not be loaded
   Errc::checksum    HALC configuration config version mismatch
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_LoadFile(C_OscHalcConfig & orc_IoData, const std::string & orc_Path,
                                                 const std::string & orc_BasePath)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_XmlParser.SetLogHeading("Loading IO data");
      c_Retval = c_XmlParser.LoadFromFile(orc_Path);
      if (!c_Retval)
      {
         if (c_XmlParser.SelectRoot() == "opensyde-node-io-config")
         {
            c_Retval = h_LoadData(orc_IoData, c_XmlParser, orc_BasePath);
         }
         else
         {
            osc_write_log_error("Loading IO data", "Could not find \"opensyde-node-io-config\" node.");
            c_Retval = Errc::config;
         }
      }
      else
      {
         osc_write_log_error("Loading IO data", "File \"" + orc_Path + "\" could not be opened.");
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading IO data", "File \"" + orc_Path + "\" does not exist.");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO description

   Save IO description
   Will overwrite the file if it already exists.

   \param[in]      orc_IoData          Pointer to storage
   \param[in]      orc_Path            Path of system definition
   \param[in]      orc_BasePath        Base path
   \param[in,out]  opc_CreatedFiles    Created files

   \return
   Errc::success    data saved
   Errc::config     data invalid
   Errc::rd_wr      could not erase pre-existing file before saving
   Errc::rd_wr      could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_SaveFile(const C_OscHalcConfig & orc_IoData, const std::string & orc_Path,
                                                 const std::string & orc_BasePath,
                                                 std::vector<std::string> * const opc_CreatedFiles)
{
   std::error_code c_Retval = C_OscHalcConfigFiler::h_PrepareForFile(orc_Path);

   if (!c_Retval)
   {
      C_OscXmlParser c_XmlParser;
      c_XmlParser.CreateNodeChild("opensyde-node-io-config");
      tgl_assert(c_XmlParser.SelectRoot() == "opensyde-node-io-config");
      c_Retval = h_SaveData(orc_IoData, c_XmlParser, orc_BasePath, opc_CreatedFiles);
      if (!c_Retval)
      {
         c_Retval = c_XmlParser.SaveToFile(orc_Path);
         if (c_Retval)
         {
            osc_write_log_error("Saving IO data", "Could not write to file \"" + orc_Path + "\".");
            c_Retval = Errc::rd_wr;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO description

   \param[out]     orc_IoData       Storage
   \param[in,out]  orc_XmlParser    XML with default state
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success     data read
   Errc::config      HALC configuration content is invalid or incomplete
   Errc::checksum    HALC configuration config version mismatch
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_LoadData(C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                                 const std::string & orc_BasePath)
{
   std::error_code c_Retval = mh_LoadIoDataBase(orc_IoData, orc_XmlParser, orc_BasePath);

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("file-version");
      //File version
      if (!c_Retval)
      {
         uint16_t u16_FileVersion = 0U;
         try
         {
            u16_FileVersion = static_cast<uint16_t>(std::stoi(orc_XmlParser.GetNodeContent()));
         }
         catch (...)
         {
            orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext("could not be converted to a number");
            c_Retval = Errc::config;
         }

         //is the file version one we know ?
         if (!c_Retval)
         {
            osc_write_log_info("Loading IO description", "Value of \"file-version\": " +
                               std::to_string(u16_FileVersion));
            //Check file version
            if (u16_FileVersion != mhu16_FILE_VERSION_1)
            {
               osc_write_log_error("Loading IO description",
                                   "Version defined by \"file-version\" is not supported.");
               c_Retval = Errc::config;
            }
         }

         //Return
         orc_XmlParser.SelectNodeParent();
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("ref-content-version");
      if (!c_Retval)
      {
         uint32_t u32_RefId = 0UL;
         try
         {
            u32_RefId = static_cast<uint16_t>(std::stoi(orc_XmlParser.GetNodeContent()));
         }
         catch (...)
         {
            orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext("could not be converted to a number");
            c_Retval = Errc::config;
         }
         if (!c_Retval)
         {
            if (u32_RefId != orc_IoData.u32_ContentVersion)
            {
               orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext(
                  "mismatch between expected " +
                  std::to_string(u32_RefId) +
                  " and current " + std::to_string(orc_IoData.u32_ContentVersion));
               c_Retval = Errc::checksum;
            }
         }
         if (!c_Retval)
         {
            //Return
            orc_XmlParser.SelectNodeParent();
         }
      }
      if (!c_Retval)
      {
         c_Retval = orc_XmlParser.SelectNodeChildError("general");
         if (!c_Retval)
         {
            // No general section data. Nothing to do.
            //Return
            orc_XmlParser.SelectNodeParent();
         }
      }
      if (!c_Retval)
      {
         c_Retval = mh_LoadIoDomains(orc_IoData, orc_XmlParser);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO data

   \param[in]      orc_IoData          Pointer to storage
   \param[in,out]  orc_XmlParser       XML with default state
   \param[in]      orc_BasePath        Base path
   \param[in,out]  opc_CreatedFiles    Created files

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_SaveData(const C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser,
                                                 const std::string & orc_BasePath,
                                                 std::vector<std::string> * const opc_CreatedFiles)
{
   std::error_code c_Retval = Errc::success;

   //File version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("file-version") == "file-version");
   orc_XmlParser.SetNodeContent(std::to_string(mhu16_FILE_VERSION_1));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Content version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("ref-content-version") == "ref-content-version");
   orc_XmlParser.SetNodeContent(std::to_string(orc_IoData.u32_ContentVersion));
   //Return
   orc_XmlParser.SelectNodeParent();
   //IO data
   c_Retval = mh_SaveIoDataBase(orc_IoData, orc_XmlParser, orc_BasePath, opc_CreatedFiles);
   if (!c_Retval)
   {
      {
         //General
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("general") == "general");
         // Datablock assignment is now only in Datapools. Fill structure with dummies to be able to open HALC projects
         // always in older tool versions too.
         orc_XmlParser.SetAttributeBool("is-safe-datablock-set", false);
         orc_XmlParser.SetAttributeUint32("safe-datablock-index", 0);
         orc_XmlParser.SetAttributeBool("is-unsafe-datablock-set", false);
         orc_XmlParser.SetAttributeUint32("unsafe-datablock-index", 0);
         //Return
         orc_XmlParser.SelectNodeParent();
      }

      //Domains
      c_Retval = mh_SaveIoDomains(orc_IoData, orc_XmlParser);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Make preparations for file

   - Removing an already existing file
   - Creating the directory if it does not exist

   \param[in]  orc_Path    Path of file

   \return
   Errc::rd_wr    could not erase pre-existing file before saving
   Errc::rd_wr    could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_PrepareForFile(const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      //erase it:
      int x_Return; //lint !e970 !e8080  //using type to match library interface
      x_Return = std::remove(orc_Path.c_str());
      if (x_Return != 0)
      {
         osc_write_log_error("Saving IO data", "Could not erase pre-existing file \"" + orc_Path + "\".");
         c_Retval = Errc::rd_wr;
      }
   }
   if (!c_Retval)
   {
      const std::string c_Folder = TglExtractFilePath(orc_Path);
      if (TglDirectoryExists(c_Folder) == false)
      {
         if (TglCreateDirectory(c_Folder) != 0)
         {
            osc_write_log_error("Saving IO data", "Could not create folder \"" + c_Folder + "\".");
            c_Retval = Errc::rd_wr;
         }
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO domain

   \param[in]      orc_IoDomain     IO data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_SaveIoDomain(const C_OscHalcConfigDomain & orc_IoDomain,
                                                     C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domain") == "domain");
   //Config
   c_Retval = mh_SaveIoChannel(orc_IoDomain.c_DomainConfig, orc_XmlParser, "config", "domain");
   //Channels
   if (!c_Retval)
   {
      c_Retval = mh_SaveIoChannels(orc_IoDomain.c_ChannelConfigs, orc_XmlParser);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO domain

   \param[in,out]  orc_IoDomain     IO domain
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::h_LoadIoDomain(C_OscHalcConfigDomain & orc_IoDomain,
                                                     C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("config");

   if (!c_Retval)
   {
      c_Retval = mh_LoadIoChannel(orc_IoDomain.c_DomainConfig, orc_XmlParser, "config");
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("channels");
      if (!c_Retval)
      {
         uint32_t u32_ExpectedCount;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedCount);
         if (!c_Retval)
         {
            uint32_t u32_ActualCount = 0UL;
            std::string c_NodeChannel = orc_XmlParser.SelectNodeChild("channel");
            //Clear any existing configuration
            orc_IoDomain.c_ChannelConfigs.clear();
            if (c_NodeChannel == "channel")
            {
               do
               {
                  C_OscHalcConfigChannel c_Channel;
                  c_Retval = mh_LoadIoChannel(c_Channel, orc_XmlParser, "channel");
                  if (!c_Retval)
                  {
                     orc_IoDomain.c_ChannelConfigs.push_back(c_Channel);
                     //Count
                     ++u32_ActualCount;
                     //Iterate
                     c_NodeChannel = orc_XmlParser.SelectNodeNext("channel");
                  }
               }
               while ((c_NodeChannel == "channel") && (!c_Retval));
               if (!c_Retval)
               {
                  //Return
                  tgl_assert(orc_XmlParser.SelectNodeParent() == "channels");
               }
            }
            if (u32_ExpectedCount != u32_ActualCount)
            {
            std::ostringstream c_TmpStream;
                c_TmpStream << "Unexpected channel count, expected: " << u32_ExpectedCount << ", got " << u32_ActualCount;
            std::string c_Tmp = c_TmpStream.str();
               orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
            }
         }
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcConfigFiler::C_OscHalcConfigFiler()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO data base

   \param[in]      orc_IoData          IO data
   \param[in,out]  orc_XmlParser       XML parser
   \param[in]      orc_BasePath        Base path
   \param[in,out]  opc_CreatedFiles    Created files

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoDataBase(const C_OscHalcDefBase & orc_IoData,
                                                        C_OscXmlParserBase & orc_XmlParser,
                                                        const std::string & orc_BasePath,
                                                        std::vector<std::string> * const opc_CreatedFiles)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("io-base-file") == "io-base-file");
   if (orc_BasePath.empty())
   {
      //To string
      c_Retval = C_OscHalcDefFiler::h_SaveData(orc_IoData, orc_XmlParser);
      //Remember file name
      orc_XmlParser.SetAttributeString("original_file_name", orc_IoData.c_OriginalFileName);
      //Remember file content
      orc_XmlParser.SetAttributeString("original_file_content", orc_IoData.c_FileString);
   }
   else
   {
      const std::string c_IoDataFileName =
         (orc_IoData.c_OriginalFileName.empty() == false) ?
         orc_IoData.c_OriginalFileName : static_cast<std::string>("halc_definition_file.xml");
      const std::string c_CombinedFileName = C_OscSystemFilerUtil::h_CombinePaths(orc_BasePath,
                                                                                  c_IoDataFileName);
      c_Retval = C_OscHalcDefFiler::h_SaveFile(orc_IoData, c_CombinedFileName);
      if (c_Retval)
      {
         //More details are in log
         c_Retval = Errc::config;
      }
      else
      {
         if (opc_CreatedFiles != nullptr)
         {
            opc_CreatedFiles->push_back(c_IoDataFileName);
         }
         orc_XmlParser.SetNodeContent(c_IoDataFileName);
      }
   }
   //Return
   if (!c_Retval)
   {
      orc_XmlParser.SelectNodeParent();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO domains

   \param[in]      orc_IoData       IO data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoDomains(const C_OscHalcConfig & orc_IoData,
                                                       C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domains") == "domains");
   orc_XmlParser.SetAttributeUint32("length", orc_IoData.GetDomainSize());
   for (uint32_t u32_ItDomain = 0UL; (u32_ItDomain < orc_IoData.GetDomainSize()) && (!c_Retval);
        ++u32_ItDomain)
   {
      const C_OscHalcConfigDomain * const pc_Domain = orc_IoData.GetDomainConfigDataConst(u32_ItDomain);
      if (pc_Domain != nullptr)
      {
         c_Retval = C_OscHalcConfigFiler::h_SaveIoDomain(*pc_Domain, orc_XmlParser);

         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "domains");
         }
      }
      else
      {
         c_Retval = Errc::config;
         osc_write_log_error("Saving IO data",
                             "Could not find domain " + std::to_string(u32_ItDomain) + ".");
      }
   }
   if (!c_Retval)
   {
      //Return
      orc_XmlParser.SelectNodeParent();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO channels

   \param[in]      orc_Channels     Channels
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoChannels(const std::vector<C_OscHalcConfigChannel> & orc_Channels,
                                                        C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channels") == "channels");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_Channels.size()));
   for (uint32_t u32_ItChannel = 0UL; (u32_ItChannel < orc_Channels.size()) && (!c_Retval);
        ++u32_ItChannel)
   {
      c_Retval = mh_SaveIoChannel(orc_Channels[u32_ItChannel], orc_XmlParser, "channel", "channels");
   }
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO channel

   \param[in]      orc_Channel         Channel
   \param[in,out]  orc_XmlParser       XML parser
   \param[in]      orc_NodeName        Node name
   \param[in]      orc_NodeParentName  Node parent name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoChannel(const C_OscHalcConfigChannel & orc_Channel,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::string & orc_NodeName,
                                                       const std::string & orc_NodeParentName)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_NodeName) == orc_NodeName);
   orc_XmlParser.SetAttributeBool("safety-relevant", orc_Channel.q_SafetyRelevant);
   orc_XmlParser.SetAttributeUint32("use-case-index", orc_Channel.u32_UseCaseIndex);
   orc_XmlParser.CreateNodeChild("name", orc_Channel.c_Name.c_str());
   orc_XmlParser.CreateNodeChild("comment", orc_Channel.c_Comment.c_str());
   c_Retval = mh_SaveIoParameterStructs(orc_Channel.c_Parameters, orc_XmlParser, orc_NodeName);
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeParentName);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO parameter structs

   \param[in]      orc_ParameterStructs   Parameter structs
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      orc_NodeName           Node name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoParameterStructs(
   const std::vector<C_OscHalcConfigParameterStruct> & orc_ParameterStructs, C_OscXmlParserBase & orc_XmlParser,
   const std::string & orc_NodeName)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameter-structs") == "parameter-structs");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_ParameterStructs.size()));
   for (uint32_t u32_ItParameter = 0UL;
        (u32_ItParameter < static_cast<uint32_t>(orc_ParameterStructs.size())) && (!c_Retval);
        ++u32_ItParameter)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameter-struct") == "parameter-struct");
      c_Retval = mh_SaveIoParameterStruct(orc_ParameterStructs[u32_ItParameter], orc_XmlParser);
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-structs");
      }
   }
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO parameter struct

   \param[in]      orc_ParameterStruct    Parameter struct
   \param[in,out]  orc_XmlParser          XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoParameterStruct(
   const C_OscHalcConfigParameterStruct & orc_ParameterStruct, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_ParameterStruct.c_ParameterElements.size() > 0UL)
   {
      c_Retval = mh_SaveIoParameters(orc_ParameterStruct.c_ParameterElements, orc_XmlParser);
   }
   else
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("single-value") == "single-value");
      c_Retval = mh_SaveIoParameter(orc_ParameterStruct, orc_XmlParser, "single-value");
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-struct");
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO parameters

   \param[in]      orc_Parameters   Parameters
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoParameters(const std::vector<C_OscHalcConfigParameter> & orc_Parameters,
                                                          C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameters") == "parameters");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_Parameters.size()));
   for (uint32_t u32_ItParameter = 0UL; (u32_ItParameter < orc_Parameters.size()) && (!c_Retval);
        ++u32_ItParameter)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameter") == "parameter");
      c_Retval = mh_SaveIoParameter(orc_Parameters[u32_ItParameter], orc_XmlParser, "parameter");
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "parameters");
      }
   }
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-struct");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO parameter

   \param[in]      orc_Parameter    Parameter
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BaseNode     Base node name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_SaveIoParameter(const C_OscHalcConfigParameter & orc_Parameter,
                                                         C_OscXmlParserBase & orc_XmlParser,
                                                         const std::string & orc_BaseNode)
{
   std::error_code c_Retval = Errc::success;
   bool q_ValueHandled = false;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("value") == "value");
   switch (orc_Parameter.c_Value.GetComplexType())
   {
   case C_OscHalcDefContent::eCT_PLAIN:
      orc_XmlParser.SetAttributeString("type",
                                       C_OscHalcDefStructFiler::h_GetTypeString(orc_Parameter.c_Value.GetType()));
      break;
   case C_OscHalcDefContent::eCT_ENUM:
      orc_XmlParser.SetAttributeString("type", "enum");
      orc_XmlParser.SetAttributeString("base-type",
                                       C_OscHalcDefStructFiler::h_GetTypeString(orc_Parameter.c_Value.GetType()));
      break;
   case C_OscHalcDefContent::eCT_BIT_MASK:
      orc_XmlParser.SetAttributeString("type", "bitmask");
      orc_XmlParser.SetAttributeString("base-type",
                                       C_OscHalcDefStructFiler::h_GetTypeString(orc_Parameter.c_Value.GetType()));
      break;
   case C_OscHalcDefContent::eCT_STRING:
      {
         std::string c_Tmp;
         orc_XmlParser.SetAttributeString("type", "string");
         orc_Parameter.c_Value.GetStringValue(c_Tmp);
         orc_XmlParser.SetAttributeString("value", c_Tmp);
         orc_XmlParser.SetAttributeUint32("strlen", orc_Parameter.c_Value.GetArraySize() - 1UL);
         q_ValueHandled = true;
      }
      break;
   default:
      break;
   }
   orc_XmlParser.SetAttributeBool("is-array", orc_Parameter.c_Value.GetArray());
   orc_XmlParser.SetAttributeUint32("array-size", orc_Parameter.c_Value.GetArraySize());
   if (q_ValueHandled == false)
   {
      c_Retval = C_OscHalcDefStructFiler::h_SaveSimpleValueAsAttribute("value", orc_XmlParser, orc_Parameter.c_Value);
   }
   if (!c_Retval)
   {
      const std::vector<std::pair<std::string,
                                  C_OscNodeDataPoolContent> > & rc_EnumItems = orc_Parameter.c_Value.GetEnumItems();
      for (std::vector<std::pair<std::string, C_OscNodeDataPoolContent> >::const_iterator c_It =
              rc_EnumItems.begin();
           (c_It != rc_EnumItems.end()) && (!c_Retval); ++c_It)
      {
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("enum-item") == "enum-item");
         orc_XmlParser.SetAttributeString("display", c_It->first);
         c_Retval = C_OscHalcDefStructFiler::h_SaveSimpleValueAsAttribute("value", orc_XmlParser, c_It->second);
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "value");
         }
      }
   }
   if (!c_Retval)
   {
      const std::vector<C_OscHalcDefContentBitmaskItem> & rc_BitmaskItems = orc_Parameter.c_Value.GetBitmaskItems();
      for (uint32_t u32_ItBitMask = 0UL; u32_ItBitMask < rc_BitmaskItems.size(); ++u32_ItBitMask)
      {
         std::stringstream c_Mask;
         const C_OscHalcDefContentBitmaskItem & rc_BitmaskItem = rc_BitmaskItems[u32_ItBitMask];
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("bitmask-selection") == "bitmask-selection");
         orc_XmlParser.SetAttributeString("display", rc_BitmaskItem.c_Display);
         orc_XmlParser.SetAttributeBool("initial-apply-value-setting", rc_BitmaskItem.q_ApplyValueSetting);
         c_Mask << "0x" << std::hex << rc_BitmaskItem.u64_Value;
         orc_XmlParser.SetAttributeString("value", c_Mask.str().c_str());
         orc_XmlParser.CreateNodeChild("comment", rc_BitmaskItem.c_Comment);
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "value");
      }
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_BaseNode);
      orc_XmlParser.CreateNodeChild("comment", orc_Parameter.c_Comment);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO data base

   \param[in,out]  orc_IoData       IO data
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoDataBase(C_OscHalcDefBase & orc_IoData,
                                                        C_OscXmlParserBase & orc_XmlParser,
                                                        const std::string & orc_BasePath)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("io-base-file");

   if (!c_Retval)
   {
      if (orc_BasePath.empty())
      {
         //From string
         c_Retval = C_OscHalcDefFiler::h_LoadData(orc_IoData, orc_XmlParser);
         //Retrieve file name
         tgl_assert(orc_XmlParser.GetAttributeStringError("original_file_name",
                                                          orc_IoData.c_OriginalFileName) == Errc::success);
         //Remember file content
         tgl_assert(orc_XmlParser.GetAttributeStringError("original_file_content",
                                                          orc_IoData.c_FileString) == Errc::success);
      }
      else
      {
         c_Retval =
         C_OscHalcDefFiler::h_LoadFile(orc_IoData,
                                           C_OscSystemFilerUtil::h_CombinePaths(orc_BasePath,
                                                                                orc_XmlParser.GetNodeContent()));
      }
      if (!c_Retval)
      {
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      else
      {
         //More details are in log
         c_Retval = Errc::config;
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO domains

   \param[in,out]  orc_IoData       IO data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoDomains(C_OscHalcConfig & orc_IoData, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("domains");

   if (!c_Retval)
   {
      uint32_t u32_ExpectedCount;
      c_Retval = orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedCount);
      if (!c_Retval)
      {
         uint32_t u32_ActualCount = 0UL;
         std::string c_NodeDomain = orc_XmlParser.SelectNodeChild("domain");
         if (c_NodeDomain == "domain")
         {
            do
            {
               const C_OscHalcConfigDomain * const pc_Domain = orc_IoData.GetDomainConfigDataConst(u32_ActualCount);
               if (pc_Domain != nullptr)
               {
                  C_OscHalcConfigDomain c_DomainCopy = *pc_Domain;
                  c_Retval = h_LoadIoDomain(c_DomainCopy, orc_XmlParser);
                  if (!c_Retval)
                  {
                     tgl_assert(!orc_IoData.SetDomainConfig(u32_ActualCount, c_DomainCopy));
                     //Count
                     ++u32_ActualCount;
                     //Iterate
                     c_NodeDomain = orc_XmlParser.SelectNodeNext("domain");
                  }
               }
               else
               {
                  orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Domain count higher than in io spec file");
                  c_Retval = Errc::config;
               }
            }
            while ((c_NodeDomain == "domain") && (!c_Retval));
            if (!c_Retval)
            {
               //Return
               tgl_assert(orc_XmlParser.SelectNodeParent() == "domains");
            }
         }
         if (u32_ExpectedCount != u32_ActualCount)
         {
            std::ostringstream c_TmpStream;
            c_TmpStream << "Unexpected domain count, expected: " << u32_ExpectedCount << ", got " << u32_ActualCount;
            std::string c_Tmp = c_TmpStream.str();
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
         }
      }
      if (!c_Retval)
      {
         //Return
         orc_XmlParser.SelectNodeParent();
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO channel

   \param[in,out]  orc_IoChannel    IO channel
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_NodeName     Node name

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoChannel(C_OscHalcConfigChannel & orc_IoChannel,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::string & orc_NodeName)
{
   std::error_code c_Retval = orc_XmlParser.GetAttributeBoolError("safety-relevant", orc_IoChannel.q_SafetyRelevant);

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.GetAttributeUint32Error("use-case-index", orc_IoChannel.u32_UseCaseIndex);
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("name");
      if (!c_Retval)
      {
         orc_IoChannel.c_Name = orc_XmlParser.GetNodeContent();
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("comment");
      if (!c_Retval)
      {
         orc_IoChannel.c_Comment = orc_XmlParser.GetNodeContent();
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
      }
   }
   if (!c_Retval)
   {
      c_Retval = mh_LoadIoParameterStructs(orc_IoChannel.c_Parameters, orc_XmlParser, orc_NodeName);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameter structs

   \param[in,out]  orc_ParameterStructs   Parameter structs
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      orc_NodeName           Node name

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoParameterStructs(
   std::vector<C_OscHalcConfigParameterStruct> & orc_ParameterStructs, C_OscXmlParserBase & orc_XmlParser,
   const std::string & orc_NodeName)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("parameter-structs");

   if (!c_Retval)
   {
      uint32_t u32_ExpectedLength = 0UL;
      //Clean up existing
      orc_ParameterStructs.clear();
      c_Retval = orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedLength);
      if (!c_Retval)
      {
         std::string c_CurrentParameterNode = orc_XmlParser.SelectNodeChild("parameter-struct");
         //Reserve
         orc_ParameterStructs.reserve(u32_ExpectedLength);
         if (c_CurrentParameterNode == "parameter-struct")
         {
            do
            {
               C_OscHalcConfigParameterStruct c_Parameter;
               c_Retval = mh_LoadIoParameterStruct(c_Parameter, orc_XmlParser);
               if (!c_Retval)
               {
                  orc_ParameterStructs.push_back(c_Parameter);
                  //Iterate
                  c_CurrentParameterNode = orc_XmlParser.SelectNodeNext("parameter-struct");
               }
            }
            while ((c_CurrentParameterNode == "parameter-struct") && (!c_Retval));
            if (!c_Retval)
            {
               //Return
               tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-structs");
            }
         }
         if (!c_Retval)
         {
            //Check length
            if (u32_ExpectedLength != orc_ParameterStructs.size())
            {
               orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext(
                  "length", "Content of attribute (" + std::to_string(u32_ExpectedLength) +
                  ") mismatches with the number of found parameters (" +
                  std::to_string(orc_ParameterStructs.size()) + ").");
               c_Retval = Errc::config;
            }
         }
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_NodeName);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameter struct

   \param[in,out]  orc_ParameterStruct    Parameter struct
   \param[in,out]  orc_XmlParser          XML parser

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoParameterStruct(C_OscHalcConfigParameterStruct & orc_ParameterStruct,
                                                               C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = mh_LoadIoParameters(orc_ParameterStruct.c_ParameterElements, orc_XmlParser);

   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("single-value") == "single-value")
      {
         c_Retval = mh_LoadIoParameter(orc_ParameterStruct, orc_XmlParser, "single-value");
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-struct");
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameters

   \param[in,out]  orc_Parameters   Parameters
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoParameters(std::vector<C_OscHalcConfigParameter> & orc_Parameters,
                                                          C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild("parameters") == "parameters")
   {
      uint32_t u32_ExpectedLength = 0UL;
      c_Retval = orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedLength);
      if (!c_Retval)
      {
         std::string c_CurrentParameterNode = orc_XmlParser.SelectNodeChild("parameter");
         //Reserve
         orc_Parameters.reserve(u32_ExpectedLength);
         if (c_CurrentParameterNode == "parameter")
         {
            do
            {
               C_OscHalcConfigParameter c_Parameter;
               c_Retval = mh_LoadIoParameter(c_Parameter, orc_XmlParser, "parameter");
               if (!c_Retval)
               {
                  orc_Parameters.push_back(c_Parameter);
                  //Iterate
                  c_CurrentParameterNode = orc_XmlParser.SelectNodeNext("parameter");
               }
            }
            while ((c_CurrentParameterNode == "parameter") && (!c_Retval));
            if (!c_Retval)
            {
               //Return
               tgl_assert(orc_XmlParser.SelectNodeParent() == "parameters");
            }
         }
         if (!c_Retval)
         {
            //Check length
            if (u32_ExpectedLength != orc_Parameters.size())
            {
               orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext(
                  "length", "Content of attribute (" + std::to_string(u32_ExpectedLength) +
                  ") mismatches with the number of found parameters (" +
                  std::to_string(orc_Parameters.size()) + ").");
               c_Retval = Errc::config;
            }
         }
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "parameter-struct");
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameter

   \param[in,out]  orc_Parameter    Parameter
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BaseName     Base name

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigFiler::mh_LoadIoParameter(C_OscHalcConfigParameter & orc_Parameter,
                                                         C_OscXmlParserBase & orc_XmlParser,
                                                         const std::string & orc_BaseName)
{
   std::string c_TypeStr;
   std::string c_BaseTypeStr;
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("value");

   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_SetType(orc_XmlParser, orc_Parameter.c_Value, c_TypeStr,
                                                      c_BaseTypeStr, "value");
      if (!c_Retval)
      {
         c_Retval = C_OscHalcDefStructFiler::h_ParseAttributeIntoContent(orc_Parameter.c_Value, orc_XmlParser,
                                                                           "value",
                                                                           c_TypeStr,
                                                                           c_BaseTypeStr, true);
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_BaseName);
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("comment");
      if (!c_Retval)
      {
         orc_Parameter.c_Comment = orc_XmlParser.GetNodeContent();
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_BaseName);
      }
   }
   return c_Retval;
}
