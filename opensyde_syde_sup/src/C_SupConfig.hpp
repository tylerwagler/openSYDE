//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       SYDEsup configuration file handler (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SUPCONFIG_HPP
#define C_SUPCONFIG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>
#include <system_error>
#include "C_OscConfFileHandler.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SupConfig :
   public stw::opensyde_core::C_OscConfFileHandler
{
public:
   C_SupConfig(void);

   // General
   bool q_Quiet;                          ///< quiet mode flag
   std::string c_LogPath;                 ///< path to log file directory
   std::string c_OperationMode;           ///< operation mode: "update" or "createpackage"
   std::string c_SupFilePath;             ///< path to service update package file

   // Package Creation
   std::string c_ProjectPath;             ///< path to openSYDE project file
   std::string c_DeviceDefinitionPath;    ///< path to device definitions file
   std::string c_UpdateViewName;          ///< view name for package creation

   // Update
   bool q_NecessaryFiles;                 ///< only transfer necessary files flag
   std::string c_CanDriver;               ///< path to CAN interface DLL
   std::string c_UnzipPath;               ///< path to temporary unzip directory

   // Secure Update
   std::string c_PublicKeyPath;           ///< path to PEM file with public key
   std::string c_Password;                ///< password for encrypted update package

protected:
   std::error_code m_LoadSettings(const std::vector<std::string> & orc_SettingsWithoutComments) override;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
