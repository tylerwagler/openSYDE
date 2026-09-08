//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Helpers for the per-interface DBC sync feature

   The "sync DBC" feature lets a user import a device's bundled DBC messages onto the
   CAN bus the device is connected to, and detect when the on-disk DBC has changed
   since the last sync via a SHA-256 fingerprint stored on the project's per-interface
   settings.

   v1 SCAFFOLD: this commit ships the discovery, hashing and fingerprint-storage
   plumbing. Actual message insertion (parsing the DBC and adding messages to the
   bus's COMM datapool via the existing C_CieUtil import pipeline) is a follow-up.
   The sync function currently validates the DBC exists, computes its SHA-256, and
   stores the fingerprint — enough to make the out-of-sync detector functional but
   not yet enough to materialize messages.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "stwerrors.hpp"
#include "C_OscNode.hpp"
#include "C_OscNodeCommFiler.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscCanMessage.hpp"
#include "C_OscCanMessageContainer.hpp"
#include "C_OscSystemBus.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_PuiSdUtil.hpp"
#include "C_UsHandler.hpp"
#include "C_CieImportDbc.hpp"
#include "C_CieExportDbc.hpp"
#include "C_CieDataPoolListAdapter.hpp"
#include "C_CieImportDataAssignment.hpp"
#include "C_CieUtil.hpp"
#include "C_CieConverter.hpp"
#include "C_SdNdeDbcSync.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Find an existing CAN message on (node, interface, protocol) that matches an incoming DBC message.

   Returns `(datapool_index, message_index)` of the existing message that should be overridden by the
   incoming one, or `(-1, -1)` if no match was found (i.e. the incoming message should be added as new).

   Match criterion is `(CanId, IsExtended)`. We deliberately ignore message name so that re-syncing a DBC
   in which a message's name has been edited still updates the existing entry — without that, a rename
   would create a new entry with a duplicate ID alongside the old one.
*/
//----------------------------------------------------------------------------------------------------------------------
std::pair<int32_t, int32_t> mh_FindOverrideTarget(const C_OscNode & orc_Node, const uint32_t ou32_InterfaceIndex,
                                                  const C_OscCanProtocol::E_Type oe_Protocol, const bool oq_Tx,
                                                  const C_OscCanMessage & orc_Incoming)
{
   std::pair<int32_t, int32_t> c_Result(-1, -1);

   const std::vector<const C_OscCanProtocol *> c_Protocols = orc_Node.GetCanProtocolsConst(oe_Protocol);
   for (std::vector<const C_OscCanProtocol *>::const_iterator c_It = c_Protocols.begin();
        (c_It != c_Protocols.end()) && (c_Result.first < 0); ++c_It)
   {
      const C_OscCanProtocol * const pc_Protocol = *c_It;
      if ((pc_Protocol != NULL) && (ou32_InterfaceIndex < pc_Protocol->c_ComMessages.size()))
      {
         const C_OscCanMessageContainer & rc_Container = pc_Protocol->c_ComMessages[ou32_InterfaceIndex];
         const std::vector<C_OscCanMessage> & rc_Messages = rc_Container.GetMessagesConst(oq_Tx);
         for (uint32_t u32_It = 0U; u32_It < rc_Messages.size(); ++u32_It)
         {
            const C_OscCanMessage & rc_Existing = rc_Messages[u32_It];
            if ((rc_Existing.u32_CanId == orc_Incoming.u32_CanId) &&
                (rc_Existing.q_IsExtended == orc_Incoming.q_IsExtended))
            {
               c_Result = std::make_pair(static_cast<int32_t>(pc_Protocol->u32_DataPoolIndex),
                                         static_cast<int32_t>(u32_It));
               break;
            }
         }
      }
   }
   return c_Result;
}
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Resolve the expected DBC file path for a (node, interface) pair.

   Convention: `<bundle_dir>/<device_name>_CAN<n>.dbc` where `<n>` is the 1-based
   interface number (matching the GUI labels CAN1, CAN2, ...). The bundle directory
   is the parent of the device's `device.syd` manifest.

   Returns an empty QString when:
   - the node index is out of range
   - the node has no associated device definition
   - the interface index is out of range
   - the interface is not a CAN interface

   The path is returned regardless of whether the file actually exists on disk; the
   caller decides what to do with a non-existent path.
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SdNdeDbcSync::h_GetExpectedDbcPath(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex)
{
   QString c_Path;

   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);

   if ((pc_Node != NULL) && (pc_Node->pc_DeviceDefinition != NULL) &&
       (ou32_InterfaceIndex < pc_Node->c_Properties.c_ComInterfaces.size()))
   {
      const C_OscNodeComInterfaceSettings & rc_Interface =
         pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex];

      if (rc_Interface.e_InterfaceType == C_OscSystemBus::eCAN)
      {
         const QFileInfo c_Manifest(pc_Node->pc_DeviceDefinition->c_FilePath.c_str());
         const QString c_BundleDir = c_Manifest.absolutePath();
         const QString c_DeviceName = pc_Node->pc_DeviceDefinition->c_DeviceName.c_str();
         const uint32_t u32_OneBased = static_cast<uint32_t>(rc_Interface.u8_InterfaceNumber) + 1U;

         c_Path = c_BundleDir + "/" + c_DeviceName + "_CAN" + QString::number(u32_OneBased) + ".dbc";
      }
   }

   return c_Path;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compute a stable hash of the project's CAN messages on a (node, interface) pair.

   Pulls the per-interface message container from the Layer 2 COMM datapool (matching the
   protocol used by the Pull/Push paths) and runs `C_OscCanMessageContainer::CalcHash` to
   get a 32-bit CRC of all message + signal fields. The hex of that CRC is the project-side
   fingerprint stored on the per-interface settings as `c_LastSyncedProjectMsgHash`.

   Returns an empty QString when the node has no Layer 2 COMM datapool yet (never-synced
   case) or when the indices are out of range. An empty fingerprint plus an empty stored
   fingerprint counts as "matches" in the state detector below — both sides agree there
   are no project messages to track.
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SdNdeDbcSync::h_ComputeProjectMessagesHash(const uint32_t ou32_NodeIndex,
                                                     const uint32_t ou32_InterfaceIndex)
{
   QString c_Result;

   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);

   if ((pc_Node != NULL) && (ou32_InterfaceIndex < pc_Node->c_Properties.c_ComInterfaces.size()))
   {
      const C_OscCanProtocol::E_Type e_Protocol =
         pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex].e_DbcProtocol;
      int32_t s32_DatapoolIndex = -1;
      for (uint32_t u32_It = 0U; u32_It < pc_Node->c_DataPools.size(); ++u32_It)
      {
         if ((pc_Node->c_DataPools[u32_It].e_Type == C_OscNodeDataPool::eCOM) &&
             (C_PuiSdUtil::h_GetRelatedCanProtocolType(ou32_NodeIndex, u32_It) == e_Protocol))
         {
            s32_DatapoolIndex = static_cast<int32_t>(u32_It);
            break;
         }
      }

      if (s32_DatapoolIndex >= 0)
      {
         const C_OscCanMessageContainer * const pc_Container =
            C_PuiSdHandler::h_GetInstance()->GetCanProtocolMessageContainer(
               ou32_NodeIndex, e_Protocol, ou32_InterfaceIndex,
               static_cast<uint32_t>(s32_DatapoolIndex));
         if (pc_Container != NULL)
         {
            uint32_t u32_Hash = 0U;
            pc_Container->CalcHash(u32_Hash);
            c_Result = QString::number(u32_Hash, 16);
         }
      }
   }

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Determine the current sync state for a (node, interface) pair.

   See `E_SyncState` for the per-state semantics. Resolution order:
   - empty stored DBC fingerprint                    -> eNEVER_SYNCED
   - DBC file gone from the expected path            -> eDBC_MISSING
   - DBC file hash diff + project msg hash diff      -> eCONFLICT
   - DBC file hash diff (project unchanged)          -> eDBC_DRIFTED
   - project msg hash diff (DBC unchanged)           -> ePROJECT_DRIFTED
   - everything matches                              -> eIN_SYNC
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeDbcSync::E_SyncState C_SdNdeDbcSync::h_GetSyncState(const uint32_t ou32_NodeIndex,
                                                           const uint32_t ou32_InterfaceIndex)
{
   E_SyncState e_State = eNEVER_SYNCED;

   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);

   if ((pc_Node != NULL) && (ou32_InterfaceIndex < pc_Node->c_Properties.c_ComInterfaces.size()))
   {
      const C_OscNodeComInterfaceSettings & rc_Interface =
         pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex];
      const QString c_StoredDbcHash = rc_Interface.c_LastSyncedDbcSha256.c_str();
      const QString c_DbcPath = h_GetExpectedDbcPath(ou32_NodeIndex, ou32_InterfaceIndex);
      const bool q_DbcExists = (c_DbcPath.isEmpty() == false) && QFile::exists(c_DbcPath);

      if (c_StoredDbcHash.isEmpty() == true)
      {
         // Never synced. If the project already has messages on this interface but no DBC
         // file exists, the natural first action is Push (create the DBC from project state)
         // — return eDBC_MISSING so the click dispatcher routes to Push instead of failing
         // a Pull on a non-existent file.
         if ((q_DbcExists == false) &&
             (h_ComputeProjectMessagesHash(ou32_NodeIndex, ou32_InterfaceIndex).isEmpty() == false))
         {
            e_State = eDBC_MISSING;
         }
         else
         {
            e_State = eNEVER_SYNCED;
         }
      }
      else if (q_DbcExists == false)
      {
         e_State = eDBC_MISSING;
      }
      else
      {
         const QString c_CurrentDbcHash = h_ComputeFileSha256(c_DbcPath);
         const QString c_StoredProjectHash = rc_Interface.c_LastSyncedProjectMsgHash.c_str();
         const QString c_CurrentProjectHash = h_ComputeProjectMessagesHash(ou32_NodeIndex, ou32_InterfaceIndex);

         const bool q_DbcDrifted = (c_CurrentDbcHash != c_StoredDbcHash);
         const bool q_ProjectDrifted = (c_CurrentProjectHash != c_StoredProjectHash);

         if (q_DbcDrifted && q_ProjectDrifted)
         {
            e_State = eCONFLICT;
         }
         else if (q_DbcDrifted)
         {
            e_State = eDBC_DRIFTED;
         }
         else if (q_ProjectDrifted)
         {
            e_State = ePROJECT_DRIFTED;
         }
         else
         {
            e_State = eIN_SYNC;
         }
      }
   }

   return e_State;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Strip the matching configured device root from an absolute path.

   Iterates the user's configured device-root list (the same list the scanner walks at
   startup), finds the root that the absolute path lives under, and returns the trailing
   segment with forward slashes — e.g. `Sensors/temp_sensor/temp_sensor_CAN1.dbc` for an
   install at `/home/tyler/.local/opt/openSYDE/devices/Sensors/temp_sensor/...`.

   Falls back to returning the absolute path unchanged if no configured root contains it
   (e.g., the bundle was added from a custom location that's not in the roots list, or
   the helper is called before settings load).

   For error popups where the user might want to copy the path into a shell, prefer the
   absolute form. This helper is intended for tooltips and other space-constrained UI.
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SdNdeDbcSync::h_GetDisplayDbcPath(const QString & orc_AbsolutePath)
{
   QString c_Result = orc_AbsolutePath;

   if (orc_AbsolutePath.isEmpty() == false)
   {
      const QString c_AbsClean = QDir::cleanPath(orc_AbsolutePath);
      const QStringList c_Roots = C_UsHandler::h_GetInstance()->GetDeviceRootPaths();

      for (int32_t s32_It = 0; s32_It < c_Roots.size(); ++s32_It)
      {
         const QString c_RootClean = QDir::cleanPath(c_Roots.at(s32_It));
         if (c_RootClean.isEmpty() == false)
         {
            const QString c_RootWithSlash = c_RootClean + "/";
            if (c_AbsClean.startsWith(c_RootWithSlash) == true)
            {
               c_Result = c_AbsClean.mid(c_RootWithSlash.length());
               break;
            }
         }
      }
   }

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compute the lowercase-hex SHA-256 of the given file's contents.

   Returns an empty QString on read failure (file missing, permission denied, etc.).
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SdNdeDbcSync::h_ComputeFileSha256(const QString & orc_FilePath)
{
   QString c_Hex;

   QFile c_File(orc_FilePath);
   if (c_File.open(QFile::ReadOnly) == true)
   {
      QCryptographicHash c_Hash(QCryptographicHash::Sha256);
      if (c_Hash.addData(&c_File) == true)
      {
         c_Hex = QString::fromLatin1(c_Hash.result().toHex());
      }
      c_File.close();
   }

   return c_Hex;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Pull the per-interface DBC for the given (node, interface) pair into the project.

   Validates that:
   - the node has an associated device definition
   - the interface exists, is a CAN interface, and is connected to a bus
   - the expected DBC file (`<device_name>_CAN<n>.dbc` next to the manifest) exists
     and is readable

   On success: parses the DBC, imports its first DBC node's TX/RX messages onto
   the openSYDE node's Layer 2 COMM datapool (auto-creating the datapool if the
   node has none), then computes and stores both the DBC SHA-256 and the
   project-side message-container CRC fingerprints on the per-interface settings.

   v1 LIMITATIONS:
   - Protocol type is hard-coded to `C_OscCanProtocol::eLAYER2`. Devices whose DBC
     belongs on a different protocol (e.g. J1939) will need a follow-up that lets
     the manifest declare or the user choose the target protocol.
   - The insertion uses `oq_UniqueAddRequested = true`, so re-syncing won't
     duplicate same-ID messages but also won't update an existing message's
     signal layout if the DBC version drifts. A "replace existing" mode is a
     follow-up.

   \param[in]   ou32_NodeIndex      Index of the node in the system definition
   \param[in]   ou32_InterfaceIndex Index of the node's CAN interface (per-node, mixed types)
   \param[out]  orc_ErrorMessage    User-facing error message on failure; empty on success

   \return
   C_NO_ERR    sync completed; messages imported and fingerprint stored
   C_RANGE     node/interface index out of range, or interface is not CAN
   C_CONFIG    node has no device definition, interface not connected to a bus,
               DBC has no nodes, or the COMM datapool could not be located/created
   C_RD_WR     DBC file does not exist, could not be read, or could not be parsed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SdNdeDbcSync::h_PullInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                        QString & orc_ErrorMessage)
{
   int32_t s32_Retval = C_NO_ERR;

   orc_ErrorMessage = "";

   C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNode(ou32_NodeIndex);

   if (pc_Node == NULL)
   {
      orc_ErrorMessage = static_cast<QString>("Node index %1 out of range.").arg(
         ou32_NodeIndex);
      s32_Retval = C_RANGE;
   }
   else if (pc_Node->pc_DeviceDefinition == NULL)
   {
      orc_ErrorMessage = "Node has no associated device definition.";
      s32_Retval = C_CONFIG;
   }
   else if (ou32_InterfaceIndex >= pc_Node->c_Properties.c_ComInterfaces.size())
   {
      orc_ErrorMessage = static_cast<QString>("Interface index %1 out of range.").arg(
         ou32_InterfaceIndex);
      s32_Retval = C_RANGE;
   }
   else
   {
      C_OscNodeComInterfaceSettings & rc_Interface = pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex];

      if (rc_Interface.e_InterfaceType != C_OscSystemBus::eCAN)
      {
         orc_ErrorMessage = "Interface is not a CAN interface.";
         s32_Retval = C_RANGE;
      }
      else if (rc_Interface.GetBusConnected() == false)
      {
         orc_ErrorMessage = "Interface is not connected to a bus.";
         s32_Retval = C_CONFIG;
      }
      else
      {
         const QString c_DbcPath = h_GetExpectedDbcPath(ou32_NodeIndex, ou32_InterfaceIndex);
         if (c_DbcPath.isEmpty() || (QFile::exists(c_DbcPath) == false))
         {
            orc_ErrorMessage = static_cast<QString>("Expected DBC file not found: %1").arg(c_DbcPath);
            s32_Retval = C_RD_WR;
         }
         else
         {
            // Per-interface project setting; defaults to Layer 2.
            const C_OscCanProtocol::E_Type e_Protocol = rc_Interface.e_DbcProtocol;

            // Step 1: parse DBC headlessly.
            C_CieConverter::C_CieCommDefinition c_CommDef;
            stw::scl::std::vector<std::string> c_Warnings;
            std::string c_ParseError;
            const int32_t s32_ParseResult =
               C_CieImportDbc::h_ImportNetwork(c_DbcPath.toStdString().c_str(),
                                               c_CommDef, c_Warnings, c_ParseError, true);

            if ((s32_ParseResult != C_NO_ERR) && (s32_ParseResult != C_WARN))
            {
               orc_ErrorMessage = static_cast<QString>("Could not parse DBC %1: %2").arg(c_DbcPath,
                                                                                             c_ParseError.c_str());
               s32_Retval = C_RD_WR;
            }
            else if (c_CommDef.c_Nodes.empty() == true)
            {
               orc_ErrorMessage = static_cast<QString>("DBC %1 contains no nodes.").arg(c_DbcPath);
               s32_Retval = C_CONFIG;
            }
            else
            {
               // Step 2: resolve / auto-create the target COMM datapool of the chosen protocol.
               int32_t s32_DatapoolIndex = -1;
               {
                  const std::vector<const C_OscNodeDataPool *> c_CommDps =
                     C_PuiSdHandler::h_GetInstance()->GetOscCanDataPools(ou32_NodeIndex, e_Protocol);
                  if (c_CommDps.empty() == true)
                  {
                     const int32_t s32_Add = C_PuiSdHandler::h_GetInstance()->AddAutoGenCommDataPool(
                        ou32_NodeIndex, e_Protocol);
                     if (s32_Add != C_NO_ERR)
                     {
                        orc_ErrorMessage = static_cast<QString>("Could not auto-create a %1 COMM datapool "
                                                                   "on the node.").arg(
                           C_OscNodeCommFiler::h_CommunicationProtocolToString(e_Protocol).c_str());
                        s32_Retval = C_CONFIG;
                     }
                  }
               }

               if (s32_Retval == C_NO_ERR)
               {
                  for (uint32_t u32_It = 0U; u32_It < pc_Node->c_DataPools.size(); ++u32_It)
                  {
                     if (pc_Node->c_DataPools[u32_It].e_Type == C_OscNodeDataPool::eCOM)
                     {
                        if (C_PuiSdUtil::h_GetRelatedCanProtocolType(ou32_NodeIndex, u32_It) == e_Protocol)
                        {
                           s32_DatapoolIndex = static_cast<int32_t>(u32_It);
                           break;
                        }
                     }
                  }
                  if (s32_DatapoolIndex < 0)
                  {
                     orc_ErrorMessage = static_cast<QString>("Could not locate a %1 COMM datapool "
                                                                "after auto-creation.").arg(
                        C_OscNodeCommFiler::h_CommunicationProtocolToString(e_Protocol).c_str());
                     s32_Retval = C_CONFIG;
                  }
               }

               if (s32_Retval == C_NO_ERR)
               {
                  // Step 3: build the assignment and call the existing converter.
                  // The user's stated convention is "one DBC file = one device interface,"
                  // so we take the first DBC node and ignore the rest.
                  C_CieImportDataAssignment c_Assignment;
                  c_Assignment.u32_OsyNodeIndex = ou32_NodeIndex;
                  c_Assignment.u32_OsyInterfaceIndex = ou32_InterfaceIndex;
                  c_Assignment.c_ImportData =
                     C_CieDataPoolListAdapter::h_GetStructureFromDbcFileImport(c_CommDef.c_Nodes[0]);
                  c_Assignment.s32_DatapoolIndexForNew = s32_DatapoolIndex;
                  // Populate per-message override indices so re-syncing replaces existing same-ID
                  // messages instead of creating duplicates. Match is by (CanId, IsExtended); name
                  // is ignored intentionally — see mh_FindOverrideTarget.
                  uint32_t u32_NewCount = 0U;
                  for (uint32_t u32_It = 0U;
                       u32_It < c_Assignment.c_ImportData.c_Core.c_OscTxMessageData.size(); ++u32_It)
                  {
                     const std::pair<int32_t, int32_t> c_OverrideInfo = mh_FindOverrideTarget(
                        *pc_Node, ou32_InterfaceIndex, e_Protocol, true,
                        c_Assignment.c_ImportData.c_Core.c_OscTxMessageData[u32_It]);
                     c_Assignment.c_TxMessageOverrideIndices.push_back(c_OverrideInfo);
                     if (c_OverrideInfo.first < 0)
                     {
                        ++u32_NewCount;
                     }
                  }
                  for (uint32_t u32_It = 0U;
                       u32_It < c_Assignment.c_ImportData.c_Core.c_OscRxMessageData.size(); ++u32_It)
                  {
                     const std::pair<int32_t, int32_t> c_OverrideInfo = mh_FindOverrideTarget(
                        *pc_Node, ou32_InterfaceIndex, e_Protocol, false,
                        c_Assignment.c_ImportData.c_Core.c_OscRxMessageData[u32_It]);
                     c_Assignment.c_RxMessageOverrideIndices.push_back(c_OverrideInfo);
                     if (c_OverrideInfo.first < 0)
                     {
                        ++u32_NewCount;
                     }
                  }
                  c_Assignment.u32_NewMessageCount = u32_NewCount;

                  std::vector<C_CieImportDataAssignment> c_Assignments;
                  c_Assignments.push_back(c_Assignment);

                  C_CieUtil::h_AdaptImportMessages(c_Assignments, e_Protocol, false);
                  // oq_UniqueAddRequested = false: respect the override indices so existing same-ID
                  // messages get updated in place rather than left untouched.
                  C_CieUtil::h_InsertMessages(c_Assignments, e_Protocol, false);

                  // Step 4: stamp the per-interface fingerprints. We record both the DBC file
                  // hash AND the project-message hash so the state detector can tell DBC drift
                  // and project drift apart on the next reload.
                  const QString c_DbcHash = h_ComputeFileSha256(c_DbcPath);
                  if (c_DbcHash.isEmpty() == true)
                  {
                     orc_ErrorMessage = static_cast<QString>("Messages imported, but the DBC could not be re-read "
                                                                "to record its fingerprint: %1").arg(c_DbcPath);
                     s32_Retval = C_RD_WR;
                  }
                  else
                  {
                     rc_Interface.c_LastSyncedDbcSha256 = c_DbcHash.toStdString().c_str();
                     const QString c_ProjectHash = h_ComputeProjectMessagesHash(ou32_NodeIndex, ou32_InterfaceIndex);
                     rc_Interface.c_LastSyncedProjectMsgHash = c_ProjectHash.toStdString().c_str();
                  }
               }
            }
         }
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Push the project's per-interface CAN messages back out to the device's DBC file.

   Builds a single-node `C_CieCommDefinition` from the project's existing TX/RX messages
   on the (node, interface) pair, then calls `C_CieExportDbc::h_ExportNetwork` to write
   it to the device's `<device_name>_CAN<n>.dbc` file (overwriting any previous content).
   Both fingerprints are stamped on success so the next reload sees this as the new
   in-sync baseline.

   v1 LIMITATIONS (mirror Pull):
   - Protocol type is hard-coded to `C_OscCanProtocol::eLAYER2`. Pushing from a node
     that has messages on a different protocol's COMM datapool will silently miss them.
   - The exported DBC carries one node named after the openSYDE device. Multi-node DBC
     export (representing other peers on the bus) is not in scope for the per-interface
     sync model.

   \param[in]   ou32_NodeIndex      Index of the node in the system definition
   \param[in]   ou32_InterfaceIndex Index of the node's CAN interface
   \param[out]  orc_ErrorMessage    User-facing error message on failure; empty on success

   \return
   C_NO_ERR    push completed; DBC written and fingerprints stamped
   C_RANGE     node/interface index out of range, or interface is not CAN
   C_CONFIG    node has no device definition / no Layer 2 COMM datapool /
               interface not connected to a bus
   C_RD_WR     DBC file could not be written or re-read for fingerprinting
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SdNdeDbcSync::h_PushInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                        QString & orc_ErrorMessage)
{
   int32_t s32_Retval = C_NO_ERR;

   orc_ErrorMessage = "";

   C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNode(ou32_NodeIndex);

   if (pc_Node == NULL)
   {
      orc_ErrorMessage = static_cast<QString>("Node index %1 out of range.").arg(
         ou32_NodeIndex);
      s32_Retval = C_RANGE;
   }
   else if (pc_Node->pc_DeviceDefinition == NULL)
   {
      orc_ErrorMessage = "Node has no associated device definition.";
      s32_Retval = C_CONFIG;
   }
   else if (ou32_InterfaceIndex >= pc_Node->c_Properties.c_ComInterfaces.size())
   {
      orc_ErrorMessage = static_cast<QString>("Interface index %1 out of range.").arg(
         ou32_InterfaceIndex);
      s32_Retval = C_RANGE;
   }
   else
   {
      C_OscNodeComInterfaceSettings & rc_Interface = pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex];

      if (rc_Interface.e_InterfaceType != C_OscSystemBus::eCAN)
      {
         orc_ErrorMessage = "Interface is not a CAN interface.";
         s32_Retval = C_RANGE;
      }
      else if (rc_Interface.GetBusConnected() == false)
      {
         orc_ErrorMessage = "Interface is not connected to a bus.";
         s32_Retval = C_CONFIG;
      }
      else
      {
         const QString c_DbcPath = h_GetExpectedDbcPath(ou32_NodeIndex, ou32_InterfaceIndex);
         if (c_DbcPath.isEmpty())
         {
            orc_ErrorMessage = "Could not derive a DBC path for this interface.";
            s32_Retval = C_RANGE;
         }
         else
         {
            // Per-interface project setting; defaults to Layer 2.
            const C_OscCanProtocol::E_Type e_Protocol = rc_Interface.e_DbcProtocol;

            int32_t s32_DatapoolIndex = -1;
            for (uint32_t u32_It = 0U; u32_It < pc_Node->c_DataPools.size(); ++u32_It)
            {
               if ((pc_Node->c_DataPools[u32_It].e_Type == C_OscNodeDataPool::eCOM) &&
                   (C_PuiSdUtil::h_GetRelatedCanProtocolType(ou32_NodeIndex, u32_It) == e_Protocol))
               {
                  s32_DatapoolIndex = static_cast<int32_t>(u32_It);
                  break;
               }
            }

            if (s32_DatapoolIndex < 0)
            {
               orc_ErrorMessage = static_cast<QString>("Node has no %1 COMM datapool to push from.").arg(
                  C_OscNodeCommFiler::h_CommunicationProtocolToString(e_Protocol).c_str());
               s32_Retval = C_CONFIG;
            }
            else
            {
               const C_OscCanMessageContainer * const pc_Container =
                  C_PuiSdHandler::h_GetInstance()->GetCanProtocolMessageContainer(
                     ou32_NodeIndex, e_Protocol, ou32_InterfaceIndex,
                     static_cast<uint32_t>(s32_DatapoolIndex));

               if (pc_Container == NULL)
               {
                  orc_ErrorMessage = "Could not access the per-interface message container.";
                  s32_Retval = C_CONFIG;
               }
               else
               {
                  // Build a one-node CieCommDefinition from the project's messages.
                  C_CieConverter::C_CieCommDefinition c_CommDef;
                  const C_OscSystemBus * const pc_Bus =
                     C_PuiSdHandler::h_GetInstance()->GetOscBus(rc_Interface.u32_BusIndex);
                  if (pc_Bus != NULL)
                  {
                     c_CommDef.c_Bus.c_Name = pc_Bus->c_Name;
                     c_CommDef.c_Bus.c_Comment = pc_Bus->c_Comment;
                  }

                  C_CieConverter::C_CieNode c_CieNode;
                  c_CieNode.c_Properties.c_Name = pc_Node->pc_DeviceDefinition->c_DeviceName;
                  c_CieNode.c_Properties.c_Comment = pc_Node->c_Properties.c_Comment;

                  stw::scl::std::vector<std::string> c_ConvertWarnings;

                  const std::vector<C_OscCanMessage> & rc_TxMsgs = pc_Container->GetMessagesConst(true);
                  for (uint32_t u32_It = 0U; u32_It < rc_TxMsgs.size(); ++u32_It)
                  {
                     C_CieConverter::C_CieNodeMessage c_Cnv;
                     C_CieDataPoolListAdapter::h_ConvertToDbcImportMessage(
                        rc_Interface.u32_BusIndex, e_Protocol, rc_TxMsgs[u32_It], c_Cnv, c_ConvertWarnings);
                     c_CieNode.c_TxMessages.push_back(c_Cnv);
                  }
                  const std::vector<C_OscCanMessage> & rc_RxMsgs = pc_Container->GetMessagesConst(false);
                  for (uint32_t u32_It = 0U; u32_It < rc_RxMsgs.size(); ++u32_It)
                  {
                     C_CieConverter::C_CieNodeMessage c_Cnv;
                     C_CieDataPoolListAdapter::h_ConvertToDbcImportMessage(
                        rc_Interface.u32_BusIndex, e_Protocol, rc_RxMsgs[u32_It], c_Cnv, c_ConvertWarnings);
                     c_CieNode.c_RxMessages.push_back(c_Cnv);
                  }
                  c_CommDef.c_Nodes.push_back(c_CieNode);

                  // Write the DBC.
                  stw::scl::std::vector<std::string> c_ExportWarnings;
                  std::string c_ExportError;
                  const int32_t s32_Export = C_CieExportDbc::h_ExportNetwork(
                     c_DbcPath.toStdString().c_str(), c_CommDef, c_ExportWarnings, c_ExportError);

                  if ((s32_Export != C_NO_ERR) && (s32_Export != C_WARN))
                  {
                     orc_ErrorMessage = static_cast<QString>("Could not write DBC %1: %2").arg(
                        c_DbcPath, c_ExportError.c_str());
                     s32_Retval = C_RD_WR;
                  }
                  else
                  {
                     // Stamp both fingerprints from the freshly-written file + project state.
                     const QString c_DbcHash = h_ComputeFileSha256(c_DbcPath);
                     if (c_DbcHash.isEmpty() == true)
                     {
                        orc_ErrorMessage = static_cast<QString>("DBC written, but the file could not be re-read "
                                                                   "to record its fingerprint: %1").arg(c_DbcPath);
                        s32_Retval = C_RD_WR;
                     }
                     else
                     {
                        rc_Interface.c_LastSyncedDbcSha256 = c_DbcHash.toStdString().c_str();
                        const QString c_ProjectHash = h_ComputeProjectMessagesHash(ou32_NodeIndex,
                                                                                   ou32_InterfaceIndex);
                        rc_Interface.c_LastSyncedProjectMsgHash = c_ProjectHash.toStdString().c_str();
                     }
                  }
               }
            }
         }
      }
   }

   return s32_Retval;
}
