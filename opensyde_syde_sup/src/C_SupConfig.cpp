//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       SYDEsup configuration file handler (implementation)

   Loads SYDEsup settings from the sydesup.conf configuration file.
   Inherits from C_OscConfFileHandler which handles the generic file loading and
   comment stripping. This class provides the concrete parsing of all SYDEsup settings.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SclStringUtil.hpp"
#include "C_SupConfig.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
/// Case-insensitive key lookup returning the trimmed value, or the default if absent.
std::string mh_GetValue(const std::vector<std::string> & orc_List, const std::string & orc_Key,
                        const std::string & orc_Default = "")
{
   const int32_t s32_Index = ListIndexOfName(orc_List, orc_Key);
   if (s32_Index >= 0)
   {
      const std::string & rc_Line = orc_List[static_cast<uint32_t>(s32_Index)];
      const std::size_t u32_Pos = rc_Line.find('=');
      if (u32_Pos != std::string::npos)
      {
         return TrimCompat(rc_Line.substr(u32_Pos + 1U));
      }
   }
   return orc_Default;
}

std::string mh_ToLower(const std::string & orc_Text)
{
   std::string c_Result = orc_Text;
   for (std::string::iterator c_It = c_Result.begin(); c_It != c_Result.end(); ++c_It)
   {
      *c_It = static_cast<char>(std::tolower(static_cast<unsigned char>(*c_It)));
   }
   return c_Result;
}
}

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Initializes all settings with their default values.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SupConfig::C_SupConfig(void) :
   C_OscConfFileHandler(),
   q_Quiet(false),
   c_LogPath(""),
   c_OperationMode(""),
   c_SupFilePath(""),
   c_ProjectPath(""),
   c_DeviceDefinitionPath(""),
   c_UpdateViewName(""),
   q_NecessaryFiles(false),
   c_CanDriver(""),
   c_UnzipPath(""),
   c_PublicKeyPath(""),
   c_Password("")
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load concrete settings from pre-parsed string list

   Parses all SYDEsup settings from the provided string list (comments and blank lines already removed).
   Boolean values are read as "true"/"false" strings (case insensitive).

   \param[in]  orc_SettingsWithoutComments   string list containing only key=value lines (no comments, no blank lines)

   \return
   Errc::success   all settings parsed successfully
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_SupConfig::m_LoadSettings(const std::vector<std::string> & orc_SettingsWithoutComments)
{
   // General
   q_Quiet          = (mh_ToLower(mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_SETTING_QUIET")) == "true");
   c_LogPath        = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_LOG_DIR");
   c_OperationMode  = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_SETTING_OPERATION_MODE");
   c_SupFilePath    = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_PACKAGE_FILE");

   // Package Creation
   c_ProjectPath          = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_PROJECT");
   c_DeviceDefinitionPath = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_DEVICE_DEFINITION");
   c_UpdateViewName       = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_SETTING_UPDATE_VIEW_NAME");

   // Update
   q_NecessaryFiles = (mh_ToLower(mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_SETTING_NECESSARY_FILES")) == "true");
   c_CanDriver      = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_CAN_INTERFACE");
   c_UnzipPath      = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_UNZIP_DIR");

   // Secure Update
   c_PublicKeyPath = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PATH_PUBLIC_KEY");
   c_Password      = mh_GetValue(orc_SettingsWithoutComments, "OSY_SYDESUP_PASSWORD");

   return stw::errors::make_error_code(Errc::success);
}
