//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Windows class of SYDEsup (header)

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYDESUPWINDOWS_HPP
#define C_SYDESUPWINDOWS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_SclString.hpp"
#include "C_OscIpDispatcherWinSock.hpp"
#include "C_SydeSup.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SydeSupWindows :
   public C_SydeSup
{
public:
   C_SydeSupWindows(void);
   virtual ~C_SydeSupWindows(void);

private:
   virtual E_Result m_OpenEthernet(void);
   virtual stw::scl::C_SclString m_GetApplicationVersion(const stw::scl::C_SclString & orc_ApplicationFileName) const;
   virtual stw::scl::C_SclString m_GetDefaultLogLocation(void) const;
   virtual stw::scl::C_SclString m_GetUnzipLocationDefaultExample(void) const;
   virtual stw::scl::C_SclString m_GetCanInterfaceUsageExample(void) const;

   stw::opensyde_core::C_OscIpDispatcherWinSock mc_EthDispatcher;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
