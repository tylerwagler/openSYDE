//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Certificates Package base

   openSYDE: X-Certificates Package base

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdint>
#include "C_OscXceBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscXceBase::hc_PACKAGE_EXT = ".syde_xcert";
const std::string C_OscXceBase::hc_PACKAGE_EXT_TMP = ".syde_xcert_tmp"; // intermediate directory
                                                                                  // before creating zip
                                                                                  // archive

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
std::vector<std::string> C_OscXceBase::mhc_WarningMessages; // global warnings e.g. if update
                                                             // position
                                                             // of
                                                             // active node is not available
std::string C_OscXceBase::mhc_ErrorMessage;        // description of error which caused
                                                             // the
                                                             // service update package to fail
const std::string C_OscXceBase::mhc_CERTIFICATES_FOLDER = "authentication_certificates";
const std::string C_OscXceBase::mhc_UPDATE_PACKAGE_PARAMETERS_FOLDER = "update_certificates";

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns fix defined service update package extension.

   \return
   service update package extension
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscXceBase::h_GetPackageExtension()
{
   return hc_PACKAGE_EXT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXceBase::mh_Init()
{
   mhc_WarningMessages.clear(); // clear old warning messages
   mhc_ErrorMessage = "";       // clear old error message
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get warnings and errors

   \param[in,out]  orc_WarningMessages    Warning messages
   \param[in,out]  orc_ErrorMessage       Error message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXceBase::mh_GetWarningsAndErrors(std::vector<std::string> & orc_WarningMessages,
                                           std::string & orc_ErrorMessage)
{
   orc_WarningMessages = mhc_WarningMessages; // set warning messages for caller
   orc_ErrorMessage = mhc_ErrorMessage;       // set error message for caller
}
