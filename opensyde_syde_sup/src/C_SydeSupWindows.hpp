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
#include <cstdint>
#include <string>
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
   virtual std::string m_GetApplicationVersion(const std::string & orc_ApplicationFileName) const;
   virtual std::string m_GetDefaultLogLocation(void) const;
   virtual std::string m_GetUnzipLocationDefaultExample(void) const;
   virtual std::string m_GetCanInterfaceUsageExample(void) const;

   stw::opensyde_core::C_OscIpDispatcherWinSock mc_EthDispatcher;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
