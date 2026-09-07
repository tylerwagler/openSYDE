//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class of openSYDE simple command line flash tool

   This contains the main functionality of the openSYDE command line flash tool.
   See about.txt and build.txt for detailed descriptions upon functionality and usage.

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <getopt.h> //note: as we use getopt.h this application is not portable to all compilers
#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#include "C_SclStringCompat.hpp"

// Linux equivalent of the Windows kbhit(): returns >0 if a byte is available on stdin.
static int kbhit(void)
{
   struct timeval c_Timeout;
   fd_set c_ReadFds;
   c_Timeout.tv_sec = 0;
   c_Timeout.tv_usec = 0;
   FD_ZERO(&c_ReadFds);
   FD_SET(STDIN_FILENO, &c_ReadFds);
   return select(STDIN_FILENO + 1, &c_ReadFds, nullptr, nullptr, &c_Timeout) > 0 ? 1 : 0;
}
#endif

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "TglFile.hpp"
#include "TglTime.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscUtilBinaryHash.hpp"
#include "C_OscCanAdapterFactory.hpp"
#include "C_OscUtils.hpp"
#include "C_BasicUpdateSequence.hpp"
#include "C_BasicFlashTool.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_BasicFlashTool::C_BasicFlashTool(void) :
   mu8_NodeId(0),
   ms32_CanBitrate(125),
   mu32_FlashloaderResetWaitTime(1000),
   mu32_RequestDownloadTimeout(40000),
   mu32_TransferDataTimeout(1000),
   mc_HexFilePath(""),
   mc_CanDriver(""),
   mq_ExitApplOnError(true),
   mq_StartAppl(true),
   mpc_CanDispatcher(nullptr)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_BasicFlashTool::~C_BasicFlashTool()
{
   if (this->mpc_CanDispatcher != nullptr)
   {
      (void)this->mpc_CanDispatcher->CAN_Exit();
      delete this->mpc_CanDispatcher;
      this->mpc_CanDispatcher = nullptr;
      osc_write_log_info("Teardown", "CAN adapter closed.");
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialization on tool startup

   Show some information on command line and setup logging

   \param[in]  os32_Argc   Number of command line arguments
   \param[in]  oppcn_Argv  Command line arguments
*/
//----------------------------------------------------------------------------------------------------------------------
void C_BasicFlashTool::Init(const int32_t os32_Argc, char_t * const * const oppcn_Argv)
{
   std::string c_LogFile;
   C_TglDateTime c_DateTime;
   std::string c_ExeName;
   const std::string c_ExeVersion = mh_GetApplicationVersion(TglGetExePath());
   const std::string c_BinaryHash = C_OscUtilBinaryHash::h_CreateBinaryHash();
#ifdef _WIN32
   {
      char_t acn_ApplicationName[MAX_PATH + 1];
      const uint32_t u32_Return = GetModuleFileNameA(nullptr, &acn_ApplicationName[0], MAX_PATH + 1);
      tgl_assert(u32_Return != 0);
      c_ExeName = acn_ApplicationName;
   }
#else
   c_ExeName = TglGetExePath();
#endif

   std::cout << "This is a very simple openSYDE tool for updating one device with one hex file." << std::endl;
   std::cout << "Version: " << c_ExeVersion.c_str() << std::endl;
   std::cout << "MD5-Checksum: " << c_BinaryHash.c_str() << std::endl;

   // setup logging
   TglGetDateTimeNow(c_DateTime);
   c_LogFile = C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_DateTime);
   // Convert  2023-08-28 09:47:50.459 to  2023-08-28_09-47-50
   // i.e. replace " " with "_", replace ":" with "-" and cut decimals (number of characters is always the same)
   c_LogFile = InsertCompat(c_LogFile, "_", 11); // " "
   c_LogFile = DeleteCompat(c_LogFile, 12, 1);
   c_LogFile = InsertCompat(c_LogFile, "-", 14); // first ":"
   c_LogFile = DeleteCompat(c_LogFile, 15, 1);
   c_LogFile = InsertCompat(c_LogFile, "-", 17); //second ":"
   c_LogFile = DeleteCompat(c_LogFile, 18, 1);
   c_LogFile = DeleteCompat(c_LogFile, 20, 4); // remove ".123"
   c_LogFile = "./Logs/" + c_LogFile + ".syde_log";

   C_OscLoggingHandler::h_SetCompleteLogFileLocation(c_LogFile);
   C_OscLoggingHandler::h_SetWriteToFileActive(true);
   C_OscLoggingHandler::h_SetWriteToConsoleActive(false);

   std::cout << "Logging to file: " << c_LogFile.c_str() << "\n" << std::endl;

   osc_write_log_info("Starting tool",
                      c_ExeName + " Version: " + c_ExeVersion + ", MD5-Checksum: " + c_BinaryHash);
   osc_write_log_info("Call",
                      "Command line: \"" + C_OscUtils::h_GetCommandLineAsString(os32_Argc, oppcn_Argv) + "\"");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse command line

   Parse command line arguments and write to internal data structure. Check that mandatory parameters got provided.
   Log read data resp. used data in default case to log file for easier troubleshooting.

   \param[in]  os32_Argc   Number of command line arguments
   \param[in]  oppcn_Argv  Command line arguments

   \return
   eRESULT_OK                 everything ok
   eRESULT_HELP_REQUESTED     command line option -h found -> only print help and do not continue with flashing
   eERR_PARSE_COMMAND_LINE    invalid or missing command line parameters
*/
//----------------------------------------------------------------------------------------------------------------------
C_BasicFlashTool::E_Result C_BasicFlashTool::ParseCommandLine(const int32_t os32_Argc,
                                                              char_t * const * const oppcn_Argv)
{
   E_Result e_Result = eRESULT_OK;
   int32_t s32_Result;
   bool q_ShowHelp = false;
   bool q_ParseError = false;

   const struct option ac_Options[] =
   {
      /* name, has_arg, flag, val */
      {
         "help",                       no_argument,         nullptr,    'h'
      },
      {
         "nodeid",                     required_argument,   nullptr,    'n'
      },
      {
         "hexfile",                    required_argument,   nullptr,    'f'
      },
      {
         "caninterface",               required_argument,   nullptr,    'i'
      },
      {
         "bitrate",                    required_argument,   nullptr,    'b'
      },
      {
         "dontexitonerror",            no_argument,         nullptr,    'e'
      },
      {
         "dontstartapplication",       no_argument,         nullptr,    'a'
      },
      {
         "flashloaderresetwaittime",   no_argument,         nullptr,    'w'
      },
      {
         "requestdownloadtimeout",     no_argument,         nullptr,    'r'
      },
      {
         "transferdatatimeout",        no_argument,         nullptr,    't'
      },
      {
         nullptr,                         0,                   nullptr,    0
      }
   };

   do
   {
      int32_t s32_Index;
      s32_Result = getopt_long(os32_Argc, oppcn_Argv, "hean:i:b:f:w:r:t:", &ac_Options[0], &s32_Index);
      if (s32_Result != -1)
      {
         switch (s32_Result)
         {
         case 'h':
            q_ShowHelp = true;
            break;
         case 'n':
            try
            {
               mu8_NodeId = static_cast<uint8_t>(std::stoll(optarg));
            }
            catch (...)
            {
               q_ParseError = true;
            }
            break;
         case 'f':
            mc_HexFilePath = optarg;
            break;
         case 'i':
            mc_CanDriver = optarg;
            break;
         case 'b':
            try
            {
               ms32_CanBitrate = std::stoi(optarg);
            }
            catch (...)
            {
               q_ParseError = true;
            }
            break;
         case 'e':
            mq_ExitApplOnError = false;
            break;
         case 'a':
            mq_StartAppl = false;
            break;
         case 'w':
            try
            {
               mu32_FlashloaderResetWaitTime = std::stoi(optarg);
            }
            catch (...)
            {
               q_ParseError = true;
            }
            break;
         case 'r':
            try
            {
               mu32_RequestDownloadTimeout = std::stoi(optarg);
            }
            catch (...)
            {
               q_ParseError = true;
            }
            break;
         case 't':
            try
            {
               mu32_TransferDataTimeout = std::stoi(optarg);
            }
            catch (...)
            {
               q_ParseError = true;
            }
            break;
         case '?': //parser reports error (missing parameter option)
            q_ParseError = true;
            break;
         default:
            q_ParseError = true;
            break;
         }
      }
   }
   while (s32_Result != -1);

   if (q_ShowHelp == true)
   {
      m_PrintHelp();
      e_Result = eRESULT_HELP_REQUESTED;
   }
   else if ((q_ParseError == true) || (mc_HexFilePath == "") || (mc_CanDriver == ""))
   {
      m_PrintHelp();
      std::cout << "Error: Invalid or missing command line parameters.\n\n";
      e_Result = eERR_PARSE_COMMAND_LINE;
   }
   else
   {
      const std::string c_LogActivity = "Parameter Value";
      const std::string c_ExitApplonError = (mq_ExitApplOnError == true) ? "yes" : "no";
      const std::string c_StartAppl = (mq_StartAppl == true) ? "yes" : "no";
      osc_write_log_info(c_LogActivity, "Node ID: " + std::to_string(mu8_NodeId));
      osc_write_log_info(c_LogActivity, "HEX file path: " + mc_HexFilePath);
      osc_write_log_info(c_LogActivity, "CAN driver: " + mc_CanDriver);
      osc_write_log_info(c_LogActivity, "Bitrate in kBit/s: " + std::to_string(ms32_CanBitrate));
      osc_write_log_info(c_LogActivity, "Flashloader reset wait time: " +
                         std::to_string(mu32_FlashloaderResetWaitTime));
      osc_write_log_info(c_LogActivity, "Request download timeout: " +
                         std::to_string(mu32_RequestDownloadTimeout));
      osc_write_log_info(c_LogActivity, "Transfer data timeout: " +
                         std::to_string(mu32_TransferDataTimeout));
      osc_write_log_info(c_LogActivity, "Exit application on error: " + c_ExitApplonError);
      osc_write_log_info(c_LogActivity, "Start application: " + c_StartAppl);
   }

   return e_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Flash node of given ID by using openSYDE's basic update sequence

   \return
   eERR_INITIALIZATION_FAILED    Error in call to C_OscBuSequences::Init
   eERR_ACTIVATE_FLASHLOADER     Error in call to C_OscBuSequences::ActivateFlashLoader
   eERR_READ_DEVICE_INFO         Error in call to C_OscBuSequences::ReadDeviceInformation
   eERR_UPDATE                   Error in call to C_OscBuSequences::UpdateNode
   eERR_RESET                    Error in call to C_OscBuSequences::ResetSystem
*/
//----------------------------------------------------------------------------------------------------------------------
C_BasicFlashTool::E_Result C_BasicFlashTool::Flash(void)
{
   E_Result e_Result = eRESULT_OK;
   C_BasicUpdateSequence c_TheSequence;
   int32_t s32_Return;

   // Build adapter config from the -d/--can-driver argument. On Linux that's a SocketCAN ifname
   // (default "can0" if empty). On Windows that's currently ignored — PEAK channel 1 is assumed
   // until the CLI gains explicit --peak-channel handling.
   stw::opensyde_core::C_OscCanAdapterConfig c_Config =
      stw::opensyde_core::C_OscCanAdapterConfig::h_GetPlatformDefault();
#ifndef _WIN32
   if (mc_CanDriver.empty() == false)
   {
      c_Config.c_ChannelId = mc_CanDriver.c_str();
   }
#endif
   c_Config.u32_BitrateBps = static_cast<uint32_t>(ms32_CanBitrate) * 1000U;

   std::string c_Error;
   stw::can::C_CanDispatcher * const pc_LocalDispatcher =
      stw::opensyde_core::C_OscCanAdapterFactory::h_CreateAdapter(c_Config, c_Error);
   if (pc_LocalDispatcher == nullptr)
   {
      osc_write_log_error("Initialization", "Could not create CAN adapter: " + c_Error);
      e_Result = eERR_INITIALIZATION_FAILED;
   }
   else
   {
      const std::error_code c_CanInitResult = pc_LocalDispatcher->CAN_Init(ms32_CanBitrate);
      if (c_CanInitResult != Errc::success)
      {
         delete pc_LocalDispatcher;
         e_Result = eERR_INITIALIZATION_FAILED;
      }
      else
      {
         //boundary: C_OscBuSequences reports std::error_code, this class keeps the int32_t flow
         s32_Return = c_TheSequence.Init(pc_LocalDispatcher, ms32_CanBitrate, mu8_NodeId).value();
         if (s32_Return != C_NO_ERR)
         {
            (void)pc_LocalDispatcher->CAN_Exit();
            delete pc_LocalDispatcher;
            e_Result = eERR_INITIALIZATION_FAILED;
         }
         else
         {
            // Hand ownership to the member so destructor cleans up.
            this->mpc_CanDispatcher = pc_LocalDispatcher;
         }
      }
   }

   if (e_Result == eRESULT_OK)
   {
      //boundary: C_OscBuSequences reports std::error_code, this class keeps the int32_t flow
      s32_Return = c_TheSequence.ActivateFlashLoader(mu32_FlashloaderResetWaitTime).value();
      if (s32_Return != C_NO_ERR)
      {
         e_Result = eERR_ACTIVATE_FLASHLOADER;
      }
   }

   if (e_Result == eRESULT_OK)
   {
      //boundary: C_OscBuSequences reports std::error_code, this class keeps the int32_t flow
      s32_Return = c_TheSequence.ReadDeviceInformation().value();
      if (s32_Return != C_NO_ERR)
      {
         e_Result = eERR_READ_DEVICE_INFO;
      }
   }

   if (e_Result == eRESULT_OK)
   {
      //boundary: C_OscBuSequences reports std::error_code, this class keeps the int32_t flow
      s32_Return =
         c_TheSequence.UpdateNode(mc_HexFilePath, mu32_RequestDownloadTimeout, mu32_TransferDataTimeout).value();
      if (s32_Return != C_NO_ERR)
      {
         e_Result = eERR_UPDATE;
      }
   }

   if ((e_Result == eRESULT_OK) && (mq_StartAppl == true))
   {
      //boundary: C_OscBuSequences reports std::error_code, this class keeps the int32_t flow
      s32_Return = c_TheSequence.ResetSystem().value();
      if (s32_Return != C_NO_ERR)
      {
         e_Result = eERR_RESET;
      }
   }

   // update log file to recent information
   C_OscLoggingHandler::h_Flush();

   if (e_Result == eRESULT_OK)
   {
      std::cout << "\nUpdate of node with ID " << std::to_string(mu8_NodeId).c_str() << " successful! " <<
         std::endl;
   }
   else
   {
      std::cout << "\nError: Update of node with ID " << std::to_string(mu8_NodeId).c_str() <<
         " failed! Result code: " << static_cast<int32_t>(e_Result) << ". See log file for details. " << std::endl;

      if (mq_ExitApplOnError == false)
      {
         m_Pause();
      }
   }
   return e_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get resource version number of a file as an C_SCLString

   Extracts the windows version number of the specified file and returns it
    in the commonly used STW format: "Vx.yyrz".
   This function is Windows specific and needs to be replaced by another solution
    when porting to a non-Windows system

   \param[in]  orc_FileName   file name to get version from

   \return
   string with version information ("V?.??r?" on error)
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_BasicFlashTool::mh_GetApplicationVersion(const std::string & orc_FileName)
{
   std::string c_Version = "V?.\?\?r?";

#ifdef _WIN32
   VS_FIXEDFILEINFO * pc_Info;
   uint32_t u32_ValSize;
   int32_t s32_InfoSize;
   uint8_t * pu8_Buffer;

   s32_InfoSize = GetFileVersionInfoSizeA(orc_FileName.c_str(), nullptr);
   if (s32_InfoSize != 0)
   {
      pu8_Buffer = new uint8_t[static_cast<uint32_t>(s32_InfoSize)];
      if (GetFileVersionInfoA(orc_FileName.c_str(), 0, s32_InfoSize, pu8_Buffer) != FALSE)
      {
         //reinterpret_cast required due to function interface
         if (VerQueryValueA(pu8_Buffer, "\\",
                            reinterpret_cast<PVOID *>(&pc_Info), //lint !e9176
                            &u32_ValSize) != FALSE)
         {
            c_Version = PrintFormattedCompat("V%lu.%02lur%lu", (pc_Info->dwFileVersionMS >> 16U),
                                     pc_Info->dwFileVersionMS & 0x0000FFFFUL,
                                     (pc_Info->dwFileVersionLS >> 16U));
         }
      }
      delete[] pu8_Buffer;
   }
#else
   (void)orc_FileName;
#endif
   return c_Version;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Print help information to command line
*/
//----------------------------------------------------------------------------------------------------------------------
void C_BasicFlashTool::m_PrintHelp(void)
{
   std::cout << "Command line parameters:\n\n";
   std::cout << "Parameter   Alternative notation        Description                            Default     Example\n";
   std::cout <<
      "============================================================================================================\n";
   std::cout << "-h          --help                      Show help                                          -h\n";
   std::cout << "-n          --nodeid                    Node ID                                <none>      -n 8\n";
   std::cout <<
      "-f          --hexfile                   Path to hex file                       <none>      -f D:\\myfile.hex\n";
   std::cout <<
      "-i          --caninterface              CAN channel identifier                 <none>      -i can0 (Linux) | -i PCAN_USBBUS1 (Windows)\n";
   std::cout << "-b          --bitrate                   Bitrate in kBit/s                      125         -b 1000\n";
   std::cout << "-e          --dontexitonerror           Don't exit application on error        do exit     -e\n";
   std::cout << "-a          --dontstartapplication      Don't start application after update   do start    -a\n";
   std::cout << "-w          --flashloaderresetwaittime  Flashloader reset wait time            1000        -w 2000\n";
   std::cout << "-r          --requestdownloadtimeout    Request download timeout               40000       -r 10000\n";
   std::cout << "-t          --transferdatatimeout       Transfer data timeout                  1000        -t 3000\n";
   std::cout << "\n";
   std::cout << "Parameters that have a \"Default\" are optional. All others are mandatory.\n";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Pause the tool and wait for user interaction to go on
*/
//----------------------------------------------------------------------------------------------------------------------
void C_BasicFlashTool::m_Pause(void)
{
   std::cout << "Press any key to quit ..." << std::endl;
   bool q_Stay = true;
   while (q_Stay == true)
   {
      // wait a moment, we are very fast
      TglSleep(1);

      // check user input
      if (kbhit() > 0)
      {
         std::cout << "Breaking ...\n" << std::endl;
         q_Stay = false;
      }
   }
}
