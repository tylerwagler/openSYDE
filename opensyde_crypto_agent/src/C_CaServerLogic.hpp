//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class reference logic implementation

   Provides reference/standard/example implementation of the logic.
   To provide your own implementations reimplement this class.

   This implementation uses a local collection of .pem files for the private keys.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CASERVERLOGICHPP
#define C_CASERVERLOGICHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>

#include "stwtypes.hpp"
#include "C_SclStringList.hpp"
#include "C_OscSecurityPemDatabase.hpp"
#include "C_OscConfFileHandler.hpp"
#include "C_CaServerPlatform.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw::osy_crypto_agent
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CaServerLogic :
   public C_CaServerPlatform
{
private:
   uint32_t mu32_PemFolderStateHash;
   bool m_PemFolderReloadNeeded();

protected:
   stw::opensyde_core::C_OscSecurityPemDatabase mc_PemDb;

   virtual int32_t m_ServiceHandlerSolveAuthChallenge(const C_InputParametersSolveAuthChallenge & orc_Parameters,
                                                      std::vector<uint8_t> & orc_ResponseValue,
                                                      std::string & orc_ErrorDetail);

public:
   // crypto agent configuration file handler
   class C_CaReferenceConfFile :
      public stw::opensyde_core::C_OscConfFileHandler
   {
   protected:
      virtual int32_t m_LoadSettings(const stw::scl::C_SclStringList & orc_SettingsWithoutComments);

   public:
      enum E_StdOutVerbosityLevel
      {
         eSTDOUT_VERBOSITY_FULL = 0,
         eSTDOUT_VERBOSITY_LOW
      };

      C_CaReferenceConfFile();
      virtual ~C_CaReferenceConfFile();

      uint16_t u16_Port;           //port to run server on
      uint8_t au8_BindAddress[4];  //bind address for server; e.g. 0.0.0.0 for any address; 127.0.0.1 for localhost
      std::string c_LogFileFolder; //folder to write log files to
      E_StdOutVerbosityLevel e_StdOutVerbosityLevel;

      std::string c_PemFileFolder; //folder to parse .pem files from specific to the reference implementation
   };

   C_CaServerLogic();
   virtual ~C_CaServerLogic();

   int32_t Initialize();

   C_CaReferenceConfFile c_Settings;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}

#endif
