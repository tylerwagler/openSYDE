//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       DiagLib configuration.

   Contains potentially target specific adaptations for the DiagLib library

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CDIAGLLIB_CONFIG_HPP
#define CDIAGLLIB_CONFIG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscUtils.hpp"

/* -- Defines ------------------------------------------------------------------------------------------------------- */

//DiagLib needs TGL_LoadStr; we have our own implementation:
#define TGL_LoadStr stw::opensyde_core::C_OscUtils::h_LoadString

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

#endif
