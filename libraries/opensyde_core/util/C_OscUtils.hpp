//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE utility functions (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef OSCUTILS_HPP
#define OSCUTILS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <map>
#include <system_error>

#include <cstdint>
#include <string>
#include "C_OscErrorCategory.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///openSYDE utility functions
class C_OscUtils
{
public:
   static bool h_CheckValidCeName(const std::string & orc_Name,
                                  const bool oq_AutomaticCeStringAdaptation = false,
                                  const uint16_t ou16_MaxLength = 31U);
   static bool h_IsFloat64NearlyEqual(const double & orf64_Float1, const double & orf64_Float2);
   static bool h_IsFloat32NearlyEqual(const float & orf32_Float1, const float & orf32_Float2);
   static std::error_code h_CreateFolderRecursively(const std::string & orc_Folder);
   static std::string h_NiceifyStringForFileName(const std::string & orc_String);
   static std::string h_NiceifyStringForCeComment(const std::string & orc_String);
   static bool h_CheckValidFileName(const std::string & orc_String);
   static bool h_CheckValidFilePath(const std::string & orc_String);
   static bool h_IsScalingActive(const double of64_Factor, const double of64_Offset);
   static double h_GetValueScaled(const double of64_Value, const double of64_Factor,
                                     const double of64_Offset, const bool oq_AllowRangeAdaptation = true);
   static double h_GetValueUnscaled(const double of64_Value, const double of64_Factor,
                                       const double of64_Offset);
   static std::string h_PosSerialNumberToString(const uint8_t * const opu8_SerialNumber);
   static std::string h_FsnSerialNumberToString(const uint8_t ou8_ManufacturerFormat,
                                                          const std::string & orc_RawSerialNumber);
   static void h_FileToString(const std::string & orc_FilePath, std::string & orc_OutputString);
   static void h_RangeCheckFloat(double & orf64_Value);

   static std::error_code h_CopyFile(const std::string & orc_SourceFile, const std::string & orc_TargetFile,
                                     std::string * const opc_ErrorPath = nullptr,
                                     std::string * const opc_ErrorMessage = nullptr);
   static std::string h_GetCommandLineAsString(const int32_t os32_Argc, char * const * const oppcn_Argv);

   //Utilities for path place holder parsing
   static std::string h_MakeIndependentOfDbProjectPath(const std::string & orc_DbProjectPath,
                                                                 const std::string & orc_OsydeProjectPath,
                                                                 const std::string & orc_Path);
   static std::string h_ResolvePlaceholderVariables(const std::string & orc_Path,
                                                              const std::string & orc_OsydeProjectPath,
                                                              const std::string & orc_DbProjectPath = "");
   static std::string h_ResolveProjIndependentPlaceholderVariables(const std::string & orc_Path);
   static std::string h_ConcatPathIfNecessary(const std::string & orc_BaseDir,
                                                        const std::string & orc_RelativeOrAbsolutePath);

   //Unique string
   static std::string h_GetUniqueName(const std::map<std::string, bool> & orc_ExistingStrings,
                                                const std::string & orc_ProposedName,
                                                const uint32_t ou32_MaxCharLimit,
                                                const std::string & orc_SkipName = "");
   static void h_GetNumberAtStringEnd(const std::string & orc_ProposedName,
                                      std::string & orc_CutString, int32_t & ors32_Number);

   // Path variables
   static const std::string hc_PATH_VARIABLE_OPENSYDE_BIN;
   static const std::string hc_PATH_VARIABLE_OPENSYDE_PROJ;
   static const std::string hc_PATH_VARIABLE_DATABLOCK_PROJ;
   static const std::string hc_PATH_VARIABLE_USER_NAME;
   static const std::string hc_PATH_VARIABLE_COMPUTER_NAME;

private:
   static const double mhf64_EPSILON;


   static void mh_GetBaseNameAndCurrentConflictNumberFromString(const std::string & orc_ConflictingValue,
                                                                const std::string & orc_SkipName,
                                                                std::string & orc_CutString,
                                                                int32_t & ors32_Number);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
