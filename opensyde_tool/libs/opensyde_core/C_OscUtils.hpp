//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE utility functions (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef OSCUTILS_HPP
#define OSCUTILS_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <map>

#include "C_SclResourceStrings.hpp"
#include "stwtypes.hpp"
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */
/// openSYDE utility functions
class C_OscUtils {
public:
  // QString-based APIs (new)
  static bool
  h_CheckValidCeName(const QString &orc_Name,
                     const bool oq_AutomaticCeStringAdaptation = false,
                     const uint16_t ou16_MaxLength = 31U);
  static bool h_IsFloat64NearlyEqual(const float64_t &orf64_Float1,
                                     const float64_t &orf64_Float2);
  static bool h_IsFloat32NearlyEqual(const float32_t &orf32_Float1,
                                     const float32_t &orf32_Float2);
  static int32_t h_CreateFolderRecursively(const QString &orc_Folder);
  static QString h_NiceifyStringForFileName(const QString &orc_String);
  static QString h_NiceifyStringForCeComment(const QString &orc_String);
  static bool h_CheckValidFileName(const QString &orc_String);
  static bool h_CheckValidFilePath(const QString &orc_String);
  static bool h_IsScalingActive(const float64_t of64_Factor,
                                const float64_t of64_Offset);
  static float64_t h_GetValueScaled(const float64_t of64_Value,
                                    const float64_t of64_Factor,
                                    const float64_t of64_Offset,
                                    const bool oq_AllowRangeAdaptation = true);
  static float64_t h_GetValueUnscaled(const float64_t of64_Value,
                                      const float64_t of64_Factor,
                                      const float64_t of64_Offset);
  static QString
  h_PosSerialNumberToString(const uint8_t *const opu8_SerialNumber);
  static QString h_FsnSerialNumberToString(const uint8_t ou8_ManufacturerFormat,
                                           const QString &orc_RawSerialNumber);
  static void h_FileToString(const QString &orc_FilePath,
                             QString &orc_OutputString);
  static void h_RangeCheckFloat(float64_t &orf64_Value);
  static QString h_LoadString(const uint16_t ou16_StringIndex);
  static int32_t h_CopyFile(const QString &orc_SourceFile,
                            const QString &orc_TargetFile,
                            QString *const opc_ErrorPath = NULL,
                            QString *const opc_ErrorMessage = NULL);
  static QString h_GetCommandLineAsString(const int32_t os32_Argc,
                                          char_t *const *const oppcn_Argv);

  // Utilities for path place holder parsing
  static QString
  h_MakeIndependentOfDbProjectPath(const QString &orc_DbProjectPath,
                                   const QString &orc_OsydeProjectPath,
                                   const QString &orc_Path);
  static QString
  h_ResolvePlaceholderVariables(const QString &orc_Path,
                                const QString &orc_OsydeProjectPath,
                                const QString &orc_DbProjectPath = "");
  static QString
  h_ResolveProjIndependentPlaceholderVariables(const QString &orc_Path);
  static QString
  h_ConcatPathIfNecessary(const QString &orc_BaseDir,
                          const QString &orc_RelativeOrAbsolutePath);

  // Tgl replacement helpers
  static QString h_IncludeTrailingDelimiter(const QString &orc_Path);
  static bool h_GetSystemUserName(QString &orc_UserName);
  static bool h_GetSystemMachineName(QString &orc_MachineName);
  static void h_HandleSystemMessages(void);
  static QString h_ChangeFileExtension(const QString &orc_FilePath,
                                       const QString &orc_NewExtension);

  // Unique string
  static QString
  h_GetUniqueName(const std::map<QString, bool> &orc_ExistingStrings,
                  const QString &orc_ProposedName,
                  const uint32_t ou32_MaxCharLimit,
                  const QString &orc_SkipName = "");
  static void h_GetNumberAtStringEnd(const QString &orc_ProposedName,
                                     QString &orc_CutString,
                                     int32_t &ors32_Number);

  // QString helper functions (replacing QString static methods)
  static QString h_IntToHex(const int64_t os64_Value, const uint32_t ou32_Digits);
  static QString h_IntToHex(const uint64_t ou64_Value, const uint32_t ou32_Digits);
  template <typename T>
  static QString h_IntToHex(const T orc_Value, const uint32_t ou32_Digits)
  {
     return h_IntToHex(static_cast<int64_t>(orc_Value), ou32_Digits);
  }

  // Path variables
  static const QString hc_PATH_VARIABLE_OPENSYDE_BIN;
  static const QString hc_PATH_VARIABLE_OPENSYDE_PROJ;
  static const QString hc_PATH_VARIABLE_DATABLOCK_PROJ;
  static const QString hc_PATH_VARIABLE_USER_NAME;
  static const QString hc_PATH_VARIABLE_COMPUTER_NAME;

private:
  static const float64_t mhf64_EPSILON;
  static stw::scl::C_SCLResourceStrings mhc_ResourceStrings;

  static void mh_GetBaseNameAndCurrentConflictNumberFromString(
      const QString &orc_ConflictingValue, const QString &orc_SkipName,
      QString &orc_CutString, int32_t &ors32_Number);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
