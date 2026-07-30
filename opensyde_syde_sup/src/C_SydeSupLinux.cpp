//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Linux class of SYDEsup (implementation)

   Implements the Linux-specific functions for SYDEsup.

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include "version_config.hpp"

#include "stwerrors.hpp"
#include "C_SydeSupLinux.hpp"

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
*/
//----------------------------------------------------------------------------------------------------------------------
C_SydeSupLinux::C_SydeSupLinux(void) :
   C_SydeSup(),
   mq_CanDllLoaded(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SydeSupLinux::~C_SydeSupLinux(void)
{
}

//-------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize CAN driver.

   The implementation shall
   * initialize a specific implementation of C_CANDispatcher
   * do whatever is required to set that dispatcher up (connect to CAN bus)
   * set mpc_CanDispatcher to point to that dispatcher

   \param[in]  orc_CanDriverPath    path to CAN driver as passed on command line
   \param[in]  ou64_BitrateBps      bitrate to initialize CAN bus with

   \return  error code
   eOK                        driver initialized
   eERR_CAN_IF_LOAD_FAILED    driver already open
*/
//----------------------------------------------------------------------------------------------------------------------
C_SydeSup::E_Result C_SydeSupLinux::m_OpenCan(const C_SclString & orc_CanDriver, const uint64_t ou64_BitrateBps)
{
   C_SydeSup::E_Result e_Result = eOK;
   int32_t s32_Return;

   (void)ou64_BitrateBps; // Bitrate is set by interface settings outside

   if (mpc_CanDispatcher == NULL)
   {
      mpc_CanDispatcher = &mc_CanDispatcher;
   }

   s32_Return = mc_CanDispatcher.CAN_Init(orc_CanDriver, 1);

   if (s32_Return != C_NO_ERR)
   {
      e_Result = eERR_SEQUENCE_CAN_INIT;
   }

   return e_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close CAN interface

   Disconnect from CAN bus and CAN driver
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SydeSupLinux::m_CloseCan(void)
{
   (void)mc_CanDispatcher.CAN_Exit();

   if (mq_CanDllLoaded == true)
   {
      (void)mc_CanDispatcher;
      mq_CanDllLoaded = false;
   }

   mpc_CanDispatcher = NULL; //not valid any more
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize Ethernet driver.

   The implementation shall
   * initialize a specific implementation of C_OSCIpDispatcher
   * do whatever is required to set that dispatcher up (nothing in most cases)
   * set mpc_EthDispatcher to point to that dispatcher

   \return  error code
   eOK                        driver initialized
   eERR_ETH_IF_LOAD_FAILED    driver load failed
*/
//----------------------------------------------------------------------------------------------------------------------
C_SydeSup::E_Result C_SydeSupLinux::m_OpenEthernet(void)
{
   mpc_EthDispatcher = &mc_EthDispatcher;
   return eOK;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get name of application as an C_SclString

   Return the version number of the running application
    in the commonly used STW format: "Vx.yyrz".

   Here for Linux: use configuration in version_config.hpp

   \param[in]  orc_ApplicationFileName    file name of running application (can help to implement on some targets)

   \return
   string with version information
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_SydeSupLinux::m_GetApplicationVersion(const C_SclString & orc_ApplicationFileName) const
{
   C_SclString c_Version;

   (void)orc_ApplicationFileName;

   c_Version.PrintFormatted("V%d.%02dr%d",
                            PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
                            PROJECT_VERSION_RELEASE);

   return c_Version;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get default log location

   \return
   Default log location
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_SydeSupLinux::m_GetDefaultLogLocation(void) const
{
   return "/var/log";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get default example for unzip location (user's temporary folder of operating system)

   \return
   Example unzip location
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_SydeSupLinux::m_GetUnzipLocationDefaultExample(void) const
{
   return "/tmp";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get CAN interface command line flag "-i" usage example

   \return
   usage example
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_SydeSupLinux::m_GetCanInterfaceUsageExample(void) const
{
   return "-i can0";
}
