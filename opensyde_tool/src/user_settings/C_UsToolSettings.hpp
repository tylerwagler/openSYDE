//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tool user settings
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_USTOOLSETTINGS_HPP
#define C_USTOOLSETTINGS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <array>
#include <QString>

#include "C_OscCryptoAgentSettings.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_UsToolSettings
{
public:
   C_UsToolSettings(void);

   void SetDefault(void);

   QString GetPathHandlingSelection(void) const;
   QString GetSkipTspSelection(void) const;
   stw::opensyde_core::C_OscCryptoAgentSettings GetCryptoAgentSettings(void) const;

   void SetSkipTspSelection(const QString & orc_Selection);
   void SetPathHandlingSelection(const QString & orc_Selection);
   void SetCryptoAgentSettings(const stw::opensyde_core::C_OscCryptoAgentSettings & orc_NewCryptoAgentSettings);

private:
   QString mc_PathHandlingSelection;  ///< Currently chosen option (Relative or Absolute)
   QString mc_SkipTspImportSelection; ///< Flag if TSP Import on Node Adding shall always be skipped
   stw::opensyde_core::C_OscCryptoAgentSettings
      mc_CryptoAgentSettings; ///< Crypto agent settings
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
