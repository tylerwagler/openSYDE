//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       XML persistence of a node's UDS configuration

   Layout, below whichever node the caller has selected:

      <uds>
         <addressing request-id="" response-id="" functional-id="" extended="" pad-frames="" pad-byte=""/>
         <timing p2-ms="" p2-star-ms="" s3-client-ms=""/>
         <sessions><session id=""/></sessions>
         <security-levels><level id=""/></security-levels>
         <seed-key algorithm=""/>
         <routines><routine id="" name="" start="" stop="" request-results=""><comment/></routine></routines>
         <dtcs><dtc code="" name="" severity=""><comment/></dtc></dtcs>
      </uds>

   Every section is optional on load and falls back to the defaults of C_OscNodeUdsConfig::Initialize.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscNodeUdsConfigFiler.hpp"
#include "TglUtils.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save the configuration below the currently selected node

   \param[in]      orc_Config      configuration
   \param[in,out]  orc_XmlParser   parser, positioned on the "uds" node; left there
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeUdsConfigFiler::h_SaveData(const C_OscNodeUdsConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.CreateAndSelectNodeChild("addressing");
   orc_XmlParser.SetAttributeUint32("request-id", orc_Config.u32_RequestId);
   orc_XmlParser.SetAttributeUint32("response-id", orc_Config.u32_ResponseId);
   orc_XmlParser.SetAttributeUint32("functional-id", orc_Config.u32_FunctionalId);
   orc_XmlParser.SetAttributeBool("extended", orc_Config.q_ExtendedId);
   orc_XmlParser.SetAttributeBool("pad-frames", orc_Config.q_PadFrames);
   orc_XmlParser.SetAttributeUint32("pad-byte", orc_Config.u8_PadByte);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");

   orc_XmlParser.CreateAndSelectNodeChild("timing");
   orc_XmlParser.SetAttributeUint32("p2-ms", orc_Config.u32_P2Ms);
   orc_XmlParser.SetAttributeUint32("p2-star-ms", orc_Config.u32_P2StarMs);
   orc_XmlParser.SetAttributeUint32("s3-client-ms", orc_Config.u32_S3ClientMs);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");

   mh_SaveIdList("sessions", "session", orc_Config.c_SupportedSessions, orc_XmlParser);
   mh_SaveIdList("security-levels", "level", orc_Config.c_SecurityLevels, orc_XmlParser);

   orc_XmlParser.CreateAndSelectNodeChild("seed-key");
   orc_XmlParser.SetAttributeString("algorithm", orc_Config.c_SeedKeyAlgorithm);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");

   orc_XmlParser.CreateAndSelectNodeChild("routines");
   for (const C_OscUdsRoutine & rc_Routine : orc_Config.c_Routines)
   {
      orc_XmlParser.CreateAndSelectNodeChild("routine");
      orc_XmlParser.SetAttributeUint32("id", rc_Routine.u16_Identifier);
      orc_XmlParser.SetAttributeString("name", rc_Routine.c_Name);
      orc_XmlParser.SetAttributeBool("start", rc_Routine.q_SupportsStart);
      orc_XmlParser.SetAttributeBool("stop", rc_Routine.q_SupportsStop);
      orc_XmlParser.SetAttributeBool("request-results", rc_Routine.q_SupportsRequestResults);
      orc_XmlParser.CreateNodeChild("comment", rc_Routine.c_Comment);
      tgl_assert(orc_XmlParser.SelectNodeParent() == "routines");
   }
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");

   orc_XmlParser.CreateAndSelectNodeChild("dtcs");
   for (const C_OscUdsDtc & rc_Dtc : orc_Config.c_Dtcs)
   {
      orc_XmlParser.CreateAndSelectNodeChild("dtc");
      orc_XmlParser.SetAttributeUint32("code", rc_Dtc.u32_Code);
      orc_XmlParser.SetAttributeString("name", rc_Dtc.c_Name);
      orc_XmlParser.SetAttributeUint32("severity", rc_Dtc.u8_Severity);
      orc_XmlParser.CreateNodeChild("comment", rc_Dtc.c_Comment);
      tgl_assert(orc_XmlParser.SelectNodeParent() == "dtcs");
   }
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load the configuration from the currently selected node

   \param[out]     orc_Config      configuration; starts from the defaults
   \param[in,out]  orc_XmlParser   parser, positioned on the "uds" node; left there

   \return
   Errc::success   data read
   Errc::config    content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscNodeUdsConfigFiler::h_LoadData(C_OscNodeUdsConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_Config.Initialize();

   if (orc_XmlParser.SelectNodeChild("addressing") == "addressing")
   {
      uint32_t u32_PadByte = orc_Config.u8_PadByte;
      c_Retval = orc_XmlParser.GetAttributeUint32Error("request-id", orc_Config.u32_RequestId);
      if (!c_Retval)
      {
         c_Retval = orc_XmlParser.GetAttributeUint32Error("response-id", orc_Config.u32_ResponseId);
      }
      if (!c_Retval)
      {
         orc_Config.u32_FunctionalId = orc_XmlParser.GetAttributeUint32("functional-id");
         orc_Config.q_ExtendedId = orc_XmlParser.GetAttributeBool("extended");
         orc_Config.q_PadFrames = orc_XmlParser.GetAttributeBool("pad-frames");
         if (orc_XmlParser.AttributeExists("pad-byte") == true)
         {
            u32_PadByte = orc_XmlParser.GetAttributeUint32("pad-byte");
         }
         orc_Config.u8_PadByte = static_cast<uint8_t>(u32_PadByte);
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }

   if ((!c_Retval) && (orc_XmlParser.SelectNodeChild("timing") == "timing"))
   {
      orc_Config.u32_P2Ms = orc_XmlParser.GetAttributeUint32("p2-ms");
      orc_Config.u32_P2StarMs = orc_XmlParser.GetAttributeUint32("p2-star-ms");
      orc_Config.u32_S3ClientMs = orc_XmlParser.GetAttributeUint32("s3-client-ms");
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }

   if (!c_Retval)
   {
      c_Retval = mh_LoadIdList("sessions", "session", orc_Config.c_SupportedSessions, orc_XmlParser);
   }
   if (!c_Retval)
   {
      c_Retval = mh_LoadIdList("security-levels", "level", orc_Config.c_SecurityLevels, orc_XmlParser);
   }

   if ((!c_Retval) && (orc_XmlParser.SelectNodeChild("seed-key") == "seed-key"))
   {
      orc_Config.c_SeedKeyAlgorithm = orc_XmlParser.GetAttributeString("algorithm");
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }

   if ((!c_Retval) && (orc_XmlParser.SelectNodeChild("routines") == "routines"))
   {
      std::string c_Current = orc_XmlParser.SelectNodeChild("routine");
      while ((!c_Retval) && (c_Current == "routine"))
      {
         C_OscUdsRoutine c_Routine;
         uint32_t u32_Id = 0U;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("id", u32_Id);
         if (!c_Retval)
         {
            c_Routine.u16_Identifier = static_cast<uint16_t>(u32_Id);
            c_Routine.c_Name = orc_XmlParser.GetAttributeString("name");
            c_Routine.q_SupportsStart = orc_XmlParser.GetAttributeBool("start");
            c_Routine.q_SupportsStop = orc_XmlParser.GetAttributeBool("stop");
            c_Routine.q_SupportsRequestResults = orc_XmlParser.GetAttributeBool("request-results");
            if (orc_XmlParser.SelectNodeChild("comment") == "comment")
            {
               c_Routine.c_Comment = orc_XmlParser.GetNodeContent();
               tgl_assert(orc_XmlParser.SelectNodeParent() == "routine");
            }
            orc_Config.c_Routines.push_back(c_Routine);
         }
         c_Current = orc_XmlParser.SelectNodeNext("routine");
      }
      if (orc_Config.c_Routines.empty() == false)
      {
         tgl_assert(orc_XmlParser.SelectNodeParent() == "routines");
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }

   if ((!c_Retval) && (orc_XmlParser.SelectNodeChild("dtcs") == "dtcs"))
   {
      std::string c_Current = orc_XmlParser.SelectNodeChild("dtc");
      while ((!c_Retval) && (c_Current == "dtc"))
      {
         C_OscUdsDtc c_Dtc;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("code", c_Dtc.u32_Code);
         if (!c_Retval)
         {
            c_Dtc.c_Name = orc_XmlParser.GetAttributeString("name");
            c_Dtc.u8_Severity = static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32("severity"));
            if (orc_XmlParser.SelectNodeChild("comment") == "comment")
            {
               c_Dtc.c_Comment = orc_XmlParser.GetNodeContent();
               tgl_assert(orc_XmlParser.SelectNodeParent() == "dtc");
            }
            orc_Config.c_Dtcs.push_back(c_Dtc);
         }
         c_Current = orc_XmlParser.SelectNodeNext("dtc");
      }
      if (orc_Config.c_Dtcs.empty() == false)
      {
         tgl_assert(orc_XmlParser.SelectNodeParent() == "dtcs");
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }

   if (c_Retval)
   {
      osc_write_log_error("Loading node definition", "Invalid content in \"uds\" section.");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeUdsConfigFiler::mh_SaveIdList(const char * const opcn_ListName, const char * const opcn_ItemName,
                                            const std::vector<uint8_t> & orc_Ids, C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.CreateAndSelectNodeChild(opcn_ListName);
   for (const uint8_t u8_Id : orc_Ids)
   {
      orc_XmlParser.CreateAndSelectNodeChild(opcn_ItemName);
      orc_XmlParser.SetAttributeUint32("id", u8_Id);
      tgl_assert(orc_XmlParser.SelectNodeParent() == opcn_ListName);
   }
   tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
}

//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscNodeUdsConfigFiler::mh_LoadIdList(const char * const opcn_ListName,
                                                       const char * const opcn_ItemName, std::vector<uint8_t> & orc_Ids,
                                                       C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild(opcn_ListName) == opcn_ListName)
   {
      std::string c_Current = orc_XmlParser.SelectNodeChild(opcn_ItemName);
      orc_Ids.clear();
      while ((!c_Retval) && (c_Current == opcn_ItemName))
      {
         uint32_t u32_Id = 0U;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("id", u32_Id);
         if (!c_Retval)
         {
            orc_Ids.push_back(static_cast<uint8_t>(u32_Id));
         }
         c_Current = orc_XmlParser.SelectNodeNext(opcn_ItemName);
      }
      if (orc_Ids.empty() == false)
      {
         tgl_assert(orc_XmlParser.SelectNodeParent() == opcn_ListName);
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "uds");
   }
   return c_Retval;
}
