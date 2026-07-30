//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tool user settings

   Tool user settings

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "C_UsToolSettings.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

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
C_UsToolSettings::C_UsToolSettings(void)
{
   SetDefault();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set default values
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsToolSettings::SetDefault(void)
{
   this->mc_PathHandlingSelection = "";
   this->mc_SkipTspImportSelection = "";
   this->mc_CryptoAgentSettings.SetDefault();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get the currently selected option for path handling

   \retval   Relative
   \retval   Absolute
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_UsToolSettings::GetPathHandlingSelection(void) const
{
   return this->mc_PathHandlingSelection;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get TSP shortcut usage

   \return
   bool

   \retval   true    TSP shortcut dialog will not appear after adding a new node (Import is skipped always)
   \retval   false   TSP shortcut dialog will appear after adding a new node
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_UsToolSettings::GetSkipTspSelection(void) const
{
   return this->mc_SkipTspImportSelection;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get crypto agent settings

   \return
   Crypto agent settings
*/
//----------------------------------------------------------------------------------------------------------------------
stw::opensyde_core::C_OscCryptoAgentSettings C_UsToolSettings::GetCryptoAgentSettings(void) const
{
   return this->mc_CryptoAgentSettings;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set flag whether TSP Import on adding a Node shall be skipped always

   \param[in]  orc_Selection  can be "Ask User" or "Skip"
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsToolSettings::SetSkipTspSelection(const QString & orc_Selection)
{
   this->mc_SkipTspImportSelection = orc_Selection;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set flag for the currently selected option for the path handling

   \param[in]  orc_Selection  can be "Ask User", "Relative" or "Absolute"
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsToolSettings::SetPathHandlingSelection(const QString & orc_Selection)
{
   this->mc_PathHandlingSelection = orc_Selection;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set crypto agent settings

   \param[in]  orc_NewCryptoAgentSettings    New crypto agent settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsToolSettings::SetCryptoAgentSettings(
   const stw::opensyde_core::C_OscCryptoAgentSettings & orc_NewCryptoAgentSettings)
{
   this->mc_CryptoAgentSettings = orc_NewCryptoAgentSettings;
}
