//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load HALC definition
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFFILER_HPP
#define C_OSCHALCDEFFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscHalcDef.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefFiler
{
public:
   static std::error_code h_LoadFile(C_OscHalcDefBase & orc_IoData, const std::string & orc_Path);
   static std::error_code h_SaveFile(const C_OscHalcDefBase & orc_IoData, const std::string & orc_Path);
   static std::error_code h_LoadData(C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_SaveData(const C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser);

   static std::error_code h_LoadAvailability(const std::string & orc_AttributeName,
                                             std::vector<C_OscHalcDefChannelAvailability> & orc_Availability,
                                             const uint32_t ou32_NumChannels, const C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_CheckUseCaseValue(const C_OscHalcDefDomain & orc_IoDataDomain);
   static std::error_code h_CheckDomainDisplayNames(const C_OscHalcDefDomain & orc_IoDataDomain);

private:
   static const uint16_t mhu16_FILE_VERSION_1 = 1U;

   C_OscHalcDefFiler(void);

   static std::error_code mh_LoadNvmData(C_OscHalcDefBase & orc_IoData, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadNvmAddressOffsetData(std::vector<uint32_t> & orc_Vector,
                                                      const C_OscXmlParserBase & orc_XmlParser,
                                                      const C_OscHalcDefBase::E_SafetyMode oe_SafetyMode,
                                                      const uint8_t ou8_NumConfigCopies, const bool oq_IsSafeVector);
   static std::error_code mh_SaveIoDomain(const C_OscHalcDefDomain & orc_IoDataDomain,
                                          C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadIoDataDomain(C_OscHalcDefDomain & orc_IoDataDomain,
                                              C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_CheckIoDataDomain(const C_OscHalcDefDomain & orc_IoDataDomain);
   static std::error_code mh_CheckDefaultUseCase(const C_OscHalcDefDomain & orc_IoDataDomain);
   static std::error_code mh_LoadChannels(std::vector<C_OscHalcDefChannelDef> & orc_Channels,
                                          C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadChannelUseCases(std::vector<C_OscHalcDefChannelUseCase> & orc_ChannelUsecases,
                                                 C_OscXmlParserBase & orc_XmlParser, const uint32_t ou32_NumChannels);
   static std::error_code mh_SplitAvailabilityString(const std::string & orc_AvailabilityString,
                                                     std::vector<std::string> & orc_SubElements,
                                                     const C_OscXmlParserBase & orc_XmlParser,
                                                     const std::string & orc_AttributeName);
   static std::error_code mh_ParseAvailabilityStringSubElements(
      const std::vector<std::string> & orc_SubElements,
      std::vector<C_OscHalcDefChannelAvailability> & orc_Availability, const uint32_t ou32_NumChannels,
      const C_OscXmlParserBase & orc_XmlParser, const std::string & orc_AttributeName);
   static std::error_code mh_CheckAvailability(const std::vector<C_OscHalcDefChannelAvailability> & orc_Availability,
                                               const C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_ConvertStringToNumber(const std::string & orc_Number, int32_t & ors32_Number,
                                                   const C_OscXmlParserBase & orc_XmlParser,
                                                   const std::string & orc_AttributeName);
   static std::error_code mh_HandleNumberSection(std::string & orc_Number, std::vector<int32_t> & orc_FoundNumbers,
                                                 bool & orq_LastNumDeclaredSection, const std::string & orc_Section,
                                                 const C_OscXmlParserBase & orc_XmlParser,
                                                 const std::string & orc_AttributeName);
   static std::error_code mh_HandleNumberSectionEnd(const std::vector<int32_t> & orc_FoundNumbers,
                                                    const bool oq_IsGroupSection,
                                                    std::vector<C_OscHalcDefChannelAvailability> & orc_Availability,
                                                    const uint32_t ou32_NumChannels,
                                                    const C_OscXmlParserBase & orc_XmlParser,
                                                    const std::string & orc_AttributeName);
   static std::string mh_GetAvailabilityString(
      const std::vector<C_OscHalcDefChannelAvailability> & orc_Availability, const bool oq_OnlySaveOnce);
   static std::error_code mh_SaveUseCase(const C_OscHalcDefChannelUseCase & orc_UseCase,
                                         C_OscXmlParserBase & orc_XmlParser);
   static std::string mh_DomainCategoryEnumToString(const C_OscHalcDefDomain::E_Category oe_Category);
   static std::error_code mh_DomainCategoryStringToEnum(const std::string & orc_Category,
                                                        C_OscHalcDefDomain::E_Category & ore_Category);
   static void mh_GetAllNames(const std::vector<C_OscHalcDefStruct> & orc_Values,
                              std::vector<std::string> & orc_Names);
   static std::error_code mh_CheckDuplicateNames(const std::string & orc_Section,
                                                 const std::string & orc_DomainSingularName,
                                                 const std::vector<std::string> & orc_Names);
   static std::string mh_SafetyModeToString(
      const C_OscHalcDefBase::E_SafetyMode & ore_NodeDataPoolElementAccess);
   static std::error_code mh_StringToSafetyMode(const std::string & orc_String,
                                                C_OscHalcDefBase::E_SafetyMode & ore_Type);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
