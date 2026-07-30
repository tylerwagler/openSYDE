//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent application class

   Main application engine.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAAPPLICATIONHPP
#define C_CAAPPLICATIONHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include "stwtypes.hpp"

#include "C_CaServerLogic.hpp"
#include "C_OscConfFileHandler.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw::osy_crypto_agent
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CaApplication
{
private:
protected:
   std::string mc_LogFile;
   std::string mc_ConfigFilePath;

   C_CaServerLogic mc_AgentServer;

   void m_InitLogging();
   void m_PrintHelp();

public:
   enum E_ExecutionResult
   {
      eOK                                 = 0,  // finished without problems (shutdown via system signal or stop
                                                // service)
      eERR_PARSE_PARAMETERS               = 10, // failed to parse command line parameters or config file; help text
                                                // requested
      eERR_INIT_FAILED_AGENT_LOGIC        = 20, // failed to initialize agent logic (e.g. loading certificates, etc.)
      eERR_INIT_FAILED_TCP                = 30, // failed to initialize TCP server (e.g. bind/listen failed)
      eERR_INIT_FAILED_LISTENER           = 40, // failed to initialize listener thread
      eERR_OPERATION_FAILED               = 50, // failed during operation (e.g. unexpected socket error during
                                                // operation, etc.)
      eERR_UNKNOWN                        = 60
   };

   C_CaApplication();
   virtual ~C_CaApplication();

   void ReportText(const std::string & orc_Text) const;

   E_ExecutionResult ParseCommandLine(const int32_t os32_Argc, char_t * const * const oppcn_Argv);

   static std::string h_GetApplicationVersion();

   E_ExecutionResult Execute(const int32_t os32_Argc, char_t * const * const oppcn_Argv);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}

#endif
