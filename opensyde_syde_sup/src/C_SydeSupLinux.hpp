//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Linux class of SYDEsup (header)

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYDESUPLINUX_HPP
#define C_SYDESUPLINUX_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_SclString.hpp"
#include "C_Can.hpp"
#include "C_OscIpDispatcherLinuxSock.hpp"
#include "C_SydeSup.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SydeSupLinux :
   public C_SydeSup
{
public:
   C_SydeSupLinux(void);
   virtual ~C_SydeSupLinux(void);

private:
   virtual void m_CloseCan(void);
   virtual E_Result m_OpenCan(const stw::scl::C_SclString & orc_CanDriver, const uint64_t ou64_BitrateBps);
   virtual E_Result m_OpenEthernet(void);
   virtual stw::scl::C_SclString m_GetApplicationVersion(const stw::scl::C_SclString & orc_ApplicationFileName) const;
   virtual stw::scl::C_SclString m_GetDefaultLogLocation(void) const;
   virtual stw::scl::C_SclString m_GetUnzipLocationDefaultExample(void) const;
   virtual stw::scl::C_SclString m_GetCanInterfaceUsageExample(void) const;

   bool mq_CanDllLoaded;
   stw::can::C_Can mc_CanDispatcher;
   stw::opensyde_core::C_OscIpDispatcherLinuxSock mc_EthDispatcher;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
