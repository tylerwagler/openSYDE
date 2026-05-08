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
#include <QFile>
#include <QFileInfo>

#include "stwerrors.hpp"
#include "C_GtGetText.hpp"
#include "C_OscNode.hpp"
#include "C_OscSystemBus.hpp"
#include "C_PuiSdHandler.hpp"
#include "C_SdNdeDbcSync.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

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
/*! \brief  Sync the per-interface DBC for the given (node, interface) pair.

   Validates that:
   - the node has an associated device definition
   - the interface exists, is a CAN interface, and is connected to a bus
   - the expected DBC file (`<device_name>_CAN<n>.dbc` next to the manifest) exists
     and is readable

   On success: computes the SHA-256 of the DBC file and writes it to the project's
   per-interface settings (`c_LastSyncedDbcSha256`). Returns C_NO_ERR.

   v1 SCAFFOLD: actual message-import-into-bus is not yet wired. This function
   currently performs only the validation + fingerprint update steps. The sync
   button will reflect the new fingerprint immediately, and on next project load
   the out-of-sync detector will recognize the file as unchanged. Adding messages
   to the bus's COMM datapool is the next follow-up.

   \param[in]   ou32_NodeIndex      Index of the node in the system definition
   \param[in]   ou32_InterfaceIndex Index of the node's CAN interface (per-node, mixed types)
   \param[out]  orc_ErrorMessage    User-facing error message on failure; empty on success

   \return
   C_NO_ERR    sync completed; fingerprint stored
   C_RANGE     node/interface index out of range, or interface is not CAN
   C_CONFIG    node has no device definition, or interface not connected to a bus
   C_RD_WR     DBC file does not exist or could not be read
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SdNdeDbcSync::h_SyncInterface(const uint32_t ou32_NodeIndex, const uint32_t ou32_InterfaceIndex,
                                        QString & orc_ErrorMessage)
{
   int32_t s32_Retval = C_NO_ERR;

   orc_ErrorMessage = "";

   C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNode(ou32_NodeIndex);

   if (pc_Node == NULL)
   {
      orc_ErrorMessage = static_cast<QString>(C_GtGetText::h_GetText("Node index %1 out of range.")).arg(
         ou32_NodeIndex);
      s32_Retval = C_RANGE;
   }
   else if (pc_Node->pc_DeviceDefinition == NULL)
   {
      orc_ErrorMessage = C_GtGetText::h_GetText("Node has no associated device definition.");
      s32_Retval = C_CONFIG;
   }
   else if (ou32_InterfaceIndex >= pc_Node->c_Properties.c_ComInterfaces.size())
   {
      orc_ErrorMessage = static_cast<QString>(C_GtGetText::h_GetText("Interface index %1 out of range.")).arg(
         ou32_InterfaceIndex);
      s32_Retval = C_RANGE;
   }
   else
   {
      C_OscNodeComInterfaceSettings & rc_Interface = pc_Node->c_Properties.c_ComInterfaces[ou32_InterfaceIndex];

      if (rc_Interface.e_InterfaceType != C_OscSystemBus::eCAN)
      {
         orc_ErrorMessage = C_GtGetText::h_GetText("Interface is not a CAN interface.");
         s32_Retval = C_RANGE;
      }
      else if (rc_Interface.GetBusConnected() == false)
      {
         orc_ErrorMessage = C_GtGetText::h_GetText("Interface is not connected to a bus.");
         s32_Retval = C_CONFIG;
      }
      else
      {
         const QString c_DbcPath = h_GetExpectedDbcPath(ou32_NodeIndex, ou32_InterfaceIndex);
         if (c_DbcPath.isEmpty() || (QFile::exists(c_DbcPath) == false))
         {
            orc_ErrorMessage = static_cast<QString>(C_GtGetText::h_GetText(
                                                       "Expected DBC file not found: %1")).arg(c_DbcPath);
            s32_Retval = C_RD_WR;
         }
         else
         {
            const QString c_Hash = h_ComputeFileSha256(c_DbcPath);
            if (c_Hash.isEmpty() == true)
            {
               orc_ErrorMessage = static_cast<QString>(C_GtGetText::h_GetText(
                                                          "Could not read DBC file: %1")).arg(c_DbcPath);
               s32_Retval = C_RD_WR;
            }
            else
            {
               // TODO(task 15b): parse the DBC via C_CieImportDbc::h_ImportNetwork, build a
               //                 C_CieImportDataAssignment, and call C_CieUtil::h_InsertMessages
               //                 to materialize the device's messages onto the connected bus's
               //                 COMM datapool. For now we only update the fingerprint.
               rc_Interface.c_LastSyncedDbcSha256 = c_Hash.toStdString().c_str();
            }
         }
      }
   }

   return s32_Retval;
}
