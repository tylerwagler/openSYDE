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

#include "stwtypes.hpp"
#include "C_SclResourceStrings.hpp"
#include <QString>

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
   // QString-based APIs (new)
   static bool h_CheckValidCeName(const QString & orc_Name,
                                  const bool oq_AutomaticCeStringAdaptation = false,
                                  const uint16_t ou16_MaxLength = 31U);
   static bool h_IsFloat64NearlyEqual(const float64_t & orf64_Float1, const float64_t & orf64_Float2);
   static bool h_IsFloat32NearlyEqual(const float32_t & orf32_Float1, const float32_t & orf32_Float2);
   static int32_t h_CreateFolderRecursively(const QString & orc_Folder);
   static QString h_NiceifyStringForFileName(const QString & orc_String);
   static QString h_NiceifyStringForCeComment(const QString & orc_String);
   static bool h_CheckValidFileName(const QString & orc_String);
   static bool h_CheckValidFilePath(const QString & orc_String);
   static bool h_IsScalingActive(const float64_t of64_Factor, const float64_t of64_Offset);
   static float64_t h_GetValueScaled(const float64_t of64_Value, const float64_t of64_Factor,
                                     const float64_t of64_Offset, const bool oq_AllowRangeAdaptation = true);
   static float64_t h_GetValueUnscaled(const float64_t of64_Value, const float64_t of64_Factor,
                                       const float64_t of64_Offset);
   static stw::scl::C_SclString h_PosSerialNumberToString(const uint8_t * const opu8_SerialNumber);
   static QString h_FsnSerialNumberToString(const uint8_t ou8_ManufacturerFormat,
                                            const QString & orc_RawSerialNumber);
   static void h_FileToString(const QString & orc_FilePath, QString & orc_OutputString);
   static void h_RangeCheckFloat(float64_t & orf64_Value);
   static stw::scl::C_SclString h_LoadString(const uint16_t ou16_StringIndex);
   static int32_t h_CopyFile(const QString & orc_SourceFile, const QString & orc_TargetFile,
                             QString * const opc_ErrorPath = NULL,
                             QString * const opc_ErrorMessage = NULL);
   static stw::scl::C_SclString h_GetCommandLineAsString(const int32_t os32_Argc, char_t * const * const oppcn_Argv);

   //Utilities for path place holder parsing
   static QString h_MakeIndependentOfDbProjectPath(const QString & orc_DbProjectPath,
                                                   const QString & orc_OsydeProjectPath,
                                                   const QString & orc_Path);
   static QString h_ResolvePlaceholderVariables(const QString & orc_Path,
                                                const QString & orc_OsydeProjectPath,
                                                const QString & orc_DbProjectPath = "");
   static QString h_ResolveProjIndependentPlaceholderVariables(const QString & orc_Path);
   static QString h_ConcatPathIfNecessary(const QString & orc_BaseDir,
                                          const QString & orc_RelativeOrAbsolutePath);

   // Tgl replacement helpers
   static QString h_IncludeTrailingDelimiter(const QString& orc_Path);
   static bool h_GetSystemUserName(QString& orc_UserName);
   static bool h_GetSystemMachineName(QString& orc_MachineName);
   static void h_HandleSystemMessages(void);
   static QString h_ChangeFileExtension(const QString& orc_FilePath, const QString& orc_NewExtension);

   //Unique string
   static QString h_GetUniqueName(const std::map<QString, bool> & orc_ExistingStrings,
                                  const QString & orc_ProposedName,
                                  const uint32_t ou32_MaxCharLimit,
                                  const QString & orc_SkipName = "");
   static void h_GetNumberAtStringEnd(const QString & orc_ProposedName,
                                      QString & orc_CutString, int32_t & ors32_Number);

   // Path variables
   static const QString hc_PATH_VARIABLE_OPENSYDE_BIN;
   static const QString hc_PATH_VARIABLE_OPENSYDE_PROJ;
   static const QString hc_PATH_VARIABLE_DATABLOCK_PROJ;
   static const QString hc_PATH_VARIABLE_USER_NAME;
   static const QString hc_PATH_VARIABLE_COMPUTER_NAME;

   // C_SclString-based APIs (deprecated) - only for functions with C_SclString parameters
   [[deprecated]] static bool h_CheckValidCeName(const stw::scl::C_SclString & orc_Name,
                                                 const bool oq_AutomaticCeStringAdaptation = false,
                                                 const uint16_t ou16_MaxLength = 31U) {
      return h_CheckValidCeName(orc_Name.ToQString(), oq_AutomaticCeStringAdaptation, ou16_MaxLength);
   }
   [[deprecated]] static int32_t h_CreateFolderRecursively(const stw::scl::C_SclString & orc_Folder) {
      return h_CreateFolderRecursively(orc_Folder.ToQString());
   }
   [[deprecated]] static stw::scl::C_SclString h_NiceifyStringForFileName(const stw::scl::C_SclString & orc_String) {
      return stw::scl::C_SclString::FromQString(h_NiceifyStringForFileName(orc_String.ToQString()));
   }
   [[deprecated]] static stw::scl::C_SclString h_NiceifyStringForCeComment(const stw::scl::C_SclString & orc_String) {
      return stw::scl::C_SclString::FromQString(h_NiceifyStringForCeComment(orc_String.ToQString()));
   }
   [[deprecated]] static bool h_CheckValidFileName(const stw::scl::C_SclString & orc_String) {
      return h_CheckValidFileName(orc_String.ToQString());
   }
   [[deprecated]] static bool h_CheckValidFilePath(const stw::scl::C_SclString & orc_String) {
      return h_CheckValidFilePath(orc_String.ToQString());
   }
   [[deprecated]] static stw::scl::C_SclString h_FsnSerialNumberToString(const uint8_t ou8_ManufacturerFormat,
                                                                         const stw::scl::C_SclString & orc_RawSerialNumber) {
      return stw::scl::C_SclString::FromQString(h_FsnSerialNumberToString(ou8_ManufacturerFormat, orc_RawSerialNumber.ToQString()));
   }
   [[deprecated]] static void h_FileToString(const stw::scl::C_SclString & orc_FilePath, stw::scl::C_SclString & orc_OutputString) {
      QString c_OutputString;
      h_FileToString(orc_FilePath.ToQString(), c_OutputString);
      orc_OutputString = stw::scl::C_SclString::FromQString(c_OutputString);
   }
   [[deprecated]] static int32_t h_CopyFile(const stw::scl::C_SclString & orc_SourceFile, const stw::scl::C_SclString & orc_TargetFile,
                                            stw::scl::C_SclString * const opc_ErrorPath = NULL,
                                            stw::scl::C_SclString * const opc_ErrorMessage = NULL) {
      QString c_ErrorPath, c_ErrorMessage;
      int32_t s32_Return = h_CopyFile(orc_SourceFile.ToQString(), orc_TargetFile.ToQString(),
                                      opc_ErrorPath ? &c_ErrorPath : NULL,
                                      opc_ErrorMessage ? &c_ErrorMessage : NULL);
      if (opc_ErrorPath) *opc_ErrorPath = stw::scl::C_SclString::FromQString(c_ErrorPath);
      if (opc_ErrorMessage) *opc_ErrorMessage = stw::scl::C_SclString::FromQString(c_ErrorMessage);
      return s32_Return;
   }

   //Utilities for path place holder parsing (deprecated)
   [[deprecated]] static stw::scl::C_SclString h_MakeIndependentOfDbProjectPath(const stw::scl::C_SclString & orc_DbProjectPath,
                                                                                const stw::scl::C_SclString & orc_OsydeProjectPath,
                                                                                const stw::scl::C_SclString & orc_Path) {
      return stw::scl::C_SclString::FromQString(h_MakeIndependentOfDbProjectPath(orc_DbProjectPath.ToQString(), orc_OsydeProjectPath.ToQString(), orc_Path.ToQString()));
   }
   [[deprecated]] static stw::scl::C_SclString h_ResolvePlaceholderVariables(const stw::scl::C_SclString & orc_Path,
                                                                             const stw::scl::C_SclString & orc_OsydeProjectPath,
                                                                             const stw::scl::C_SclString & orc_DbProjectPath = "") {
      return stw::scl::C_SclString::FromQString(h_ResolvePlaceholderVariables(orc_Path.ToQString(), orc_OsydeProjectPath.ToQString(), orc_DbProjectPath.ToQString()));
   }
   [[deprecated]] static stw::scl::C_SclString h_ResolveProjIndependentPlaceholderVariables(const stw::scl::C_SclString & orc_Path) {
      return stw::scl::C_SclString::FromQString(h_ResolveProjIndependentPlaceholderVariables(orc_Path.ToQString()));
   }
   [[deprecated]] static stw::scl::C_SclString h_ConcatPathIfNecessary(const stw::scl::C_SclString & orc_BaseDir,
                                                                       const stw::scl::C_SclString & orc_RelativeOrAbsolutePath) {
      return stw::scl::C_SclString::FromQString(h_ConcatPathIfNecessary(orc_BaseDir.ToQString(), orc_RelativeOrAbsolutePath.ToQString()));
   }

   // Tgl replacement helpers (deprecated)
   [[deprecated]] static stw::scl::C_SclString h_IncludeTrailingDelimiter(const stw::scl::C_SclString& orc_Path) {
      return stw::scl::C_SclString::FromQString(h_IncludeTrailingDelimiter(orc_Path.ToQString()));
   }
   [[deprecated]] static bool h_GetSystemUserName(stw::scl::C_SclString& orc_UserName) {
      QString c_UserName;
      bool q_Return = h_GetSystemUserName(c_UserName);
      orc_UserName = stw::scl::C_SclString::FromQString(c_UserName);
      return q_Return;
   }
   [[deprecated]] static bool h_GetSystemMachineName(stw::scl::C_SclString& orc_MachineName) {
      QString c_MachineName;
      bool q_Return = h_GetSystemMachineName(c_MachineName);
      orc_MachineName = stw::scl::C_SclString::FromQString(c_MachineName);
      return q_Return;
   }
   [[deprecated]] static stw::scl::C_SclString h_ChangeFileExtension(const stw::scl::C_SclString& orc_FilePath, const stw::scl::C_SclString& orc_NewExtension) {
      return stw::scl::C_SclString::FromQString(h_ChangeFileExtension(orc_FilePath.ToQString(), orc_NewExtension.ToQString()));
   }

   //Unique string (deprecated)
   [[deprecated]] static stw::scl::C_SclString h_GetUniqueName(const std::map<stw::scl::C_SclString, bool> & orc_ExistingStrings,
                                                               const stw::scl::C_SclString & orc_ProposedName,
                                                               const uint32_t ou32_MaxCharLimit,
                                                               const stw::scl::C_SclString & orc_SkipName = "") {
      std::map<QString, bool> c_ConvertedMap;
      for (const auto & rc_Entry : orc_ExistingStrings) {
         c_ConvertedMap[rc_Entry.first.ToQString()] = rc_Entry.second;
      }
      return stw::scl::C_SclString::FromQString(h_GetUniqueName(c_ConvertedMap, orc_ProposedName.ToQString(), ou32_MaxCharLimit, orc_SkipName.ToQString()));
   }
   [[deprecated]] static void h_GetNumberAtStringEnd(const stw::scl::C_SclString & orc_ProposedName,
                                                     stw::scl::C_SclString & orc_CutString, int32_t & ors32_Number) {
      QString c_CutString;
      h_GetNumberAtStringEnd(orc_ProposedName.ToQString(), c_CutString, ors32_Number);
      orc_CutString = stw::scl::C_SclString::FromQString(c_CutString);
   }

private:
   static const float64_t mhf64_EPSILON;
   static stw::scl::C_SCLResourceStrings mhc_ResourceStrings;

   static void mh_GetBaseNameAndCurrentConflictNumberFromString(const QString & orc_ConflictingValue,
                                                                const QString & orc_SkipName,
                                                                QString & orc_CutString,
                                                                int32_t & ors32_Number);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
