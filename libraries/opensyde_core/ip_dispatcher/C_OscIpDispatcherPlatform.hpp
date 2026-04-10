//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Platform-agnostic IP dispatcher alias

   Maps C_OscIpDispatcherWinSock to the correct platform implementation so that
   GUI code can use a single name on both Windows and Linux.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCIPDISPATCHERPLATFORM_HPP
#define C_OSCIPDISPATCHERPLATFORM_HPP

#ifdef _WIN32
#include "C_OscIpDispatcherWinSock.hpp"
#else
#include "C_OscIpDispatcherLinuxSock.hpp"
namespace stw
{
namespace opensyde_core
{
typedef C_OscIpDispatcherLinuxSock C_OscIpDispatcherWinSock;
}
}
#endif

#endif
