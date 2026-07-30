//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent application class

   For details see documentation in .hpp file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdio>
#include <string>
#include <getopt.h> //for command line parsing; available in GCC-style toolchains

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_CaApplication.hpp"
#include "TglFile.hpp"
#include "version_config.hpp"
#include "C_SclString.hpp"
#include "C_OscUtils.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscConfFileHandler.hpp"
#include "C_OscUtilBinaryHash.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::osy_crypto_agent;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_CaApplication::C_CaApplication() :
   mc_ConfigFilePath("./osy_crypto_agent.conf")
{
}

//----------------------------------------------------------------------------------------------------------------------
C_CaApplication::~C_CaApplication()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize logging engine.

   Log file location is an optional parameter, so this is a part of initialization of optional parameters!
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaApplication::m_InitLogging(void)
{
   std::string c_LogPath;

   // log path (gets created, so just check for non empty):
   if (c_LogPath == "")
   {
      c_LogPath = mc_AgentServer.c_Settings.c_LogFileFolder;
   }

   // add trailing path delimiter in case there is none
   c_LogPath = TglFileIncludeTrailingDelimiter(c_LogPath).c_str();
   mc_LogFile = c_LogPath + "osy_crypto_agent.log";

   // if the log file already exists try to delete it; we do not want to fill up the file system
   // just intended for local logging
   remove(mc_LogFile.c_str());

   // log to file
   C_OscLoggingHandler::h_SetCompleteLogFileLocation(mc_LogFile);
   std::cout << "The following output also is logged to the file: " << mc_LogFile.c_str() << &std::endl;
   C_OscLoggingHandler::h_SetWriteToConsoleActive(true);

   if (mc_AgentServer.c_Settings.e_StdOutVerbosityLevel ==
       C_CaServerLogic::C_CaReferenceConfFile::eSTDOUT_VERBOSITY_FULL)
   {
      //log everything to console
      C_OscLoggingHandler::h_SetConsoleMinLogType(C_OscLoggingHandler::eLOG_TYPE_INFO);
   }
   else
   {
      //only log errors to console
      C_OscLoggingHandler::h_SetConsoleMinLogType(C_OscLoggingHandler::eLOG_TYPE_ERROR);
   }
   C_OscLoggingHandler::h_SetWriteToFileActive(true, true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get options from command line

   Parse command line parameters.
   We only support help and path to a config file.
   The actual configuration for the crypto agent is expected to be in the config file.

   \param[in]   os32_Argc     number of command line arguments
   \param[in]   oppcn_Argv    command line arguments

   \return
   eOK                    init OK
   eERR_PARSE_PARAMETERS  invalid or inconsistent parameters; help requested and displayed (no need to continue)
*/
//----------------------------------------------------------------------------------------------------------------------
C_CaApplication::E_ExecutionResult C_CaApplication::ParseCommandLine(const int32_t os32_Argc,
                                                                     char_t * const * const oppcn_Argv)
{
   int32_t s32_Result;
   bool q_ConfigFileSpecified = false;
   E_ExecutionResult e_Result = eOK;

   const struct option ac_OPTIONS[] =
   {
      {
         "help",                 no_argument,       NULL, 'h'
      },
      {
         "configfile",           required_argument, NULL, 'c'
      },
      {
         NULL, 0, NULL, 0
      }
   };

   do
   {
      int32_t s32_Index;
      s32_Result = getopt_long(os32_Argc, oppcn_Argv, "hc:", &ac_OPTIONS[0], &s32_Index);
      if (s32_Result != -1)
      {
         switch (s32_Result)
         {
         case 'h':
            this->m_PrintHelp();
            e_Result = eERR_PARSE_PARAMETERS;
            break;
         case 'c':
            mc_ConfigFilePath = optarg;
            q_ConfigFileSpecified = true;
            break;
         default:
            ReportText("Invalid parameter specified. Use -h or --help for usage information.");
            e_Result = eERR_PARSE_PARAMETERS;
            break;
         }
      }
      else
      {
         //-1 = finished parsing
      }
   }
   while (s32_Result != -1);

   if (e_Result == eOK)
   {
      const C_SclString c_Date = __DATE__;
      const C_SclString c_Time = __TIME__;
      const int32_t s32_Return = mc_AgentServer.c_Settings.LoadSettings(mc_ConfigFilePath.c_str());

      if (mc_AgentServer.c_Settings.e_StdOutVerbosityLevel ==
          C_CaServerLogic::C_CaReferenceConfFile::eSTDOUT_VERBOSITY_FULL)
      {
         //print ASCII art banner to console
         //http://patorjk.com/software/taag/#p=display&f=Slant&t=openSYDE+Crypto+Agent
         ReportText(
            "                         _______  ______  ______   ______                 __           ___                    __");
         ReportText(
            "  ____  ____  ___  ____ / ___/\\ \\/ / __ \\/ ____/  / ____/______  ______  / /_____     /   | ____ ____  ____  / /_");
         ReportText(
            " / __ \\/ __ \\/ _ \\/ __ \\\\__ \\  \\  / / / / __/    / /   / ___/ / / / __ \\/ __/ __ \\   / /| |/ __ `/ _ \\/ __ \\/ __/");
         ReportText(
            "/ /_/ / /_/ /  __/ / / /__/ /  / / /_/ / /___   / /___/ /  / /_/ / /_/ / /_/ /_/ /  / ___ / /_/ /  __/ / / / /_");
         ReportText(
            "\\____/ .___/\\___/_/ /_/____/  /_/_____/_____/   \\____/_/   \\__, / .___/\\__/\\____/  /_/  |_\\__, /\\___/_/ /_/\\__/");
         ReportText("    /_/                                                   /____/_/                       /____/");
      }

      //initialize logging as early as possible, to have the details in log file in case of problems
      this->m_InitLogging();

      //write basic info to console and log file:
      osc_write_log_info("Crypto Agent version",
                         "Version: " + C_CaApplication::h_GetApplicationVersion() +  "  MD5-Hash: " +
                         C_OscUtilBinaryHash::h_CreateBinaryHash());
      osc_write_log_info("Crypto Agent version", "Binary: " + TglGetExePath());
      osc_write_log_info("Crypto Agent version", "Build date: " + c_Date + " " + c_Time);
      osc_write_log_info("Application startup",
                         "Using TCP server port: " + C_SclString::IntToStr(mc_AgentServer.c_Settings.u16_Port));
      osc_write_log_info("Application startup",
                         "Binding server port to IP: " +
                         C_OscUtils::h_Ip4ToString(mc_AgentServer.c_Settings.au8_BindAddress));

      if (s32_Return != C_NO_ERR)
      {
         osc_write_log_warning("Application startup",
                               "Could not load configuration file: \"" + mc_ConfigFilePath +
                               "\". Working with default settings.");
         if (q_ConfigFileSpecified == true)
         {
            //the caller explicitly specified a config file which cannot be loaded; this is considered critical
            osc_write_log_error("Application startup",
                                "Configuration file was explicitly specified but could not be loaded. Aborting.");
            e_Result = eERR_PARSE_PARAMETERS;
         }
      }
   }
   return e_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   report text to user

   \param[in]     orc_Text     text to report
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaApplication::ReportText(const std::string & orc_Text) const
{
   (std::cout << orc_Text.c_str()) << "\n";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   print usage information for user
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CaApplication::m_PrintHelp()
{
   ReportText(
      "This tool \"openSYDE Crypto Agent\" is part of the openSYDE tool chain by STW (Sensor-Technik Wiedemann GmbH).");

   ReportText("Command line parameters:");
   ReportText("Parameter             Description                                      Default                  Example");
   ReportText(
      "==================================================================================================================================");
   ReportText("-h     --help         Print help                                                                -h");
   ReportText(
      "-c     --configfile   Path to configuration file with settings         ./osy_crypto_agent.conf  -c ../config/osy_crypto_agent.conf");
   ReportText("");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get application version number as an C_SclString

   The version number is defined in version.hpp, which will be included from resources.rc.
   Format: "Vx.yyrz".

   \param[in]   orc_FileName    file name to get version from

   \return
   string with version information ("V?.??r?" on error)
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_CaApplication::h_GetApplicationVersion(void)
{
   C_SclString c_Version;

   c_Version.StringPrintFormatted("V%d.%02dr%d", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_RELEASE);

   return c_Version.c_str();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Execute crypto agent operation

   * parse command line

   * initialize server logic
   * start server to handle incoming requests
   * finish if:
   ** stop service received by server
   ** being killed using system mechanisms (e.g. sending kill signal, closing console, etc.)

   \param[in]   os32_Argc     number of command line arguments
   \param[in]   oppcn_Argv    command line arguments

   \return
   eOK                           operation started and finished without problems
   eERR_*                        see comments at those constants details will be in log
*/
//----------------------------------------------------------------------------------------------------------------------
C_CaApplication::E_ExecutionResult C_CaApplication::Execute(const int32_t os32_Argc, char_t * const * const oppcn_Argv)
{
   int32_t s32_Return;
   E_ExecutionResult e_Result;

   //print our own name
   this->ReportText("openSYDE Crypto Agent " + C_CaApplication::h_GetApplicationVersion());
   e_Result = this->ParseCommandLine(os32_Argc, oppcn_Argv);

   if (e_Result == C_CaApplication::eOK)
   {
      s32_Return = mc_AgentServer.Initialize();
      if (s32_Return != C_NO_ERR)
      {
         osc_write_log_error("Initialization failed",
                             "Crypto agent server initialization failed. Check log for details.");
         e_Result = eERR_INIT_FAILED_AGENT_LOGIC;
      }
      else
      {
         s32_Return = mc_AgentServer.Execute(this->mc_AgentServer.c_Settings.u16_Port,
                                             this->mc_AgentServer.c_Settings.au8_BindAddress);
         switch (s32_Return)
         {
         case C_NO_ERR:
            e_Result = eOK;
            break;
         case C_WARN:
            e_Result = eERR_OPERATION_FAILED;
            break;
         case C_COM:
            e_Result = eERR_INIT_FAILED_TCP;
            break;
         case C_NOACT:
            e_Result = eERR_INIT_FAILED_LISTENER;
            break;
         default:
            e_Result = eERR_UNKNOWN;
            break;
         }
      }
   }

   return e_Result;
}
