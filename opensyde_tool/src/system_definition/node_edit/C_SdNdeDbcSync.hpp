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
   /// Bidirectional sync state for a (node, interface) pair. Drives the Sync button's
   /// label/tooltip and the action it dispatches on click.
   enum E_SyncState
   {
      eNEVER_SYNCED,    ///< no DBC fingerprint stored yet — first-time sync available
      eIN_SYNC,         ///< both DBC file and project messages match the last-synced baseline
      eDBC_DRIFTED,     ///< DBC file changed on disk; project messages unchanged — Pull wanted
      ePROJECT_DRIFTED, ///< project messages changed; DBC file unchanged — Push wanted
      eCONFLICT,        ///< both sides drifted since last sync — user must pick a direction
      eDBC_MISSING      ///< DBC file at the expected path is gone — Push (re-emit) is the only path
   };

   static QString h_GetExpectedDbcPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);
   static QString h_ComputeFileSha256(const QString & orc_FilePath);
   static QString h_ComputeProjectMessagesHash(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);
   static E_SyncState h_GetSyncState(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);
   static int32_t h_SyncInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                  QString & orc_ErrorMessage);

private:
   C_SdNdeDbcSync(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
