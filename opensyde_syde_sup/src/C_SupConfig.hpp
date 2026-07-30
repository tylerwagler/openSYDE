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
#include "stwtypes.hpp"
#include "C_SclString.hpp"
#include "C_OscConfFileHandler.hpp"
#include "C_OscCryptoAgentSettings.hpp"

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
   stw::scl::C_SclString c_LogPath;       ///< path to log file directory
   stw::scl::C_SclString c_OperationMode; ///< operation mode: "update" or "createpackage"
   stw::scl::C_SclString c_SupFilePath;   ///< path to service update package file

   // Package Creation
   stw::scl::C_SclString c_ProjectPath;          ///< path to openSYDE project file
   stw::scl::C_SclString c_DeviceDefinitionPath; ///< path to device definitions file
   stw::scl::C_SclString c_UpdateViewName;       ///< view name for package creation

   // Update
   bool q_NecessaryFiles;             ///< only transfer necessary files flag
   stw::scl::C_SclString c_CanDriver; ///< path to CAN interface DLL
   stw::scl::C_SclString c_UnzipPath; ///< path to temporary unzip directory

   // Secure Update
   stw::scl::C_SclString c_PublicKeyPath; ///< path to PEM file with public key
   stw::scl::C_SclString c_Password;      ///< password for encrypted update package

   // Crypto Agent
   stw::opensyde_core::C_OscCryptoAgentSettings c_CryptoAgentSettings; // crypto agent settings

protected:
   virtual int32_t m_LoadSettings(const stw::scl::C_SclStringList & orc_SettingsWithoutComments);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
