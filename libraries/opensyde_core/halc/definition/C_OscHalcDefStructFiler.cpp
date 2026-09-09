//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC definition struct section

   Load HALC definition struct section

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <limits>
#include <sstream>
#include <system_error>

#include <cstdint>
#include "TglUtils.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscNodeDataPoolContentUtil.hpp"
#include "C_OscHalcDefStructFiler.hpp"
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::scl;

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const uint32_t C_OscHalcDefStructFiler::hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH = 31UL;
const std::string C_OscHalcDefStructFiler::mhc_FALSE = "FALSE";
const std::string C_OscHalcDefStructFiler::mhc_TRUE = "TRUE";

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameters

   \param[out]     orc_Structs            Storage
   \param[in,out]  orc_XmlParser          XML with default state
   \param[in]      orc_UseCases           All available channel use cases for this domain
   \param[in]      orc_CurrentNodeName    Current XML node name
   \param[in]      orc_SectionNodeName    XML node name of section to parse
   \param[in]      orc_GroupNodeName      XML node name of section elements to parse
   \param[in]      orc_SingleNodeName     XML node name of group elements to parse
   \param[in]      oq_RequireId           Flag if ID is required in this section
   \param[in]      oq_RequireSection      Flag if this entire section is required
   \param[in]      ou32_DomainNameLength  Domain name length

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_LoadStructs(std::vector<C_OscHalcDefStruct> & orc_Structs,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                       const std::string & orc_CurrentNodeName,
                                                       const std::string & orc_SectionNodeName,
                                                       const std::string & orc_GroupNodeName,
                                                       const std::string & orc_SingleNodeName, const bool oq_RequireId,
                                                       const bool oq_RequireSection,
                                                       const uint32_t ou32_DomainNameLength)
{
   std::error_code c_Retval = Errc::success;

   orc_Structs.clear();
   if (orc_XmlParser.SelectNodeChild(orc_SectionNodeName) == orc_SectionNodeName)
   {
      std::string c_NodeChannel = orc_XmlParser.SelectNodeChild(orc_GroupNodeName);
      if (c_NodeChannel == orc_GroupNodeName)
      {
         do
         {
            C_OscHalcDefStruct c_Struct;
            c_Retval = mh_LoadStruct(c_Struct, orc_XmlParser, orc_UseCases, orc_GroupNodeName, orc_SingleNodeName,
                                       oq_RequireId, ou32_DomainNameLength);
            if (!c_Retval)
            {
               orc_Structs.push_back(c_Struct);
               c_NodeChannel = orc_XmlParser.SelectNodeNext(orc_GroupNodeName);
            }
         }
         while ((c_NodeChannel == orc_GroupNodeName) && (!c_Retval));
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SectionNodeName);
         }
      }

      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_CurrentNodeName);
      }
   }
   else
   {
      if (oq_RequireSection)
      {
         orc_XmlParser.ReportErrorForNodeMissing(orc_SectionNodeName);
         c_Retval = Errc::config;
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save generic struct data

   \param[in]      orc_Structs            Generic struct data
   \param[in]      orc_UseCases           All available channel use cases for this domain
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      orc_CurrentNodeName    Current node name
   \param[in]      orc_SectionNodeName    Section node name
   \param[in]      orc_GroupNodeName      Group node name
   \param[in]      orc_SingleNodeName     Single node name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_SaveStructs(const std::vector<C_OscHalcDefStruct> & orc_Structs,
                                                       const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::string & orc_CurrentNodeName,
                                                       const std::string & orc_SectionNodeName,
                                                       const std::string & orc_GroupNodeName,
                                                       const std::string & orc_SingleNodeName)
{
   std::error_code c_Retval = Errc::success;

   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_SectionNodeName) == orc_SectionNodeName);
   for (uint32_t u32_ItStruct = 0UL; (u32_ItStruct < orc_Structs.size()) && (!c_Retval); ++u32_ItStruct)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_GroupNodeName) == orc_GroupNodeName);
      c_Retval = C_OscHalcDefStructFiler::mh_SaveStruct(orc_Structs[u32_ItStruct], orc_UseCases, orc_XmlParser,
                                                          orc_GroupNodeName,
                                                          orc_SingleNodeName);
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SectionNodeName);
      }
   }
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_CurrentNodeName);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse simplest data type values

   \param[in]      orc_TypeStr         Original type string
   \param[in,out]  orc_Content         Content to set
   \param[in,out]  orc_XmlParser       XML with default state
   \param[in]      orc_AttributeName   Attribute to look for value

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_ParseSimplestTypeValue(const std::string & orc_TypeStr,
                                                                  C_OscNodeDataPoolContent & orc_Content,
                                                                  const C_OscXmlParserBase & orc_XmlParser,
                                                                  const std::string & orc_AttributeName)
{
   std::string c_ItemStr;
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError(orc_AttributeName, c_ItemStr);

   if (!c_Retval)
   {
      if (c_ItemStr == "")
      {
         orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "is empty");
         c_Retval = Errc::config;
      }
      else
      {
         switch (orc_Content.GetType())
         {
         case C_OscNodeDataPoolContent::eUINT8:
            if (orc_TypeStr == "bool")
            {
                if (LowerCaseCompat(c_ItemStr) == LowerCaseCompat(mhc_FALSE))
                {
                   orc_Content.SetValueU8(static_cast<uint8_t>(false));
                }
                else if (LowerCaseCompat(c_ItemStr) == LowerCaseCompat(mhc_TRUE))
               {
                  orc_Content.SetValueU8(static_cast<uint8_t>(true));
               }
               else
               {
                  if (!mh_CheckValidUint(c_ItemStr))
                  {
                     orc_Content.SetValueU8(static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
                  }
                  else
                  {
                     c_Retval = Errc::config;
                  }
               }
            }
            else
            {
               if (!mh_CheckValidUint(c_ItemStr))
               {
                  orc_Content.SetValueU8(static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
               }
               else
               {
                  c_Retval = Errc::config;
               }
            }
            break;
         case C_OscNodeDataPoolContent::eUINT16:
            if (!mh_CheckValidUint(c_ItemStr))
            {
               orc_Content.SetValueU16(static_cast<uint16_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eUINT32:
            if (!mh_CheckValidUint(c_ItemStr))
            {
               orc_Content.SetValueU32(orc_XmlParser.GetAttributeUint32(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eUINT64:
            if (!mh_CheckValidUint(c_ItemStr))
            {
               orc_Content.SetValueU64(orc_XmlParser.GetAttributeSint64(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eSINT8:
            if (!mh_CheckValidSint(c_ItemStr))
            {
               orc_Content.SetValueS8(static_cast<int8_t>(orc_XmlParser.GetAttributeSint32(orc_AttributeName)));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eSINT16:
            if (!mh_CheckValidSint(c_ItemStr))
            {
               orc_Content.SetValueS16(static_cast<int16_t>(orc_XmlParser.GetAttributeSint32(orc_AttributeName)));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eSINT32:
            if (!mh_CheckValidSint(c_ItemStr))
            {
               orc_Content.SetValueS32(orc_XmlParser.GetAttributeSint32(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eSINT64:
            if (!mh_CheckValidSint(c_ItemStr))
            {
               orc_Content.SetValueS64(orc_XmlParser.GetAttributeSint64(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eFLOAT32:
            if (!mh_CheckValidDouble(c_ItemStr))
            {
               orc_Content.SetValueF32(orc_XmlParser.GetAttributeFloat32(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         case C_OscNodeDataPoolContent::eFLOAT64:
            if (!mh_CheckValidDouble(c_ItemStr))
            {
               orc_Content.SetValueF64(orc_XmlParser.GetAttributeFloat64(orc_AttributeName));
            }
            else
            {
               c_Retval = Errc::config;
            }
            break;
         default:
            break;
         }
         if (c_Retval == Errc::config)
         {
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext(orc_AttributeName, "Could not parse attribute content into type \"" +
                                                                         orc_TypeStr + "\"");
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse simplest data type values

   \param[in]   orc_TypeStr      Original type string
   \param[out]  ore_Type         Type value
   \param[in]   orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     String is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_GetTypeForSimplestTypeString(const std::string & orc_TypeStr,
                                                                        C_OscNodeDataPoolContent::E_Type & ore_Type,
                                                                        const C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_TypeStr == "float64")
   {
      ore_Type = C_OscNodeDataPoolContent::eFLOAT64;
   }
   else if (orc_TypeStr == "float32")
   {
      ore_Type = C_OscNodeDataPoolContent::eFLOAT32;
   }
   else if (orc_TypeStr == "sint64")
   {
      ore_Type = C_OscNodeDataPoolContent::eSINT64;
   }
   else if (orc_TypeStr == "sint32")
   {
      ore_Type = C_OscNodeDataPoolContent::eSINT32;
   }
   else if (orc_TypeStr == "sint16")
   {
      ore_Type = C_OscNodeDataPoolContent::eSINT16;
   }
   else if (orc_TypeStr == "sint8")
   {
      ore_Type = C_OscNodeDataPoolContent::eSINT8;
   }
   else if (orc_TypeStr == "uint64")
   {
      ore_Type = C_OscNodeDataPoolContent::eUINT64;
   }
   else if (orc_TypeStr == "uint32")
   {
      ore_Type = C_OscNodeDataPoolContent::eUINT32;
   }
   else if (orc_TypeStr == "uint16")
   {
      ore_Type = C_OscNodeDataPoolContent::eUINT16;
   }
   else if ((orc_TypeStr == "uint8") || (orc_TypeStr == "bool"))
   {
      ore_Type = C_OscNodeDataPoolContent::eUINT8;
   }
   else
   {
      orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("type", "Unexpected value");
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get type string

   \param[in]  oe_Type  Type

   \return
   Type string
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHalcDefStructFiler::h_GetTypeString(const C_OscNodeDataPoolContent::E_Type oe_Type)
{
   std::string c_Retval;

   switch (oe_Type)
   {
   case C_OscNodeDataPoolContent::eUINT8:
      c_Retval = "uint8";
      break;
   case C_OscNodeDataPoolContent::eUINT16:
      c_Retval = "uint16";
      break;
   case C_OscNodeDataPoolContent::eUINT32:
      c_Retval = "uint32";
      break;
   case C_OscNodeDataPoolContent::eUINT64:
      c_Retval = "uint64";
      break;
   case C_OscNodeDataPoolContent::eSINT8:
      c_Retval = "sint8";
      break;
   case C_OscNodeDataPoolContent::eSINT16:
      c_Retval = "sint16";
      break;
   case C_OscNodeDataPoolContent::eSINT32:
      c_Retval = "sint32";
      break;
   case C_OscNodeDataPoolContent::eSINT64:
      c_Retval = "sint64";
      break;
   case C_OscNodeDataPoolContent::eFLOAT32:
      c_Retval = "float32";
      break;
   case C_OscNodeDataPoolContent::eFLOAT64:
      c_Retval = "float64";
      break;
   default:
      break;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save simple value type as string

   \param[in]      orc_Node         Attribute name
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_Content      Content

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_SaveSimpleValueAsAttribute(const std::string & orc_Node,
                                                                      C_OscXmlParserBase & orc_XmlParser,
                                                                      const C_OscNodeDataPoolContent & orc_Content)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Content.GetArray() == false)
   {
      uint32_t u32_Tmp;
      uint64_t u64_Tmp;
      int64_t s64_Tmp;
      switch (orc_Content.GetType())
      {
      case C_OscNodeDataPoolContent::eUINT8:
         u32_Tmp = orc_Content.GetValueU8();
         orc_XmlParser.SetAttributeUint32(orc_Node, u32_Tmp);
         break;
      case C_OscNodeDataPoolContent::eUINT16:
         u32_Tmp = orc_Content.GetValueU16();
         orc_XmlParser.SetAttributeUint32(orc_Node, u32_Tmp);
         break;
      case C_OscNodeDataPoolContent::eUINT32:
         u32_Tmp = orc_Content.GetValueU32();
         orc_XmlParser.SetAttributeUint32(orc_Node, u32_Tmp);
         break;
      case C_OscNodeDataPoolContent::eUINT64:
         u64_Tmp = orc_Content.GetValueU64();
         orc_XmlParser.SetAttributeSint64(orc_Node, static_cast<int64_t>(u64_Tmp));
         break;
      case C_OscNodeDataPoolContent::eSINT8:
         s64_Tmp = orc_Content.GetValueS8();
         orc_XmlParser.SetAttributeSint64(orc_Node, s64_Tmp);
         break;
      case C_OscNodeDataPoolContent::eSINT16:
         s64_Tmp = orc_Content.GetValueS16();
         orc_XmlParser.SetAttributeSint64(orc_Node, s64_Tmp);
         break;
      case C_OscNodeDataPoolContent::eSINT32:
         s64_Tmp = orc_Content.GetValueS32();
         orc_XmlParser.SetAttributeSint64(orc_Node, s64_Tmp);
         break;
      case C_OscNodeDataPoolContent::eSINT64:
         s64_Tmp = orc_Content.GetValueS64();
         orc_XmlParser.SetAttributeSint64(orc_Node, s64_Tmp);
         break;
      case C_OscNodeDataPoolContent::eFLOAT32:
         orc_XmlParser.SetAttributeFloat32(orc_Node, orc_Content.GetValueF32());
         break;
      case C_OscNodeDataPoolContent::eFLOAT64:
         orc_XmlParser.SetAttributeFloat64(orc_Node, orc_Content.GetValueF64());
         break;
      default:
         tgl_assert(false);
         break;
      }
   }
   else
   {
      c_Retval = Errc::config;
      osc_write_log_error("Saving HALC definition", "arrays not supported");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set type

   \param[in,out]  orc_XmlParser          XML parser
   \param[in,out]  orc_Content            Content
   \param[in,out]  orc_TypeStr            Type str
   \param[in,out]  orc_BaseTypeStr        Base type str
   \param[in]      orc_CurrentNodeName    Current node name

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_SetType(C_OscXmlParserBase & orc_XmlParser,
                                                   C_OscHalcDefContent & orc_Content, std::string & orc_TypeStr,
                                                   std::string & orc_BaseTypeStr,
                                                   const std::string & orc_CurrentNodeName)
{
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError("type", orc_TypeStr);

   if (!c_Retval)
   {
      orc_Content.SetArray(false);
      if (orc_TypeStr == "float64")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eFLOAT64);
      }
      else if (orc_TypeStr == "float32")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eFLOAT32);
      }
      else if (orc_TypeStr == "sint64")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eSINT64);
      }
      else if (orc_TypeStr == "sint32")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eSINT32);
      }
      else if (orc_TypeStr == "sint16")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eSINT16);
      }
      else if (orc_TypeStr == "sint8")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eSINT8);
      }
      else if (orc_TypeStr == "uint64")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eUINT64);
      }
      else if (orc_TypeStr == "uint32")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eUINT32);
      }
      else if (orc_TypeStr == "uint16")
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eUINT16);
      }
      else if ((orc_TypeStr == "uint8") || (orc_TypeStr == "bool"))
      {
         orc_Content.SetComplexType(C_OscHalcDefContent::eCT_PLAIN);
         orc_Content.SetType(C_OscNodeDataPoolContent::eUINT8);
      }
      else if (orc_TypeStr == "enum")
      {
         c_Retval = orc_XmlParser.GetAttributeStringError("base-type", orc_BaseTypeStr);
         if (!c_Retval)
         {
            C_OscNodeDataPoolContent::E_Type e_Type;
            c_Retval =
               C_OscHalcDefStructFiler::h_GetTypeForSimplestTypeString(orc_BaseTypeStr, e_Type, orc_XmlParser);
            if (!c_Retval)
            {
               orc_Content.SetType(e_Type);
               orc_Content.SetComplexType(C_OscHalcDefContent::eCT_ENUM);
               if ((e_Type == C_OscNodeDataPoolContent::eFLOAT32) || (e_Type == C_OscNodeDataPoolContent::eFLOAT64))
               {
                  orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("base-type", "Float type not supported");
                  c_Retval = Errc::config;
               }
            }
         }
      }
      else if (orc_TypeStr == "bitmask")
      {
         c_Retval = orc_XmlParser.GetAttributeStringError("base-type", orc_BaseTypeStr);
         if (!c_Retval)
         {
            C_OscNodeDataPoolContent::E_Type e_Type;
            c_Retval =
               C_OscHalcDefStructFiler::h_GetTypeForSimplestTypeString(orc_BaseTypeStr, e_Type, orc_XmlParser);
            if (!c_Retval)
            {
               orc_Content.SetType(e_Type);
               orc_Content.SetComplexType(C_OscHalcDefContent::eCT_BIT_MASK);
               if ((e_Type == C_OscNodeDataPoolContent::eFLOAT32) ||
                   (e_Type == C_OscNodeDataPoolContent::eFLOAT64) ||
                   (e_Type == C_OscNodeDataPoolContent::eSINT8)   ||
                   (e_Type == C_OscNodeDataPoolContent::eSINT16)  ||
                   (e_Type == C_OscNodeDataPoolContent::eSINT32)  ||
                   (e_Type == C_OscNodeDataPoolContent::eSINT64))
               {
                  orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("base-type",
                                                                               "Float type and signed type not supported");
                  c_Retval = Errc::config;
               }
            }
         }
      }
      else if (orc_TypeStr == "string")
      {
         uint32_t u32_StringLength;
         c_Retval = orc_XmlParser.GetAttributeUint32Error("strlen", u32_StringLength);
         if (!c_Retval)
         {
            orc_Content.SetType(C_OscNodeDataPoolContent::eSINT8);
            orc_Content.SetArray(true);
            //Inluding \0
            orc_Content.SetArraySize(u32_StringLength + 1UL);
            orc_Content.SetComplexType(C_OscHalcDefContent::eCT_STRING);
         }
      }
      else
      {
         orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("type", "Unexpected value");
         c_Retval = Errc::config;
      }
   }
   //Parse enum
   if (!c_Retval)
   {
      std::string c_EnumItemNode = orc_XmlParser.SelectNodeChild("enum-item");
      if (c_EnumItemNode == "enum-item")
      {
         if (orc_TypeStr != "enum")
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
               "\"enum-item\" node found for type \"" + orc_TypeStr + "\"");
            c_Retval = Errc::config;
         }
         else
         {
            do
            {
               C_OscNodeDataPoolContent c_Content = orc_Content;
               std::string c_Display;
               c_Retval = orc_XmlParser.GetAttributeStringError("display", c_Display);
               if (!c_Retval)
               {
                  c_Retval = h_ParseSimplestTypeValue(orc_BaseTypeStr, c_Content, orc_XmlParser, "value");
               }
               if (!c_Retval)
               {
                  if (orc_Content.AddEnumItem(c_Display, c_Content))
                  {
                     orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
                        "\"enum-item\" display name \"" + c_Display + "\" is duplicate");
                     c_Retval = Errc::config;
                  }
               }
               if (!c_Retval)
               {
                  c_EnumItemNode = orc_XmlParser.SelectNodeNext("enum-item");
               }
            }
            while ((c_EnumItemNode == "enum-item") && (!c_Retval));
         }
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == orc_CurrentNodeName);
         }
      }
      else
      {
         if (orc_TypeStr == "enum")
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
               "No \"enum-item\" node found for type enum");
            c_Retval = Errc::config;
         }
      }
   }
   //Parse bitmask
   if (!c_Retval)
   {
      std::string c_BitmaskItemNode = orc_XmlParser.SelectNodeChild("bitmask-selection");
      if (c_BitmaskItemNode == "bitmask-selection")
      {
         if (orc_TypeStr != "bitmask")
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
               "\"bitmask-selection\" node found for type \"" + orc_TypeStr + "\"");
            c_Retval = Errc::config;
         }
         else
         {
            do
            {
               C_OscHalcDefContentBitmaskItem c_BitmaskItem;
               if (orc_XmlParser.SelectNodeChild("comment") == "comment")
               {
                  c_BitmaskItem.c_Comment = orc_XmlParser.GetNodeContent();
                  //Return
                  tgl_assert(orc_XmlParser.SelectNodeParent() == "bitmask-selection");
               }
               c_Retval = orc_XmlParser.GetAttributeStringError("display", c_BitmaskItem.c_Display);
               if (!c_Retval)
               {
                  std::string c_Content;
                  c_Retval = orc_XmlParser.GetAttributeStringError("initial-apply-value-setting", c_Content);
                  if (!c_Retval)
                  {
                      if (LowerCaseCompat(c_Content) == LowerCaseCompat(mhc_FALSE))
                      {
                         c_BitmaskItem.q_ApplyValueSetting = false;
                      }
                      else if (LowerCaseCompat(c_Content) == LowerCaseCompat(mhc_TRUE))
                     {
                        c_BitmaskItem.q_ApplyValueSetting = true;
                     }
                     else
                     {
                        c_BitmaskItem.q_ApplyValueSetting = orc_XmlParser.GetAttributeBool(
                           "initial-apply-value-setting");
                     }
                  }
               }
               if (!c_Retval)
               {
                  std::string c_Content;
                  c_Retval = orc_XmlParser.GetAttributeStringError("value", c_Content);
                  if (!c_Retval)
                  {
                     if (c_BitmaskItem.SetValueByString(c_Content))
                     {
                        orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("value",
                                                                                           "content \"" + c_Content +
                                                                                           "\" cannot be parsed");
                        c_Retval = Errc::config;
                     }
                  }
               }
               if (!c_Retval)
               {
                  //Add bitmask item
                  orc_Content.AddBitmaskItem(c_BitmaskItem);
                  //Select next one if any
                  c_BitmaskItemNode = orc_XmlParser.SelectNodeNext("bitmask-selection");
               }
            }
            while ((c_BitmaskItemNode == "bitmask-selection") && (!c_Retval));
         }
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == orc_CurrentNodeName);
         }
      }
      else
      {
         if (orc_TypeStr == "bitmask")
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
               "No \"bitmask-selection\" node found for type bitmask");
            c_Retval = Errc::config;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse attribute content

   \param[in,out]  orc_Content            Content to set
   \param[in,out]  orc_XmlParser          XML with default state
   \param[in]      orc_AttributeName      Attribute name to search for
   \param[in]      orc_Type               Type string
   \param[in]      orc_BaseType           Base type string
   \param[in]      oq_RequireAttribute    Flag if attribute is required

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::h_ParseAttributeIntoContent(C_OscHalcDefContent & orc_Content,
                                                                     const C_OscXmlParserBase & orc_XmlParser,
                                                                     const std::string & orc_AttributeName,
                                                                     const std::string & orc_Type,
                                                                     const std::string & orc_BaseType,
                                                                     const bool oq_RequireAttribute)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.AttributeExists(orc_AttributeName))
   {
      if (orc_Type == "float64")
      {
         orc_Content.SetValueF64(orc_XmlParser.GetAttributeFloat64(orc_AttributeName));
      }
      else if (orc_Type == "float32")
      {
         orc_Content.SetValueF32(orc_XmlParser.GetAttributeFloat32(orc_AttributeName));
      }
      else if (orc_Type == "sint64")
      {
         orc_Content.SetValueS64(orc_XmlParser.GetAttributeSint64(orc_AttributeName));
      }
      else if (orc_Type == "sint32")
      {
         orc_Content.SetValueS32(orc_XmlParser.GetAttributeSint32(orc_AttributeName));
      }
      else if (orc_Type == "sint16")
      {
         orc_Content.SetValueS16(static_cast<int16_t>(orc_XmlParser.GetAttributeSint32(orc_AttributeName)));
      }
      else if (orc_Type == "sint8")
      {
         orc_Content.SetValueS8(static_cast<int8_t>(orc_XmlParser.GetAttributeSint32(orc_AttributeName)));
      }
      else if (orc_Type == "uint64")
      {
         orc_Content.SetValueU64(orc_XmlParser.GetAttributeSint64(orc_AttributeName));
      }
      else if (orc_Type == "uint32")
      {
         orc_Content.SetValueU32(orc_XmlParser.GetAttributeUint32(orc_AttributeName));
      }
      else if (orc_Type == "uint16")
      {
         orc_Content.SetValueU16(static_cast<uint16_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
      }
      else if (orc_Type == "uint8")
      {
         orc_Content.SetValueU8(static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
      }
      else if (orc_Type == "bool")
      {
         const std::string c_ItemStr = orc_XmlParser.GetAttributeString(orc_AttributeName);
          if (LowerCaseCompat(c_ItemStr) == LowerCaseCompat(mhc_FALSE))
          {
             orc_Content.SetValueU8(static_cast<uint8_t>(false));
          }
          else if (LowerCaseCompat(c_ItemStr) == LowerCaseCompat(mhc_TRUE))
         {
            orc_Content.SetValueU8(static_cast<uint8_t>(true));
         }
         else
         {
            orc_Content.SetValueU8(static_cast<uint8_t>(orc_XmlParser.GetAttributeUint32(orc_AttributeName)));
         }
      }
      else if (orc_Type == "enum")
      {
         const std::string c_ItemStr = orc_XmlParser.GetAttributeString(orc_AttributeName);
         //Try enum
         if (orc_Content.SetEnumValue(c_ItemStr))
         {
            //Try simple value
            c_Retval = h_ParseSimplestTypeValue(orc_BaseType, orc_Content, orc_XmlParser, orc_AttributeName);
         }
      }
      else if (orc_Type == "bitmask")
      {
         c_Retval = h_ParseSimplestTypeValue(orc_BaseType, orc_Content, orc_XmlParser, orc_AttributeName);
      }
      else if (orc_Type == "string")
      {
         const std::string c_ItemStr = orc_XmlParser.GetAttributeString(orc_AttributeName);
         if (orc_Content.SetStringValue(c_ItemStr.c_str()))
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Unexpected value \"" + orc_Type + "\" for string");
            c_Retval = Errc::config;
         }
      }
      else
      {
         orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Unexpected type \"" + orc_Type + "\"");
         c_Retval = Errc::config;
      }
   }
   else
   {
      if (oq_RequireAttribute)
      {
         orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "is missing");
         c_Retval = Errc::config;
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefStructFiler::C_OscHalcDefStructFiler(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameter struct

   \param[out]     orc_Struct             Storage
   \param[in,out]  orc_XmlParser          XML with default state
   \param[in]      orc_UseCases           All available channel use cases for this domain
   \param[in]      orc_GroupNodeName      XML node name of section elements to parse
   \param[in]      orc_SingleNodeName     XML node name of group elements to parse
   \param[in]      oq_RequireId           Flag if ID is required in this section
   \param[in]      ou32_DomainNameLength  Domain name length

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_LoadStruct(C_OscHalcDefStruct & orc_Struct,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                       const std::string & orc_GroupNodeName,
                                                       const std::string & orc_SingleNodeName, const bool oq_RequireId,
                                                       const uint32_t ou32_DomainNameLength)
{
   std::string c_Type;
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError("type", c_Type);

   if (!c_Retval)
   {
      if (c_Type == "struct")
      {
         if (orc_XmlParser.AttributeExists("id"))
         {
            orc_Struct.c_Id = orc_XmlParser.GetAttributeString("id");
         }
         else
         {
            if (oq_RequireId)
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("id", "is missing");
               c_Retval = Errc::config;
            }
         }
         if (!c_Retval)
         {
            c_Retval = orc_XmlParser.GetAttributeStringError("display", orc_Struct.c_Display);
            if (!c_Retval)
            {
               if ((orc_Struct.c_Display.length() + ou32_DomainNameLength) >
                   C_OscHalcDefStructFiler::hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH)
               {
                  orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(
                     "display",
                     "content \"" + orc_Struct.c_Display +
                     "\" is too long to be combined with domain \"singular-name\" (or \"name\" if not existing) node, maximum allowed characters of combined value: " +
                     std::to_string(
                        C_OscHalcDefStructFiler::
                        hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH) +
                     " (Current: " +
                      std::to_string(orc_Struct.
                                            c_Display.
                                            length() +
                                           ou32_DomainNameLength) +
                     ").");
                  c_Retval = Errc::config;
               }
            }
         }
         if (!c_Retval)
         {
            std::string c_Availability;
            c_Retval = orc_XmlParser.GetAttributeStringError("availability", c_Availability);
            if (!c_Retval)
            {
               c_Retval = C_OscHalcDefStructFiler::mh_ParseAttributeAvailability(
                  orc_Struct.c_UseCaseAvailabilities,
                  c_Availability, orc_UseCases, orc_XmlParser);
            }
         }
         if (!c_Retval)
         {
            std::string c_NodeParameter;
            if (orc_XmlParser.SelectNodeChild("comment") == "comment")
            {
               orc_Struct.c_Comment = orc_XmlParser.GetNodeContent();
               //Return
               tgl_assert(orc_XmlParser.SelectNodeParent() == orc_GroupNodeName);
            }
            c_NodeParameter = orc_XmlParser.SelectNodeChild(orc_SingleNodeName);
            if (c_NodeParameter == orc_SingleNodeName)
            {
               do
               {
                  C_OscHalcDefElement c_Element;
                  c_Retval = mh_LoadDataElement(c_Element, orc_XmlParser, orc_UseCases,
                                                  orc_Struct.c_UseCaseAvailabilities,
                                                  orc_SingleNodeName, oq_RequireId, ou32_DomainNameLength);
                  if (!c_Retval)
                  {
                     orc_Struct.c_StructElements.push_back(c_Element);
                     c_NodeParameter = orc_XmlParser.SelectNodeNext(orc_SingleNodeName);
                  }
               }
               while ((c_NodeParameter == orc_SingleNodeName) && (!c_Retval));
               if (!c_Retval)
               {
                  //Return
                  tgl_assert(orc_XmlParser.SelectNodeParent() == orc_GroupNodeName);
               }
            }
         }
      }
      else
      {
         c_Retval = mh_LoadDataElement(orc_Struct, orc_XmlParser, orc_UseCases, orc_Struct.c_UseCaseAvailabilities,
                                         orc_GroupNodeName, oq_RequireId, ou32_DomainNameLength);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save generic struct data

   \param[in]      orc_Struct          Generic struct data
   \param[in]      orc_UseCases        All available channel use cases for this domain
   \param[in,out]  orc_XmlParser       XML parser
   \param[in]      orc_GroupNodeName   Group node name
   \param[in]      orc_SingleNodeName  Single node name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_SaveStruct(const C_OscHalcDefStruct & orc_Struct,
                                                       const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
                                                       C_OscXmlParserBase & orc_XmlParser,
                                                       const std::string & orc_GroupNodeName,
                                                       const std::string & orc_SingleNodeName)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Struct.c_StructElements.size() > 0UL)
   {
      std::string c_Availability;
      if (orc_Struct.c_Id.empty() == false)
      {
         orc_XmlParser.SetAttributeString("id", orc_Struct.c_Id);
      }
      orc_XmlParser.SetAttributeString("display", orc_Struct.c_Display);
      orc_XmlParser.SetAttributeString("type", "struct");
      c_Retval = mh_GetAvailabilityString(orc_Struct.c_UseCaseAvailabilities, orc_UseCases, c_Availability);
      if (!c_Retval)
      {
         orc_XmlParser.SetAttributeString("availability", c_Availability);
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("comment") == "comment");
         orc_XmlParser.SetNodeContent(orc_Struct.c_Comment);
         tgl_assert(orc_XmlParser.SelectNodeParent() == orc_GroupNodeName);
         for (uint32_t u32_ItElem = 0UL; (u32_ItElem < orc_Struct.c_StructElements.size()) && (!c_Retval);
              ++u32_ItElem)
         {
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(orc_SingleNodeName) == orc_SingleNodeName);
            c_Retval = mh_SaveDataElement(orc_Struct.c_StructElements[u32_ItElem], orc_UseCases, orc_XmlParser,
                                            orc_SingleNodeName);
            tgl_assert(orc_XmlParser.SelectNodeParent() == orc_GroupNodeName);
         }
      }
   }
   else
   {
      c_Retval = mh_SaveDataElement(orc_Struct, orc_UseCases, orc_XmlParser, orc_GroupNodeName);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO parameter

   \param[out]     orc_Element            Storage
   \param[in,out]  orc_XmlParser          XML with default state
   \param[in]      orc_UseCases           All available channel use cases for this domain
   \param[in]      orc_BaseAvailability   Availability of base node
   \param[in]      orc_SingleNodeName     XML node name of group elements to parse
   \param[in]      oq_RequireId           Flag if ID is required in this section
   \param[in]      ou32_DomainNameLength  Domain name length

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_LoadDataElement(
   C_OscHalcDefElement & orc_Element, C_OscXmlParserBase & orc_XmlParser,
   const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases, const std::vector<uint32_t> & orc_BaseAvailability,
   const std::string & orc_SingleNodeName, const bool oq_RequireId, const uint32_t ou32_DomainNameLength)
{
   std::error_code c_Retval = Errc::success;

   std::string c_TypeStr;
   std::string c_BaseTypeStr;

   if (orc_XmlParser.SelectNodeChild("comment") == "comment")
   {
      orc_Element.c_Comment = orc_XmlParser.GetNodeContent();
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SingleNodeName);
   }
   if (orc_XmlParser.AttributeExists("id"))
   {
      orc_Element.c_Id = orc_XmlParser.GetAttributeString("id");
   }
   else
   {
      if (oq_RequireId)
      {
         orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("id", "is missing");
         c_Retval = Errc::config;
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.GetAttributeStringError("display", orc_Element.c_Display);
      if (!c_Retval)
      {
         if ((orc_Element.c_Display.length() + ou32_DomainNameLength) >
             C_OscHalcDefStructFiler::hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH)
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(
               "display",
               "content \"" + orc_Element.c_Display +
               "\" is too long to be combined with domain \"singular-name\" (or \"name\" if not existing) node, maximum allowed characters of combined value: " +
               std::to_string(
                  C_OscHalcDefStructFiler::
                  hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH) +
               " (Current: " +
               std::to_string(
                  orc_Element.c_Display.length() +
                  ou32_DomainNameLength) +
               ").");
            c_Retval = Errc::config;
         }
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.AttributeExists("availability"))
      {
         const std::string c_Availability = orc_XmlParser.GetAttributeString("availability");
         c_Retval = C_OscHalcDefStructFiler::mh_ParseAttributeAvailability(orc_Element.c_UseCaseAvailabilities,
                                                                             c_Availability, orc_UseCases,
                                                                             orc_XmlParser);
      }
      else
      {
         if (orc_BaseAvailability.size() > 0UL)
         {
            orc_Element.c_UseCaseAvailabilities = orc_BaseAvailability;
         }
         else
         {
            //Assuming all
            c_Retval = C_OscHalcDefStructFiler::mh_ParseAttributeAvailability(orc_Element.c_UseCaseAvailabilities,
                                                                                "all", orc_UseCases, orc_XmlParser);
         }
      }
   }
   //Parse content first
   if (!c_Retval)
   {
      c_Retval = h_SetType(orc_XmlParser, orc_Element.c_InitialValue, c_TypeStr, c_BaseTypeStr, orc_SingleNodeName);
      if (!c_Retval)
      {
         //Initialize all the same
         orc_Element.c_MaxValue = orc_Element.c_InitialValue;
         orc_Element.c_MinValue = orc_Element.c_InitialValue;
      }
   }

   //Parse attributes last (might require enum values!)
   if (!c_Retval)
   {
      if (orc_XmlParser.AttributeExists("min-value"))
      {
         c_Retval = h_ParseAttributeIntoContent(orc_Element.c_MinValue, orc_XmlParser, "min-value", c_TypeStr,
                                                  c_BaseTypeStr, true);
      }
      else
      {
         //Default
         if ((c_TypeStr == "enum") || (c_TypeStr == "bitmask"))
         {
            mh_SetMinValForType(c_BaseTypeStr, orc_Element.c_MinValue);
         }
         else
         {
            mh_SetMinValForType(c_TypeStr, orc_Element.c_MinValue);
         }
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.AttributeExists("max-value"))
      {
         c_Retval = h_ParseAttributeIntoContent(orc_Element.c_MaxValue, orc_XmlParser, "max-value", c_TypeStr,
                                                  c_BaseTypeStr, true);
      }
      else
      {
         //Default
         if ((c_TypeStr == "enum") || (c_TypeStr == "bitmask"))
         {
            mh_SetMaxValForType(c_BaseTypeStr, orc_Element.c_MaxValue);
         }
         else
         {
            mh_SetMaxValForType(c_TypeStr, orc_Element.c_MaxValue);
         }
      }
   }
   //Parse init value last as this also requires min and max to already be set
   if (!c_Retval)
   {
      //Default zero
      C_OscNodeDataPoolContentUtil::h_ZeroContent(orc_Element.c_InitialValue);
      //Overwrite if attribute present
      c_Retval = h_ParseAttributeIntoContent(orc_Element.c_InitialValue, orc_XmlParser, "initial-value", c_TypeStr,
                                               c_BaseTypeStr,
                                               orc_Element.GetComplexType() != C_OscHalcDefContent::eCT_BIT_MASK);

      //Check bitmask content
      if (C_OscHalcDefStructFiler::mh_CheckInitialBitmaskContentValid(orc_Element.c_InitialValue,
                                                                      orc_XmlParser) == false)
      {
         c_Retval = Errc::config;
      }

      if (!c_Retval)
      {
         C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Tmp;
         //Check if value in range
         c_Retval =
            C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(orc_Element.c_MinValue, orc_Element.c_MaxValue,
                                                                  orc_Element.c_InitialValue, e_Tmp,
                                                                  C_OscNodeDataPoolContentUtil::eLEAVE_VALUE);
         if (c_Retval)
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("initial-value",
                                                                               "value could not be set in min max range");
            c_Retval = Errc::config;
         }
      }
   }
   //Handle enum min/max
   if (!c_Retval)
   {
      C_OscHalcDefStructFiler::mh_HandleEnumMinMax(orc_Element);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data element

   \param[in]      orc_Element         Element
   \param[in]      orc_UseCases        All available channel use cases for this domain
   \param[in,out]  orc_XmlParser       XML parser
   \param[in]      orc_SingleNodeName  Single node name

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_SaveDataElement(
   const C_OscHalcDefElement & orc_Element, const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
   C_OscXmlParserBase & orc_XmlParser, const std::string & orc_SingleNodeName)
{
   std::error_code c_Retval = Errc::success;
   bool q_SpecialValueHandling = false;
   const std::string c_BaseType = h_GetTypeString(orc_Element.GetType());

   if (orc_Element.c_Id.empty() == false)
   {
      orc_XmlParser.SetAttributeString("id", orc_Element.c_Id);
   }
   orc_XmlParser.SetAttributeString("display", orc_Element.c_Display);
   //Comment before any other child elements
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("comment") == "comment");
   orc_XmlParser.SetNodeContent(orc_Element.c_Comment);
   tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SingleNodeName);
   //Type
   switch (orc_Element.GetComplexType())
   {
   case C_OscHalcDefContent::eCT_BIT_MASK:
      {
         const std::vector<C_OscHalcDefContentBitmaskItem> & rc_BitmaskItems = orc_Element.GetBitmaskItems();
         orc_XmlParser.SetAttributeString("type", "bitmask");
         orc_XmlParser.SetAttributeString("base-type", c_BaseType);
         //Items
         for (uint32_t u32_It = 0UL; (u32_It < rc_BitmaskItems.size()) && (!c_Retval); ++u32_It)
         {
            const C_OscHalcDefContentBitmaskItem & rc_Bitmask = rc_BitmaskItems[u32_It];
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("bitmask-selection") == "bitmask-selection");
            orc_XmlParser.SetAttributeString("display", rc_Bitmask.c_Display);
            orc_XmlParser.SetAttributeBool("initial-apply-value-setting", rc_Bitmask.q_ApplyValueSetting);
            orc_XmlParser.SetAttributeString("value", C_OscHalcDefStructFiler::mh_ConvertToHex(
                                                rc_Bitmask.u64_Value).c_str());
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("comment") == "comment");
            orc_XmlParser.SetNodeContent(rc_Bitmask.c_Comment);
            tgl_assert(orc_XmlParser.SelectNodeParent() == "bitmask-selection");
            tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SingleNodeName);
         }
      }
      break;
   case C_OscHalcDefContent::eCT_ENUM:
      {
         const std::vector<std::pair<std::string,
                                     C_OscNodeDataPoolContent> > & rc_EnumItems = orc_Element.GetEnumItems();
         orc_XmlParser.SetAttributeString("type", "enum");
         orc_XmlParser.SetAttributeString("base-type", c_BaseType);
         //Items
         for (std::vector<std::pair<std::string, C_OscNodeDataPoolContent> >::const_iterator c_It =
                 rc_EnumItems.begin();
              (c_It != rc_EnumItems.end()) && (!c_Retval); ++c_It)
         {
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("enum-item") == "enum-item");
            orc_XmlParser.SetAttributeString("display", c_It->first);
            c_Retval = h_SaveSimpleValueAsAttribute("value", orc_XmlParser, c_It->second);
            if (!c_Retval)
            {
               tgl_assert(orc_XmlParser.SelectNodeParent() == orc_SingleNodeName);
            }
         }
      }
      break;
   case C_OscHalcDefContent::eCT_STRING:
      {
         std::string c_Tmp;
         orc_XmlParser.SetAttributeString("type", "string");
         tgl_assert(!orc_Element.c_InitialValue.GetStringValue(c_Tmp));
         orc_XmlParser.SetAttributeString("initial-value", c_Tmp);
         orc_XmlParser.SetAttributeUint32("strlen", orc_Element.c_InitialValue.GetArraySize() - 1UL);
         q_SpecialValueHandling = true;
      }
      break;
   case C_OscHalcDefContent::eCT_PLAIN:
      orc_XmlParser.SetAttributeString("type", c_BaseType);
      break;
   default:
      tgl_assert(false);
      break;
   }
   if (q_SpecialValueHandling == false)
   {
      //Handle values
      c_Retval = h_SaveSimpleValueAsAttribute("initial-value", orc_XmlParser, orc_Element.c_InitialValue);
      if (!c_Retval)
      {
         c_Retval = h_SaveSimpleValueAsAttribute("max-value", orc_XmlParser, orc_Element.c_MaxValue);
      }
      if (!c_Retval)
      {
         c_Retval = h_SaveSimpleValueAsAttribute("min-value", orc_XmlParser, orc_Element.c_MinValue);
      }
   }
   if (!c_Retval)
   {
      std::string c_Availability;
      c_Retval = mh_GetAvailabilityString(orc_Element.c_UseCaseAvailabilities, orc_UseCases, c_Availability);
      if (!c_Retval)
      {
         orc_XmlParser.SetAttributeString("availability", c_Availability);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse availability string

   \param[out]  orc_Availability       Parsed availability output
   \param[in]   orc_AttributeContent   Availability string to parse
   \param[in]   orc_UseCases           All known use-cases
   \param[in]   orc_XmlParser          XML parser

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_ParseAttributeAvailability(
   std::vector<uint32_t> & orc_Availability, const std::string & orc_AttributeContent,
   const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases, const C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (LowerCaseCompat(orc_AttributeContent) == "all")
   {
      for (uint32_t u32_ItUseCase = 0UL; u32_ItUseCase < orc_UseCases.size(); ++u32_ItUseCase)
      {
         orc_Availability.push_back(u32_ItUseCase);
      }
   }
   else
   {
      std::vector<std::string> c_SplittedString;
      TokenizeCompat(orc_AttributeContent, ",", c_SplittedString);
      for (int32_t s32_ItSplit = 0L; s32_ItSplit < static_cast<int32_t>(c_SplittedString.size()); ++s32_ItSplit)
      {
          const std::string c_CurSplit = TrimCompat(c_SplittedString[s32_ItSplit]);
         bool q_Found = false;
         for (uint32_t u32_ItUseCase = 0UL; u32_ItUseCase < orc_UseCases.size(); ++u32_ItUseCase)
         {
            const C_OscHalcDefChannelUseCase & rc_CurUseCase = orc_UseCases[u32_ItUseCase];
            if (rc_CurUseCase.c_Id == c_CurSplit)
            {
               q_Found = true;
               orc_Availability.push_back(u32_ItUseCase);
               break;
            }
         }
         if (q_Found == false)
         {
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("availability",
                                                                         "Attribute content \"" + c_CurSplit +
                                                                         "\"could not be matched to any use case");
            c_Retval = Errc::config;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get availability string

   \param[in]      orc_Availability    Availability
   \param[in]      orc_UseCases        Use cases
   \param[in,out]  orc_Output          Output

   \return
   Errc::success    availability string assembled
   Errc::config     at least one availability index is out of range of the use-case count
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_GetAvailabilityString(
   const std::vector<uint32_t> & orc_Availability, const std::vector<C_OscHalcDefChannelUseCase> & orc_UseCases,
   std::string & orc_Output)
{
   std::error_code c_Retval = Errc::success;

   orc_Output = "";
   for (uint32_t u32_It = 0UL; u32_It < orc_Availability.size(); ++u32_It)
   {
      if (orc_Availability[u32_It] < orc_UseCases.size())
      {
         const C_OscHalcDefChannelUseCase & rc_Availability = orc_UseCases[orc_Availability[u32_It]];
         if (orc_Output.empty() == false)
         {
            orc_Output += ",";
         }
         orc_Output += rc_Availability.c_Id;
      }
      else
      {
         osc_write_log_error("Saving HALC definition",
                             "Availability index " + std::to_string(orc_Availability[u32_It]) +
                             " no longer in range of use-case count " +
                             std::to_string(orc_UseCases.size()));
         c_Retval = Errc::config;
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set maximum value for type

   \param[in]      orc_TypeStr   Type
   \param[in,out]  orc_Content   Content to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefStructFiler::mh_SetMaxValForType(const std::string & orc_TypeStr,
                                                  C_OscNodeDataPoolContent & orc_Content)
{
   if (orc_TypeStr == "float64")
   {
      orc_Content.SetValueF64(std::numeric_limits<double>::max());
   }
   else if (orc_TypeStr == "float32")
   {
      orc_Content.SetValueF32(std::numeric_limits<float>::max());
   }
   else if (orc_TypeStr == "sint64")
   {
      orc_Content.SetValueS64(std::numeric_limits<int64_t>::max());
   }
   else if (orc_TypeStr == "sint32")
   {
      orc_Content.SetValueS32(std::numeric_limits<int32_t>::max());
   }
   else if (orc_TypeStr == "sint16")
   {
      orc_Content.SetValueS16(std::numeric_limits<int16_t>::max());
   }
   else if (orc_TypeStr == "sint8")
   {
      orc_Content.SetValueS8(std::numeric_limits<int8_t>::max());
   }
   else if (orc_TypeStr == "uint64")
   {
      orc_Content.SetValueU64(std::numeric_limits<uint64_t>::max());
   }
   else if (orc_TypeStr == "uint32")
   {
      orc_Content.SetValueU32(std::numeric_limits<uint32_t>::max());
   }
   else if (orc_TypeStr == "uint16")
   {
      orc_Content.SetValueU16(std::numeric_limits<uint16_t>::max());
   }
   else if (orc_TypeStr == "uint8")
   {
      orc_Content.SetValueU8(std::numeric_limits<uint8_t>::max());
   }
   else if (orc_TypeStr == "bool")
   {
      orc_Content.SetValueU8(1);
   }
   else if (orc_TypeStr == "string")
   {
      for (uint32_t u32_It = 0UL; u32_It < orc_Content.GetArraySize(); ++u32_It)
      {
         orc_Content.SetValueArrS8Element(std::numeric_limits<int8_t>::max(), u32_It);
      }
   }
   else
   {
      //Unexpected
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set minimum value for type

   \param[in]      orc_TypeStr   Type
   \param[in,out]  orc_Content   Content to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefStructFiler::mh_SetMinValForType(const std::string & orc_TypeStr,
                                                  C_OscNodeDataPoolContent & orc_Content)
{
   if (orc_TypeStr == "float64")
   {
      orc_Content.SetValueF64(-std::numeric_limits<double>::max());
   }
   else if (orc_TypeStr == "float32")
   {
      orc_Content.SetValueF32(-std::numeric_limits<float>::max());
   }
   else if (orc_TypeStr == "sint64")
   {
      orc_Content.SetValueS64(std::numeric_limits<int64_t>::min());
   }
   else if (orc_TypeStr == "sint32")
   {
      orc_Content.SetValueS32(std::numeric_limits<int32_t>::min());
   }
   else if (orc_TypeStr == "sint16")
   {
      orc_Content.SetValueS16(std::numeric_limits<int16_t>::min());
   }
   else if (orc_TypeStr == "sint8")
   {
      orc_Content.SetValueS8(std::numeric_limits<int8_t>::min());
   }
   else if (orc_TypeStr == "uint64")
   {
      orc_Content.SetValueU64(std::numeric_limits<uint64_t>::min());
   }
   else if (orc_TypeStr == "uint32")
   {
      orc_Content.SetValueU32(std::numeric_limits<uint32_t>::min());
   }
   else if (orc_TypeStr == "uint16")
   {
      orc_Content.SetValueU16(std::numeric_limits<uint16_t>::min());
   }
   else if (orc_TypeStr == "uint8")
   {
      orc_Content.SetValueU8(std::numeric_limits<uint8_t>::min());
   }
   else if (orc_TypeStr == "bool")
   {
      orc_Content.SetValueU8(0);
   }
   else if (orc_TypeStr == "string")
   {
      for (uint32_t u32_It = 0UL; u32_It < orc_Content.GetArraySize(); ++u32_It)
      {
         orc_Content.SetValueArrS8Element(std::numeric_limits<int8_t>::min(), u32_It);
      }
   }
   else
   {
      //Unexpected
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check initial bitmask content valid

   \param[in]  orc_Content    Content
   \param[in]  orc_XmlParser  XML parser

   \return
   true  valid
   false invalid
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscHalcDefStructFiler::mh_CheckInitialBitmaskContentValid(const C_OscHalcDefContent & orc_Content,
                                                                 const C_OscXmlParserBase & orc_XmlParser)
{
   bool q_Retval = true;

   if (orc_Content.GetComplexType() == C_OscHalcDefContent::eCT_BIT_MASK)
   {
      std::string c_Is = "invalid";
      std::string c_Should = "invalid";
      C_OscHalcDefContent c_Copy = orc_Content;
      const std::vector<C_OscHalcDefContentBitmaskItem> & rc_Bitmasks = c_Copy.GetBitmaskItems();
      for (std::vector<C_OscHalcDefContentBitmaskItem>::const_iterator c_ItBitmask = rc_Bitmasks.begin();
           c_ItBitmask != rc_Bitmasks.end(); ++c_ItBitmask)
      {
         c_Copy.SetBitmask(c_ItBitmask->c_Display, c_ItBitmask->q_ApplyValueSetting);
      }
      switch (orc_Content.GetType()) //lint !e788 not all enum constants used; this is unsigned only
      {
      case C_OscNodeDataPoolContent::eUINT8:
         if (orc_Content.GetValueU8() != c_Copy.GetValueU8())
         {
            q_Retval = false;
            c_Is = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(orc_Content.GetValueU8()));
            c_Should = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(c_Copy.GetValueU8()));
         }
         break;
      case C_OscNodeDataPoolContent::eUINT16:
         if (orc_Content.GetValueU16() != c_Copy.GetValueU16())
         {
            q_Retval = false;
            c_Is = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(orc_Content.GetValueU16()));
            c_Should = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(c_Copy.GetValueU16()));
         }
         break;
      case C_OscNodeDataPoolContent::eUINT32:
         if (orc_Content.GetValueU32() != c_Copy.GetValueU32())
         {
            q_Retval = false;
            c_Is = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(orc_Content.GetValueU32()));
            c_Should = C_OscHalcDefStructFiler::mh_ConvertToHex(static_cast<uint64_t>(c_Copy.GetValueU32()));
         }
         break;
      case C_OscNodeDataPoolContent::eUINT64:
         if (orc_Content.GetValueU64() != c_Copy.GetValueU64())
         {
            q_Retval = false;
            c_Is = C_OscHalcDefStructFiler::mh_ConvertToHex(orc_Content.GetValueU64());
            c_Should = C_OscHalcDefStructFiler::mh_ConvertToHex(c_Copy.GetValueU64());
         }
         break;
      default:
         //invalid
         orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("invalid type for bitmask detected");
         break;
      }
      if (q_Retval == false)
      {
         orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("initial-value",
                                                                      "Bitmask has invalid initial value \"" +
                                                                      c_Is + "\" which should be \"" + c_Should + "\"");
      }
   }

   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert to hex

   \param[in]  ou64_Value  Value

   \return
   Value in hex
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHalcDefStructFiler::mh_ConvertToHex(const uint64_t ou64_Value)
{
   std::string c_Retval;

   std::stringstream c_Mask;
   c_Mask << "0x" << std::hex << ou64_Value;
   c_Retval = c_Mask.str().c_str();
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle enum min max

   \param[in,out]  orc_Element   Element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefStructFiler::mh_HandleEnumMinMax(C_OscHalcDefElement & orc_Element)
{
   if (orc_Element.GetComplexType() == C_OscHalcDefContent::eCT_ENUM)
   {
      const std::vector<std::pair<std::string,
                                  C_OscNodeDataPoolContent> > & rc_Enums = orc_Element.c_InitialValue.GetEnumItems();
      for (std::vector<std::pair<std::string, C_OscNodeDataPoolContent> >::const_iterator c_It =
              rc_Enums.begin();
           c_It != rc_Enums.end(); ++c_It)
      {
         if (c_It == rc_Enums.begin())
         {
            //Init
            tgl_assert(!orc_Element.c_MinValue.SetEnumValue(c_It->first));
            tgl_assert(!orc_Element.c_MaxValue.SetEnumValue(c_It->first));
         }
         else
         {
            C_OscHalcDefContent c_Tmp = orc_Element.c_MinValue;
            tgl_assert(!c_Tmp.SetEnumValue(c_It->first));
            if (c_Tmp < orc_Element.c_MinValue)
            {
               orc_Element.c_MinValue = c_Tmp;
            }
            if (c_Tmp > orc_Element.c_MaxValue)
            {
               orc_Element.c_MaxValue = c_Tmp;
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check valid uint

   \param[in]  orc_Item    Item

   \return
   Errc::success    valid
   Errc::range      invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_CheckValidUint(const std::string & orc_Item)
{
   uint64_t u64_Value;
   std::error_code c_Retval = Errc::success;

   std::stringstream c_Stream(orc_Item.c_str());
   c_Stream >> u64_Value;
   if (c_Stream.fail())
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check valid sint

   \param[in]  orc_Item    Item

   \return
   Errc::success    valid
   Errc::range      invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_CheckValidSint(const std::string & orc_Item)
{
   int64_t s64_Value;
   std::error_code c_Retval = Errc::success;

   std::stringstream c_Stream(orc_Item.c_str());
   c_Stream >> s64_Value;
   if (c_Stream.fail())
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check valid double

   \param[in]  orc_Item    Item

   \return
   Errc::success    valid
   Errc::range      invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefStructFiler::mh_CheckValidDouble(const std::string & orc_Item)
{
   double f64_Value;
   std::error_code c_Retval = Errc::success;

   std::stringstream c_Stream(orc_Item.c_str());
   c_Stream >> f64_Value;
   if (c_Stream.fail())
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}
