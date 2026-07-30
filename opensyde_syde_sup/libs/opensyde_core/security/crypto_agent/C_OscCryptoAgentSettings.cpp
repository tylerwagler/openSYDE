//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Crypto agentagent settings

   Crypto agent settings

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_OscCryptoAgentSettings.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
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
C_OscCryptoAgentSettings::C_OscCryptoAgentSettings()
{
   SetDefault();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set default
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCryptoAgentSettings::SetDefault()
{
   this->c_CryptoAgentExecutablePath = "../connectors/crypto_agent/osy_crypto_agent.exe";
   this->c_CryptoAgentConfigFilePath = "./osy_crypto_agent.conf";
   this->au8_CryptoAgentIp[0UL] = 127U;
   this->au8_CryptoAgentIp[1UL] = 0U;
   this->au8_CryptoAgentIp[2UL] = 0U;
   this->au8_CryptoAgentIp[3UL] = 1U;
   this->u16_CryptoAgentPort = 50963UL;
   this->q_CryptoAgentAutoStart = true;
   this->q_CryptoAgentAutoStop = true;
}
