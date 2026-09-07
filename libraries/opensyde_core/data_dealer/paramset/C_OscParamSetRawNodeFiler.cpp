//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Raw parameter set file reader/writer (implementation)

   Raw parameter set file reader/writer

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdio>
#include <string>
#include <system_error>

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscChecksummedXml.hpp"
#include "C_OscParamSetRawNodeFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclStringCompat.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
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
/*! \brief   Load parameter set node

   Load parameter set node data from XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[out]    orc_Node                   data storage
   \param[in,out] orc_XmlParser              XML with specified node active
   \param[in,out] orq_MissingOptionalContent Flag for indication of optional content missing
                                             Warning: flag is never set to false if optional content is present

   \return
   Errc::success   data read
   Errc::config    content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscParamSetRawNodeFiler::h_LoadRawNode(C_OscParamSetRawNode & orc_Node,
                                                         C_OscXmlParserBase & orc_XmlParser,
                                                         bool & orq_MissingOptionalContent)
{
   std::error_code c_Retval = C_OscParamSetFilerBase::mh_LoadNodeName(orc_Node.c_Name, orc_XmlParser);

   if (!c_Retval)
   {
      c_Retval = C_OscParamSetFilerBase::mh_LoadDataPoolInfos(orc_Node.c_DataPools, orc_XmlParser,
                                                              orq_MissingOptionalContent);
      if (!c_Retval)
      {
         c_Retval = C_OscParamSetRawNodeFiler::mh_LoadEntries(orc_Node.c_Entries, orc_XmlParser);
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set node

   Save parameter set node to XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[in]     orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::h_SaveRawNode(const C_OscParamSetRawNode & orc_Node, C_OscXmlParserBase & orc_XmlParser)
{
   C_OscParamSetFilerBase::mh_SaveNodeName(orc_Node.c_Name, orc_XmlParser);
   C_OscParamSetFilerBase::mh_SaveDataPoolInfos(orc_Node.c_DataPools, orc_XmlParser);
   C_OscParamSetRawNodeFiler::mh_SaveEntries(orc_Node.c_Entries, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscParamSetRawNodeFiler::C_OscParamSetRawNodeFiler(void) :
   C_OscParamSetFilerBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load parameter set entries

   Load parameter set entries data from XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[out]    orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active

   \return
   Errc::success   data read
   Errc::config    content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscParamSetRawNodeFiler::mh_LoadEntries(std::vector<C_OscParamSetRawEntry> & orc_Entries,
                                                          C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_Entries.clear();
   if (orc_XmlParser.SelectNodeChild("raw") == "raw")
   {
      std::string c_SelectedNode = orc_XmlParser.SelectNodeChild("raw-entry");

      if (c_SelectedNode == "raw-entry")
      {
         do
         {
            C_OscParamSetRawEntry c_Item;
            c_Retval = C_OscParamSetRawNodeFiler::mh_LoadEntry(c_Item, orc_XmlParser);
            if (!c_Retval)
            {
               orc_Entries.push_back(c_Item);
            }

            //Next
            c_SelectedNode = orc_XmlParser.SelectNodeNext("raw-entry");
         }
         while ((c_SelectedNode == "raw-entry") && (!c_Retval));
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "raw");
      }
      else
      {
         osc_write_log_error("Loading Dataset data", "Could not find \"node\".\"raw\".\"raw-entry\" node.");
         c_Retval = Errc::config;
      }
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "node");
   }
   else
   {
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set entries

   Save parameter set entries to XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[in]     orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::mh_SaveEntries(const std::vector<C_OscParamSetRawEntry> & orc_Entries,
                                               C_OscXmlParserBase & orc_XmlParser)
{
   //Clean start
   if (orc_XmlParser.SelectNodeChild("raw") == "raw")
   {
      tgl_assert(orc_XmlParser.DeleteNode() == "raw");
      tgl_assert(orc_XmlParser.SelectRoot() == "opensyde-parameter-sets");
   }
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("raw") == "raw");
   for (uint32_t u32_Index = 0U; u32_Index < orc_Entries.size(); u32_Index++)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("raw-entry") == "raw-entry");
      C_OscParamSetRawNodeFiler::mh_SaveEntry(orc_Entries[u32_Index], orc_XmlParser);
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "raw");
   }
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "node");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load parameter set entry

   Load parameter set entry data from XML file
   pre-condition: the passed XML parser has the active node set to "raw-entry"
   post-condition: the passed XML parser has the active node set to the same "raw-entry"

   \param[out]    orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active

   \return
   Errc::success   data read
   Errc::config    content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscParamSetRawNodeFiler::mh_LoadEntry(C_OscParamSetRawEntry & orc_Entry,
                                                        C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild("address") == "address")
   {
      try
      {
         orc_Entry.u32_StartAddress = static_cast<uint32_t>(std::stoll(orc_XmlParser.GetNodeContent()));
      }
      catch (...)
      {
         osc_write_log_error("Loading Dataset data", "Node \"node\".\"raw\".\"raw-entry\".\"address\" contains non-integer value (" +
                             orc_XmlParser.GetNodeContent() + ").");
         c_Retval = Errc::config;
      }
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "raw-entry");
   }
   else
   {
      osc_write_log_error("Loading Dataset data", "Could not find \"node\".\"raw\".\"raw-entry\".\"address\" node.");
      c_Retval = Errc::config;
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("value") == "value")
      {
          const std::string c_Content = orc_XmlParser.GetNodeContent();
          std::vector<std::string> c_Tokens;
          TokenizeCompat(c_Content, ";", c_Tokens);
          orc_Entry.c_Bytes.reserve(c_Tokens.size());
          for (int32_t s32_It = 0; (s32_It < c_Tokens.size()) && (!c_Retval); ++s32_It)
          {
             const std::string & rc_Token = c_Tokens[s32_It];
            try
            {
               orc_Entry.c_Bytes.push_back(static_cast<uint8_t>(std::stoi(rc_Token)));
            }
            catch (...)
            {
               osc_write_log_error("Loading Dataset data", "Node \"node\".\"raw\".\"raw-entry\".\"value\" contains non-integer value (" +
                                   rc_Token + ").");

               c_Retval = Errc::config;
            }
         }
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "raw-entry");
      }
      else
      {
         osc_write_log_error("Loading Dataset data", "Could not find \"node\".\"raw\".\"raw-entry\".\"value\" node.");
         c_Retval = Errc::config;
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("size") == "size")
      {
         uint32_t u32_Size = 0;
         try
         {
            u32_Size = static_cast<uint32_t>(std::stoll(orc_XmlParser.GetNodeContent()));
         }
         catch (...)
         {
            c_Retval = Errc::config;
         }
         if (!c_Retval)
         {
            if (u32_Size != orc_Entry.c_Bytes.size())
            {
               c_Retval = Errc::config;
            }
         }
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "raw-entry");
      }
      else
      {
         osc_write_log_error("Loading Dataset data", "Could not find \"node\".\"raw\".\"raw-entry\".\"size\" node.");
         c_Retval = Errc::config;
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set entry

   Save parameter set entry to XML file
   pre-condition: the passed XML parser has the active node set to "raw-entry"
   post-condition: the passed XML parser has the active node set to the same "raw-entry"

   \param[in]     orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::mh_SaveEntry(const C_OscParamSetRawEntry & orc_Entry,
                                             C_OscXmlParserBase & orc_XmlParser)
{
   std::string c_Bytes;

   orc_XmlParser.CreateNodeChild("address", std::to_string(orc_Entry.u32_StartAddress));
   orc_XmlParser.CreateNodeChild("size", std::to_string(orc_Entry.c_Bytes.size()));
   if (orc_Entry.c_Bytes.size() > 0)
   {
      c_Bytes = std::to_string(orc_Entry.c_Bytes[0]);
      for (uint32_t u32_It = 1; u32_It < orc_Entry.c_Bytes.size(); ++u32_It)
      {
         c_Bytes += ';';
         c_Bytes += std::to_string(orc_Entry.c_Bytes[u32_It]);
      }
   }
   else
   {
      c_Bytes = "";
   }
   orc_XmlParser.CreateNodeChild("value", c_Bytes);
}
