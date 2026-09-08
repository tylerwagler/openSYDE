//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export HALC configuration of an openSYDE node.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTHALC_HPP
#define C_OSCEXPORTHALC_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "stwtypes.hpp"
#include <string>
#include <vector>
#include "C_OscHalcConfig.hpp"
#include "C_OscNodeDataPool.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscExportHalc
{
public:
   static std::string h_GetFileName(const bool oq_IsSafe);
   static uint16_t h_ConvertOverallCodeVersion(const uint16_t ou16_GenCodeVersion);
   static std::error_code h_CreateSourceCode(const std::string & orc_Path, const uint16_t ou16_GenCodeVersion,
                                             const C_OscHalcConfig & orc_HalcConfig,
                                             const stw::opensyde_core::C_OscNodeDataPool & orc_Datapool,
                                             const std::string & orc_ExportToolInfo);

protected:
   static const bool mhq_IS_HEADER_FILE = false;
   static const bool mhq_IS_IMPLEMENTATION_FILE = true;

   static std::error_code mh_CreateHeaderFile(const std::string & orc_ExportToolInfo,
                                              const std::string & orc_Path, const C_OscHalcConfig & orc_HalcConfig,
                                              const std::string & orc_ProjectId, const bool oq_IsSafe,
                                              const uint16_t ou16_GenCodeVersion);

   static std::error_code mh_CreateImplementationFile(const std::string & orc_ExportToolInfo,
                                                      const std::string & orc_Path,
                                                      const C_OscHalcConfig & orc_HalcConfig,
                                                      const std::string & orc_ProjectId, const bool oq_IsSafe,
                                                      const uint16_t ou16_GenCodeVersion);

   static void mh_AddHeader(const std::string & orc_ExportToolInfo, std::vector<std::string> & orc_Data,
                            const bool oq_FileType, const bool oq_IsSafe);
   static void mh_AddIncludes(std::vector<std::string> & orc_Data, const bool oq_FileType, const bool oq_IsSafe);
   static void mh_AddDefines(std::vector<std::string> & orc_Data,  const C_OscHalcConfig & orc_HalcConfig,
                             const std::string & orc_ProjectId, const bool oq_FileType, const bool oq_IsSafe,
                             const uint16_t ou16_GenCodeVersion);
   static void mh_AddGlobalVariables(std::vector<std::string> & orc_Data, const C_OscHalcConfig & orc_HalcConfig,
                                     const bool oq_FileType, const bool oq_IsSafe);
   static std::string mh_GetMagicName(const std::string & orc_ProjectId, const bool oq_IsSafe);
   static void mh_AddDpListElementReferences(std::vector<std::string> & orc_Data,
                                             const std::vector<C_OscHalcDefStruct> & orc_DefinitionArray,
                                             const std::vector<C_OscHalcConfigChannel> & orc_ConfigArray,
                                             const C_OscHalcDefDomain::E_VariableSelector & ore_Type,
                                             const std::string & orc_DomainSingularName,
                                             const bool oq_IsArray, const C_OscHalcDefBase::E_SafetyMode oe_SafetyMode,
                                             const bool oq_IsSafe);
   static std::string mh_GetDpListElementReference(
      const std::vector<C_OscHalcDefStruct> & orc_DefinitionArray,
      const C_OscHalcDefDomain::E_VariableSelector & ore_Type, const C_OscHalcDefElement & orc_Element,
      const uint32_t ou32_StructIndex, const uint32_t ou32_ElementIndex,
      const std::string & orc_DomainSingularName, const bool oq_IsArray, const bool oq_IsSafe);
   static bool mh_DropChannel(const bool oq_IsChannelSafe, const C_OscHalcDefBase::E_SafetyMode oe_SafetyMode,
                              const bool oq_IsSafeCase);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
