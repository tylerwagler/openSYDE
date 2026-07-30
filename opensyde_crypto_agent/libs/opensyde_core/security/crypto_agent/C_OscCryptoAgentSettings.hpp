//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Crypto agent settings
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCRYPTOAGENTSETTINGS_HPP
#define C_OSCCRYPTOAGENTSETTINGS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_SclString.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCryptoAgentSettings
{
public:
   C_OscCryptoAgentSettings();

   void SetDefault(void);

   stw::scl::C_SclString c_CryptoAgentExecutablePath; ///< Crypto agent executable file path
   stw::scl::C_SclString c_CryptoAgentConfigFilePath; ///< Crypto agent config file path
   uint8_t au8_CryptoAgentIp[4UL];                    ///< Crypto agent IP
   uint16_t u16_CryptoAgentPort;                      ///< Crypto agent port
   bool q_CryptoAgentAutoStart;                       ///< Crypto agent auto start
   bool q_CryptoAgentAutoStop;                        ///< Crypto agent auto stop
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
