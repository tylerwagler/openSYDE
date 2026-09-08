//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC definition

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cctype>
#include <fstream>
#include <string>
#include <system_error>

#include "TglFile.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscUtils.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscHalcDefFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscHalcDefStructFiler.hpp"
#include "C_SclStringCompat.hpp"

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
/*! \brief  Load HALC definition

   \param[out]  orc_IoData    Storage
   \param[in]   orc_Path      Path to HALC definition

   \return
   Errc::success    data read
   Errc::range      specified system definition file does not exist
   Errc::noact      specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config     IO file content is invalid or incomplete
                    IO file could not be loaded
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_LoadFile(C_OscHalcDefBase & orc_IoData, const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_Retval = c_XmlParser.LoadFromFile(orc_Path);
      if (!c_Retval)
      {
         c_XmlParser.SetLogHeading("Loading HALC definition");
         c_Retval = c_XmlParser.SelectRootError("opensyde-HALC-description");
         if (!c_Retval)
         {
            c_Retval = h_LoadData(orc_IoData, c_XmlParser);
         }
         if (!c_Retval)
         {
            //Save file to string
            C_OscUtils::h_FileToString(orc_Path, orc_IoData.c_FileString);
            //Remember name
            orc_IoData.c_OriginalFileName = TglExtractFileName(orc_Path);
            //Trigger postprocessing before finish
            orc_IoData.HandleFileLoadPostProcessing();
         }
      }
      else
      {
         osc_write_log_error("Loading HALC definition",
                             "File \"" + orc_Path + "\" could not be opened.");
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading HALC definition", "File \"" + orc_Path + "\" does not exist.");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save HALC definition

   Save HALC definition
   Will overwrite the file if it already exists.

   \param[in]  orc_IoData  Pointer to storage
   \param[in]  orc_Path    Path of system definition

   \return
   Errc::success    data saved
   Errc::rd_wr      could not erase pre-existing file before saving
   Errc::rd_wr      could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_SaveFile(const C_OscHalcDefBase & orc_IoData, const std::string & orc_Path)
{
   const std::error_code c_Retval = C_OscSystemFilerUtil::h_SaveStringToFile(orc_IoData.c_FileString, orc_Path,
                                                                             "Saving HALC definition");

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load HALC definition

   \param[out]     orc_IoData       Storage
   \param[in,out]  orc_XmlParser    XML with default state

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_LoadData(C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("file-version");

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
         osc_write_log_info("Loading HALC definition", "Value of \"file-version\": " +
                            std::to_string(u16_FileVersion));
         //Check file version
         if (u16_FileVersion != mhu16_FILE_VERSION_1)
         {
            osc_write_log_error("Loading HALC definition",
                                "Version defined by \"file-version\" is not supported.");
            c_Retval = Errc::config;
         }
      }

      //Return
      orc_XmlParser.SelectNodeParent();
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("content-version");
      if (!c_Retval)
      {
         try
         {
            orc_IoData.u32_ContentVersion = static_cast<uint16_t>(std::stoi(orc_XmlParser.GetNodeContent()));
         }
         catch (...)
         {
            orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext("could not be converted to a number");
            c_Retval = Errc::config;
         }
         if (!c_Retval)
         {
            //Return
            orc_XmlParser.SelectNodeParent();
         }
      }
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("device-name");
      if (!c_Retval)
      {
         orc_IoData.c_DeviceName = orc_XmlParser.GetNodeContent();
         //Return
         orc_XmlParser.SelectNodeParent();
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("safety-mode") == "safety-mode")
      {
         const std::string c_NodeContent = orc_XmlParser.GetNodeContent();
         if (C_OscHalcDefFiler::mh_StringToSafetyMode(c_NodeContent, orc_IoData.e_SafetyMode))
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Unknown type");
            c_Retval = Errc::config;
         }
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      else
      {
         orc_IoData.e_SafetyMode = C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING;
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("number-of-configuration-copies") == "number-of-configuration-copies")
      {
         const std::string c_NodeContent = orc_XmlParser.GetNodeContent();
         try
         {
            orc_IoData.u8_NumConfigCopies = static_cast<uint8_t>(std::stoi(c_NodeContent));
            if (orc_IoData.u8_NumConfigCopies > 4U)
            {
               orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Number out of valid range");
               c_Retval = Errc::config;
            }
         }
         catch (...)
         {
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Number not valid");
            c_Retval = Errc::config;
         }
         //Return
         orc_XmlParser.SelectNodeParent();
      }
      else
      {
         orc_IoData.u8_NumConfigCopies = 1U;
      }
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefFiler::mh_LoadNvmData(orc_IoData, orc_XmlParser);
   }
   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("domain");
      if (!c_Retval)
      {
         std::string c_DomainNode;
         do
         {
            C_OscHalcDefDomain c_Domain;
            c_Retval = mh_LoadIoDataDomain(c_Domain, orc_XmlParser);
            if (!c_Retval)
            {
               orc_IoData.AddDomain(c_Domain);
               c_DomainNode = orc_XmlParser.SelectNodeNext("domain");
            }
         }
         while ((c_DomainNode == "domain") && (!c_Retval));
         if (!c_Retval)
         {
            //Return
            orc_XmlParser.SelectNodeParent();
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO data

   \param[in]      orc_IoData       Pointer to storage
   \param[in,out]  orc_XmlParser    XML with default state

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_SaveData(const C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   //File version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("file-version") == "file-version");
   orc_XmlParser.SetNodeContent(std::to_string(mhu16_FILE_VERSION_1));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Content version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("content-version") == "content-version");
   orc_XmlParser.SetNodeContent(std::to_string(orc_IoData.u32_ContentVersion));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Device
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("device-name") == "device-name");
   orc_XmlParser.SetNodeContent(orc_IoData.c_DeviceName);
   //Return
   orc_XmlParser.SelectNodeParent();
   //Safety mode
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("safety-mode") == "safety-mode");
   orc_XmlParser.SetNodeContent(mh_SafetyModeToString(orc_IoData.e_SafetyMode));
   //Return
   orc_XmlParser.SelectNodeParent();
   //Num copies
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(
                 "number-of-configuration-copies") == "number-of-configuration-copies");
   orc_XmlParser.SetNodeContent(std::to_string(orc_IoData.u8_NumConfigCopies));
   //Return
   orc_XmlParser.SelectNodeParent();
   //NVM config
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("nvm-config") == "nvm-config");
   orc_XmlParser.SetAttributeBool("active", orc_IoData.q_NvmBasedConfig);
   if (orc_IoData.q_NvmBasedConfig)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(
                    "datapools-start-address-offset") == "datapools-start-address-offset");
      for (uint32_t u32_It = 0UL; u32_It < orc_IoData.c_NvmNonSafeAddressOffset.size(); ++u32_It)
      {
         const std::string c_AttributeName = "address-offset-datapool-" + std::to_string(
            u32_It + 1U) + "-non-safe";
         orc_XmlParser.SetAttributeUint32(c_AttributeName, orc_IoData.c_NvmNonSafeAddressOffset[u32_It]);
      }
      for (uint32_t u32_It = 0UL; u32_It < orc_IoData.c_NvmSafeAddressOffset.size(); ++u32_It)
      {
         const std::string c_AttributeName = "address-offset-datapool-" + std::to_string(
            u32_It + 1U) + "-safe";
         orc_XmlParser.SetAttributeUint32(c_AttributeName, orc_IoData.c_NvmSafeAddressOffset[u32_It]);
      }
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "nvm-config");
   }
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild(
                 "lists-reserved-size") == "lists-reserved-size");
   orc_XmlParser.SetAttributeUint32("parameters-list-size", orc_IoData.u32_NvmReservedListSizeParameters);
   orc_XmlParser.SetAttributeUint32("input-values-list-size",
                                    orc_IoData.u32_NvmReservedListSizeInputValues);
   orc_XmlParser.SetAttributeUint32("output-values-list-size",
                                    orc_IoData.u32_NvmReservedListSizeOutputValues);
   orc_XmlParser.SetAttributeUint32("status-values-list-size",
                                    orc_IoData.u32_NvmReservedListSizeStatusValues);
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "nvm-config");
   //Return
   orc_XmlParser.SelectNodeParent();
   for (uint32_t u32_ItDomain = 0UL; (u32_ItDomain < orc_IoData.GetDomainSize()) && (!c_Retval);
        ++u32_ItDomain)
   {
      const C_OscHalcDefDomain * const pc_Domain = orc_IoData.GetDomainDefDataConst(u32_ItDomain);
      tgl_assert(pc_Domain != nullptr);
      if (pc_Domain != nullptr)
      {
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domain") == "domain");
         c_Retval = mh_SaveIoDomain(*pc_Domain, orc_XmlParser);
         //Return
         orc_XmlParser.SelectNodeParent();
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse IO channel use-case availability

   \param[in]   orc_AttributeName   Attribute name
   \param[out]  orc_Availability    Storage
   \param[in]   ou32_NumChannels    Number of available channels for this domain
   \param[in]   orc_XmlParser       XML parser

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_LoadAvailability(const std::string & orc_AttributeName,
                                                      std::vector<C_OscHalcDefChannelAvailability> & orc_Availability,
                                                      const uint32_t ou32_NumChannels,
                                                      const C_OscXmlParserBase & orc_XmlParser)
{
   std::string c_AvailabilityString;
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError(orc_AttributeName, c_AvailabilityString);

   if (!c_Retval)
   {
      std::vector<std::string> c_SubElements;
      c_Retval = C_OscHalcDefFiler::mh_SplitAvailabilityString(c_AvailabilityString, c_SubElements,
                                                                 orc_XmlParser, orc_AttributeName);

      orc_Availability.clear();
      if (!c_Retval)
      {
         c_Retval = C_OscHalcDefFiler::mh_ParseAvailabilityStringSubElements(c_SubElements, orc_Availability,
                                                                               ou32_NumChannels, orc_XmlParser,
                                                                               orc_AttributeName);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check use case value

   \param[in]  orc_IoDataDomain  IO data domain

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_CheckUseCaseValue(const C_OscHalcDefDomain & orc_IoDataDomain)
{
   std::error_code c_Retval = Errc::success;

   for (uint32_t u32_ItUseCase = 0UL;
        (u32_ItUseCase < orc_IoDataDomain.c_ChannelUseCases.size()) && (!c_Retval); ++u32_ItUseCase)
   {
      for (uint32_t u32_ItOtherUseCase = 0UL;
           (u32_ItOtherUseCase < orc_IoDataDomain.c_ChannelUseCases.size()) && (!c_Retval);
           ++u32_ItOtherUseCase)
      {
         if (u32_ItUseCase != u32_ItOtherUseCase)
         {
            const C_OscHalcDefChannelUseCase & rc_UseCase = orc_IoDataDomain.c_ChannelUseCases[u32_ItUseCase];
            const C_OscHalcDefChannelUseCase & rc_OtherUseCase = orc_IoDataDomain.c_ChannelUseCases[u32_ItOtherUseCase];
            if ((rc_UseCase.c_Value.GetArray() == rc_OtherUseCase.c_Value.GetArray()) &&
                (rc_UseCase.c_Value.GetType() == rc_OtherUseCase.c_Value.GetType()))
            {
               if (rc_UseCase.c_Value == rc_OtherUseCase.c_Value)
               {
                  osc_write_log_error("Loading HALC definition",
                                      "Duplicate value found in \"value\" attribute of \"channel-use-case\" section in domain \"" + orc_IoDataDomain.c_SingularName +
                                      "\".");
                  c_Retval = Errc::config;
               }
            }
            else
            {
               osc_write_log_error("Loading HALC definition",
                                   "\"value\" attribute of \"channel-use-case\" section is of different types in domain \"" + orc_IoDataDomain.c_SingularName +
                                   "\".");
               c_Retval = Errc::config;
            }
         }
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check domain display names

   \param[in]  orc_IoDataDomain  IO data domain

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::h_CheckDomainDisplayNames(const C_OscHalcDefDomain & orc_IoDataDomain)
{
   std::error_code c_Retval = Errc::success;

   std::vector<std::string> c_Names;

   C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_ChannelValues.c_Parameters, c_Names);
   C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_DomainValues.c_Parameters, c_Names);

   c_Retval = C_OscHalcDefFiler::mh_CheckDuplicateNames("parameters", orc_IoDataDomain.c_SingularName, c_Names);
   if (!c_Retval)
   {
      c_Names.clear();
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_ChannelValues.c_InputValues, c_Names);
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_DomainValues.c_InputValues, c_Names);
      c_Retval = C_OscHalcDefFiler::mh_CheckDuplicateNames("inputs", orc_IoDataDomain.c_SingularName, c_Names);
   }
   if (!c_Retval)
   {
      c_Names.clear();
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_ChannelValues.c_OutputValues, c_Names);
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_DomainValues.c_OutputValues, c_Names);
      c_Retval = C_OscHalcDefFiler::mh_CheckDuplicateNames("outputs", orc_IoDataDomain.c_SingularName, c_Names);
   }
   if (!c_Retval)
   {
      c_Names.clear();
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_ChannelValues.c_StatusValues, c_Names);
      C_OscHalcDefFiler::mh_GetAllNames(orc_IoDataDomain.c_DomainValues.c_StatusValues, c_Names);
      c_Retval = C_OscHalcDefFiler::mh_CheckDuplicateNames("status", orc_IoDataDomain.c_SingularName, c_Names);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefFiler::C_OscHalcDefFiler(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load NVM data

   \param[in,out]  orc_IoData       IO data
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_LoadNvmData(C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_IoData.c_NvmNonSafeAddressOffset.clear();
   orc_IoData.c_NvmSafeAddressOffset.clear();
   if (orc_XmlParser.SelectNodeChild("nvm-config") == "nvm-config")
   {
      if (orc_XmlParser.AttributeExists("active"))
      {
         orc_IoData.q_NvmBasedConfig = orc_XmlParser.GetAttributeBool("active");
      }
      else
      {
         orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("active", "is missing");
         c_Retval = Errc::config;
      }
      if (!c_Retval)
      {
         if (orc_XmlParser.SelectNodeChild("datapools-start-address-offset") == "datapools-start-address-offset")
         {
            c_Retval = mh_LoadNvmAddressOffsetData(orc_IoData.c_NvmNonSafeAddressOffset, orc_XmlParser,
                                                     orc_IoData.e_SafetyMode, orc_IoData.u8_NumConfigCopies, false);
            if (!c_Retval)
            {
               c_Retval = mh_LoadNvmAddressOffsetData(orc_IoData.c_NvmSafeAddressOffset, orc_XmlParser,
                                                        orc_IoData.e_SafetyMode, orc_IoData.u8_NumConfigCopies, true);
            }
            //Return
            orc_XmlParser.SelectNodeParent();
         }
         else
         {
            if (orc_IoData.q_NvmBasedConfig)
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("datapools-start-address-offset",
                                                                                  "is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
         }
         if (orc_XmlParser.SelectNodeChild("lists-reserved-size") == "lists-reserved-size")
         {
            if (orc_XmlParser.AttributeExists("parameters-list-size"))
            {
               orc_IoData.u32_NvmReservedListSizeParameters = orc_XmlParser.GetAttributeUint32(
                  "parameters-list-size");
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("parameters-list-size",
                                                                                  "is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
            if (orc_XmlParser.AttributeExists("input-values-list-size"))
            {
               orc_IoData.u32_NvmReservedListSizeInputValues = orc_XmlParser.GetAttributeUint32(
                  "input-values-list-size");
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("input-values-list-size",
                                                                                  "is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
            if (orc_XmlParser.AttributeExists("output-values-list-size"))
            {
               orc_IoData.u32_NvmReservedListSizeOutputValues = orc_XmlParser.GetAttributeUint32(
                  "output-values-list-size");
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("output-values-list-size",
                                                                                  "is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
            if (orc_XmlParser.AttributeExists("status-values-list-size"))
            {
               orc_IoData.u32_NvmReservedListSizeStatusValues = orc_XmlParser.GetAttributeUint32(
                  "status-values-list-size");
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("status-values-nvm-list-size",
                                                                                  "is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
            //Return
            orc_XmlParser.SelectNodeParent();
         }
         else
         {
            if (orc_IoData.q_NvmBasedConfig)
            {
               orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext(
                  "node \"lists-reserved-size\" is missing, but nvm-config is active");
               c_Retval = Errc::config;
            }
         }
      }
      //Return
      orc_XmlParser.SelectNodeParent();
   }
   else
   {
      orc_IoData.q_NvmBasedConfig = false;
      orc_IoData.u32_NvmReservedListSizeParameters = 100UL;
      orc_IoData.u32_NvmReservedListSizeInputValues = 100UL;
      orc_IoData.u32_NvmReservedListSizeOutputValues = 100UL;
      orc_IoData.u32_NvmReservedListSizeStatusValues = 100UL;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load NVM address offset data

   \param[in,out]  orc_Vector             Vector
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      oe_SafetyMode          Safety mode
   \param[in]      ou8_NumConfigCopies    Number of config copies
   \param[in]      oq_IsSafeVector        Is safe vector

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_LoadNvmAddressOffsetData(std::vector<uint32_t> & orc_Vector,
                                                               const C_OscXmlParserBase & orc_XmlParser,
                                                               const C_OscHalcDefBase::E_SafetyMode oe_SafetyMode,
                                                               const uint8_t ou8_NumConfigCopies,
                                                               const bool oq_IsSafeVector)
{
   std::error_code c_Retval = Errc::success;
   const std::string c_StartingPart = "address-offset-datapool-";
   const std::string c_ClosingPart = oq_IsSafeVector ? "-safe" : "-non-safe";

   for (uint8_t u8_It = 0U; u8_It < ou8_NumConfigCopies; ++u8_It)
   {
      //lint -e{9114} kept for readability
      const std::string c_CompleteAttributeName = c_StartingPart +
                                                            std::to_string(u8_It + 1U) +
                                                            c_ClosingPart;
      if (orc_XmlParser.AttributeExists(c_CompleteAttributeName))
      {
         orc_Vector.push_back(orc_XmlParser.GetAttributeUint32(c_CompleteAttributeName));
      }
      else
      {
         if ((((oe_SafetyMode == C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING) ||
               (oe_SafetyMode == C_OscHalcDefBase::eTWO_LEVELS_WITHOUT_DROPPING)) ||
              ((oe_SafetyMode == C_OscHalcDefBase::eONE_LEVEL_ALL_NON_SAFE) &&
               (oq_IsSafeVector == false))) ||
             ((oe_SafetyMode == C_OscHalcDefBase::eONE_LEVEL_ALL_SAFE) &&
              (oq_IsSafeVector == true)))
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(c_CompleteAttributeName,
                                                                               "is missing, but nvm-config is active");
            c_Retval = Errc::config;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save IO domain data

   \param[in]      orc_IoDataDomain    Pointer to storage
   \param[in,out]  orc_XmlParser       XML with default state

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_SaveIoDomain(const C_OscHalcDefDomain & orc_IoDataDomain,
                                                   C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_XmlParser.SetAttributeString("id", orc_IoDataDomain.c_Id);
   //Name
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("name") == "name");
   orc_XmlParser.SetNodeContent(orc_IoDataDomain.c_Name);
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
   //Singular name
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("singular-name") == "singular-name");
   orc_XmlParser.SetNodeContent(orc_IoDataDomain.c_SingularName);
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
   //Category
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("category") == "category");
   orc_XmlParser.SetNodeContent(C_OscHalcDefFiler::mh_DomainCategoryEnumToString(orc_IoDataDomain.e_Category));
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
   //Channels
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channels") == "channels");
   orc_XmlParser.SetAttributeUint32("count", static_cast<uint32_t>(orc_IoDataDomain.c_Channels.size()));
   for (uint32_t u32_ItChannel = 0UL; u32_ItChannel < orc_IoDataDomain.c_Channels.size(); ++u32_ItChannel)
   {
      const C_OscHalcDefChannelDef & rc_Channel = orc_IoDataDomain.c_Channels[u32_ItChannel];
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel") == "channel");
      orc_XmlParser.SetAttributeString("name", rc_Channel.c_Name);
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "channels");
   }
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
   //Channel Usecases
   if (orc_IoDataDomain.c_ChannelUseCases.size() > 0UL)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel-use-cases") == "channel-use-cases");
      for (uint32_t u32_ItCu = 0UL; (u32_ItCu < orc_IoDataDomain.c_ChannelUseCases.size()) && (!c_Retval);
           ++u32_ItCu)
      {
         const C_OscHalcDefChannelUseCase & rc_UseCase = orc_IoDataDomain.c_ChannelUseCases[u32_ItCu];
         if (u32_ItCu == 0UL)
         {
            //First saves the type
            orc_XmlParser.SetAttributeString("type",
                                             C_OscHalcDefStructFiler::h_GetTypeString(rc_UseCase.c_Value.GetType()));
         }
         tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel-use-case") == "channel-use-case");
         c_Retval = mh_SaveUseCase(orc_IoDataDomain.c_ChannelUseCases[u32_ItCu], orc_XmlParser);
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-use-cases");
         }
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_DomainValues.c_Parameters,
                                                          orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                          "domain",
                                                          "domain-parameters", "parameter", "parameter-element");
   }
   if (!c_Retval)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("domain-values") == "domain-values");
      c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_DomainValues.c_InputValues,
                                                          orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                          "domain-values",
                                                          "input-values", "value", "value-element");
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_DomainValues.c_OutputValues,
                                                          orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                          "domain-values",
                                                          "output-values", "value", "value-element");
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_DomainValues.c_StatusValues,
                                                          orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                          "domain-values",
                                                          "status-values", "value", "value-element");
   }
   if (!c_Retval)
   {
      //Return
      tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");

      {
         c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_ChannelValues.c_Parameters,
                                                             orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                             "domain",
                                                             "channel-parameters", "parameter", "parameter-element");

         if (!c_Retval)
         {
            tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("channel-values") == "channel-values");
            c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_ChannelValues.c_InputValues,
                                                                orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                                "channel-values",
                                                                "input-values", "value", "value-element");
         }
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_ChannelValues.c_OutputValues,
                                                                orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                                "channel-values",
                                                                "output-values", "value", "value-element");
         }
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_SaveStructs(orc_IoDataDomain.c_ChannelValues.c_StatusValues,
                                                                orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                                                "channel-values",
                                                                "status-values", "value", "value-element");
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
/*! \brief  Load IO domain

   \param[out]     orc_IoDataDomain    Storage
   \param[in,out]  orc_XmlParser       XML with default state

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_LoadIoDataDomain(C_OscHalcDefDomain & orc_IoDataDomain,
                                                       C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = orc_XmlParser.GetAttributeStringError("id", orc_IoDataDomain.c_Id);

   if (!c_Retval)
   {
      c_Retval = orc_XmlParser.SelectNodeChildError("name");
      if (!c_Retval)
      {
         orc_IoDataDomain.c_Name = orc_XmlParser.GetNodeContent();
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("singular-name") == "singular-name")
      {
         orc_IoDataDomain.c_SingularName = orc_XmlParser.GetNodeContent();
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
      else
      {
         //Optional
         orc_IoDataDomain.c_SingularName = orc_IoDataDomain.c_Name;
      }
      c_Retval = mh_LoadChannels(orc_IoDataDomain.c_Channels, orc_XmlParser);
      //Check
      if (!c_Retval)
      {
         const uint32_t u32_LONGEST_CONST_VAR_NAME_OFFSET = 13UL;
         if (orc_IoDataDomain.c_SingularName.length() > (C_OscHalcDefStructFiler::
                                                         hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH -
                                                         u32_LONGEST_CONST_VAR_NAME_OFFSET))
         {
            orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext(
               "content of domain \"singular-name\" (or \"name\" if not existing) node is too long, maximum allowed characters: " +
               std::to_string(C_OscHalcDefStructFiler::hu32_MAX_ALLOWED_COMBINED_VARIABLE_LENGTH -
                                     u32_LONGEST_CONST_VAR_NAME_OFFSET) +
               " (Current: " +
               std::to_string(orc_IoDataDomain.
                                     c_SingularName.length()) +
               ")");
            c_Retval = Errc::config;
         }
      }
   }
   if (!c_Retval)
   {
      //Category
      if (orc_XmlParser.SelectNodeChild("category") == "category")
      {
         if (C_OscHalcDefFiler::mh_DomainCategoryStringToEnum(orc_XmlParser.GetNodeContent(),
                                                              orc_IoDataDomain.e_Category))
         {
            c_Retval = Errc::config;
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext("Unknown value");
         }
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
      else
      {
         //Optional
         orc_IoDataDomain.e_Category = C_OscHalcDefDomain::eCA_OTHER;
      }
   }
   if (!c_Retval)
   {
      c_Retval = mh_LoadChannelUseCases(orc_IoDataDomain.c_ChannelUseCases, orc_XmlParser,
                                          static_cast<uint32_t>(orc_IoDataDomain.c_Channels.size()));
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_DomainValues.c_Parameters, orc_XmlParser,
                                                          orc_IoDataDomain.c_ChannelUseCases,  "domain",
                                                          "domain-parameters", "parameter", "parameter-element", true,
                                                          false, orc_IoDataDomain.c_SingularName.length());
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("domain-values") == "domain-values")
      {
         c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_DomainValues.c_InputValues,
                                                             orc_XmlParser,
                                                             orc_IoDataDomain.c_ChannelUseCases,
                                                             "domain-values",
                                                             "input-values", "value", "value-element", false,
                                                             false, orc_IoDataDomain.c_SingularName.length());
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_DomainValues.c_OutputValues,
                                                                orc_XmlParser,
                                                                orc_IoDataDomain.c_ChannelUseCases,
                                                                "domain-values", "output-values", "value",
                                                                "value-element",
                                                                false, false, orc_IoDataDomain.c_SingularName.length());
         }
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_DomainValues.c_StatusValues,
                                                                orc_XmlParser,
                                                                orc_IoDataDomain.c_ChannelUseCases,
                                                                "domain-values", "status-values", "value",
                                                                "value-element",
                                                                false, false, orc_IoDataDomain.c_SingularName.length());
         }
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
         }
      }
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_ChannelValues.c_Parameters, orc_XmlParser,
                                                          orc_IoDataDomain.c_ChannelUseCases,  "domain",
                                                          "channel-parameters", "parameter", "parameter-element", true,
                                                          false, orc_IoDataDomain.c_SingularName.length());
   }
   if (!c_Retval)
   {
      if (orc_XmlParser.SelectNodeChild("channel-values") == "channel-values")
      {
         c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_ChannelValues.c_InputValues,
                                                             orc_XmlParser,
                                                             orc_IoDataDomain.c_ChannelUseCases,
                                                             "channel-values",
                                                             "input-values", "value", "value-element", false,
                                                             false, orc_IoDataDomain.c_SingularName.length());
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_ChannelValues.c_OutputValues,
                                                                orc_XmlParser,
                                                                orc_IoDataDomain.c_ChannelUseCases,
                                                                "channel-values", "output-values", "value",
                                                                "value-element",
                                                                false, false, orc_IoDataDomain.c_SingularName.length());
         }
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefStructFiler::h_LoadStructs(orc_IoDataDomain.c_ChannelValues.c_StatusValues,
                                                                orc_XmlParser,
                                                                orc_IoDataDomain.c_ChannelUseCases,
                                                                "channel-values", "status-values", "value",
                                                                "value-element",
                                                                false, false, orc_IoDataDomain.c_SingularName.length());
         }
         if (!c_Retval)
         {
            //Return
            tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
         }
      }
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefFiler::mh_CheckIoDataDomain(orc_IoDataDomain);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check IO data domain

   \param[in]  orc_IoDataDomain  IO data domain

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_CheckIoDataDomain(const C_OscHalcDefDomain & orc_IoDataDomain)
{
   std::error_code c_Retval = C_OscHalcDefFiler::mh_CheckDefaultUseCase(orc_IoDataDomain);

   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefFiler::h_CheckUseCaseValue(orc_IoDataDomain);
   }
   if (!c_Retval)
   {
      c_Retval = C_OscHalcDefFiler::h_CheckDomainDisplayNames(orc_IoDataDomain);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check default use case

   \param[in]  orc_IoDataDomain  IO data domain

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_CheckDefaultUseCase(const C_OscHalcDefDomain & orc_IoDataDomain)
{
   std::error_code c_Retval = Errc::success;

   // check all channels if a default use case exists
   for (uint32_t u32_ItChannel = 0UL; (u32_ItChannel < orc_IoDataDomain.c_Channels.size()) && (!c_Retval);
        ++u32_ItChannel)
   {
      bool q_FoundOne = false;

      for (uint32_t u32_ItUseCase = 0UL;
           (u32_ItUseCase < orc_IoDataDomain.c_ChannelUseCases.size()) && (!c_Retval); ++u32_ItUseCase)
      {
         const C_OscHalcDefChannelUseCase & rc_UseCase = orc_IoDataDomain.c_ChannelUseCases[u32_ItUseCase];
         for (uint32_t u32_ItDefault = 0UL;
              (u32_ItDefault < rc_UseCase.c_DefaultChannels.size()) && (!c_Retval); ++u32_ItDefault)
         {
            if (u32_ItChannel == rc_UseCase.c_DefaultChannels[u32_ItDefault])
            {
               if (q_FoundOne == false)
               {
                  q_FoundOne = true;
               }
               else
               {
                  osc_write_log_error("Loading HALC definition",
                                      "Ambiguous default in attribute \"is-default-for\" for use-case index "  +
                                      std::to_string(
                                         u32_ItUseCase) + " in domain \"" + orc_IoDataDomain.c_SingularName +
                                      "\".");
                  c_Retval = Errc::config;
                  break;
               }
            }
         }
      }
      if (q_FoundOne == false)
      {
         osc_write_log_error("Loading HALC definition",
                             "Could not find default in attribute \"is-default-for\" for channel index " +
                             std::to_string(
                                u32_ItChannel) + " in domain \"" + orc_IoDataDomain.c_SingularName + "\".");
         c_Retval = Errc::config;
         break;
      }
   }

   // check all use cases if default is available
   for (uint32_t u32_ItUseCase = 0UL;
        (u32_ItUseCase < orc_IoDataDomain.c_ChannelUseCases.size()) && (!c_Retval); ++u32_ItUseCase)
   {
      const C_OscHalcDefChannelUseCase & rc_UseCase = orc_IoDataDomain.c_ChannelUseCases[u32_ItUseCase];

      for (uint32_t u32_ItDefault = 0UL;
           (u32_ItDefault < rc_UseCase.c_DefaultChannels.size()) && (!c_Retval); ++u32_ItDefault)
      {
         bool q_DefaultAvailable = false;
         for (uint32_t u32_ItAvail = 0UL; (u32_ItAvail < rc_UseCase.c_Availability.size()) && (!c_Retval);
              ++u32_ItAvail)
         {
            if (rc_UseCase.c_DefaultChannels[u32_ItDefault] == rc_UseCase.c_Availability[u32_ItAvail].u32_ValueIndex)
            {
               q_DefaultAvailable = true;
               break;
            }
         }
         if (q_DefaultAvailable == false)
         {
            osc_write_log_error("Loading HALC definition",
                                "Default in attribute \"is-default-for\" of use-case index " +
                                std::to_string(
                                   u32_ItUseCase) +
                                " (\"" + rc_UseCase.c_Display + "\") is not available for this use-case in domain \"" + orc_IoDataDomain.c_SingularName +
                                "\".");
            c_Retval = Errc::config;
         }
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO channels

   \param[out]     orc_Channels     Storage
   \param[in,out]  orc_XmlParser    XML with default state

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_LoadChannels(std::vector<C_OscHalcDefChannelDef> & orc_Channels,
                                                   C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = orc_XmlParser.SelectNodeChildError("channels");

   orc_Channels.clear();
   if (!c_Retval)
   {
      uint32_t u32_ExpectedCount = 0UL;
      c_Retval = orc_XmlParser.GetAttributeUint32Error("count", u32_ExpectedCount);
      orc_Channels.reserve(u32_ExpectedCount);
      if ((u32_ExpectedCount > 0UL) && (!c_Retval))
      {
         c_Retval = orc_XmlParser.SelectNodeChildError("channel");
         if (!c_Retval)
         {
            std::string c_NodeChannel;
            do
            {
               C_OscHalcDefChannelDef c_Channel;
               c_Retval = orc_XmlParser.GetAttributeStringError("name", c_Channel.c_Name);
               if (!c_Retval)
               {
                  orc_Channels.push_back(c_Channel);
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
         //Check
         if (u32_ExpectedCount != orc_Channels.size())
         {
            const std::string c_Error = "Expected " + std::to_string(u32_ExpectedCount) +
                                        " channels, got " + std::to_string(orc_Channels.size()) + " channels";
            orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(c_Error);
            c_Retval = Errc::config;
         }
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load IO channel use-cases

   \param[out]     orc_ChannelUsecases    Storage
   \param[in,out]  orc_XmlParser          XML with default state
   \param[in]      ou32_NumChannels       Number of available channels for this domain

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_LoadChannelUseCases(std::vector<C_OscHalcDefChannelUseCase> & orc_ChannelUsecases,
                                                          C_OscXmlParserBase & orc_XmlParser,
                                                          const uint32_t ou32_NumChannels)
{
   std::error_code c_Retval = Errc::success;

   orc_ChannelUsecases.clear();
   if (orc_XmlParser.SelectNodeChild("channel-use-cases") == "channel-use-cases")
   {
      C_OscNodeDataPoolContent c_Value;
      std::string c_TypeStr;
      c_Value.SetArray(false);
      c_Retval = orc_XmlParser.GetAttributeStringError("type", c_TypeStr);
      if (!c_Retval)
      {
         C_OscNodeDataPoolContent::E_Type e_Type;
         c_Retval = C_OscHalcDefStructFiler::h_GetTypeForSimplestTypeString(c_TypeStr, e_Type, orc_XmlParser);
         if (!c_Retval)
         {
            c_Value.SetType(e_Type);
         }
      }
      if (!c_Retval)
      {
         std::string c_NodeChannel = orc_XmlParser.SelectNodeChild("channel-use-case");
         if (c_NodeChannel == "channel-use-case")
         {
            do
            {
               C_OscHalcDefChannelUseCase c_UseCase;
               c_Retval = orc_XmlParser.GetAttributeStringError("id", c_UseCase.c_Id);
               if (!c_Retval)
               {
                  c_Retval = orc_XmlParser.GetAttributeStringError("display", c_UseCase.c_Display);
               }
               if (!c_Retval)
               {
                  c_UseCase.c_Value = c_Value;
                  c_Retval = C_OscHalcDefStructFiler::h_ParseSimplestTypeValue(c_TypeStr, c_UseCase.c_Value,
                                                                                 orc_XmlParser,
                                                                                 "value");
               }
               if (!c_Retval)
               {
                  c_Retval = h_LoadAvailability("availability", c_UseCase.c_Availability, ou32_NumChannels,
                                                  orc_XmlParser);
                  if (!c_Retval)
                  {
                     C_OscHalcDefFiler::mh_CheckAvailability(c_UseCase.c_Availability, orc_XmlParser);
                  }
               }
               if (!c_Retval)
               {
                  std::vector<C_OscHalcDefChannelAvailability> c_TmpAvailability;
                  if (orc_XmlParser.AttributeExists("is-default-for"))
                  {
                     c_Retval = h_LoadAvailability("is-default-for", c_TmpAvailability, ou32_NumChannels,
                                                     orc_XmlParser);
                  }
                  c_UseCase.c_DefaultChannels.clear();
                  for (uint32_t u32_It = 0UL; (u32_It < c_TmpAvailability.size()) && (!c_Retval); ++u32_It)
                  {
                     const C_OscHalcDefChannelAvailability & rc_Availability = c_TmpAvailability[u32_It];
                     c_UseCase.c_DefaultChannels.push_back(rc_Availability.u32_ValueIndex);
                     if (rc_Availability.c_DependentValues.size() > 0UL)
                     {
                        orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("is-default-for",
                                                                                     "Found invalid grouped section");
                        c_Retval = Errc::config;
                     }
                  }
               }
               if (!c_Retval)
               {
                  if (orc_XmlParser.SelectNodeChild("comment") == "comment")
                  {
                     c_UseCase.c_Comment = orc_XmlParser.GetNodeContent();
                     //Return
                     tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-use-case");
                  }
                  orc_ChannelUsecases.push_back(c_UseCase);
                  c_NodeChannel = orc_XmlParser.SelectNodeNext("channel-use-case");
               }
            }
            while ((c_NodeChannel == "channel-use-case") && (!c_Retval));
            if (!c_Retval)
            {
               //Return
               tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-use-cases");
            }
         }
      }
      if (!c_Retval)
      {
         //Return
         tgl_assert(orc_XmlParser.SelectNodeParent() == "domain");
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse IO channel use-case availability string into its string sub elements

   \param[in]   orc_AvailabilityString    Availability string
   \param[out]  orc_SubElements           String subelements
   \param[in]   orc_XmlParser             XML parser
   \param[in]   orc_AttributeName         Attribute name

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_SplitAvailabilityString(const std::string & orc_AvailabilityString,
                                                              std::vector<std::string> & orc_SubElements,
                                                              const C_OscXmlParserBase & orc_XmlParser,
                                                              const std::string & orc_AttributeName)
{
   std::error_code c_Retval = Errc::success;
   bool q_IgnoreComma = false;

   std::string c_CurSubElement;

   orc_SubElements.clear();

   //Parse subelements
   for (uint32_t u32_ItChar = 0UL; (u32_ItChar < orc_AvailabilityString.length()) && (!c_Retval);
        ++u32_ItChar)
   {
      const char_t cn_CurChar = orc_AvailabilityString[u32_ItChar + 1U];
      if (cn_CurChar == ',')
      {
         //Char:comma
         if (q_IgnoreComma)
         {
            //Special: part of enumeration, just add, don't start new section
            c_CurSubElement += cn_CurChar;
         }
         else
         {
            if (c_CurSubElement.empty() == false)
            {
               //Finish substring
               orc_SubElements.push_back(c_CurSubElement);
               //Clean
               c_CurSubElement = "";
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName,
                                                                                  "contains empty section");
               c_Retval = Errc::config;
            }
         }
      }
      else
      {
         //Char:any except comma
         c_CurSubElement += cn_CurChar;
         if (cn_CurChar == '{')
         {
            if (q_IgnoreComma)
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName,
                                                                                  "does currently not support nested sections of '{' characters");
               c_Retval = Errc::config;
            }
            else
            {
               q_IgnoreComma = true;
            }
         }
         else if (cn_CurChar == '}')
         {
            if (q_IgnoreComma)
            {
               q_IgnoreComma = false;
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName,
                                                                                  "contains unexpected '}' character");
               c_Retval = Errc::config;
            }
         }
         else
         {
            //No special handling
         }
      }
   }
   //Handle last element
   if (c_CurSubElement.empty() == false)
   {
      //Finish substring
      orc_SubElements.push_back(c_CurSubElement);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse IO channel use-case availability string into its string sub elements

   \param[in]      orc_SubElements     String subelements
   \param[in,out]  orc_Availability    Availability
   \param[in]      ou32_NumChannels    Number of available channels for this domain
   \param[in]      orc_XmlParser       XML parser
   \param[in]      orc_AttributeName   Attribute name

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_ParseAvailabilityStringSubElements(
   const std::vector<std::string> & orc_SubElements, std::vector<C_OscHalcDefChannelAvailability> & orc_Availability,
   const uint32_t ou32_NumChannels, const C_OscXmlParserBase & orc_XmlParser, const std::string & orc_AttributeName)
{
   std::error_code c_Retval = Errc::success;

   for (uint32_t u32_ItSubStr = 0UL; (u32_ItSubStr < orc_SubElements.size()) && (!c_Retval);
        ++u32_ItSubStr)
   {
      const std::string & rc_CurSubStr = orc_SubElements[u32_ItSubStr];
      if (rc_CurSubStr == "all")
      {
         for (uint32_t u32_ItChannel = 0UL; u32_ItChannel < ou32_NumChannels; ++u32_ItChannel)
         {
            C_OscHalcDefChannelAvailability c_Availability;
            c_Availability.u32_ValueIndex = u32_ItChannel;
            orc_Availability.push_back(c_Availability);
         }
      }
      else
      {
         std::string c_Number;
         std::vector<int32_t> c_FoundNumbers;
         bool q_IsGroupSection = false;
         bool q_LastNumDeclaredSection = false;

         for (uint32_t u32_ItChar = 0UL; (u32_ItChar < rc_CurSubStr.length()) && (!c_Retval); ++u32_ItChar)
         {
            const char_t cn_CurChar = rc_CurSubStr[u32_ItChar + 1U];
            if (std::isdigit(cn_CurChar) != 0)
            {
               //Number
               c_Number += cn_CurChar;
            }
            else
            {
               //What to do with this number
               if (cn_CurChar == '-')
               {
                  if (q_LastNumDeclaredSection)
                  {
                     orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "contains unexpected character sequence with multiple '-' characters:\"" +
                                                                                        rc_CurSubStr + "\"");
                     c_Retval = Errc::config;
                  }
                  else
                  {
                     int32_t s32_NumberTmp = 0;
                     //Handle any left over number
                     c_Retval = C_OscHalcDefFiler::mh_ConvertStringToNumber(c_Number, s32_NumberTmp, orc_XmlParser,
                                                                              orc_AttributeName);
                     c_Number = "";
                     c_FoundNumbers.push_back(s32_NumberTmp);
                     q_LastNumDeclaredSection = true;
                  }
               }
               else if ((cn_CurChar == '{') || (cn_CurChar == '}'))
               {
                  q_IsGroupSection = true;
               }
               else if (cn_CurChar == ',')
               {
                  //Handle vector
                  c_Retval = C_OscHalcDefFiler::mh_HandleNumberSection(c_Number, c_FoundNumbers,
                                                                         q_LastNumDeclaredSection, rc_CurSubStr,
                                                                         orc_XmlParser, orc_AttributeName);
               }
               else
               {
                  orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "contains unexpected character '" +
                                                                                     std::to_string(
                                                                                        cn_CurChar) + "'");
                  c_Retval = Errc::config;
               }
            }
         }
         //Handle vector
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefFiler::mh_HandleNumberSection(c_Number, c_FoundNumbers,
                                                                   q_LastNumDeclaredSection, rc_CurSubStr,
                                                                   orc_XmlParser, orc_AttributeName);
         }
         //Handle availability
         if (!c_Retval)
         {
            c_Retval = C_OscHalcDefFiler::mh_HandleNumberSectionEnd(c_FoundNumbers, q_IsGroupSection,
                                                                      orc_Availability,
                                                                      ou32_NumChannels, orc_XmlParser,
                                                                      orc_AttributeName);
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check availability

   \param[in]  orc_Availability  Availability
   \param[in]  orc_XmlParser     XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_CheckAvailability(
   const std::vector<C_OscHalcDefChannelAvailability> & orc_Availability, const C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   //Compare each with everyone
   for (uint32_t u32_ItAvailability = 0UL; u32_ItAvailability < orc_Availability.size(); ++u32_ItAvailability)
   {
      const C_OscHalcDefChannelAvailability & rc_Availability = orc_Availability[u32_ItAvailability];
      for (uint32_t u32_ItOtherAvailability = 0UL; u32_ItOtherAvailability < orc_Availability.size();
           ++u32_ItOtherAvailability)
      {
         if (u32_ItAvailability != u32_ItOtherAvailability)
         {
            const C_OscHalcDefChannelAvailability & rc_OtherAvailability = orc_Availability[u32_ItOtherAvailability];
            //Check duplicate usages
            //Only this test is necessary as all dependency indices
            // have to be a value index for each group at least once
            if (rc_Availability.u32_ValueIndex == rc_OtherAvailability.u32_ValueIndex)
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext("availability", "contains duplicate usage of channel index " +
                                                                                  std::to_string(
                                                                                     rc_OtherAvailability.u32_ValueIndex));
               c_Retval = Errc::config;
            }
         }
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse IO channel use-case availability string into number

   \param[in]   orc_Number          String number
   \param[out]  ors32_Number        Converted number
   \param[in]   orc_XmlParser       XML parser
   \param[in]   orc_AttributeName   Attribute name

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_ConvertStringToNumber(const std::string & orc_Number, int32_t & ors32_Number,
                                                            const C_OscXmlParserBase & orc_XmlParser,
                                                            const std::string & orc_AttributeName)
{
   std::error_code c_Retval = Errc::success;

   try
   {
      ors32_Number = std::stoi(orc_Number);
   }
   catch (...)
   {
      orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName,
                                                                         "contains unexpected character sequence \"" +
                                                                         orc_Number + "\"");
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle number section segment

   \param[in,out]  orc_Number                   String number
   \param[in,out]  orc_FoundNumbers             Last found numbers
   \param[in,out]  orq_LastNumDeclaredSection   Flag if last element of section should be connected to this number
   \param[in]      orc_Section                  Section string for user information
   \param[in]      orc_XmlParser                XML parser
   \param[in]      orc_AttributeName            Attribute name

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_HandleNumberSection(std::string & orc_Number,
                                                          std::vector<int32_t> & orc_FoundNumbers,
                                                          bool & orq_LastNumDeclaredSection,
                                                          const std::string & orc_Section,
                                                          const C_OscXmlParserBase & orc_XmlParser,
                                                          const std::string & orc_AttributeName)
{
   std::error_code c_Retval = Errc::success;
   int32_t s32_NumberTmp = 0;

   //Handle any left over number
   c_Retval =
      C_OscHalcDefFiler::mh_ConvertStringToNumber(orc_Number, s32_NumberTmp, orc_XmlParser, orc_AttributeName);
   orc_Number = "";
   if (!c_Retval)
   {
      if (orq_LastNumDeclaredSection)
      {
         orq_LastNumDeclaredSection = false;
         if (orc_FoundNumbers.size() > 0UL)
         {
            const int32_t s32_NumberStart =
               orc_FoundNumbers[static_cast<std::vector<int32_t>::size_type>(orc_FoundNumbers.size() - 1UL)];
            orc_FoundNumbers.pop_back();
            if (s32_NumberStart < s32_NumberTmp)
            {
               for (int32_t s32_It = s32_NumberStart; s32_It <= s32_NumberTmp; ++s32_It)
               {
                  orc_FoundNumbers.push_back(s32_It);
               }
            }
            else
            {
               orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "contains invalid usage of '-' character in \"" +
                                                                                  orc_Section + "\"");
               c_Retval = Errc::config;
            }
         }
         else
         {
            orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "contains confusing usage of '-' character in \"" +
                                                                               orc_Section + "\"");
            c_Retval = Errc::config;
         }
      }
      else
      {
         orc_FoundNumbers.push_back(s32_NumberTmp);
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle number section segment end

   \param[in]      orc_FoundNumbers    Last found numbers
   \param[in]      oq_IsGroupSection   Flag if current section is group of elements
   \param[in,out]  orc_Availability    Availability
   \param[in]      ou32_NumChannels    Number of available channels for this domain
   \param[in]      orc_XmlParser       XML parser
   \param[in]      orc_AttributeName   Attribute name

   \return
   Errc::success    data read
   Errc::config     string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_HandleNumberSectionEnd(
   const std::vector<int32_t> & orc_FoundNumbers, const bool oq_IsGroupSection,
   std::vector<C_OscHalcDefChannelAvailability> & orc_Availability, const uint32_t ou32_NumChannels,
   const C_OscXmlParserBase & orc_XmlParser, const std::string & orc_AttributeName)
{
   std::error_code c_Retval = Errc::success;

   //Check
   for (uint32_t u32_ItNum = 0UL; u32_ItNum < orc_FoundNumbers.size(); ++u32_ItNum)
   {
      if (orc_FoundNumbers[u32_ItNum] >= static_cast<int32_t>(ou32_NumChannels))
      {
         orc_XmlParser.ReportErrorForAttributeContentStartingWithXmlContext(orc_AttributeName, "has number " +
                                                                            std::to_string(orc_FoundNumbers[
                                                                                                     u32_ItNum]) +
                                                                            " which is out of range of number of channels: " +
                                                                            std::to_string(ou32_NumChannels));
         c_Retval = Errc::config;
      }
   }
   //Assign
   if (oq_IsGroupSection)
   {
      if (orc_FoundNumbers.size() > 0UL)
      {
         {
            C_OscHalcDefChannelAvailability c_Availability;
            c_Availability.u32_ValueIndex = orc_FoundNumbers[0UL];
            for (uint32_t u32_ItChannel = 1UL; u32_ItChannel < orc_FoundNumbers.size(); ++u32_ItChannel)
            {
               c_Availability.c_DependentValues.push_back(orc_FoundNumbers[u32_ItChannel]);
            }
            orc_Availability.push_back(c_Availability);
         }
         //handle other sorting
         for (uint32_t u32_ItOthers = 1UL; u32_ItOthers < orc_FoundNumbers.size(); ++u32_ItOthers)
         {
            C_OscHalcDefChannelAvailability c_Availability;
            c_Availability.u32_ValueIndex = orc_FoundNumbers[u32_ItOthers];
            for (uint32_t u32_ItDepend = 0UL; u32_ItDepend < orc_FoundNumbers.size(); ++u32_ItDepend)
            {
               if (orc_FoundNumbers[u32_ItDepend] != orc_FoundNumbers[u32_ItOthers])
               {
                  c_Availability.c_DependentValues.push_back(orc_FoundNumbers[u32_ItDepend]);
               }
            }
            orc_Availability.push_back(c_Availability);
         }
      }
   }
   else
   {
      for (uint32_t u32_ItNum = 0UL; u32_ItNum < orc_FoundNumbers.size(); ++u32_ItNum)
      {
         C_OscHalcDefChannelAvailability c_Availability;
         c_Availability.u32_ValueIndex = orc_FoundNumbers[u32_ItNum];
         orc_Availability.push_back(c_Availability);
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get availability string

   \param[in]  orc_Availability  Availability
   \param[in]  oq_OnlySaveOnce   Only save once

   \return
   Availability string
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHalcDefFiler::mh_GetAvailabilityString(
   const std::vector<C_OscHalcDefChannelAvailability> & orc_Availability, const bool oq_OnlySaveOnce)
{
   std::string c_Retval;

   for (uint32_t u32_ItAva = 0UL; u32_ItAva < orc_Availability.size(); ++u32_ItAva)
   {
      const C_OscHalcDefChannelAvailability & rc_Avail = orc_Availability[u32_ItAva];
      if (rc_Avail.c_DependentValues.size() == 0UL)
      {
         if (c_Retval.empty() == false)
         {
            c_Retval += ",";
         }
         c_Retval += std::to_string(rc_Avail.u32_ValueIndex);
      }
      else
      {
         bool q_Continue = true;
         if (oq_OnlySaveOnce)
         {
            //Check sorted ascending (only save each pairing exactly once)
            uint32_t u32_RefVal = rc_Avail.u32_ValueIndex;
            for (uint32_t u32_ItDep = 0UL; u32_ItDep < rc_Avail.c_DependentValues.size(); ++u32_ItDep)
            {
               if (u32_RefVal < rc_Avail.c_DependentValues[u32_ItDep])
               {
                  u32_RefVal = rc_Avail.c_DependentValues[u32_ItDep];
               }
               else
               {
                  q_Continue = false;
                  break;
               }
            }
         }
         if (q_Continue)
         {
            if (c_Retval.empty() == false)
            {
               c_Retval += ",";
            }
            c_Retval += "{";
            c_Retval += std::to_string(rc_Avail.u32_ValueIndex);
            for (uint32_t u32_ItDep = 0UL; u32_ItDep < rc_Avail.c_DependentValues.size(); ++u32_ItDep)
            {
               c_Retval += ",";
               c_Retval += std::to_string(rc_Avail.c_DependentValues[u32_ItDep]);
            }
            c_Retval += "}";
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save use case

   \param[in]      orc_UseCase      Use case
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data saved
   Errc::config     data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_SaveUseCase(const C_OscHalcDefChannelUseCase & orc_UseCase,
                                                  C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_XmlParser.SetAttributeString("id", orc_UseCase.c_Id);
   orc_XmlParser.SetAttributeString("display", orc_UseCase.c_Display);
   c_Retval = C_OscHalcDefStructFiler::h_SaveSimpleValueAsAttribute("value", orc_XmlParser, orc_UseCase.c_Value);
   orc_XmlParser.SetAttributeString("availability",
                                    C_OscHalcDefFiler::mh_GetAvailabilityString(orc_UseCase.c_Availability, true));
   {
      std::vector<C_OscHalcDefChannelAvailability> c_TmpAvailabilities;
      for (uint32_t u32_It = 0UL; u32_It < orc_UseCase.c_DefaultChannels.size(); ++u32_It)
      {
         C_OscHalcDefChannelAvailability c_TmpAvailability;
         c_TmpAvailability.u32_ValueIndex = orc_UseCase.c_DefaultChannels[u32_It];
         c_TmpAvailabilities.push_back(c_TmpAvailability);
      }
      if (c_TmpAvailabilities.size() > 0UL)
      {
         orc_XmlParser.SetAttributeString("is-default-for",
                                          C_OscHalcDefFiler::mh_GetAvailabilityString(c_TmpAvailabilities, false));
      }
   }
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("comment") == "comment");
   orc_XmlParser.SetNodeContent(orc_UseCase.c_Comment);
   //Return
   tgl_assert(orc_XmlParser.SelectNodeParent() == "channel-use-case");
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Domain category enum to string

   \param[in]  oe_Category    Category

   \return
   string representation of oe_Category
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHalcDefFiler::mh_DomainCategoryEnumToString(const C_OscHalcDefDomain::E_Category oe_Category)
{
   std::string c_Retval;

   switch (oe_Category)
   {
   case C_OscHalcDefDomain::eCA_INPUT:
      c_Retval = "input";
      break;
   case C_OscHalcDefDomain::eCA_OUTPUT:
      c_Retval = "output";
      break;
   case C_OscHalcDefDomain::eCA_OTHER:
      c_Retval = "other";
      break;
   default:
      break;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Domain category string to enum

   \param[in]   orc_Category  Category
   \param[out]  ore_Category  Category

   \return
   Errc::success    no error
   Errc::range      String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_DomainCategoryStringToEnum(const std::string & orc_Category,
                                                                 C_OscHalcDefDomain::E_Category & ore_Category)
{
   std::error_code c_Retval = Errc::success;

   if (orc_Category == "input")
   {
      ore_Category = C_OscHalcDefDomain::eCA_INPUT;
   }
   else if (orc_Category == "output")
   {
      ore_Category = C_OscHalcDefDomain::eCA_OUTPUT;
   }
   else if (orc_Category == "other")
   {
      ore_Category = C_OscHalcDefDomain::eCA_OTHER;
   }
   else
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get all names

   \param[in]      orc_Values    Values
   \param[in,out]  orc_Names     Names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefFiler::mh_GetAllNames(const std::vector<C_OscHalcDefStruct> & orc_Values,
                                       std::vector<std::string> & orc_Names)
{
   for (uint32_t u32_ItVal = 0UL; u32_ItVal < orc_Values.size(); ++u32_ItVal)
   {
      const C_OscHalcDefStruct & rc_Struct = orc_Values[u32_ItVal];
      if (rc_Struct.c_StructElements.size() == 0UL)
      {
         orc_Names.push_back(rc_Struct.c_Display);
      }
      for (uint32_t u32_ItValElem = 0UL; u32_ItValElem < rc_Struct.c_StructElements.size(); ++u32_ItValElem)
      {
         const C_OscHalcDefElement & rc_Elem = rc_Struct.c_StructElements[u32_ItValElem];
         orc_Names.push_back(rc_Elem.c_Display);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check duplicate names

   \param[in]      orc_Section               Section
   \param[in]      orc_DomainSingularName    Domain singular name
   \param[in,out]  orc_Names                 Names

   \return
   Errc::success    data read
   Errc::config     IO definition content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_CheckDuplicateNames(const std::string & orc_Section,
                                                          const std::string & orc_DomainSingularName,
                                                          const std::vector<std::string> & orc_Names)
{
   std::error_code c_Retval = Errc::success;

   for (uint32_t u32_ItName1 = 0UL; (u32_ItName1 < orc_Names.size()) && (!c_Retval); ++u32_ItName1)
   {
      for (uint32_t u32_ItName2 = u32_ItName1; (u32_ItName2 < orc_Names.size()) && (!c_Retval);
           ++u32_ItName2)
      {
         if (u32_ItName1 != u32_ItName2)
         {
            const std::string & rc_Name = orc_Names[u32_ItName2];
            if (orc_Names[u32_ItName1] == rc_Name)
            {
               c_Retval = Errc::config;
               osc_write_log_error("Loading HALC definition",
                                   "Duplicate display name found in domain \"" + orc_DomainSingularName +
                                   "\", section \"" + orc_Section + "\": \"" + orc_Names[u32_ItName1] + "\"");
            }
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Safety mode to string

   \param[in]  ore_NodeDataPoolElementAccess    Node data pool element access

   \return
   Safety mode as string
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscHalcDefFiler::mh_SafetyModeToString(
   const C_OscHalcDefBase::E_SafetyMode & ore_NodeDataPoolElementAccess)
{
   std::string c_Retval;

   switch (ore_NodeDataPoolElementAccess)
   {
   case C_OscHalcDefBase::eTWO_LEVELS_WITHOUT_DROPPING:
      c_Retval = "two-levels-without-dropping";
      break;
   case C_OscHalcDefBase::eONE_LEVEL_ALL_SAFE:
      c_Retval = "one-level-all-safe";
      break;
   case C_OscHalcDefBase::eONE_LEVEL_ALL_NON_SAFE:
      c_Retval = "one-level-all-non-safe";
      break;
   case C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING:
   default:
      c_Retval = "two-levels-with-dropping";
      break;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  String to safety mode

   \param[in]   orc_String    String
   \param[out]  ore_Type      Type

   \return
   Errc::success    valid type
   Errc::range      unknown type
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscHalcDefFiler::mh_StringToSafetyMode(const std::string & orc_String,
                                                         C_OscHalcDefBase::E_SafetyMode & ore_Type)
{
   std::error_code c_Retval = Errc::success;

   if (orc_String == "two-levels-with-dropping")
   {
      ore_Type = C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING;
   }
   else if (orc_String == "two-levels-without-dropping")
   {
      ore_Type = C_OscHalcDefBase::eTWO_LEVELS_WITHOUT_DROPPING;
   }
   else if (orc_String == "one-level-all-safe")
   {
      ore_Type = C_OscHalcDefBase::eONE_LEVEL_ALL_SAFE;
   }
   else if (orc_String == "one-level-all-non-safe")
   {
      ore_Type = C_OscHalcDefBase::eONE_LEVEL_ALL_NON_SAFE;
   }
   else
   {
      ore_Type = C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING;
      c_Retval = Errc::range;
   }
   return c_Retval;
}
