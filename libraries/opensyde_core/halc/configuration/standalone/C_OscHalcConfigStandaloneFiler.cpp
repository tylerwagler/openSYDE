//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for standalone HALC config

   Filer for standalone HALC config

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <system_error>
#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscHalcConfigFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscHalcConfigStandaloneFiler.hpp"
#include "C_SclStringUtil.hpp"

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
/*! \brief  Default constructor/destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcConfigStandaloneFiler::C_OscHalcConfigStandaloneFiler(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO standalone description

   \param[out]  orc_IoData    Storage
   \param[in]   orc_Path      Path to IO description

   \return
   Errc::success    data read
   Errc::range      specified file does not exist
   Errc::noact      specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigStandaloneFiler::h_LoadFileStandalone(C_OscHalcConfigStandalone & orc_IoData,
                                                                     const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_XmlParser.SetLogHeading("Loading IO standalone data");
      c_Retval = c_XmlParser.LoadFromFile(orc_Path);
      if (!c_Retval)
      {
         if (c_XmlParser.SelectRoot() == "opensyde-node-io-config-standalone")
         {
            c_Retval = h_LoadDataStandalone(orc_IoData, c_XmlParser);
         }
         else
         {
            osc_write_log_error("Loading IO standalone data",
                                "Could not find \"opensyde-node-io-config-standalone\" node.");
            c_Retval = Errc::config;
         }
      }
      else
      {
         osc_write_log_error("Loading IO standalone data", "File \"" + orc_Path + "\" could not be opened.");
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading IO standalone data", "File \"" + orc_Path + "\" does not exist.");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO standalone description

   Save IO standalone description
   Will overwrite the file if it already exists.

   \param[in]  orc_IoData  Pointer to storage
   \param[in]  orc_Path    Path of file

   \return
   Errc::success    data saved
   Errc::config     data invalid
   Errc::rd_wr      could not erase pre-existing file before saving
   Errc::rd_wr      could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigStandaloneFiler::h_SaveFileStandalone(const C_OscHalcConfigStandalone & orc_IoData,
                                                                     const std::string & orc_Path)
{
   std::error_code c_Retval = C_OscHalcConfigFiler::h_PrepareForFile(orc_Path);

   if (!c_Retval)
   {
      C_OscXmlParser c_XmlParser;
      c_XmlParser.CreateNodeChild("opensyde-node-io-config-standalone");
      tgl_assert(c_XmlParser.SelectRoot() == "opensyde-node-io-config-standalone");
      c_Retval = h_SaveDataStandalone(orc_IoData, c_XmlParser);
      if (!c_Retval)
      {
         c_Retval = c_XmlParser.SaveToFile(orc_Path);
         if (c_Retval)
         {
            osc_write_log_error("Saving IO standalone data", "Could not write to file \"" + orc_Path + "\".");
            c_Retval = Errc::rd_wr;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO standalone data

   \param[out]     orc_IoData       Storage
   \param[in,out]  orc_XmlParser    XML with default state

   \return
   Errc::success    data read
   Errc::config     HALC configuration content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigStandaloneFiler::h_LoadDataStandalone(C_OscHalcConfigStandalone & orc_IoData,
                                                                     C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   // Device Type
   if (orc_XmlParser.SelectNodeChild("definition-content-version") == "definition-content-version")
   {
      orc_IoData.u32_DefinitionContentVersion = 0UL;
      try
      {
         orc_IoData.u32_DefinitionContentVersion = static_cast<uint16_t>(std::stoi(orc_XmlParser.GetNodeContent()));
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      catch (...)
      {
         c_Retval = Errc::config;
      }
   }
   else
   {
      c_Retval = Errc::config;
   }

   // Definition content version
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("device-type") == "device-type")
      {
         orc_IoData.c_DeviceType = orc_XmlParser.GetNodeContent();
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      else
      {
         c_Retval = Errc::config;
      }
   }

   if (!c_Retval)
   {
      // Domains
      if (orc_XmlParser.SelectNodeChild("domains") == "domains")
      {
         std::string c_NodeDomain = orc_XmlParser.SelectNodeChild("domain");

         while ((c_NodeDomain == "domain") && (!c_Retval))
         {
            C_OscHalcConfigStandaloneDomain c_Domain;

            // The not stand alone part
            c_Retval = C_OscHalcConfigFiler::h_LoadIoDomain(c_Domain, orc_XmlParser);

            // Domain Id
            if (orc_XmlParser.SelectNodeChild("domain-id") == "domain-id")
            {
               c_Domain.c_Id = orc_XmlParser.GetNodeContent();
               //Return
               orc_XmlParser.SelectNodeParent();
            }
            else
            {
               c_Retval = Errc::config;
            }

            if (!c_Retval)
            {
               c_Domain.c_Channels.reserve(c_Domain.c_ChannelConfigs.size());

               // Channel Names
               if (orc_XmlParser.SelectNodeChild("channel-names") == "channel-names")
               {
                  std::string c_NodeChannelId = orc_XmlParser.SelectNodeChild("channel-name");
                  bool q_AtLeastOneChannelId = false;

                  while ((c_NodeChannelId == "channel-name") && (!c_Retval))
                  {
                     C_OscHalcConfigStandaloneChannel c_ChannelId;

                     q_AtLeastOneChannelId = true;

                     if (orc_XmlParser.SelectNodeChild("name") == "name")
                     {
                        C_OscHalcDefChannelDef c_ChannelDef;

                        // Channel Name
                        c_ChannelDef.c_Name = orc_XmlParser.GetNodeContent();
                        c_Domain.c_Channels.push_back(c_ChannelDef);

                        //Return
                        orc_XmlParser.SelectNodeParent();
                     }
                     else
                     {
                        c_Retval = Errc::config;
                     }

                     // Parameter Ids
                     if (!c_Retval)
                     {
                        if (orc_XmlParser.SelectNodeChild("parameter-ids") == "parameter-ids")
                        {
                           std::string c_NodeParameterId = orc_XmlParser.SelectNodeChild("parameter-id");
                           bool q_AtLeastOneParamterId = false;

                           while (c_NodeParameterId == "parameter-id")
                           {
                              c_ChannelId.c_ParameterIds.push_back(orc_XmlParser.GetNodeContent());

                              q_AtLeastOneParamterId = true;

                              // Iterate over the channel ids
                              c_NodeParameterId = orc_XmlParser.SelectNodeNext("parameter-id");
                           }

                           if (q_AtLeastOneParamterId == true)
                           {
                              //Return to parameter-ids
                              orc_XmlParser.SelectNodeParent();
                           }

                           //Return to channel-name
                           orc_XmlParser.SelectNodeParent();
                        }

                        c_Domain.c_StandaloneChannels.push_back(c_ChannelId);

                        // Iterate over the channel ids
                        c_NodeChannelId = orc_XmlParser.SelectNodeNext("channel-name");
                     }
                     else
                     {
                        break;
                     }
                  }

                  if (q_AtLeastOneChannelId == true)
                  {
                     //Return to channel-names
                     orc_XmlParser.SelectNodeParent();
                  }

                  //Return to domain
                  orc_XmlParser.SelectNodeParent();
               }
            }

            if (!c_Retval)
            {
               orc_IoData.c_Domains.push_back(c_Domain);

               // Iterate over the domains
               c_NodeDomain = orc_XmlParser.SelectNodeNext("domain");
            }
            else
            {
               break;
            }
         }

         if (!c_Retval)
         {
            //Return "domains"
            orc_XmlParser.SelectNodeParent();
         }
      }
      else
      {
         c_Retval = Errc::config;
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO standalone data

   \param[in]      orc_IoData       Pointer to storage
   \param[in,out]  orc_XmlParser    XML with default state

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcConfigStandaloneFiler::h_SaveDataStandalone(const C_OscHalcConfigStandalone & orc_IoData,
                                                                     C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;
   uint32_t u32_DomainCounter;

   // Device Type
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("definition-content-version") == "definition-content-version");
   orc_XmlParser.SetNodeContent(std::to_string(orc_IoData.u32_DefinitionContentVersion));
   //Return
   orc_XmlParser.SelectNodeParent();

   // Definition content version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("device-type") == "device-type");
   orc_XmlParser.SetNodeContent(orc_IoData.c_DeviceType);
   //Return
   orc_XmlParser.SelectNodeParent();

   // Domains
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domains") == "domains");

   for (u32_DomainCounter = 0U; u32_DomainCounter < orc_IoData.c_Domains.size(); ++u32_DomainCounter)
   {
      const C_OscHalcConfigStandaloneDomain & rc_Domain = orc_IoData.c_Domains[u32_DomainCounter];

      // Vector size must be identical
      tgl_assert(rc_Domain.c_ChannelConfigs.size() == rc_Domain.c_StandaloneChannels.size());
      tgl_assert(rc_Domain.c_Channels.size() == rc_Domain.c_StandaloneChannels.size());

      // Save the default not stand alone part
      c_Retval = C_OscHalcConfigFiler::h_SaveIoDomain(rc_Domain, orc_XmlParser);
      // Still in "domain"

      // Save the stand alone part of domain
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domain-id") == "domain-id");
      orc_XmlParser.SetNodeContent(rc_Domain.c_Id);
      //Return
      orc_XmlParser.SelectNodeParent();

      if (!c_Retval)
      {
         // Save the channel ids as stand alone part
         uint32_t u32_ChannelCounter;

         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel-names") == "channel-names");

         for (u32_ChannelCounter = 0U; u32_ChannelCounter < rc_Domain.c_StandaloneChannels.size(); ++u32_ChannelCounter)
         {
            const C_OscHalcConfigStandaloneChannel & rc_ChannelId = rc_Domain.c_StandaloneChannels[u32_ChannelCounter];
            uint32_t u32_ParameterCounter;

            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel-name") == "channel-name");
            orc_XmlParser.CreateNodeChild("name", rc_Domain.c_Channels[u32_ChannelCounter].c_Name);

            // Parameter Ids of channel
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameter-ids") == "parameter-ids");

            for (u32_ParameterCounter = 0U; u32_ParameterCounter < rc_ChannelId.c_ParameterIds.size();
                 ++u32_ParameterCounter)
            {
               tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("parameter-id") == "parameter-id");
               orc_XmlParser.SetNodeContent(rc_ChannelId.c_ParameterIds[u32_ParameterCounter]);
               //Return
               orc_XmlParser.SelectNodeParent();
            }

            //Return "channel-name""
            tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-name");

            //Return "channel-names"
            tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-names");
         }

         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");

         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domains");
      }
   }

   return c_Retval;
}
