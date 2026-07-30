//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for simple device configuration sequence

    Simple means beside others: CAN only, openSYDE flashloader only, no routing ...

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "TglTime.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscDcBasicSequences.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
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
C_OscDcBasicSequences::C_OscDcBasicSequences(void) :
   mpc_CanDispatcher(NULL),
   mpc_IpDispatcher(NULL),
   mpc_TpCan(NULL),
   mpc_TpIp(NULL)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDcBasicSequences::~C_OscDcBasicSequences()
{
   this->mpc_CanDispatcher = NULL; //do not delete ! not owned by us
   this->mpc_IpDispatcher = NULL;  //do not delete ! not owned by us

   if (this->mpc_TpCan != NULL)
   {
      delete this->mpc_TpCan;
      this->mpc_TpCan = NULL;
   }
   if (this->mpc_TpIp != NULL)
   {
      delete this->mpc_TpIp;
      this->mpc_TpIp = NULL;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize transport protocol and openSYDE protocol driver.

   CAN and Ethernet variant. Only one of the dispatchers can be used, not both at the same time.

   \param[in]  opc_CanDispatcher Pointer to concrete CAN dispatcher
   \param[in]  opc_IpDispatcher  Pointer to concrete IP dispatcher

   \return
   C_NO_ERR    everything ok
   else        error occurred, see log file for details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::Init(stw::can::C_CanDispatcher * const opc_CanDispatcher,
                                    C_OscIpDispatcher * const opc_IpDispatcher)
{
   int32_t s32_Return = C_NO_ERR;

   const C_SclString c_LogActivity = "Initialization";

   m_ReportProgress(s32_Return, "Starting the initialization of CAN driver and protocol ... ");

   this->mpc_CanDispatcher = opc_CanDispatcher;
   this->mpc_IpDispatcher = opc_IpDispatcher;

   if ((this->mpc_CanDispatcher == NULL) && (this->mpc_IpDispatcher == NULL))
   {
      s32_Return = C_COM;
      osc_write_log_error(c_LogActivity, "Could not use CAN or IP! Both dispatchers are invalid.");
   }
   else if ((this->mpc_CanDispatcher != NULL) && (this->mpc_IpDispatcher != NULL))
   {
      s32_Return = C_COM;
      osc_write_log_error(c_LogActivity, "Can only use CAN or IP! Both dispatchers are valid.");
   }
   else
   {
      // Nothing to do
   }

   if (s32_Return == C_NO_ERR)
   {
      // CAN usage
      if (this->mpc_CanDispatcher != NULL)
      {
         mpc_TpCan = new C_OscProtocolDriverOsyTpCan();
         s32_Return = mpc_TpCan->SetDispatcher(this->mpc_CanDispatcher);
         if (s32_Return != C_NO_ERR)
         {
            osc_write_log_error(c_LogActivity, "Setting CAN dispatcher for CAN transport protocol failed!");
         }
         else
         {
            s32_Return = mc_OsyProtocol.SetTransportProtocol(mpc_TpCan);
            if (s32_Return != C_NO_ERR)
            {
               osc_write_log_error(c_LogActivity,
                                   "Setting CAN transport protocol to the openSYDE protocol driver failed!");
            }
         }
      }
      else
      {
         // IP usage
         //Broadcast
         s32_Return = mpc_IpDispatcher->InitUdp();

         if (s32_Return == C_NO_ERR)
         {
            mpc_TpIp = new C_OscProtocolDriverOsyTpIp();

            s32_Return = mpc_TpIp->SetDispatcher(this->mpc_IpDispatcher, 0U);
            if (s32_Return != C_NO_ERR)
            {
               osc_write_log_error(c_LogActivity, "Setting IP dispatcher for IP transport protocol failed!");
            }
            else
            {
               s32_Return = mc_OsyProtocol.SetTransportProtocol(mpc_TpIp);
               if (s32_Return != C_NO_ERR)
               {
                  osc_write_log_error(c_LogActivity,
                                      "Setting IP transport protocol to the openSYDE protocol driver failed!");
               }
            }
         }
         else
         {
            osc_write_log_error(c_LogActivity, "Could not initialize UDP. Error Code: " +
                                C_SclString::IntToStr(s32_Return));
            s32_Return = C_COM;
         }
      }
   }

   if (s32_Return == C_NO_ERR)
   {
      C_OscProtocolDriverOsyNode c_Client;
      c_Client.u8_NodeIdentifier = 126;
      c_Client.u8_BusIdentifier = 0U;

      if (this->mpc_TpCan != NULL)
      {
         s32_Return = mpc_TpCan->SetNodeIdentifiersForBroadcasts(c_Client);
      }

      if (s32_Return != C_NO_ERR)
      {
         osc_write_log_warning(c_LogActivity, "Could not configure the clients broadcast node ID.");
      }
   }

   m_ReportProgress(s32_Return, "Initialization finished.");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Bring openSYDE devices in flashloader state

   Sequence:
   * send openSYDE CAN-TP broadcast "RequestProgramming"
   * send openSYDE broadcast "EcuReset"
   * send openSYDE "DiagnosticSessionControl(PreProgramming)" broadcast for 5 seconds every 5 milliseconds

   The com driver is expected to be initialized for broadcasting as it is done in Init().

   \param[in]  ou32_FlashloaderResetWaitTime    Flashloader reset wait time

   \return
   C_NO_ERR   Sequence finished
   else       error occurred
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::ScanEnterFlashloader(const uint32_t ou32_FlashloaderResetWaitTime)
{
   int32_t s32_Return = C_NO_ERR;
   const C_SclString c_LogActivity = "Scan Activate Flashloader";
   const uint32_t u32_SCAN_TIME_MS = 5000U;
   uint32_t u32_WaitTime = ou32_FlashloaderResetWaitTime;

   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastRequestProgrammingResults> c_Results;

   m_ReportProgress(s32_Return, "Starting the scan for flashloader activation ... ");

   if (u32_WaitTime < u32_SCAN_TIME_MS)
   {
      // The scan time is necessary for the manual triggering of the nodes
      u32_WaitTime = u32_SCAN_TIME_MS;
   }

   s32_Return = m_BroadcastRequestProgramming(c_Results);

   m_ReportProgress(C_NO_ERR, "Broadcasting \"request programming\" flag: " +
                    C_SclString::IntToStr(c_Results.size()) + " device(s) answered. ");

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error(c_LogActivity,
                          "openSYDE ECU reset broadcast failed with error: " + C_SclString::IntToStr(s32_Return));
   }
   else
   {
      // check results for request accepted flags
      for (uint32_t u32_ResponseIndex = 0U; u32_ResponseIndex < c_Results.size(); u32_ResponseIndex++)
      {
         if (c_Results[u32_ResponseIndex].q_RequestAccepted == false)
         {
            s32_Return = C_COM;
            break;
         }
      }
   }

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error(c_LogActivity,
                          "At least one reached device did not accept setting the \"request programming\" flag.");
   }
   else
   {
      //broadcast "ResetToFlashloader"
      s32_Return =
         m_BroadcastEcuReset(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER);

      if (s32_Return != C_NO_ERR)
      {
         osc_write_log_error(c_LogActivity,
                             "openSYDE request node reset failed with error: " + C_SclString::IntToStr(s32_Return));
      }
   }

   // Always continue with broadcast. If previous steps did not work, user can do the manual reset while we
   // broadcast.
   const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();

   if (this->mpc_CanDispatcher != NULL)
   {
      // If no devices answered, give hint about "reset your device NOW"
      if (c_Results.size() == 0)
      {
         C_SclString c_Text;
         c_Text.PrintFormatted("You now have %u seconds time to turn on your target device ...",
                               u32_WaitTime / 1000);
         m_ReportProgress(C_WARN, c_Text);
      }

      do
      {
         // openSYDE "DiagnosticSessionControl(PreProgramming)" broadcast
         s32_Return = m_BroadcastSendEnterPreProgrammingSession();
         if (s32_Return != C_NO_ERR)
         {
            osc_write_log_error(c_LogActivity,
                                "Sending broadcast to enter preprogramming session failed with result " +
                                C_SclString::IntToStr(s32_Return));

            s32_Return = C_COM;
         }

         if (s32_Return != C_NO_ERR)
         {
            break;
         }

         TglSleep(5);
      }
      while (TglGetTickCount() < (u32_WaitTime + u32_StartTime));

      //Previous broadcasts might have caused responses placed in the receive queues of the device
      // specific driver instances. Dump them.
      (void)this->mpc_CanDispatcher->DispatchIncoming();
   }
   else if (this->mpc_IpDispatcher != NULL)
   {
      // In case of IP: Just wait for the specified time, then try to connect and bring node to preprogramming
      // session.
      TglSleep(u32_WaitTime);
   }
   else
   {
      // Nothing to do. Should not happen.
   }

   m_ClearTpDispatcherQueue();

   m_ReportProgress(s32_Return, "Scan for flashloader activation finished. ");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Scan for connected openSYDE devices

   Assumptions:
   * All devices are in flashloader mode.
   * The com driver is initialized and has set up for broadcasting as it is done in Init()

   Sequence:
   * Broadcast "ReadSerialNumber"
   * For all nodes that have an unique ID: directed read device name

   \return
   C_NO_ERR    everything ok
   else        error occurred, see log file for details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::ScanGetInfo(void)
{
   int32_t s32_Return = C_NO_ERR;
   const C_SclString c_LogActivity = "Scan Device Info";

   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> c_ReadSnResult;
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> c_ReadSnResultExt;
   std::vector<stw::scl::C_SclString> c_DeviceNames;

   m_ReportProgress(s32_Return, "Starting the scan for getting devices information ... ");

   // broadcast: "ReadSerialNumber"
   s32_Return = m_BroadcastReadSerialNumber(c_ReadSnResult, c_ReadSnResultExt, c_DeviceNames);

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error(c_LogActivity, "openSYDE serial number broadcast failed with error: " +
                          C_SclString::IntToStr(s32_Return));
   }
   else
   {
      uint32_t u32_ResultCounter;

      std::vector<uint32_t> c_UniqueIdIndices;
      std::vector<C_OscDcDeviceInformation> c_DeviceInfoResult;
      bool q_SecurityFeatureUsed = false;

      if (this->mpc_IpDispatcher != NULL)
      {
         // In case of IP we expect the device names to be available from the broadcast results. Check if we have as
         // many device names as results
         tgl_assert(c_DeviceNames.size() == (c_ReadSnResult.size() + c_ReadSnResultExt.size()));
      }

      osc_write_log_info("Scan CAN for openSYDE devices",
                         "Sequence finished. Standard nodes found: " + C_SclString::IntToStr(c_ReadSnResult.size()));
      osc_write_log_info("Scan CAN for openSYDE devices",
                         "Sequence finished. Extended nodes found: " + C_SclString::IntToStr(
                            c_ReadSnResultExt.size()));

      // Fill the result container with the standard results
      for (u32_ResultCounter = 0U; u32_ResultCounter < c_ReadSnResult.size(); ++u32_ResultCounter)
      {
         C_OscDcDeviceInformation c_DeviceInfo;
         c_DeviceInfo.SetSerialNumber(c_ReadSnResult[u32_ResultCounter].c_SerialNumber);
         c_DeviceInfo.SetNodeId(c_ReadSnResult[u32_ResultCounter].c_SenderId.u8_NodeIdentifier);
         if (this->mpc_IpDispatcher != NULL)
         {
            c_DeviceInfo.SetDeviceName(c_DeviceNames[u32_ResultCounter]);
         }

         c_DeviceInfoResult.push_back(c_DeviceInfo);
      }

      // Fill the result container with the extended results
      for (u32_ResultCounter = 0U; u32_ResultCounter < c_ReadSnResultExt.size(); ++u32_ResultCounter)
      {
         C_OscDcDeviceInformation c_DeviceInfo;
         // Serial number length is not necessary, it was already cross checked with the string
         c_DeviceInfo.SetSerialNumber(c_ReadSnResultExt[u32_ResultCounter].c_SerialNumber);
         c_DeviceInfo.SetNodeId(c_ReadSnResultExt[u32_ResultCounter].c_SenderId.u8_NodeIdentifier);
         c_DeviceInfo.SetExtendedInfo(c_ReadSnResultExt[u32_ResultCounter].u8_SubNodeId,
                                      c_ReadSnResultExt[u32_ResultCounter].q_SecurityActivated);
         //lint -e{514}  Using operator with a bool value was intended and is no accident
         q_SecurityFeatureUsed |= c_ReadSnResultExt[u32_ResultCounter].q_SecurityActivated;
         if (this->mpc_IpDispatcher != NULL)
         {
            c_DeviceInfo.SetDeviceName(c_DeviceNames[c_ReadSnResult.size() + u32_ResultCounter]);
         }
         c_DeviceInfoResult.push_back(c_DeviceInfo);
      }

      // Search all unique ID's
      for (u32_ResultCounter = 0U; u32_ResultCounter < c_DeviceInfoResult.size(); ++u32_ResultCounter)
      {
         bool q_UniqueId = true;

         for (uint32_t u32_UniqueIdCheckCounter = 0U;
              u32_UniqueIdCheckCounter < c_DeviceInfoResult.size();
              ++u32_UniqueIdCheckCounter)
         {
            if ((u32_ResultCounter != u32_UniqueIdCheckCounter) &&
                (c_DeviceInfoResult[u32_ResultCounter].u8_NodeId ==
                 c_DeviceInfoResult[u32_UniqueIdCheckCounter].u8_NodeId))
            {
               q_UniqueId = false;
               if (q_SecurityFeatureUsed == true)
               {
                  // Special case: In case of at least one node with active security no broadcasts
                  // can be used by the configuration sequence. Therefore the node IDs must be unique for
                  // using direct communication.
                  s32_Return = C_CHECKSUM;

                  osc_write_log_error(c_LogActivity,
                                      "At least one node has the security feature activated and at least"
                                      " one node ID is not unique. Node ID: "  +
                                      C_SclString::IntToStr(c_DeviceInfoResult[u32_ResultCounter].u8_NodeId));
               }
               break;
            }
         }
         if (q_UniqueId == true)
         {
            c_UniqueIdIndices.push_back(u32_ResultCounter);
         }
      }

      tgl_assert((c_ReadSnResult.size() + c_ReadSnResultExt.size()) == c_DeviceInfoResult.size());

      if ((s32_Return == C_NO_ERR) &&
          (this->mpc_CanDispatcher != NULL)) // Only try to read the device name if we do not have them already from the
                                             // IP dispatcher results and if we have a CAN dispatcher to read with
      {
         C_OscProtocolDriverOsyNode c_CurSenderId;
         C_SclString c_Result;
         uint8_t u8_NumberCode = 0;
         uint32_t u32_UniqueIdIndicesCounter;
         C_OscProtocolDriverOsyNode c_Client;

         c_Client.u8_NodeIdentifier = 126;
         c_Client.u8_BusIdentifier = 0U;

         // Get the device names for all devices with unique ids
         for (u32_UniqueIdIndicesCounter = 0U;
              u32_UniqueIdIndicesCounter < c_UniqueIdIndices.size();
              ++u32_UniqueIdIndicesCounter)
         {
            // Get the correct sender id
            const uint32_t u32_DeviceInfoIndex = c_UniqueIdIndices[u32_UniqueIdIndicesCounter];

            // The first indexes are always the standard SNR results
            if (u32_DeviceInfoIndex < c_ReadSnResult.size())
            {
               c_CurSenderId = c_ReadSnResult[u32_DeviceInfoIndex].c_SenderId;
            }
            else
            {
               const uint32_t u32_ReadSnrResultExtIndex =
                  u32_DeviceInfoIndex - static_cast<uint32_t>(c_ReadSnResult.size());
               // all above must be the extended SNR results
               tgl_assert(u32_ReadSnrResultExtIndex < c_ReadSnResultExt.size());
               c_CurSenderId = c_ReadSnResultExt[u32_ReadSnrResultExtIndex].c_SenderId;
            }

            // set up temporary node IDs
            s32_Return = mc_OsyProtocol.SetNodeIdentifiers(c_Client, c_CurSenderId);

            if (s32_Return != C_NO_ERR)
            {
               osc_write_log_error(c_LogActivity,
                                   "Could not configure the node IDs! Is the server node ID within range?");
            }
            else
            {
               s32_Return = mc_OsyProtocol.OsyReadDeviceName(c_Result, &u8_NumberCode);

               if (s32_Return != C_NO_ERR)
               {
                  osc_write_log_error(c_LogActivity, "Could not read the device's device name! Details: " +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(s32_Return,
                                                                                               u8_NumberCode));
               }
               else
               {
                  c_DeviceInfoResult[u32_DeviceInfoIndex].SetDeviceName(c_Result);
               }
            }
         }
      }

      m_ReportProgress(s32_Return, "Found " + C_SclString::IntToStr(c_DeviceInfoResult.size()) + " device(s).");
      m_ReportDevicesInfoRead(c_DeviceInfoResult, q_SecurityFeatureUsed);
   }

   m_ReportProgress(s32_Return, "Scan for getting devices information finished.");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast to reset devices

   Function to get all devices out of flashloader after calls to ScanEnterFlashloader() and ScanGetInfo().
   The com driver is expected to be initialized for broadcasting as it is done in Init().

   \return
   C_NO_ERR    everything ok
   else        error occurred, see log file for details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::ResetSystem(void)
{
   int32_t s32_Return = C_NO_ERR;

   m_ReportProgress(s32_Return, "Starting system reset broadcast...");

   s32_Return = m_BroadcastEcuReset(C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON);

   m_ReportProgress(s32_Return, "System reset broadcast finished.");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Configure device

  \param[in]   ou8_CurrentNodeId   Node ID that the Node we want to configure currently has
  \param[in]   ou8_NewNodeId       Node ID we want to configure to Node with ou8_CurrentNodeId
  \param[in]   ou32_Bitrate        Bitrate we want to configure to Node with ou8_CurrentNodeId
  \param[in]   ou8_InterfaceIndex  Interface the target is connected

   \return
   C_NO_ERR    everything ok
   else        error occurred, see log file for details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::ConfigureDevice(const uint8_t ou8_CurrentNodeId, const uint8_t ou8_NewNodeId,
                                               const uint32_t ou32_Bitrate, const uint8_t ou8_InterfaceIndex)
{
   int32_t s32_Return = C_NO_ERR;
   const C_OscProtocolDriverOsyNode c_ClientId(0, 126);
   const C_OscProtocolDriverOsyNode c_CurrentServerId(0, ou8_CurrentNodeId);

   m_ReportProgress(s32_Return, "Starting device configuration...");

   //set up protocol for communication (which node we want to configure)
   s32_Return = mc_OsyProtocol.SetNodeIdentifiers(c_ClientId, c_CurrentServerId);
   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error("ProtocolSetup",
                          "Could not configure Node IDs for transport protocol. Are the IDs in range?");
   }
   else
   {
      uint8_t u8_Nrc;
      s32_Return = mc_OsyProtocol.OsyDiagnosticSessionControl(
         C_OscProtocolDriverOsy::hu8_DIAGNOSTIC_SESSION_PROGRAMMING,
         &u8_Nrc);

      if (s32_Return == C_NO_ERR)
      {
         //set security level 1; we want to change ID and Bitrate...
         const uint8_t u8_SECURITY_LEVEL = 1U;
         bool q_SecureMode;
         uint64_t u64_Seed;
         bool q_AuthenticationActive;
         bool q_TrafficEncryptionActive;
         std::vector<uint8_t> c_TrafficEncryptionInitVector;
         C_SclString c_LogActivity;

         c_LogActivity = "Security Access";
         s32_Return = mc_OsyProtocol.OsySecurityAccessRequestSeed(u8_SECURITY_LEVEL, q_SecureMode, u64_Seed,
                                                                  q_AuthenticationActive,
                                                                  q_TrafficEncryptionActive,
                                                                  c_TrafficEncryptionInitVector, &u8_Nrc);

         if (q_SecureMode == true)
         {
            osc_write_log_error(c_LogActivity, "SecurityAccess request reported that security is on. "
                                "No security support here. Use openSYDE GUI tool for this feature.");
            s32_Return = C_CONFIG;
         }
         else if (s32_Return != C_NO_ERR)
         {
            osc_write_log_error(c_LogActivity, "Did not get a security seed from the target device! Details: " +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(s32_Return, u8_Nrc));
         }
         else
         {
            //hard coded keys are the best keys
            const uint32_t u32_KEY = 23U; // fixed in UDS stack for non secure mode
            if (u64_Seed != 42U)
            {
               //Do not consider this an error: older server implementations could return a value of zero
               // to signal that the level was already unlocked. This is described as valid in the UDS standard
               // but not on the openSYDE protocol specification. In any case we need to ignore to stay compatible.
               const C_SclString c_Tmp =
                  "Received seed in non secure mode does not match the expected value, expected: 42, got " +
                  C_SclString::IntToStr(u64_Seed);
               osc_write_log_warning(c_LogActivity, c_Tmp.c_str());
            }

            s32_Return = mc_OsyProtocol.OsySecurityAccessSendKey(u8_SECURITY_LEVEL, u32_KEY, &u8_Nrc);
            if (s32_Return != C_NO_ERR)
            {
               osc_write_log_error(c_LogActivity, "The target device did not access the security key! Details: " +
                                   C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(s32_Return, u8_Nrc));
            }
         }
      }
      //now we can start setting the Node ID and Bitrate
      if (s32_Return == C_NO_ERR)
      {
         const C_SclString c_LogActivity = "NodeConfiguration";
         const C_OscProtocolDriverOsyNode c_NewServerId(0, ou8_NewNodeId);
         C_SclString c_ProgressLogMsg = "";
         c_ProgressLogMsg.PrintFormatted(
            "Configuring Node ID \"%d\" to Node with current ID \"%d\" on Interface CAN %u.",
            ou8_NewNodeId, ou8_CurrentNodeId, ou8_InterfaceIndex + 1U);
         s32_Return = mc_OsyProtocol.OsySetNodeIdForChannel(0, ou8_InterfaceIndex, c_NewServerId, &u8_Nrc);
         m_ReportProgress(s32_Return, c_ProgressLogMsg);
         if (s32_Return != C_NO_ERR)
         {
            osc_write_log_error(c_LogActivity, "Could not set Node ID! Details: " +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(s32_Return, u8_Nrc));
         }
         else
         {
            c_ProgressLogMsg = "";
            c_ProgressLogMsg.PrintFormatted(
               "Configuring Bitrate %u kbit/s to Node on Interface CAN %u.",
               ou32_Bitrate, ou8_InterfaceIndex + 1U);
            s32_Return = mc_OsyProtocol.OsySetBitrate(0, ou8_InterfaceIndex, ou32_Bitrate * 1000U,
                                                      &u8_Nrc);
            m_ReportProgress(s32_Return, c_ProgressLogMsg);

            if (s32_Return != C_NO_ERR)
            {
               osc_write_log_error(c_LogActivity, "Could not set Bitrate! Details: " +
                                   C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(s32_Return, u8_Nrc));
            }
         }
      }
   }

   m_ReportProgress(s32_Return, "Device configuration finished.");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Configure device by serialnumber

  \param[in]   orc_SerialNumber    Serialnumber of node which should be configured
  \param[in]   ou8_NewNodeId       Node ID we want to configure the current used interface of Node with
                                   the orc_SerialNumber

   \return
   C_NO_ERR    everything ok
   else        error occurred, see log file for details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::ConfigureDeviceBySerialNumber(
   const stw::opensyde_core::C_OscProtocolSerialNumber & orc_SerialNumber, const uint8_t ou8_NewNodeId)
{
   int32_t s32_Return = C_NO_ERR;
   const C_SclString c_LogActivity = "Configure Device by Serialnumber";

   m_ReportProgress(s32_Return, "Starting device configuration...");

   // All openSYDE nodes must be in default session.
   // The broadcast SetNodeIdBySerialNumber works only in this session.
   s32_Return = m_BroadcastSendEnterDefaultSession();

   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error(c_LogActivity,
                          "Sending broadcast to enter default session failed with result " +
                          C_SclString::IntToStr(s32_Return));
   }
   else
   {
      C_OscProtocolDriverOsyNode c_ServerIdOfCurBus;
      C_SclString c_ProgressLogMsg = "";

      c_ServerIdOfCurBus.u8_NodeIdentifier = ou8_NewNodeId;
      c_ServerIdOfCurBus.u8_BusIdentifier = 0U; // bus id not supported here to be set concretely. Default: 0

      c_ProgressLogMsg.PrintFormatted(
         "Configuring Node ID \"%d\" to Node with Serialnumber \"%s\".",
         ou8_NewNodeId, orc_SerialNumber.GetSerialNumberAsFormattedString().c_str());

      m_ReportProgress(s32_Return, c_ProgressLogMsg);

      // Sending the broadcasts to configure the node id depending of the serialnumber
      // Two different broadcast services depending of the extended format
      if (orc_SerialNumber.q_ExtFormatUsed == false)
      {
         s32_Return = m_BroadcastSetNodeIdBySerialNumber(
            orc_SerialNumber,
            c_ServerIdOfCurBus);
      }
      else
      {
         s32_Return = m_BroadcastSetNodeIdBySerialNumberExtended(
            orc_SerialNumber,
            0, // sub node id not supported here. Default: 0
            c_ServerIdOfCurBus);
      }

      if (s32_Return != C_NO_ERR)
      {
         osc_write_log_error(c_LogActivity,
                             "Sending broadcasts to configure Node ID failed with result " +
                             C_SclString::IntToStr(s32_Return));
      }
   }

   m_ReportProgress(s32_Return, "Device configuration finished.");

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Utility function to convert device information of found devices to string

   \param[in]  orc_DeviceInfoResult    Device information results
   \param[in]  oq_SecurityFeatureUsed  Security feature used for at least one node
   \param[in]  oq_PrintDetailedSnInfo  Optional flag for printing more details about the found Serialnumbers

   \return
   String with information
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscDcBasicSequences::h_DevicesInfoToString(
   const std::vector<C_OscDcDeviceInformation> & orc_DeviceInfoResult, const bool oq_SecurityFeatureUsed,
   const bool oq_PrintDetailedSnInfo)
{
   C_SclString c_Information;

   c_Information = "Found " + C_SclString::IntToStr(orc_DeviceInfoResult.size()) + " device(s): \n";

   for (uint32_t u32_ItDevices = 0; u32_ItDevices < orc_DeviceInfoResult.size(); ++u32_ItDevices)
   {
      const C_OscDcDeviceInformation & rc_CurDevice = orc_DeviceInfoResult[u32_ItDevices];
      c_Information += " Device #" + C_SclString::IntToStr(u32_ItDevices + 1) + "\n";
      if (rc_CurDevice.q_DeviceNameValid == true)
      {
         c_Information += "   Device name: " + rc_CurDevice.c_DeviceName + "\n";
      }
      if (rc_CurDevice.q_NodeIdValid == true)
      {
         c_Information += "   Node ID: " + C_SclString::IntToStr(rc_CurDevice.u8_NodeId) + "\n";
      }
      if (rc_CurDevice.q_IpAddressValid == true)
      {
         c_Information += "   IP address: ";
         c_Information += C_SclString::IntToStr(rc_CurDevice.au8_IpAddress[0]) + ".";
         c_Information += C_SclString::IntToStr(rc_CurDevice.au8_IpAddress[1]) + ".";
         c_Information += C_SclString::IntToStr(rc_CurDevice.au8_IpAddress[2]) + ".";
         c_Information += C_SclString::IntToStr(rc_CurDevice.au8_IpAddress[3]) + "\n";
      }
      if (rc_CurDevice.c_SerialNumber.q_IsValid == true)
      {
         c_Information += "   Serial number: " + rc_CurDevice.c_SerialNumber.GetSerialNumberAsFormattedString() +
                          "\n";

         if (oq_PrintDetailedSnInfo == true)
         {
            const C_SclString c_SnExtFormat = ((rc_CurDevice.c_SerialNumber.q_ExtFormatUsed == true) ? "Yes" : "No");
            c_Information += "   Serial number extended format: " + c_SnExtFormat + "\n";
            c_Information += "   Serial number manufacturer format: " +
                             C_SclString::IntToStr(rc_CurDevice.c_SerialNumber.u8_SerialNumberManufacturerFormat)  +
                             "\n";
         }
      }
      if (rc_CurDevice.q_ExtendedInfoValid == true)
      {
         c_Information += "   Sub node ID: " + C_SclString::IntToStr(rc_CurDevice.u8_SubNodeId) + "\n";
         c_Information += "   Security activated: ";
         c_Information += rc_CurDevice.q_SecurityActivated ? "yes" : "no";
         c_Information += "\n";
      }
   }

   if (orc_DeviceInfoResult.size() > 0)
   {
      c_Information += " Security feature used for at least one device: ";
      c_Information += oq_SecurityFeatureUsed ? "yes" : "no";
      c_Information += "\n";
   }

   return c_Information;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Prepare for shutting down class

   To be called by child classes on shutdown, before they destroy all owned class instances
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDcBasicSequences::PrepareForDestruction(void)
{
   //we are about to destroy the dispatcher; make sure TP disconnects from it
   if (this->mpc_TpCan != NULL)
   {
      mpc_TpCan->SetDispatcher(NULL);
   }
   if (this->mpc_TpIp != NULL)
   {
      mpc_TpIp->SetDispatcher(NULL, 0U);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Reports some information about the current sequence

   To be overridden by application.
   Default implementation here: print to console and log file.

   \param[in]  os32_Result       Result of service
   \param[in]  orc_Information   Text information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDcBasicSequences::m_ReportProgress(const int32_t os32_Result, const C_SclString & orc_Information)
{
   std::cout << "Info: " << orc_Information.c_str() << " Result: " << os32_Result << std::endl;

   if (os32_Result == C_NO_ERR)
   {
      osc_write_log_info("Progress", orc_Information);
   }
   else if (os32_Result == C_WARN)
   {
      osc_write_log_warning("Progress", orc_Information);
   }
   else
   {
      osc_write_log_error("Progress", orc_Information);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Reports device information read from found devices

   Called by ScanGetInfo() after it has scanned information from connected devices.
   Default implementation here: print read information to console and log file

   \param[in]  orc_DeviceInfoResult    Device information results
   \param[in]  oq_SecurityFeatureUsed  Security feature used for at least one node
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDcBasicSequences::m_ReportDevicesInfoRead(const std::vector<C_OscDcDeviceInformation> & orc_DeviceInfoResult,
                                                    const bool oq_SecurityFeatureUsed)
{
   std::cout << h_DevicesInfoToString(orc_DeviceInfoResult, oq_SecurityFeatureUsed).c_str() << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "RequestProgramming" via the active transport protocol (CAN or IP)

   Maps the IP result type to the CAN result type for a unified interface.

   \param[out]  orc_Results   Result list with one entry per responding device

   \return
   C_NO_ERR   request sent; zero or more responses collected
   C_COM      send error
   C_CONFIG   no transport protocol available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastRequestProgramming(
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastRequestProgrammingResults> & orc_Results) const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastRequestProgramming(orc_Results);
   }
   else if (this->mpc_TpIp != NULL)
   {
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastRequestProgrammingResults> c_IpResults;
      s32_Return = mpc_TpIp->BroadcastRequestProgramming(c_IpResults);

      // Map IP results to the CAN result format (IP address is not forwarded)
      orc_Results.clear();
      for (uint32_t u32_Idx = 0U; u32_Idx < c_IpResults.size(); ++u32_Idx)
      {
         C_OscProtocolDriverOsyTpCan::C_BroadcastRequestProgrammingResults c_Result;
         c_Result.c_SenderId = c_IpResults[u32_Idx].c_NodeId;
         c_Result.q_RequestAccepted = c_IpResults[u32_Idx].q_RequestAccepted;
         orc_Results.push_back(c_Result);
      }
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "EcuReset" via the active transport protocol (CAN or IP)

   CAN uses BroadcastEcuReset; IP uses BroadcastNetReset with the same reset type constant.

   \param[in]  ou8_ResetType  Reset type (use constants from C_OscProtocolDriverOsyTpBase)

   \return
   C_NO_ERR   broadcast sent
   C_COM      send error
   C_CONFIG   no transport protocol available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastEcuReset(const uint8_t ou8_ResetType) const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastEcuReset(ou8_ResetType);
   }
   else if (this->mpc_TpIp != NULL)
   {
      s32_Return = mpc_TpIp->BroadcastNetReset(ou8_ResetType);
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "EnterDefaultSession" via the active transport protocol (CAN or IP)

   IP transport protocol does not support this broadcast; the call is treated as a no-op for IP.

   \return
   C_NO_ERR   broadcast sent (CAN) or no-op (IP)
   C_COM      send error (CAN only)
   C_CONFIG   no transport protocol available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastSendEnterDefaultSession(void) const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastSendEnterDefaultSession();
   }
   else if (this->mpc_TpIp != NULL)
   {
      // IP transport protocol does not support this broadcast
      s32_Return = C_NO_ERR;
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "EnterPreProgrammingSession" via the active transport protocol (CAN or IP)

   IP transport protocol does not support this broadcast; the call is treated as a no-op for IP.

   \return
   C_NO_ERR   broadcast sent (CAN) or no-op (IP)
   C_COM      send error (CAN only)
   C_CONFIG   no transport protocol available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastSendEnterPreProgrammingSession(void) const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastSendEnterPreProgrammingSession();
   }
   else if (this->mpc_TpIp != NULL)
   {
      // IP transport protocol does not support this broadcast
      s32_Return = C_NO_ERR;
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clear the transport protocol dispatcher queue

   For CAN: clears the CAN dispatcher's receive queue.
   For IP: no dispatcher queue to clear.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDcBasicSequences::m_ClearTpDispatcherQueue(void)
{
   if (this->mpc_TpCan != NULL)
   {
      mpc_TpCan->ClearDispatcherQueue();
   }
   // IP transport protocol does not have a dispatcher queue
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "ReadSerialNumber" via the active transport protocol (CAN or IP)

   For IP, BroadcastGetDeviceInfo is used and the results are mapped to the CAN result types.
   The device name contained in the IP result is not forwarded (ScanGetInfo reads it separately).

   \param[out]  orc_Responses          Standard serial number responses
   \param[out]  orc_ExtendedResponses  Extended serial number responses
   \param[out]  orc_DeviceNames        Device names of responding devices (only for IP transport; empty for CAN)

   \return
   C_NO_ERR   broadcast sent; zero or more responses collected
   C_COM      send error
   C_CONFIG   no transport protocol available
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastReadSerialNumber(
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults> & orc_Responses,
   std::vector<C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults> & orc_ExtendedResponses,
   std::vector<stw::scl::C_SclString> & orc_DeviceNames)
const
{
   int32_t s32_Return;

   orc_DeviceNames.clear();

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastReadSerialNumber(orc_Responses, orc_ExtendedResponses);
   }
   else if (this->mpc_TpIp != NULL)
   {
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoResults> c_DeviceInfos;
      std::vector<C_OscProtocolDriverOsyTpIp::C_BroadcastGetDeviceInfoExtendedResults> c_DeviceExtendedInfos;
      s32_Return = mpc_TpIp->BroadcastGetDeviceInfo(c_DeviceInfos, c_DeviceExtendedInfos);

      // Map standard IP results to CAN serial number result format
      orc_Responses.clear();
      for (uint32_t u32_Idx = 0U; u32_Idx < c_DeviceInfos.size(); ++u32_Idx)
      {
         C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberResults c_Result;
         c_Result.c_SenderId = c_DeviceInfos[u32_Idx].c_NodeId;
         c_Result.c_SerialNumber = c_DeviceInfos[u32_Idx].c_SerialNumber;
         orc_Responses.push_back(c_Result);
         orc_DeviceNames.push_back(c_DeviceInfos[u32_Idx].c_DeviceName);
      }

      // Map extended IP results to CAN extended serial number result format
      orc_ExtendedResponses.clear();
      for (uint32_t u32_Idx = 0U; u32_Idx < c_DeviceExtendedInfos.size(); ++u32_Idx)
      {
         C_OscProtocolDriverOsyTpCan::C_BroadcastReadEcuSerialNumberExtendedResults c_ResultExt;

         c_ResultExt.c_SenderId = c_DeviceExtendedInfos[u32_Idx].c_NodeId;
         c_ResultExt.c_SerialNumber = c_DeviceExtendedInfos[u32_Idx].c_SerialNumber;
         c_ResultExt.u8_SubNodeId = c_DeviceExtendedInfos[u32_Idx].u8_SubNodeId;
         c_ResultExt.q_SecurityActivated = c_DeviceExtendedInfos[u32_Idx].q_SecurityActivated;
         orc_ExtendedResponses.push_back(c_ResultExt);
         orc_DeviceNames.push_back(c_DeviceExtendedInfos[u32_Idx].c_DeviceName);
      }
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "SetNodeIdBySerialNumber" (standard format) via the active transport protocol

   IP transport protocol uses IP address assignment instead of node ID assignment; not supported here.

   \param[in]  orc_SerialNumber   Serial number of the target device
   \param[in]  orc_NewNodeId      New node ID to assign

   \return
   C_NO_ERR   broadcast sent
   C_COM      send error
   C_CONFIG   no transport protocol available
   C_NOACT    not supported for the active transport protocol (IP)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastSetNodeIdBySerialNumber(const C_OscProtocolSerialNumber & orc_SerialNumber,
                                                                  const C_OscProtocolDriverOsyNode & orc_NewNodeId)
const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastSetNodeIdBySerialNumber(orc_SerialNumber, orc_NewNodeId);
   }
   else
   {
      uint8_t au8_ResponseIpAddress[4] = {0, 0, 0, 0};
      s32_Return = mpc_TpIp->BroadcastSetIpAddress(orc_SerialNumber, orc_NewNodeId, au8_ResponseIpAddress);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Broadcast "SetNodeIdBySerialNumber" (extended format) via the active transport protocol

   IP transport protocol uses IP address assignment instead of node ID assignment; not supported here.

   \param[in]  orc_SerialNumber   Serial number of the target device
   \param[in]  ou8_SubNodeId      Sub node ID of the target device
   \param[in]  orc_NewNodeId      New node ID to assign

   \return
   C_NO_ERR   broadcast sent
   C_COM      send error
   C_CONFIG   no transport protocol available
   C_NOACT    not supported for the active transport protocol (IP)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDcBasicSequences::m_BroadcastSetNodeIdBySerialNumberExtended(
   const C_OscProtocolSerialNumber & orc_SerialNumber, const uint8_t ou8_SubNodeId,
   const C_OscProtocolDriverOsyNode & orc_NewNodeId) const
{
   int32_t s32_Return;

   if (this->mpc_TpCan != NULL)
   {
      s32_Return = mpc_TpCan->BroadcastSetNodeIdBySerialNumberExtended(orc_SerialNumber, ou8_SubNodeId, orc_NewNodeId);
   }
   else
   {
      uint8_t au8_ResponseIpAddress[4] = {0, 0, 0, 0};
      s32_Return = this->mpc_TpIp->BroadcastSetIpAddressExtended(orc_SerialNumber, orc_NewNodeId, ou8_SubNodeId,
                                                                 au8_ResponseIpAddress);
   }

   return s32_Return;
}
