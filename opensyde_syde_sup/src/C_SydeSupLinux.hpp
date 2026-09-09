//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Linux class of SYDEsup (header)

   See cpp file for detailed description

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYDESUPLINUX_HPP
#define C_SYDESUPLINUX_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <pthread.h>

#include <cstdint>
#include <string>
#include "C_OscIpDispatcherLinuxSock.hpp"
#include "C_SydeSup.hpp"

// Symbols declared with this attribute shall be visible as the API of the library
// No other symbols shall be visible to avoid namin conflicts with other libaries
// using same modules (e.g. SCL or TGL modules).
#define LIB_PUBLIC __attribute__ ((visibility("default")))

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class LIB_PUBLIC C_SydeSupLinux :
   public C_SydeSup
{
public:
   C_SydeSupLinux(void);
   virtual ~C_SydeSupLinux(void);

   E_Result InitParameters(const std::string & orc_SupFilePath, const std::string & orc_CanInterface,
                           const std::string & orc_LogPath, const std::string & orc_UnzipPath);
   void SetUpdateFilePath(const std::string & orc_SupFilePath);
   E_Result UpdateTaskStart(void);
   E_Result UpdateTaskCheckResult(uint8_t & oru8_Progress);
   E_Result GetNextInfoText(std::string & orc_Info);

private:
   pthread_t mx_UpdateTaskHandle; //lint !e8080 //interfacing with library
   bool mq_UpdateTaskRunning = false;
   E_Result me_UpdateTaskResult;
   uint8_t mu8_UpdateProgress = 0U;
   std::vector<std::string> mac_UpdateInfo;
   uint32_t mu32_InfoIndex = 0U;

   static void * mh_UpdateTask(void * const opv_Arg);

   virtual E_Result m_OpenEthernet(void);
   virtual std::string m_GetApplicationVersion(const std::string & orc_ApplicationFileName) const;
   virtual std::string m_GetDefaultLogLocation(void) const;
   virtual std::string m_GetUnzipLocationDefaultExample(void) const;
   virtual std::string m_GetCanInterfaceUsageExample(void) const;

   stw::opensyde_core::C_OscIpDispatcherLinuxSock mc_EthDispatcher;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
