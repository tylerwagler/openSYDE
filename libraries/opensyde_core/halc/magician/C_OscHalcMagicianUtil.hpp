//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility for HALC magician related functionality
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCMAGICIANUTIL_HPP
#define C_OSCHALCMAGICIANUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcConfigDomain.hpp"
#include "C_OscNodeDataPoolListElement.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcMagicianUtil
{
public:
   C_OscHalcMagicianUtil(void);

   static std::string h_GetDatapoolName(const bool oq_IsSafe, const uint32_t ou32_CopyIndex = 0UL);
   static std::string h_GetDatapoolComment(const bool oq_IsSafe, const uint32_t ou32_CopyIndex = 0UL);
   static std::string h_GetListName(const C_OscHalcDefDomain::E_VariableSelector oe_Type);
   static int32_t h_GetVariableName(const std::vector<C_OscHalcDefStruct> & orc_DefinitionArray,
                                    const uint32_t ou32_ParameterIndexStruct, const uint32_t ou32_ParameterIndexElement,
                                    const std::string & orc_DomainSingularName,
                                    std::string & orc_Name, const uint32_t ou32_CurChannel = 0UL);
   static std::string h_CombineVariableName(const std::string & orc_Domain,
                                                      const std::string & orc_ParameterName);
   static C_OscNodeDataPoolListElement h_GetChanNumVariable(const std::string & orc_DomainSingularName,
                                                            const bool oq_IsSafe, const uint32_t ou32_NumChannels,
                                                            const bool oq_AddDataset);
   static std::string h_GetChanNumVariableName(const std::string & orc_DomainSingularName);
   static C_OscNodeDataPoolListElement h_GetUseCaseVariable(const std::string & orc_DomainSingularName,
                                                            const bool oq_IsSafe, const uint32_t ou32_NumChannels,
                                                            const bool oq_AddDataset);
   static std::string h_GetUseCaseVariableName(const std::string & orc_DomainSingularName);
   static C_OscNodeDataPoolListElement h_GetSafetyFlagVariable(const std::string & orc_DomainSingularName,
                                                               const bool oq_IsSafe, const uint32_t ou32_NumChannels,
                                                               const bool oq_AddDataset);
   static std::string h_GetSafetyFlagVariableName(const std::string & orc_DomainSingularName);
   static void h_SetCommonDpElementDefaults(C_OscNodeDataPoolListElement & orc_Element);

private:
   static std::string mh_GetElementName(const C_OscHalcDefElement & orc_Param,
                                                  const std::string & orc_Domain,
                                                  const uint32_t ou32_CurChannel);
   static void mh_HandleGenericType(C_OscNodeDataPoolListElement & orc_Element, const uint32_t ou32_NumChannels,
                                    const bool oq_AddDataset, const bool oq_UseU16);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
