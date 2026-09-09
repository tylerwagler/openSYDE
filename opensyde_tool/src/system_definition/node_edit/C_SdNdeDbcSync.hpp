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

#include <cstdint>

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
   /// Strip the matching configured device root from an absolute DBC path so tooltips
   /// can show the install-root-relative form (e.g. `Sensors/temp_sensor/temp_sensor_CAN1.dbc`)
   /// instead of the noisy absolute prefix. Falls back to the absolute path if no
   /// configured root contains it.
   static QString h_GetDisplayDbcPath(const QString & orc_AbsolutePath);
   static QString h_ComputeFileSha256(const QString & orc_FilePath);
   static QString h_ComputeProjectMessagesHash(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);
   static E_SyncState h_GetSyncState(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex);

   /// Pull: import the device's DBC file onto the project's per-interface CAN messages.
   /// Existing same-ID messages are updated in place; new messages are added.
   /// Both fingerprints are stamped on success.
   static int32_t h_PullInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                  QString & orc_ErrorMessage);

   /// Push: write the project's per-interface CAN messages back out to the device's DBC
   /// file (overwrites the file). Both fingerprints are stamped on success.
   static int32_t h_PushInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                  QString & orc_ErrorMessage);

private:
   C_SdNdeDbcSync(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
