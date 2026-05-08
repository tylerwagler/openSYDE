//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Helpers for the per-interface DBC sync feature

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDBCSYNC_HPP
#define C_SDNDEDBCSYNC_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QString>

#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDbcSync
{
public:
   static QString h_GetExpectedDbcPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);
   static QString h_ComputeFileSha256(const QString & orc_FilePath);
   static int32_t h_SyncInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                  QString & orc_ErrorMessage);

private:
   C_SdNdeDbcSync(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
