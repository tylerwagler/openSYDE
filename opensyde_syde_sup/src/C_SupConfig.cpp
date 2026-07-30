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

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_SclString.hpp"
#include "C_OscUtils.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SupConfig.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

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
   c_Password(""),
   c_CryptoAgentSettings()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load concrete settings from pre-parsed string list

   Parses all SYDEsup settings from the provided string list (comments and blank lines already removed).
   Boolean values are read as "true"/"false" strings (case sensitive).
   Integer values are read using ToInt().

   \param[in]  orc_SettingsWithoutComments   string list containing only key=value lines (no comments, no blank lines)

   \return
   C_NO_ERR   all settings parsed successfully
   C_CONFIG   IP address or port could not be parsed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SupConfig::m_LoadSettings(const C_SclStringList & orc_SettingsWithoutComments)
{
   int32_t s32_Result;

   // General
   q_Quiet          = (orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_QUIET").LowerCase() == "true");
   c_LogPath        = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_LOG_DIR");
   c_OperationMode  = orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_OPERATION_MODE");
   c_SupFilePath    = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_PACKAGE_FILE");

   // Package Creation
   c_ProjectPath          = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_PROJECT");
   c_DeviceDefinitionPath = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_DEVICE_DEFINITION");
   c_UpdateViewName       = orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_UPDATE_VIEW_NAME");

   // Update
   q_NecessaryFiles = (orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_NECESSARY_FILES").LowerCase() == "true");
   c_CanDriver      = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_CAN_INTERFACE");
   c_UnzipPath      = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_UNZIP_DIR");

   // Secure Update
   c_PublicKeyPath = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_PUBLIC_KEY");
   c_Password      = orc_SettingsWithoutComments.Values("OSY_SYDESUP_PASSWORD");

   // Crypto Agent
   c_CryptoAgentSettings.c_CryptoAgentExecutablePath =
      orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_CRYPTO_AGENT_EXECUTABLE");
   c_CryptoAgentSettings.c_CryptoAgentConfigFilePath =
      orc_SettingsWithoutComments.Values("OSY_SYDESUP_PATH_CRYPTO_AGENT_CONFIG_FILE");
   c_CryptoAgentSettings.q_CryptoAgentAutoStart =
      (orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_CRYPTO_AGENT_AUTO_START").LowerCase() == "true");
   c_CryptoAgentSettings.q_CryptoAgentAutoStop  =
      (orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_CRYPTO_AGENT_AUTO_STOP").LowerCase() == "true");

   const C_SclString c_BindAddress = orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_CRYPTO_AGENT_IP");

   c_CryptoAgentSettings.au8_CryptoAgentIp[0] = 127U;
   c_CryptoAgentSettings.au8_CryptoAgentIp[1] = 0U;
   c_CryptoAgentSettings.au8_CryptoAgentIp[2] = 0U;
   c_CryptoAgentSettings.au8_CryptoAgentIp[3] = 1U;

   s32_Result = C_OscUtils::h_StringToIp4(c_BindAddress, c_CryptoAgentSettings.au8_CryptoAgentIp);

   c_CryptoAgentSettings.u16_CryptoAgentPort = 50963U; // default port
   if (s32_Result == C_NO_ERR)
   {
      try
      {
         c_CryptoAgentSettings.u16_CryptoAgentPort =
            static_cast<uint16_t>(orc_SettingsWithoutComments.Values("OSY_SYDESUP_SETTING_CRYPTO_AGENT_PORT").ToInt());
      }
      catch (...)
      {
         s32_Result = C_CONFIG;
      }
   }

   return s32_Result;
}
