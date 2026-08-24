//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class for C code export.
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTUTI_HPP
#define C_OSCEXPORTUTI_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include "C_SclStringList.hpp"
#include "C_OscNodeDataPool.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscExportUti
{
public:
   static std::string h_GetSectionSeparator(const std::string & orc_SectionName);
   static std::string h_GetHeaderSeparator(void);
   static std::string h_GetCreationToolInfo(const std::string & orc_ExportToolInfo);
   static void h_AddExternCeStart(stw::scl::C_SclStringList & orc_Data);
   static void h_AddExternCeEnd(stw::scl::C_SclStringList & orc_Data);
   static void h_AddProjectIdDef(stw::scl::C_SclStringList & orc_Data, const std::string & orc_MagicName,
                                 const bool oq_HeaderFile);
   static void h_AddProjIdFunctionPrototype(stw::scl::C_SclStringList & orc_Data,
                                            const std::string & orc_MagicName);
   static int32_t h_SaveToFile(stw::scl::C_SclStringList & orc_Data, const std::string & orc_Path,
                               const std::string & orc_FileName, const bool oq_HeaderFile);
   static void h_CollectFilePaths(std::vector<std::string> & orc_FilePaths,
                                  const std::string & orc_Path, const std::string & orc_FileName,
                                  const bool oq_SourceCode);

   static std::string h_GetTypePrefix(const C_OscNodeDataPoolContent::E_Type oe_Type, const bool oq_IsArray);
   static std::string h_GetElementTypeAsString(const C_OscNodeDataPoolContent::E_Type oe_Type);
   static std::string h_GetElementCeName(const std::string & orc_Name, const bool oq_IsArray,
                                                   const C_OscNodeDataPoolContent::E_Type oe_Type,
                                                   const std::string & orc_ArrayPos = "0");

   static std::string h_FloatToStrGe(const float32_t of32_Value,  bool * const opq_InfOrNan = NULL);
   static std::string h_FloatToStrGe(const float64_t of64_Value, bool * const opq_InfOrNan = NULL);
   static bool h_CheckInfOrNan(const std::string & orc_String);
   static void h_AddDecimalPointIfNone(std::string & orc_FloatString);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
