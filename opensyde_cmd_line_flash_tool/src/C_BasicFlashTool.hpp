//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class of openSYDE simple command line flash tool
   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_BASICFLASHTOOL_HPP
#define C_BASICFLASHTOOL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include "C_CanDispatcher.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_BasicFlashTool
{
public:
   enum E_Result
   {
      eRESULT_OK = 0,
      eRESULT_HELP_REQUESTED = 1,
      eERR_PARSE_COMMAND_LINE = 10,
      eERR_INITIALIZATION_FAILED = 20,
      eERR_ACTIVATE_FLASHLOADER = 30,
      eERR_READ_DEVICE_INFO = 40,
      eERR_UPDATE = 50,
      eERR_RESET = 60
  // description of error codes in about.txt
   };

   C_BasicFlashTool(void);
   virtual ~C_BasicFlashTool(void);

   void Init(const int32_t os32_Argc = 0, char_t * const * const oppcn_Argv = NULL);
   E_Result ParseCommandLine(const int32_t os32_Argc, char_t * const * const oppcn_Argv);
   E_Result Flash(void);

private:
   uint8_t mu8_NodeId;
   int32_t ms32_CanBitrate;
   uint32_t mu32_FlashloaderResetWaitTime;
   uint32_t mu32_RequestDownloadTimeout;
   uint32_t mu32_TransferDataTimeout;
   std::string mc_HexFilePath;
   std::string mc_CanDriver;
   bool mq_ExitApplOnError;
   bool mq_StartAppl;

   stw::can::C_CanDispatcher * mpc_CanDispatcher;

   static std::string mh_GetApplicationVersion(const std::string & orc_FileName);
   void m_PrintHelp(void);
   void m_Pause(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
