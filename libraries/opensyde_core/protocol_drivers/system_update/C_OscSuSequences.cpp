//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Sequences for system update.

   For details cf. documentation in .h file.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include "C_SclStringUtil.hpp"

#include <set>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscSuSequences.hpp"
#include <string>
#include "C_SclDateTime.hpp"
#include "C_SclChecksums.hpp"
#include "TglUtils.hpp"
#include "TglTime.hpp"
#include "TglFile.hpp"
#include "C_OscHexFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscUtils.hpp"
#include "C_OscDataDealerNvmSafe.hpp"
#include "C_OscDiagProtocolOsy.hpp"
#include "C_OscSecurityPem.hpp"
#include "C_OscUpdateUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSuSequences::C_DoFlash::C_DoFlash(void) :
   q_SendSecureAuthenticationEnabledState(false),
   q_SecureAuthenticationEnabled(false),
   q_SendTrafficEncryptionEnabledState(false),
   q_TrafficEncryptionEnabled(false),
   q_SendDebuggerEnabledState(false),
   q_DebuggerEnabled(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether the instance contains any data to be written

   \return  true: configuration contains data that needs writing
            false: empty config

*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSuSequences::C_DoFlash::IsAnyActionRequired() const
{
   return ((this->c_FilesToFlash.size() > 0) ||
           (this->c_FilesToWriteToNvm.size() > 0) ||
           (this->c_PemFile != "") ||
           (this->q_SendDebuggerEnabledState) ||
           (this->q_SendSecureAuthenticationEnabledState) ||
           (this->q_SendTrafficEncryptionEnabledState));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: check whether node is an active node on a specific bus

   Assumptions:
   * mpc_SystemDefinition and mc_ActiveNodes must be valid
   * node and bus index must be within the range of nodes contained in mpc_SystemDefinition

   \param[in]  ou32_NodeIndex     index of node within mpc_SystemDefinition
   \param[in]  ou32_BusIndex      index of bus within mpc_SystemDefinition
   \param[out] ore_ProtocolType   if found: type of flashloader protocol defined for node (otherwise: eFL_NONE)
   \param[out] orc_NodeId         if found: node id of the node on the specified bus

   \return
   true    node is active on specified bus
   false   node not active on specified bus
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSuSequences::m_IsNodeActive(const uint32_t ou32_NodeIndex, const uint32_t ou32_BusIndex,
                                      C_OscNodeProperties::E_FlashLoaderProtocol & ore_ProtocolType,
                                      C_OscProtocolDriverOsyNode & orc_NodeId) const
{
   bool q_Return = false;
   const C_OscNode & rc_Node = this->mpc_SystemDefinition->c_Nodes[ou32_NodeIndex];

   ore_ProtocolType = C_OscNodeProperties::eFL_NONE;

   //is the node intends to be addressed at all (defined by parameter set in Init() function)
   if (this->mc_ActiveNodes[ou32_NodeIndex] == 1U)
   {
      for (uint16_t u16_Interface = 0U; u16_Interface < rc_Node.c_Properties.c_ComInterfaces.size(); u16_Interface++)
      {
         const C_OscNodeComInterfaceSettings & rc_Settings = rc_Node.c_Properties.c_ComInterfaces[u16_Interface];

         if ((rc_Settings.GetBusConnected() == true) && (rc_Settings.u32_BusIndex == ou32_BusIndex) &&
             (rc_Settings.q_IsUpdateEnabled == true))
         {
            //we cannot know for sure whether the target is in flashloader or application mode
            //prerequisite: a node is either
            //* a full openSYDE node (Flashloader + Application) or
            //* an STW Flashloader node
            //openSYDE node ?
            if (rc_Node.c_Properties.e_DiagnosticServer == C_OscNodeProperties::eDS_OPEN_SYDE)
            {
               tgl_assert(rc_Node.c_Properties.e_FlashLoader == C_OscNodeProperties::eFL_OPEN_SYDE);
            }

            q_Return = true;
            ore_ProtocolType = rc_Node.c_Properties.e_FlashLoader;
            orc_NodeId.u8_NodeIdentifier = rc_Settings.u8_NodeId;
            orc_NodeId.u8_BusIdentifier = this->mpc_SystemDefinition->c_Buses[ou32_BusIndex].u8_BusId;
            break; //found the interface that is connected to the specified bus
         }
      }
   }
   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reports some information about the current sequence

   To be overridden by application.
   Default implementation here: print to console.

   The public procedure functions report their progress using either this function or the polymorphous variation.
   If a procedure function finishes without problems it reports a progress of 100 before it returns.
   If it has problems the last progress reported can be < 100.
   Depending on the function it can report its whole process from "0 to 100" or individual parts
    (e.g.: SystemUpdate() will report from 0..100 for each node that is updated).
   The return value can be used to abort an ongoing sequence.
   However, not all calls to the function check the return value.

   \param[in]  oe_Step           Step of node configuration
   \param[in]  os32_Result       Result of service
   \param[in]  ou8_Progress      Progress of sequence in percentage
   \param[in]  orc_Information   Additional text information

   \return
   Flag for aborting sequence
   - true   abort sequence
   - false  continue sequence
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSuSequences::m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result,
                                        const uint8_t ou8_Progress, const std::string & orc_Information)
{
   std::cout << "Step: " << static_cast<uint32_t>(oe_Step) << " Result: " << os32_Result << " Progress: " <<
      static_cast<uint32_t>(ou8_Progress) << " Info: "  << orc_Information.c_str() << "\n";

   return false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reports some information about the current sequence for a specific server

   To be overridden by application.
   Default implementation here: print to console.

   For more details see description of the other m_ReportProgress() function

   \param[in]  oe_Step           Step of node configuration
   \param[in]  os32_Result       Result of service
   \param[in]  ou8_Progress      Progress of sequence in percentage (goes from 0..100 for each function)
   \param[in]  orc_Server        Affected node
   \param[in]  orc_Information   Additional text information

   \return
   Flag for aborting sequence
   - true   abort sequence
   - false  continue sequence
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSuSequences::m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result,
                                        const uint8_t ou8_Progress, const C_OscProtocolDriverOsyNode & orc_Server,
                                        const std::string & orc_Information)
{
   std::cout << "Step: " << static_cast<uint32_t>(oe_Step) << " Result: " << os32_Result << " Progress: " <<
      static_cast<uint32_t>(ou8_Progress) << " Bus Id: " << static_cast<uint32_t>(orc_Server.u8_BusIdentifier) <<
      " Node Id: " << static_cast<uint32_t>(orc_Server.u8_NodeIdentifier) << " Info: " <<
      orc_Information.c_str() << "\n";

   return false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reports information read from openSYDE server node

   Called by ReadDeviceInformation() after it has read information from an openSYDE node.
   Default implementation here: print read information to console

   \param[in]     orc_Info         Information read from node
   \param[in]     ou32_NodeIndex   Index of node within mpc_SystemDefinition
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSuSequences::m_ReportOpenSydeFlashloaderInformationRead(const C_OsyDeviceInformation & orc_Info,
                                                                  const uint32_t ou32_NodeIndex)
{
   std::vector<std::string> c_Text;

   h_OpenSydeFlashloaderInformationToText(orc_Info, c_Text);

   std::cout << "openSYDE device information found for node with index " << ou32_NodeIndex << "\n";
   for (uint32_t u32_Line = 0U; u32_Line < c_Text.size(); u32_Line++)
   {
      std::cout << c_Text[u32_Line].c_str() << "\n";
   }
}


//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Flash one openSYDE address based node

   Flash one or more files.

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * list of files must be > 0
   * files must be present and in Intel or Motorola hex format
   * node is active

   * Reports progress from 0..100 for the overall process
   * Reports 0..100 for each file being flashed

   \param[in]      orc_FilesToFlash              Files to flash
   \param[in]      orc_OtherAcceptedDeviceNames  Other accepted device names
   \param[in]      ou32_RequestDownloadTimeout   Maximum time in ms it can take to erase one continuous area in flash
   \param[in]      ou32_TransferDataTimeout      Maximum time in ms it can take to write up to 4kB of data to flash
   \param[in,out]  orq_SetProgrammingMode        In: Flag if programming mode must be set.
                                                 Out: Flag if programming mode was set.
   \param[out]     orc_StateHexFiles             States of all handled hex files

   \return
   Errc::success     flashed all files
   Errc::rd_wr       one of the files is not a valid Intel or Motorola hex file
                     could not split up hex file in individual areas
   Errc::config      no signature block found in hex file
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::noact       could not extract device name from hex file
   Errc::overflow    device name of device does not match name contained in hex file
   Errc::busy        procedure aborted by user (as returned by m_ReportProgress)
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_FlashNodeOpenSydeHex(
   const std::vector<std::string> & orc_FilesToFlash, const std::vector<std::string> & orc_OtherAcceptedDeviceNames,
   const uint32_t ou32_RequestDownloadTimeout, const uint32_t ou32_TransferDataTimeout, bool & orq_SetProgrammingMode,
   std::vector<C_OscSuSequencesNodeHexFileStates> & orc_StateHexFiles)
{
   std::error_code c_Return = Errc::success;
   //C_HexFile reports its own hex_file category, which is not the STW one
   std::error_code c_HexError = stw::hex_file::HexFileErrc::success;

   std::vector<uint32_t> c_SignatureAddresses(orc_FilesToFlash.size()); ///< addresses of signatures within hex files

   //C_OscHexFile cannot be copied; so we cannot put it into a resizable vector
   //-> create instances manually
   std::vector<C_OscHexFile *> c_Files(orc_FilesToFlash.size());
   for (uint32_t u32_File = 0U; u32_File < c_Files.size(); u32_File++)
   {
      c_Files[u32_File] = new C_OscHexFile();
   }

   //try to open files to check whether we have valid hex files before we start messing with the target's flash memory:
   for (uint32_t u32_File = 0U; (u32_File < orc_FilesToFlash.size()) && (c_Return == Errc::success); u32_File++)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_HEX_OPEN_START, C_NO_ERR, 0U, mc_CurrentNode,
                             "Opening HEX file " + orc_FilesToFlash[u32_File] + ".");
      c_HexError = c_Files[u32_File]->LoadFromFile(orc_FilesToFlash[u32_File].c_str());
      if (c_HexError)
      {
         const std::string c_ErrorText = c_Files[u32_File]->ErrorCodeToErrorText(c_HexError);
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_HEX_OPEN_ERROR, C_RD_WR, 0U, mc_CurrentNode,
                                "Opening HEX file. Reason: " + c_ErrorText + ".");

         orc_StateHexFiles[u32_File].e_FileLoaded = eSUSEQ_STATE_ERROR;
         c_Return = Errc::rd_wr;
      }
      else
      {
         orc_StateHexFiles[u32_File].e_FileLoaded = eSUSEQ_STATE_NO_ERR;

         //try to find signature address in hex file:
         c_Return = c_Files[u32_File]->GetSignatureBlockAddress(c_SignatureAddresses[u32_File]);
         if (c_Return != Errc::success)
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_HEX_SIGNATURE_ERROR, c_Return.value(), 0U, mc_CurrentNode,
                                   "Could not find signature block in HEX file.");
            orc_StateHexFiles[u32_File].e_SignatureBlockAddressFromFileRead = eSUSEQ_STATE_ERROR;
            c_Return = Errc::config;
         }
         else
         {
            orc_StateHexFiles[u32_File].e_SignatureBlockAddressFromFileRead = eSUSEQ_STATE_NO_ERR;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      std::string c_DeviceName;
      uint8_t u8_NrCode;

      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_DEVICE_NAME_START, C_NO_ERR, 10U, mc_CurrentNode,
                             "X-checking device name of device against HEX file contents ...");
      //get target device name for comparison with PC-side files:
      c_Return = this->mpc_ComDriver->SendOsyReadDeviceName(mc_CurrentNode, c_DeviceName, &u8_NrCode);
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_DEVICE_NAME_COMM_ERROR, c_Return.value(), 10U,
                                mc_CurrentNode, "Could not read device name from device. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         c_Return = Errc::com;
      }

      if (c_Return == Errc::success)
      {
         //for all files check whether:
         //* device type in file matches target's device type
         for (uint32_t u32_File = 0U; (u32_File < orc_FilesToFlash.size()) && (c_Return == Errc::success); u32_File++)
         {
            std::string c_DeviceNameHexFile;

            //get device ID from hex file
            c_Return = c_Files[u32_File]->ScanDeviceIdFromHexFile(c_DeviceNameHexFile);
            if (c_Return != Errc::success)
            {
               std::string c_ErrorText = "Could not read device name from file " + orc_FilesToFlash[u32_File] +
                                         ". Reason: ";
               if (c_Return == Errc::noact)
               {
                  c_ErrorText += "Device name not found.";
               }
               else if (c_Return == Errc::config)
               {
                  c_ErrorText += "Ambiguous device names found.";
               }
               else
               {
                  c_ErrorText += "Undefined error.";
               }

               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_DEVICE_NAME_FILE_ERROR, c_Return.value(), 10U,
                                      mc_CurrentNode, c_ErrorText);
               orc_StateHexFiles[u32_File].e_NodeNameCompared = eSUSEQ_STATE_ERROR;
               c_Return = Errc::noact;
            }
            else
            {
               bool q_IsSame = false;
               //Check actual device name
               if (UpperCaseCompat(TrimCompat(c_DeviceName)) == UpperCaseCompat(TrimCompat(c_DeviceNameHexFile)))
               {
                  orc_StateHexFiles[u32_File].e_NodeNameCompared = eSUSEQ_STATE_NO_ERR;
                  q_IsSame = true;
               }
               else
               {
                  //Check other accepted names
                  for (uint32_t u32_ItName = 0UL;
                       (u32_ItName < orc_OtherAcceptedDeviceNames.size()) && (q_IsSame == false); ++u32_ItName)
                  {
                     if (UpperCaseCompat(TrimCompat(orc_OtherAcceptedDeviceNames[u32_ItName])) ==
                         UpperCaseCompat(TrimCompat(c_DeviceNameHexFile)))
                     {
                        orc_StateHexFiles[u32_File].e_NodeNameCompared = eSUSEQ_STATE_NO_ERR;
                        q_IsSame = true;
                     }
                  }
               }
               if (q_IsSame == false)
               {
                  const std::string c_ErrorText = "Device names of device and HEX file " + orc_FilesToFlash[u32_File] +
                                                  " do not match. Device reported: \"" +
                                                  UpperCaseCompat(TrimCompat(c_DeviceName)) +
                                                  "\". HEX file contains: \"" +
                                                  UpperCaseCompat(TrimCompat(c_DeviceNameHexFile)) + "\".";
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_DEVICE_NAME_MATCH_ERROR, C_OVERFLOW, 10U,
                                         mc_CurrentNode, c_ErrorText);

                  orc_StateHexFiles[u32_File].e_NodeNameCompared = eSUSEQ_STATE_ERROR;
                  c_Return = Errc::overflow;
               }
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //for all files check whether:
      //* address ranges contained in file are available on the target
      //we need to enter the programming session for that:
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_START, C_NO_ERR, 20U, mc_CurrentNode,
                             "Checking memory availability ...");
      if (orq_SetProgrammingMode == true)
      {
         // In the whole update sequence, setting the programming mode only one time
         c_Return = this->mpc_ComDriver->SendOsySetProgrammingMode(mc_CurrentNode);
         orq_SetProgrammingMode = false;
      }
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_SESSION_ERROR, c_Return.value(), 20U,
                                mc_CurrentNode,
                                "Could not activate programming session.");

         if (c_Return != Errc::checksum)
         {
            c_Return = Errc::com;
         }
      }
      else
      {
         for (uint32_t u32_File = 0U; u32_File < orc_FilesToFlash.size(); u32_File++)
         {
            //do we have enough space for the hex file data ?
            const stw::hex_file::C_HexDataDump * const pc_HexDump = c_Files[u32_File]->GetDataDump(c_HexError);
            if (c_HexError)
            {
               std::string c_ErrorText;
               c_ErrorText = "Could not split up HEX file data of file " + orc_FilesToFlash[u32_File] +
                             " into handy chunks. Reason: " + c_Files[u32_File]->ErrorCodeToErrorText(c_HexError);
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_FILE_ERROR, C_RD_WR, 20U, mc_CurrentNode,
                                      c_ErrorText);
               orc_StateHexFiles[u32_File].e_DataDumpFromFileRead = eSUSEQ_STATE_ERROR;
               c_Return = Errc::rd_wr;
               break;
            }
            else
            {
               orc_StateHexFiles[u32_File].e_DataDumpFromFileRead = eSUSEQ_STATE_NO_ERR;

               for (uint16_t u16_Area = 0U; u16_Area < pc_HexDump->at_Blocks.size(); u16_Area++)
               {
                  uint8_t u8_NrCode;
                  c_Return =
                     this->mpc_ComDriver->SendOsyCheckFlashMemoryAvailable(
                        mc_CurrentNode,
                        pc_HexDump->at_Blocks[u16_Area].u32_AddressOffset,
                        pc_HexDump->at_Blocks[u16_Area].au8_Data.size(),
                        &u8_NrCode);
                  if (c_Return != Errc::success)
                  {
                     std::string c_ErrorText;
                     c_ErrorText = PrintFormattedCompat(
                        "Could not get confirmation about flash memory availability. (File: %s" \
                        " Offset: 0x%08x Size: 0x%08x). Details: %s",
                        orc_FilesToFlash[u32_File].c_str(),
                        pc_HexDump->at_Blocks[u16_Area].u32_AddressOffset,
                        static_cast<uint32_t>(pc_HexDump->at_Blocks[u16_Area].au8_Data.size()),
                        C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode).c_str());
                     (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_NOT_OK, c_Return.value(), 20U,
                                            mc_CurrentNode, c_ErrorText);
                     c_Return = Errc::com;
                     break;
                  }
               }

               orc_StateHexFiles[u32_File].e_AvailableFlashMemoryChecked =
                  (c_Return == Errc::success) ? eSUSEQ_STATE_NO_ERR : eSUSEQ_STATE_ERROR;
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //all prerequisites checked; commence the flashing ...
      //write fingerprint
      c_Return = this->m_WriteFingerPrintOsy();
      if (c_Return == Errc::success)
      {
         //now do the real flashing ...
         for (uint32_t u32_File = 0U; u32_File < orc_FilesToFlash.size(); u32_File++)
         {
            const stw::hex_file::C_HexDataDump * const pc_HexDump = c_Files[u32_File]->GetDataDump(c_HexError);
            //we would not have gotten here if we could not get a decent dump ...
            tgl_assert(pc_HexDump != nullptr);

            if (pc_HexDump != nullptr)
            {
               // Save file index
               this->mu32_CurrentFile = u32_File;

               //lint -e{613}  //see assertion
               c_Return = m_FlashOneFileOpenSydeHex(*pc_HexDump, c_SignatureAddresses[u32_File],
                                                      ou32_RequestDownloadTimeout, ou32_TransferDataTimeout,
                                                      orc_StateHexFiles[u32_File]);
               if (c_Return != Errc::success)
               {
                  //error report is already in subfunction
                  break;
               }
            }
         }
      }
   }

   //clean up hex file instances:
   for (uint32_t u32_File = 0U; u32_File < c_Files.size(); u32_File++)
   {
      delete c_Files[u32_File];
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Flash one hex file to openSYDE address based node

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * driver is correctly set up
   * target is in programming session with security level 3 active
   * file was checked to be OK (valid hex file; signature present)
   * file matches the target device name

   Reports progress from 0..100

   \param[in]      orc_HexDataDump               Dump of file to flash
   \param[in]      ou32_SignatureAddress         address of signature block within hex file
   \param[in]      ou32_RequestDownloadTimeout   Maximum time in ms it can take to erase one continuous area in flash
   \param[in]      ou32_TransferDataTimeout      Maximum time in ms it can take to write up to 4kB of data to flash
   \param[in,out]  orc_StateHexFile              State of hex file

   \return
   Errc::success    file flashed
   Errc::com        communication driver reported problem (details will be written to log file)
   Errc::busy       procedure aborted by user (as returned by m_ReportProgress)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_FlashOneFileOpenSydeHex(const stw::hex_file::C_HexDataDump & orc_HexDataDump,
                                                            const uint32_t ou32_SignatureAddress,
                                                            const uint32_t ou32_RequestDownloadTimeout,
                                                            const uint32_t ou32_TransferDataTimeout,
                                                            C_OscSuSequencesNodeHexFileStates & orc_StateHexFile)
{
   std::error_code c_Return = Errc::success;
   uint32_t u32_TotalNumberOfBytes = 0U;
   uint32_t u32_TotalNumberOfBytesFlashed = 0U;

   (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_START, C_NO_ERR, 0U, mc_CurrentNode,
                          "Flashing HEX file ...");

   //get total number of bytes for progress calculations:
   for (int32_t s32_Area = 0U; s32_Area < static_cast<int32_t>(orc_HexDataDump.at_Blocks.size()); s32_Area++)
   {
      u32_TotalNumberOfBytes += static_cast<uint32_t>(orc_HexDataDump.at_Blocks[s32_Area].au8_Data.size());
   }

   //flash all areas
   for (int32_t s32_Area = 0U; s32_Area < static_cast<int32_t>(orc_HexDataDump.at_Blocks.size()); s32_Area++)
   {
      const uint32_t u32_AreaSize = orc_HexDataDump.at_Blocks[s32_Area].au8_Data.size();
      uint32_t u32_MaxBlockLength = 0U;
      uint8_t u8_NrCode;
      bool q_Abort;

      //calculate progress percentage:
      // (we just need a rough approximation; so integer calculation will suffice)
      tgl_assert(u32_TotalNumberOfBytes != 0U); //prerequisite for function: non-empty hex file
      // Prevent an overflow when file is bigger than 43MB
      //lint -e{414}  //see assertion
      uint8_t u8_ProgressPercentage =
         static_cast<uint8_t>((static_cast<uint64_t>(u32_TotalNumberOfBytesFlashed) * 100ULL) /
                              static_cast<uint64_t>(u32_TotalNumberOfBytes));
      q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_START, C_NO_ERR, u8_ProgressPercentage,
                                 mc_CurrentNode,
                                 "Erasing flash memory for area " + std::to_string(s32_Area + 1) + "...");
      if (q_Abort == true)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_ABORTED, C_NO_ERR, u8_ProgressPercentage,
                                mc_CurrentNode, "Flashing HEX file aborted.");

         c_Return = Errc::busy;
      }
      else
      {
         //set a proper timeout
         (void)this->mpc_ComDriver->OsySetPollingTimeout(mc_CurrentNode, ou32_RequestDownloadTimeout);

         c_Return = this->mpc_ComDriver->SendOsyRequestDownload(
            mc_CurrentNode,
            orc_HexDataDump.at_Blocks[s32_Area].u32_AddressOffset,
            orc_HexDataDump.at_Blocks[s32_Area].au8_Data.size(),
            u32_MaxBlockLength, &u8_NrCode);

         if (c_Return != Errc::success)
         {
            std::string c_Error;
            c_Error = PrintFormattedCompat("Erasing flash memory for area %d failed (Offset: 0x%08X Size: 0x%08X). Details: %s",
                                   s32_Area + 1,
                                   orc_HexDataDump.at_Blocks[s32_Area].u32_AddressOffset,
                                   static_cast<uint32_t>(orc_HexDataDump.at_Blocks[s32_Area].au8_Data.size()),
                                   C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                            u8_NrCode).c_str());
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_ERASE_ERROR, c_Return.value(),
                                   u8_ProgressPercentage, mc_CurrentNode, c_Error);

            orc_StateHexFile.e_RequestDownloadSent = eSUSEQ_STATE_ERROR;
            c_Return = Errc::com;
         }
         else
         {
            orc_StateHexFile.e_RequestDownloadSent = eSUSEQ_STATE_NO_ERR;
         }
      }
      if (c_Return == Errc::success)
      {
         //perform the actual transfer
         uint8_t u8_BlockSequenceCounter = 1U;
         uint32_t u32_RemainingBytes = u32_AreaSize;
         std::vector<uint8_t> c_Data;
         const uint32_t u32_AdaptedTransferDataTimeout = m_GetAdaptedTransferDataTimeout(ou32_TransferDataTimeout,
                                                                                         u32_MaxBlockLength,
                                                                                         mc_CurrentNode.u8_BusIdentifier);

         //set a proper timeout
         (void)this->mpc_ComDriver->OsySetPollingTimeout(mc_CurrentNode, u32_AdaptedTransferDataTimeout);

         while (u32_RemainingBytes > 0U)
         {
            std::string c_Text;
            c_Text = PrintFormattedCompat("Writing data for area %02d/%02d  byte %08u/%08u ...",
                                  s32_Area + 1, orc_HexDataDump.at_Blocks.size(),
                                  u32_AreaSize - u32_RemainingBytes, u32_AreaSize);
            q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_TRANSFER_START, C_NO_ERR,
                                       u8_ProgressPercentage, mc_CurrentNode, c_Text);
            if (q_Abort == true)
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_ABORTED, C_NO_ERR, u8_ProgressPercentage,
                                      mc_CurrentNode, "Flashing HEX file aborted.");

               c_Return = Errc::busy;
            }
            else
            {
               //subtract 4 bytes from the reported size; compensated for an issue in older server implementations
               // reporting an incorrect size; #62305
               if (u32_RemainingBytes > (u32_MaxBlockLength - 4U))
               {
                  c_Data.resize(static_cast<size_t>(u32_MaxBlockLength) - 4U);
               }
               else
               {
                  c_Data.resize(u32_RemainingBytes);
               }

               (void)memcpy(&c_Data[0],
                            &orc_HexDataDump.at_Blocks[s32_Area].au8_Data[static_cast<int32_t>(u32_AreaSize -
                                                                                               u32_RemainingBytes)],
                            c_Data.size());

               c_Return = this->mpc_ComDriver->SendOsyTransferData(mc_CurrentNode, u8_BlockSequenceCounter, c_Data,
                                                                     &u8_NrCode);
               if (c_Return == Errc::success)
               {
                  u32_RemainingBytes -= static_cast<uint32_t>(c_Data.size());
                  u8_BlockSequenceCounter = (u8_BlockSequenceCounter < 0xFFU) ? (u8_BlockSequenceCounter + 1U) : 0x00U;
                  u32_TotalNumberOfBytesFlashed += static_cast<uint32_t>(c_Data.size());
                  // Prevent an overflow when file is bigger than 43MB
                  //lint -e{414}  //see assertion at initial assignment
                  u8_ProgressPercentage =
                     static_cast<uint8_t>((static_cast<uint64_t>(u32_TotalNumberOfBytesFlashed) * 100ULL) /
                                          static_cast<uint64_t>(u32_TotalNumberOfBytes));
               }
               else
               {
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_TRANSFER_ERROR, c_Return.value(),
                                         u8_ProgressPercentage, mc_CurrentNode, "Could not write data. Details: " +
                                         C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                                  u8_NrCode));
                  c_Return = Errc::com;
               }
            }
            if (c_Return != Errc::success)
            {
               break;
            }
         }

         orc_StateHexFile.e_AllTransferDataSent =
            (c_Return == Errc::success) ? eSUSEQ_STATE_NO_ERR : eSUSEQ_STATE_ERROR;
      }

      // Reset the timeout. The services with the specific timeouts are finished.
      (void)this->mpc_ComDriver->OsyResetPollingTimeout(mc_CurrentNode);

      if (c_Return == Errc::success)
      {
         //area transferred ...
         //report "final" status:
         std::string c_Text;
         c_Text = PrintFormattedCompat("Writing data for area %02d/%02d  byte %08u/%08u ...",
                               s32_Area + 1, orc_HexDataDump.at_Blocks.size(), u32_AreaSize, u32_AreaSize);
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_TRANSFER_START, C_NO_ERR,
                                u8_ProgressPercentage, mc_CurrentNode, c_Text);

         //if it's the last area we need to check the signature
         if (s32_Area == (static_cast<int32_t>(orc_HexDataDump.at_Blocks.size()) - 1))
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_EXIT_FINAL_START, C_NO_ERR,
                                   u8_ProgressPercentage, mc_CurrentNode,
                                   "Finalizing the final area and checking the signature ...");
            c_Return =
               this->mpc_ComDriver->SendOsyRequestTransferExitAddressBased(mc_CurrentNode, true,
                                                                           ou32_SignatureAddress,
                                                                           &u8_NrCode);
         }
         else
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_EXIT_START, C_NO_ERR,
                                   u8_ProgressPercentage, mc_CurrentNode, "Finalizing the area ...");
            c_Return = this->mpc_ComDriver->SendOsyRequestTransferExitAddressBased(mc_CurrentNode, false, 0U,
                                                                                     &u8_NrCode);
         }

         if (c_Return != Errc::success)
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_AREA_EXIT_ERROR, c_Return.value(),
                                   u8_ProgressPercentage, mc_CurrentNode, "Could not finalize the area. Details: " +
                                   C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                            u8_NrCode));
            orc_StateHexFile.e_RequestTransferAddressExitSent = eSUSEQ_STATE_ERROR;
            c_Return = Errc::com;
         }
         else
         {
            orc_StateHexFile.e_RequestTransferAddressExitSent = eSUSEQ_STATE_NO_ERR;
         }
      }

      if (c_Return != Errc::success)
      {
         break;
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_FINISHED, C_NO_ERR, 100U, mc_CurrentNode,
                             "Flashing HEX file finished.");
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Flash one openSYDE file based node

   Writes one or more files.

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * list of files must be > 0
   * files must be present on client side file system
   * node is active

   * Reports progress from 0..100 for the overall process
   * Reports 0..100 for each file being flashed

   \param[in]      orc_FilesToFlash              Files to write
   \param[in]      ou32_RequestDownloadTimeout   Maximum time in ms it can take to prepare one file on the target file
   \param[in]      ou32_TransferDataTimeout      Maximum time in ms it can take to write up to 4kB of data to the target
                                                  file
   \param[in]      orc_ProtocolFeatures          available protocol features
   \param[in,out]  orq_SetProgrammingMode        In: Flag if programming mode must be set.
                                                 Out: Flag if programming mode was set.
   \param[out]     orc_StateOtherFiles           States of all handled files

   \return
   Errc::success     flashed all files

   Errc::rd_wr       one of the files is not a valid Intel or Motorola hex file
                     could not split up hex file in individual areas
   Errc::config      no signature block found in hex file
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::noact       could not extract device name from hex file
   Errc::overflow    device name of device does not match name contained in hex file
   Errc::busy        procedure aborted by user (as returned by m_ReportProgress)
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_FlashNodeOpenSydeFile(
   const std::vector<std::string> & orc_FilesToFlash, const uint32_t ou32_RequestDownloadTimeout,
   const uint32_t ou32_TransferDataTimeout, const C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ProtocolFeatures,
   bool & orq_SetProgrammingMode, std::vector<C_OscSuSequencesNodeOtherFileStates> & orc_StateOtherFiles)
{
   std::error_code c_Return = Errc::success;

   //start the actual transfers
   //we need to enter the programming session for that:
   (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_START, C_NO_ERR, 20U, mc_CurrentNode,
                          "Checking memory availability ...");
   if (orq_SetProgrammingMode == true)
   {
      // In the whole update sequence, setting the programming mode only one time
      c_Return = this->mpc_ComDriver->SendOsySetProgrammingMode(mc_CurrentNode);
      orq_SetProgrammingMode = false;
   }
   if (c_Return != Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_SESSION_ERROR, c_Return.value(), 20U, mc_CurrentNode,
                             "Could not activate programming session.");
      if (c_Return != Errc::checksum)
      {
         c_Return = Errc::com;
      }
   }

   if (c_Return == Errc::success)
   {
      //all prerequisites checked; commence the flashing ...
      //write fingerprint
      c_Return = this->m_WriteFingerPrintOsy();
      if (c_Return == Errc::success)
      {
         //now do the real flashing ...
         for (uint32_t u32_File = 0U; u32_File < orc_FilesToFlash.size(); u32_File++)
         {
            // Save file index
            this->mu32_CurrentFile = u32_File;

            c_Return = m_FlashOneFileOpenSydeFile(orc_FilesToFlash[u32_File], ou32_RequestDownloadTimeout,
                                                    ou32_TransferDataTimeout, orc_ProtocolFeatures,
                                                    orc_StateOtherFiles[u32_File]);
            if (c_Return != Errc::success)
            {
               //error report is already in subfunction
               break;
            }
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Flash one file to openSYDE file based node

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * driver is correctly set up
   * target is in programming session with security level 3 active
   * file was checked to be present

   Reports progress from 0..100

   When reporting the file name to the target only the file name of the file will be transferred.
   (i.e. path information will be removed).

   \param[in]     orc_FileToFlash               File to write
   \param[in]     ou32_RequestDownloadTimeout   Maximum time in ms it can take to prepare one file on the target file
   \param[in]     ou32_TransferDataTimeout      Maximum time in ms it can take to write up to 4kB of data to the target
                                                 file
   \param[in]     orc_ProtocolFeatures          available protocol features
   \param[in,out] orc_StateOtherFile            State of file

   \return
   Errc::success    file flashed
   Errc::com        communication driver reported problem (details will be written to log file)
   Errc::rd_wr      could not read from input file
   Errc::busy       procedure aborted by user (as returned by m_ReportProgress)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_FlashOneFileOpenSydeFile(
   const std::string & orc_FileToFlash, const uint32_t ou32_RequestDownloadTimeout,
   const uint32_t ou32_TransferDataTimeout, const C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ProtocolFeatures,
   C_OscSuSequencesNodeOtherFileStates & orc_StateOtherFile)
{
   std::error_code c_Return = Errc::success;
   int32_t s32_FileApiReturn; //2GB file limit will be fine for us
   uint32_t u32_TotalNumberOfBytes = 0U;

   std::FILE * pc_File;
   uint32_t u32_MaxBlockLength = 0U;
   uint8_t u8_NrCode;
   uint32_t u32_TransferCrc = 0xFFFFFFFFU;
   uint8_t u8_ProgressPercentage = 0U;

   (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_START, C_NO_ERR, 0U, mc_CurrentNode,
                          "Transferring file ...");
   pc_File = std::fopen(orc_FileToFlash.c_str(), "rb");
   if (pc_File == nullptr)
   {
      c_Return = Errc::rd_wr;
   }
   else
   {
      //get total number of bytes for progress calculations:
      s32_FileApiReturn = std::fseek(pc_File, 0, SEEK_END);
      if (s32_FileApiReturn != 0)
      {
         c_Return = Errc::rd_wr;
         (void)std::fclose(pc_File);
         pc_File = nullptr;
      }
      else
      {
         s32_FileApiReturn = static_cast<int32_t>(std::ftell(pc_File));
         if (s32_FileApiReturn < 0)
         {
            c_Return = Errc::rd_wr;
            (void)std::fclose(pc_File);
            pc_File = nullptr;
         }
         else
         {
            //s32_FileApiReturn already checked for negative value (so we can only flash files up to 2GB)
            u32_TotalNumberOfBytes = static_cast<uint32_t>(s32_FileApiReturn);
            std::rewind(pc_File); //start over ...
         }
      }
   }

   //flash file
   if (c_Return == Errc::success)
   {
      const bool q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_PREPARE_START, C_NO_ERR, 0U,
                                            mc_CurrentNode,
                                            "Preparing file system for file \"" +
                                            TglExtractFileName(orc_FileToFlash) + "\"...");

      orc_StateOtherFile.e_FileLoaded = eSUSEQ_STATE_NO_ERR;

      if (q_Abort == true)
      {
         c_Return = Errc::busy;
      }
      else
      {
         //set a proper timeout
         (void)this->mpc_ComDriver->OsySetPollingTimeout(mc_CurrentNode, ou32_RequestDownloadTimeout);

         c_Return = this->mpc_ComDriver->SendOsyRequestFileTransfer(
            mc_CurrentNode, TglExtractFileName(orc_FileToFlash), u32_TotalNumberOfBytes, u32_MaxBlockLength,
            &u8_NrCode);

         if (c_Return != Errc::success)
         {
            std::string c_Error;
            c_Error = PrintFormattedCompat("Preparing file system for file \"%s\" failed. Details: %s",
                                   TglExtractFileName(orc_FileToFlash).c_str(),
                                   C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                            u8_NrCode).c_str());
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_PREPARE_ERROR, c_Return.value(),
                                   0U, mc_CurrentNode, c_Error);

            orc_StateOtherFile.e_RequestFileTransferSent = eSUSEQ_STATE_ERROR;
            c_Return = Errc::com;
         }
         else
         {
            orc_StateOtherFile.e_RequestFileTransferSent = eSUSEQ_STATE_NO_ERR;
         }
      }

      if ((c_Return != Errc::success) && (pc_File != nullptr))
      {
         (void)std::fclose(pc_File);
         pc_File = nullptr;
      }
   }
   else
   {
      orc_StateOtherFile.e_FileLoaded = eSUSEQ_STATE_ERROR;
   }

   if (c_Return == Errc::success)
   {
      //perform the actual transfer
      uint8_t u8_BlockSequenceCounter = 1U;
      uint32_t u32_RemainingBytes = u32_TotalNumberOfBytes;
      std::vector<uint8_t> c_Data;
      uint32_t u32_TotalNumberOfBytesFlashed = 0U;
      const uint32_t u32_AdaptedTransferDataTimeout = m_GetAdaptedTransferDataTimeout(ou32_TransferDataTimeout,
                                                                                      u32_MaxBlockLength,
                                                                                      mc_CurrentNode.u8_BusIdentifier);

      //set a proper timeout
      (void)this->mpc_ComDriver->OsySetPollingTimeout(mc_CurrentNode, u32_AdaptedTransferDataTimeout);

      while (u32_RemainingBytes > 0U)
      {
         std::string c_Text;
         bool q_Abort;
         tgl_assert(u32_TotalNumberOfBytes != 0U); //prerequisite for function: non-empty hex file
         // Prevent an overflow when file is bigger than 43MB
         //lint -e{414}  //see assertion
         u8_ProgressPercentage = static_cast<uint8_t>((static_cast<uint64_t>(u32_TotalNumberOfBytesFlashed) * 100ULL) /
                                                      static_cast<uint64_t>(u32_TotalNumberOfBytes));

         c_Text = PrintFormattedCompat("Writing data byte %08u/%08u ...", u32_TotalNumberOfBytesFlashed,
                               u32_TotalNumberOfBytes);
         q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_TRANSFER_START, C_NO_ERR,
                                    u8_ProgressPercentage, mc_CurrentNode, c_Text);
         if (q_Abort == true)
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_ABORTED, C_NO_ERR, u8_ProgressPercentage,
                                   mc_CurrentNode, "Flashing file aborted.");
            c_Return = Errc::busy;
         }
         else
         {
            //subtract 4 bytes from the reported size; compensated for an issue in older server implementations
            // reporting an incorrect size; #62305
            if (u32_RemainingBytes > (u32_MaxBlockLength - 4U))
            {
               c_Data.resize(static_cast<size_t>(u32_MaxBlockLength) - 4U);
            }
            else
            {
               c_Data.resize(u32_RemainingBytes);
            }

            //lint -e{668}  //file cannot be NULL if we get here
            s32_FileApiReturn =
               static_cast<int32_t>(std::fread(&c_Data[0], 1U, static_cast<size_t>(c_Data.size()), pc_File));
            if (s32_FileApiReturn != static_cast<int32_t>(c_Data.size()))
            {
               //it's not ideal that we have to abort in the middle of the procedure
               //an alternative would be to read in the file before starting
               //But the files might get large and the client might be limited in RAM.
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_TRANSFER_ERROR,
                                      c_Return.value(), u8_ProgressPercentage, mc_CurrentNode,
                                      "Could not read from input file.");
               c_Return = Errc::rd_wr;
            }
            else
            {
               c_Return = this->mpc_ComDriver->SendOsyTransferData(mc_CurrentNode, u8_BlockSequenceCounter, c_Data,
                                                                     &u8_NrCode);
               if (c_Return == Errc::success)
               {
                  //update continuous CRC:
                  C_SclChecksums::CalcCRC32(&c_Data[0], static_cast<uint32_t>(c_Data.size()), u32_TransferCrc);

                  u32_RemainingBytes -= static_cast<uint32_t>(c_Data.size());
                  u8_BlockSequenceCounter =
                     (u8_BlockSequenceCounter < 0xFFU) ? (u8_BlockSequenceCounter + 1U) : 0x00U;
                  u32_TotalNumberOfBytesFlashed += static_cast<uint32_t>(c_Data.size());
               }
               else
               {
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_TRANSFER_ERROR,
                                         c_Return.value(), u8_ProgressPercentage, mc_CurrentNode,
                                         "Could not write data. Details: " +
                                         C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                                  u8_NrCode));
                  c_Return = Errc::com;
               }
            }
         }
         if (c_Return != Errc::success)
         {
            break;
         }
      }

      orc_StateOtherFile.e_AllTransferDataSent = (c_Return == Errc::success) ? eSUSEQ_STATE_NO_ERR : eSUSEQ_STATE_ERROR;

      // File is not necessary anymore. Close it.
      if (pc_File != nullptr)
      {
         (void)std::fclose(pc_File);
         pc_File = nullptr;
      }
   }

   // Reset the timeout. The services with the specific timeouts are finished.
   (void)this->mpc_ComDriver->OsyResetPollingTimeout(mc_CurrentNode);

   if (c_Return == Errc::success)
   {
      //area transferred ...
      //report "final" status:
      std::string c_Text;
      bool q_RejectedByTargetLayer = false;
      c_Text = PrintFormattedCompat("Writing data byte %08u/%08u ...", u32_TotalNumberOfBytes,
                            u32_TotalNumberOfBytes);
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_TRANSFER_START, C_NO_ERR,
                             u8_ProgressPercentage, mc_CurrentNode, c_Text);

      //finalize transfer:
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_EXIT_START, C_NO_ERR,
                             u8_ProgressPercentage, mc_CurrentNode,
                             "Finalizing the transfer and checking the transfer CRC ...");
      //finalize transfer CRC:
      u32_TransferCrc ^= 0xFFFFFFFFU;

      c_Return =
         this->mpc_ComDriver->SendOsyRequestTransferExitFileBased(mc_CurrentNode, u32_TransferCrc,
                                                                  &u8_NrCode);
      if (c_Return != Errc::success)
      {
         orc_StateOtherFile.e_RequestTransferFileExitSent = eSUSEQ_STATE_ERROR;
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_EXIT_ERROR, c_Return.value(),
                                u8_ProgressPercentage, mc_CurrentNode, "Could not finalize the transfer. Details: " +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         if ((c_Return == Errc::warn) && (u8_NrCode == C_OscProtocolDriverOsy::hu8_NR_CODE_GENERAL_PROGRAMMING_FAILURE))
         {
            q_RejectedByTargetLayer = true;
         }
         c_Return = Errc::com;
      }
      else
      {
         orc_StateOtherFile.e_RequestTransferFileExitSent = eSUSEQ_STATE_NO_ERR;
      }

      //if the target supports reading the "FileBasedTransferExitResult" and the result of "TransferExit" was either
      // "OK" or "rejected by target layer" then use that service to read string information about result
      if (orc_ProtocolFeatures.q_FileBasedTransferExitResultAvailable == true)
      {
         std::string c_TransferExitResult;
         if ((c_Return == Errc::success) || (q_RejectedByTargetLayer == true))
         {
            //do not overwrite c_Return; this function shall still fail if finalize failed
            std::error_code c_LocalReturn = Errc::success;
            c_LocalReturn = this->mpc_ComDriver->SendOsyRequestFileBasedTransferExitResult(
               mc_CurrentNode, c_TransferExitResult, &u8_NrCode);
            if (c_LocalReturn != Errc::success)
            {
               orc_StateOtherFile.e_RequestTransferFileExitResultSent = eSUSEQ_STATE_ERROR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_EXIT_ERROR, c_LocalReturn.value(),
                                      u8_ProgressPercentage, mc_CurrentNode,
                                      "Could not read exit result text. Details: " +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_LocalReturn,
                                                                                               u8_NrCode));
               c_Return = Errc::com;
               c_TransferExitResult = "unkown (reading information failed)";
            }
            else
            {
               orc_StateOtherFile.e_RequestTransferFileExitResultSent = eSUSEQ_STATE_NO_ERR;
            }
         }
         else
         {
            orc_StateOtherFile.e_RequestTransferFileExitResultSent = eSUSEQ_STATE_NOT_EXECUTED;
            c_TransferExitResult = "unkown (reading information not supported by server device)";
         }
         m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_RESULT_STRING, C_NO_ERR, 100U, mc_CurrentNode,
                          "Result of file transfer: \"" + c_TransferExitResult + "\"");
      }
      else
      {
         orc_StateOtherFile.e_RequestTransferFileExitResultSent = eSUSEQ_STATE_NOT_NEEDED;
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_FINISHED, C_NO_ERR, 100U, mc_CurrentNode,
                             "Flashing file finished.");
   }

   return c_Return;
} //lint !e593 //pc_File is closed in all paths; looks like a false positive

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write one or more NVM parameter set file(s) to openSYDE node

   Files to be written must be in valid openSYDE .psi_syde file format

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * list of files must be > 0
   * node is active

   * Reports progress from 0..100 for the overall process

   Sequence performed:
   * check server for MaxNumberOfBlockLength
   * set received MaxNumberOfBlockLength in installed TP
   * for all files to write
   ** open file
   ** write file content to device

   Strategy for setting up required "DataDealer":
    To set up the DataDealerNvm we need to set up a DiagProtocol; but we only have a ProtocolDriverOsy
    So we create a DiagProtocol and copy over the known settings from the already up-and-running ProtocolDriverOsy.

   \param[in]      orc_FilesToWrite              Files to write to NVM
   \param[in]      orc_ProtocolFeatures          Information about available protocol features
   \param[in]      oq_SetProgrammingMode         In: Flag if programming mode must be set.
   \param[out]     orc_StatePsiFiles             States of all handled psi files

   \return
   Errc::success     all files were written
   Errc::rd_wr       one of the files is not a valid .psi_syde file or does not exist
   Errc::config      one of the files contains data for zero or more than one device
                     (expected: data for exactly one device)
   Errc::default_    one of the files is present but checksum is invalid
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::busy        procedure aborted by user (as returned by m_ReportProgress)
   Errc::range       At least one feature of the openSYDE Flashloader is not available for NVM writing
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_WriteNvmOpenSyde(
   const std::vector<std::string> & orc_FilesToWrite,
   const C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ProtocolFeatures, const bool oq_SetProgrammingMode,
   std::vector<C_OscSuSequencesNodePsiFileStates> & orc_StatePsiFiles)
{
   std::error_code c_Return = Errc::success;

   //Get pointer to OSY protocol driver provided by comm driver:
   C_OscProtocolDriverOsyTpBase * const pc_TransportProtocol = mpc_ComDriver->GetOsyTransportProtocol(mu32_CurrentNode);

   tgl_assert(pc_TransportProtocol != nullptr);
   if (pc_TransportProtocol != nullptr)
   {
      uint16_t u16_MaxBlockLength = 0U;
      C_OscProtocolDriverOsyNode c_Client;
      C_OscProtocolDriverOsyNode c_Server;
      C_OscDiagProtocolOsy c_DiagProtocol;
      C_OscNode & rc_Node = this->mpc_SystemDefinition->c_Nodes[mu32_CurrentNode];

      //get node-IDs from ProtocolDriver and set in DiagProtocol:
      pc_TransportProtocol->GetNodeIdentifiers(c_Client, c_Server);
      c_Return = c_DiagProtocol.SetNodeIdentifiers(c_Client, c_Server);
      tgl_assert(c_Return == Errc::success);
      //Set transport protocol in DiagProtocol:
      c_Return = c_DiagProtocol.SetTransportProtocol(pc_TransportProtocol);
      tgl_assert(c_Return == Errc::success);

      //set up DataDealer:
      C_OscDataDealerNvmSafe c_Dealer(&rc_Node, mu32_CurrentNode, &c_DiagProtocol);

      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_START, C_NO_ERR, 0U, mc_CurrentNode,
                             "Writing parameter set image files ...");

      //do we have the required features ?
      if ((orc_ProtocolFeatures.q_FlashloaderCanWriteToNvm == false) ||
          (orc_ProtocolFeatures.q_MaxNumberOfBlockLengthAvailable == false))
      {
         c_Return = Errc::range;
         // Both features are necessary to write NVM files to flashloader
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_AVAILABLE_FEATURE_ERROR, c_Return.value(), 5U,
                                mc_CurrentNode,
                                "The node has not the Flashloader feature to write a Parameter Set Image file.");
      }

      if (c_Return == Errc::success)
      {
         if (oq_SetProgrammingMode == true)
         {
            // In the whole update sequence, setting the programming mode only one time
            c_Return = this->mpc_ComDriver->SendOsySetProgrammingMode(mc_CurrentNode);
            // Last step with this security level in this sequence
         }
         if (c_Return != Errc::success)
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_SESSION_ERROR, c_Return.value(), 5U,
                                   mc_CurrentNode,
                                   "Could not activate programming session.");
            if (c_Return != Errc::checksum)
            {
               c_Return = Errc::com;
            }
         }
      }

      if (c_Return == Errc::success)
      {
         //check server for MaxNumberOfBlockLength
         c_Return = c_DiagProtocol.OsyReadMaxNumberOfBlockLength(u16_MaxBlockLength);
         if (c_Return != Errc::success)
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_MAX_SIZE_ERROR, c_Return.value(), 5U,
                                   mc_CurrentNode,
                                   "Could not get max number of block length from device.");
            //this information is not available in older Flashloaders
            //the application is responsible to not try this sequence for those servers
            c_Return = Errc::com;
         }
      }
      if (c_Return == Errc::success)
      {
         //set received MaxNumberOfBlockLength in installed TP
         c_DiagProtocol.SetMaxServiceSize(u16_MaxBlockLength);

         for (uint16_t u16_File = 0U; u16_File < orc_FilesToWrite.size(); u16_File++)
         {
            C_OscSuSequencesNodePsiFileStates & rc_State = orc_StatePsiFiles[u16_File];
            //logic: split 80% (between 10% and 90%) by the number of files
            const float32_t f32_Percent = (((80.0F / static_cast<float32_t>(orc_FilesToWrite.size()))) *
                                           static_cast<float32_t>(u16_File));
            const uint8_t u8_Percent = 10U + static_cast<uint8_t>(f32_Percent);
            bool q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_OPEN_FILE_START, C_NO_ERR, u8_Percent,
                                            mc_CurrentNode,
                                            "Reading parameter set image file \"" + orc_FilesToWrite[u16_File] +
                                            "\"...");
            if (q_Abort == true)
            {
               c_Return = Errc::busy;
            }
            else
            {
               c_Return = c_Dealer.NvmSafeReadFileWithCrc(orc_FilesToWrite[u16_File]);
               if (c_Return != Errc::success)
               {
                  rc_State.e_FileLoaded = eSUSEQ_STATE_ERROR;
                  if (c_Return == Errc::checksum)
                  {
                     // Remap to have a unique return value for this case
                     c_Return = Errc::default_;
                  }
               }
               else
               {
                  rc_State.e_FileLoaded = eSUSEQ_STATE_NO_ERR;
               }
            }
            if (c_Return != Errc::success)
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_OPEN_FILE_ERROR, c_Return.value(), u8_Percent,
                                      mc_CurrentNode,
                                      "Could not read parameter set image file \"" + orc_FilesToWrite[u16_File] +
                                      "\"!");
            }
            else
            {
               q_Abort = m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_WRITE_FILE_START, C_NO_ERR,
                                          u8_Percent, mc_CurrentNode,
                                          "Writing data of parameter set image file \"" +
                                          orc_FilesToWrite[u16_File] + "\" to device ...");
               if (q_Abort == true)
               {
                  c_Return = Errc::busy;
               }
               else
               {
                  int32_t s32_ResultDetail;
                  c_Return =
                     c_Dealer.NvmSafeWriteParameterSetFile(orc_FilesToWrite[u16_File], s32_ResultDetail);
               }
               if (c_Return != Errc::success)
               {
                  rc_State.e_PsiFileWrote = eSUSEQ_STATE_ERROR;
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_WRITE_FILE_ERROR, c_Return.value(),
                                         u8_Percent,
                                         mc_CurrentNode,
                                         "Could not write data of parameter set image file \"" +
                                         orc_FilesToWrite[u16_File] + "\" to device !");
                  //C_OVERFLOW, C_CONFIG: would be a systematic error in this implementation
                  //rest: C_COM
                  c_Return = Errc::com;
               }
               else
               {
                  rc_State.e_PsiFileWrote = eSUSEQ_STATE_NO_ERR;
               }
            }
            if (c_Return == Errc::success)
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_FILE_FINISHED, c_Return.value(), u8_Percent,
                                      mc_CurrentNode, "Writing parameter set image file to device finished.");
            }
            else
            {
               break;
            }
         }
      }

      if (c_Return == Errc::success)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_FINISHED, c_Return.value(), 100U, mc_CurrentNode,
                                "Writing parameter set image file(s) to device finished.");
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write one PEM file to openSYDE node

   File to be written must be in valid .pem file format

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * orc_FileToWrite must be a valid file path
   * node is active

   * Reports progress from 0..100 for the overall process

   \param[in]      orc_FileToWrite               PEM file to write
   \param[in]      orc_ProtocolFeatures          Information about available protocol features
   \param[in,out]  orq_SetProgrammingMode        In: Flag if programming mode must be set.
                                                 Out: Flag if programming mode was set.
   \param[out]     orc_StateSecuritySettings     State of security settings (info will be merged in)

   \return
   Errc::success     file was written or nothing to do
   Errc::rd_wr       the file is not a valid .pem file or does not exist
   Errc::warn        the file is present but key details of PEM file could not be extracted
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::range       At least one feature of the openSYDE Flashloader is not available for NVM writing
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_WritePemOpenSydeFile(
   const std::string & orc_FileToWrite, const C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ProtocolFeatures,
   bool & orq_SetProgrammingMode, C_OscSuSequencesNodeSecuritySettingsStates & orc_StateSecuritySettings)
{
   std::error_code c_Return = Errc::success;

   if (orc_FileToWrite != "")
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_START, C_NO_ERR, 0U, mc_CurrentNode,
                             "Writing PEM file ...");

      if (orc_ProtocolFeatures.q_SupportsSecurityAuthentication == false)
      {
         c_Return = Errc::range;
         // Security feature is necessary to write PEM file to flashloader
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_AVAILABLE_FEATURE_ERROR, c_Return.value(), 5U,
                                mc_CurrentNode,
                                "The node has not the Flashloader features to support security authentication"
                                " and to write PEM files.");
      }
      else
      {
         C_OscSecurityPem c_PemFile;
         std::string c_ErrorMessage;

         c_Return = c_PemFile.LoadFromFile(orc_FileToWrite.c_str(), c_ErrorMessage);

         if (c_Return == Errc::success)
         {
            const std::vector<uint8_t> c_PubKeyDecoded = c_PemFile.GetKeyInfo().GetX509CertificateData();
            std::vector<uint8_t> c_PubKeyModulus;
            std::vector<uint8_t> c_PubKeyExponent;

            orc_StateSecuritySettings.e_FileLoaded = eSUSEQ_STATE_NO_ERR;

            c_Return = C_OscSecurityPem::h_ExtractModulusAndExponent(c_PubKeyDecoded, c_PubKeyModulus,
                                                                       c_PubKeyExponent,
                                                                       c_ErrorMessage);

            if (c_Return == Errc::success)
            {
               orc_StateSecuritySettings.e_PemFileExtracted = eSUSEQ_STATE_NO_ERR;

               if (orq_SetProgrammingMode == true)
               {
                  // In the whole update sequence, setting the programming mode only one time
                  const uint8_t u8_SECURITY_LEVEL = 1U;
                  c_Return =
                     this->mpc_ComDriver->SendOsySetProgrammingMode(mc_CurrentNode, &u8_SECURITY_LEVEL);
                  orq_SetProgrammingMode = false;
               }
               if (c_Return != Errc::success)
               {
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_SESSION_ERROR, c_Return.value(), 10U,
                                         mc_CurrentNode,
                                         "Could not activate programming session.");
                  if (c_Return != Errc::checksum)
                  {
                     c_Return = Errc::com;
                  }
               }
            }
            else
            {
               orc_StateSecuritySettings.e_PemFileExtracted = eSUSEQ_STATE_ERROR;
            }

            if (c_Return == Errc::success)
            {
               const std::vector<uint8_t> c_KeySerialNumber = c_PemFile.GetKeyInfo().GetCertificateSerialNumber();
               uint8_t u8_NrCode;

               c_Return = this->mpc_ComDriver->SendOsyWriteSecurityAuthenticationKey(this->mc_CurrentNode,
                                                                                       c_PubKeyModulus,
                                                                                       c_PubKeyExponent,
                                                                                       c_KeySerialNumber,
                                                                                       &u8_NrCode);

               if (c_Return != Errc::success)
               {
                  (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_SEND_ERROR, c_Return.value(),
                                         75U, mc_CurrentNode,
                                         "Could not write security authentication key. Details: " +
                                         C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                                  u8_NrCode));
                  orc_StateSecuritySettings.e_SecureAuthenticationKeySent = eSUSEQ_STATE_ERROR;
                  c_Return = Errc::com;
               }
               else
               {
                  orc_StateSecuritySettings.e_SecureAuthenticationKeySent = eSUSEQ_STATE_NO_ERR;
               }
            }
            else
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_EXTRACT_KEY_ERROR, c_Return.value(),
                                      50U, mc_CurrentNode,
                                      "Could not load PEM file. Details: " + c_ErrorMessage);
               c_Return = Errc::warn;
            }
         }
         else
         {
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_OPEN_FILE_ERROR, c_Return.value(),
                                   25U, mc_CurrentNode,
                                   "Could not extract security authentication key from decoded public key. Details: " +
                                   c_ErrorMessage);
            orc_StateSecuritySettings.e_FileLoaded = eSUSEQ_STATE_ERROR;
            c_Return = Errc::rd_wr;
         }
      }

      if (c_Return == Errc::success)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_FINISHED, c_Return.value(), 100U, mc_CurrentNode,
                                "Writing PEM file to device finished.");
      }
   }
   else
   {
      // Nothing to do, no error
      orc_StateSecuritySettings.e_FileLoaded = eSUSEQ_STATE_NOT_NEEDED;
      orc_StateSecuritySettings.e_SecureAuthenticationKeySent = eSUSEQ_STATE_NOT_NEEDED;
      c_Return = Errc::success;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write security related settings to openSYDE node

   Contains:
   * secure authentication on/off
   * traffic encryption on/off
   * debugger interface on/off

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * node is active

   \param[in]      orc_ApplicationsToWrite       Update configuration with all states for sending or not sending
   \param[in]      orc_ProtocolFeatures          Information about available protocol features
   \param[in,out]  orq_SetProgrammingMode        In: Flag if programming mode must be set.
                                                 Out: Flag if programming mode was already set.
   \param[out]     orc_StateSecuritySettings     State of security settings (info will be merged in)

   \return
   Errc::success     all settings were written or no write process was required
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::range       At least one feature of the openSYDE Flashloader is not available for writing
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_WriteOpenSydeNodeStates(
   const C_OscSuSequences::C_DoFlash & orc_ApplicationsToWrite,
   const C_OscProtocolDriverOsy::C_ListOfFeatures & orc_ProtocolFeatures, bool & orq_SetProgrammingMode,
   C_OscSuSequencesNodeSecuritySettingsStates & orc_StateSecuritySettings)
{
   std::error_code c_Return = Errc::success;
   const uint8_t u8_SECURITY_LEVEL = 1U;

   //We need to make sure we are in programming mode if we want to write at least one of the flags:
   if ((orc_ApplicationsToWrite.q_SendDebuggerEnabledState == true) ||
       (orc_ApplicationsToWrite.q_SendSecureAuthenticationEnabledState == true) ||
       (orc_ApplicationsToWrite.q_SendTrafficEncryptionEnabledState == true))
   {
      if (orq_SetProgrammingMode == true)
      {
         // In the whole update sequence, setting the programming mode only one time
         c_Return = this->mpc_ComDriver->SendOsySetProgrammingMode(mc_CurrentNode, &u8_SECURITY_LEVEL);
         orq_SetProgrammingMode = false;
      }
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_ENTER_SESSION_ERROR, c_Return.value(),
                                10U, mc_CurrentNode,
                                "Could not activate programming session.");
         if (c_Return != Errc::checksum)
         {
            c_Return = Errc::com;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      // secure authentication state
      if (orc_ApplicationsToWrite.q_SendSecureAuthenticationEnabledState == true)
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_SECURE_AUTHENTICATION_WRITE_START, C_NO_ERR, 0U,
                                mc_CurrentNode,
                                "Writing security authentication activation ...");

         if (orc_ProtocolFeatures.q_SupportsSecurityAuthentication == true)
         {
            uint8_t u8_NrCode;
            // Only RSA 1024 supported at the moment, so 0 for security authentication algorithm
            c_Return = this->mpc_ComDriver->SendOsyWriteSecurityAuthenticationActivation(
               this->mc_CurrentNode,
               orc_ApplicationsToWrite.q_SecureAuthenticationEnabled,
               0U, &u8_NrCode);

            if (c_Return == Errc::success)
            {
               orc_StateSecuritySettings.e_SecureAuthenticationFlagSent = eSUSEQ_STATE_NO_ERR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_SECURE_AUTHENTICATION_WRITE_FINISHED,
                                      c_Return.value(),
                                      100U,
                                      mc_CurrentNode,
                                      "Writing security authentication activation to device finished.");
            }
            else
            {
               orc_StateSecuritySettings.e_SecureAuthenticationFlagSent = eSUSEQ_STATE_ERROR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_SECURE_AUTHENTICATION_WRITE_SEND_ERROR,
                                      c_Return.value(),
                                      50U, mc_CurrentNode,
                                      "Could not write security authentication activation. Details: " +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                               u8_NrCode));
               c_Return = Errc::com;
            }
         }
         else
         {
            // Security feature is necessary to write security activation state to flashloader
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_SECURE_AUTHENTICATION_WRITE_AVAILABLE_FEATURE_ERROR,
                                   c_Return.value(), 5U,
                                   mc_CurrentNode,
                                   "The node has not the Flashloader features to support security authentication.");

            c_Return = Errc::range;
         }
      }
      else
      {
         orc_StateSecuritySettings.e_SecureAuthenticationFlagSent = eSUSEQ_STATE_NOT_NEEDED;
      }

      // traffic encryption state
      if ((c_Return == Errc::success) && (orc_ApplicationsToWrite.q_SendTrafficEncryptionEnabledState == true))
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_TRAFFIC_ENCRYPTION_WRITE_START, C_NO_ERR, 0U,
                                mc_CurrentNode,
                                "Writing traffic encryption activation ...");

         if (orc_ProtocolFeatures.q_SupportsSecurityTrafficEncryption == true)
         {
            uint8_t u8_NrCode;
            // Only ECDH/AES supported, so 0 for traffic encryption algorithm
            c_Return = this->mpc_ComDriver->SendOsyWriteSecurityTrafficEncryptionActivation(
               this->mc_CurrentNode,
               orc_ApplicationsToWrite.q_TrafficEncryptionEnabled,
               0U, &u8_NrCode);

            if (c_Return == Errc::success)
            {
               orc_StateSecuritySettings.e_TrafficEncryptionFlagSent = eSUSEQ_STATE_NO_ERR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_TRAFFIC_ENCRYPTION_WRITE_FINISHED, c_Return.value(),
                                      100U,
                                      mc_CurrentNode, "Writing traffic encryption activation to device finished.");
            }
            else
            {
               orc_StateSecuritySettings.e_TrafficEncryptionFlagSent = eSUSEQ_STATE_ERROR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_TRAFFIC_ENCRYPTION_WRITE_SEND_ERROR,
                                      c_Return.value(),
                                      50U, mc_CurrentNode,
                                      "Could not write traffic encryption activation. Details: " +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                               u8_NrCode));
               c_Return = Errc::com;
            }
         }
         else
         {
            // Feature is necessary to write traffic encryption state to flashloader
            (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_TRAFFIC_ENCRYPTION_WRITE_AVAILABLE_FEATURE_ERROR,
                                   c_Return.value(), 5U,
                                   mc_CurrentNode,
                                   "The node has not the Flashloader features to support traffic encryption.");

            c_Return = Errc::range;
         }
      }
      else
      {
         orc_StateSecuritySettings.e_TrafficEncryptionFlagSent = eSUSEQ_STATE_NOT_NEEDED;
      }

      // Debugger state
      if ((c_Return == Errc::success) && (orc_ApplicationsToWrite.q_SendDebuggerEnabledState == true))
      {
         (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_START, C_NO_ERR, 0U, mc_CurrentNode,
                                "Writing debugger state ...");

         if (((orc_ProtocolFeatures.q_SupportsDebuggerOn == true) &&
              (orc_ApplicationsToWrite.q_DebuggerEnabled == true)) ||
             ((orc_ProtocolFeatures.q_SupportsDebuggerOff == true) &&
              (orc_ApplicationsToWrite.q_DebuggerEnabled == false)))
         {
            uint8_t u8_NrCode;
            c_Return = this->mpc_ComDriver->SendOsyWriteDebuggerEnabled(this->mc_CurrentNode,
                                                                          orc_ApplicationsToWrite.q_DebuggerEnabled,
                                                                          &u8_NrCode);

            if (c_Return == Errc::success)
            {
               orc_StateSecuritySettings.e_DebuggerFlagSent = eSUSEQ_STATE_NO_ERR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_FINISHED, c_Return.value(), 100U,
                                      mc_CurrentNode, "Writing debugger state to device finished.");
            }
            else
            {
               orc_StateSecuritySettings.e_DebuggerFlagSent = eSUSEQ_STATE_ERROR;
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_SEND_ERROR, c_Return.value(),
                                      50U, mc_CurrentNode,
                                      "Could not write debugger state. Details: " +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                               u8_NrCode));
               c_Return = Errc::com;
            }
         }
         else
         {
            // Debugger state change feature is necessary to write debugger state to flashloader
            if (orc_ApplicationsToWrite.q_DebuggerEnabled == true)
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_AVAILABLE_FEATURE_ERROR,
                                      c_Return.value(),
                                      5U,
                                      mc_CurrentNode,
                                      "The node has not the Flashloader feature to enable the debugger.");
            }
            else
            {
               (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_AVAILABLE_FEATURE_ERROR,
                                      c_Return.value(),
                                      5U,
                                      mc_CurrentNode,
                                      "The node has not the Flashloader feature to disable the debugger.");
            }
            c_Return = Errc::range;
         }
      }
      else
      {
         orc_StateSecuritySettings.e_DebuggerFlagSent = eSUSEQ_STATE_NOT_NEEDED;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write openSYDE flashing fingerprint

   Assumptions/prerequisites (not explicitly checked by this function):
   * mc_CurrentNode contains ID of node to work with
   * server node must be in Flashloader mode
   * server node must be in programming session

   * Reports progress as 30

   \return
   Errc::success    flashed all files
   Errc::com        error result from device (see log for details)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_WriteFingerPrintOsy(void)
{
   //all prerequisites checked; commence the flashing ...
   //write fingerprint
   const C_SclDateTime c_Now = C_SclDateTime::Now();
   uint8_t au8_Date[3];
   uint8_t au8_Time[3];
   std::string c_UserName;
   bool q_Return;
   uint8_t u8_NrCode;
   std::error_code c_Return = Errc::success;

   au8_Date[0] = static_cast<uint8_t>(c_Now.mu16_Year % 1000U);
   au8_Date[1] = static_cast<uint8_t>(c_Now.mu16_Month);
   au8_Date[2] = static_cast<uint8_t>(c_Now.mu16_Day);
   au8_Time[0] = static_cast<uint8_t>(c_Now.mu16_Hour);
   au8_Time[1] = static_cast<uint8_t>(c_Now.mu16_Minute);
   au8_Time[2] = static_cast<uint8_t>(c_Now.mu16_Second);

   (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FINGERPRINT_START, C_NO_ERR, 30U, mc_CurrentNode,
                          "Writing fingerprint ...");
   q_Return = stw::tgl::TglGetSystemUserName(c_UserName);
   if (q_Return != true)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FINGERPRINT_NAME_NOT_READABLE, C_WARN, 30U, mc_CurrentNode,
                             "Could not get user name from system. Using \"unknown\".");
      c_UserName = "unknown";
   }

   c_Return = this->mpc_ComDriver->SendOsyWriteApplicationSoftwareFingerprint(mc_CurrentNode, au8_Date, au8_Time,
                                                                                c_UserName, &u8_NrCode);
   if (c_Return != Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FINGERPRINT_ERROR, c_Return.value(), 30U, mc_CurrentNode,
                             "Could not write fingerprint. Details:" +
                             C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
      c_Return = Errc::com;
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read information of openSYDE server node

   Utility function used to read openSYDE information from server node and report it.
   See documentation of ReadDeviceInformation() for full description.

   \param[in]     ou8_ProgressToReport    progress value to report when invoking virtual reporting function
   \param[in]     ou32_NodeIndex          Index of node within mpc_SystemDefinition
   \param[in,out] orc_NodeState           Connect states of node

   \return
   Errc::com        communication driver reported problem (details will be written to log file)
   Errc::checksum   Security related error (something went wrong while handshaking with the server)
   Errc::success    information read
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_ReadDeviceInformationOpenSyde(const uint8_t ou8_ProgressToReport,
                                                                  const uint32_t ou32_NodeIndex,
                                                                  C_OscSuSequencesNodeConnectStates & orc_NodeState)
{
   C_OsyDeviceInformation c_Info;
   uint8_t u8_NrCode;
   std::error_code c_Return = Errc::success;

   // If connected via Ethernet we might need to reconnect (in case we ran into the session timeout)
   c_Return = this->m_ReconnectToTargetServer();

   if (c_Return != Errc::success)
   {
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_RECONNECT_ERROR, c_Return.value(), ou8_ProgressToReport,
                             mc_CurrentNode,
                             "Could not reconnect to node");
      if (c_Return != Errc::checksum)
      {
         c_Return = Errc::com;
      }
   }

   if (c_Return == Errc::success)
   {
      //node is active and flashable directly on the bus that the client is connected to
      //to prevent any session timeout issues request the pre-programming session and activate security
      // level 1
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_SET_SESSION_START, C_NO_ERR, ou8_ProgressToReport, mc_CurrentNode,
                             "Activating PreProgramming session ...");
      c_Return =
         this->mpc_ComDriver->SendOsySetPreProgrammingMode(mc_CurrentNode, false, &u8_NrCode,
                                                           &orc_NodeState.q_AuthenticationNecessary,
                                                           &orc_NodeState.q_TrafficEncryptionNecessary);
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_SET_SESSION_ERROR, c_Return.value(), ou8_ProgressToReport,
                                mc_CurrentNode,
                                "Error activating PreProgramming session. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));

         if (c_Return != Errc::checksum)
         {
            c_Return = Errc::com;
         }
         else
         {
            // set flag to report that security access failed
            orc_NodeState.q_SecurityAccessError = true;
            orc_NodeState.q_SecurityOptionsActiveSet = true;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //get device name
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_DEVICE_NAME_START, C_NO_ERR, ou8_ProgressToReport, mc_CurrentNode,
                             "Reading device name ...");
      c_Return = this->mpc_ComDriver->SendOsyReadDeviceName(mc_CurrentNode, c_Info.c_DeviceName, &u8_NrCode);
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_DEVICE_NAME_ERROR, c_Return.value(), ou8_ProgressToReport,
                                mc_CurrentNode,
                                "Error reading device name. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         c_Return = Errc::com;
      }
   }

   //get information about all flash blocks
   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASH_BLOCKS_START, C_NO_ERR, ou8_ProgressToReport, mc_CurrentNode,
                             "Reading flash block information ...");

      //we need security level 1 for that:
      c_Return = this->mpc_ComDriver->SendOsySetSecurityLevel(mc_CurrentNode, 1U, &u8_NrCode);
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASH_BLOCKS_SECURITY_ERROR, c_Return.value(),
                                ou8_ProgressToReport, mc_CurrentNode,
                                "Error setting security level for reading flash block information. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         if (c_Return != Errc::checksum)
         {
            c_Return = Errc::com;
         }
      }
      else
      {
         const C_OscNode & rc_CurNode = this->mpc_SystemDefinition->c_Nodes[ou32_NodeIndex];
         //this information is only available for address based devices
         tgl_assert(rc_CurNode.u32_SubDeviceIndex < rc_CurNode.pc_DeviceDefinition->c_SubDevices.size());
         if (rc_CurNode.pc_DeviceDefinition->c_SubDevices[rc_CurNode.u32_SubDeviceIndex].
             q_FlashloaderOpenSydeIsFileBased == false)
         {
            c_Return = this->mpc_ComDriver->SendOsyReadAllFlashBlockData(mc_CurrentNode, c_Info.c_Applications,
                                                                           &u8_NrCode);
            if (c_Return != Errc::success)
            {
               (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASH_BLOCKS_ERROR, c_Return.value(), ou8_ProgressToReport,
                                      mc_CurrentNode, "Error reading flash block information. Details:" +
                                      C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return,
                                                                                               u8_NrCode));
               c_Return = Errc::com;
            }
         }
         else
         {
            c_Info.c_Applications.resize(0);
         }
      }
   }

   //get rest of information
   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASHLOADER_INFO_START, C_NO_ERR, ou8_ProgressToReport,
                             mc_CurrentNode, "Reading even more information ...");
      c_Return = this->mpc_ComDriver->SendOsyReadInformationFromFlashloader(mc_CurrentNode,
                                                                              c_Info.c_MoreInformation,
                                                                              &u8_NrCode);
      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASHLOADER_INFO_ERROR, c_Return.value(), ou8_ProgressToReport,
                                mc_CurrentNode, "Error reading even more information. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         c_Return = Errc::com;
      }
      else
      {
         orc_NodeState.c_AvailableFeatures = c_Info.c_MoreInformation.c_AvailableFeatures;
      }
   }

   // check whether debugger is active
   if ((c_Return == Errc::success) &&
       ((c_Info.c_MoreInformation.c_AvailableFeatures.q_SupportsDebuggerOn == true) ||
        (c_Info.c_MoreInformation.c_AvailableFeatures.q_SupportsDebuggerOff == true)))
   {
      (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASHLOADER_CHECK_DEBUGGER_ACTIVATION_START, C_NO_ERR,
                             ou8_ProgressToReport,
                             mc_CurrentNode, "Reading debugger activation state ...");
      c_Return = this->mpc_ComDriver->SendOsyReadDebuggerEnabled(mc_CurrentNode,
                                                                   orc_NodeState.q_DebuggerEnabled,
                                                                   &u8_NrCode);

      if (c_Return != Errc::success)
      {
         (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FLASHLOADER_CHECK_DEBUGGER_ACTIVATION_ERROR, c_Return.value(),
                                ou8_ProgressToReport,
                                mc_CurrentNode, "Error reading debugger activation state. Details:" +
                                C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));
         c_Return = Errc::com;
      }
   }

   if (c_Return == Errc::success)
   {
      // information was read and flags were set
      orc_NodeState.q_SecurityOptionsActiveSet = true;
   }

   (void)this->m_DisconnectFromTargetServer();

   if (c_Return == Errc::success)
   {
      //report findings to application:
      this->m_ReportOpenSydeFlashloaderInformationRead(c_Info, ou32_NodeIndex);
      orc_NodeState.e_InformationRead = eSUSEQ_STATE_NO_ERR;
   }
   else
   {
      orc_NodeState.e_InformationRead = eSUSEQ_STATE_ERROR;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Set up class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSuSequences::C_OscSuSequences(void) :
   C_OscComSequencesBase(true, true),
   mu32_CurrentNode(0U),
   mu32_CurrentFile(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Tear down class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSuSequences::~C_OscSuSequences(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy referenced files to a temporary folder

   Copy all files to be flashed and/or written to NVM to a "temp" folder.
   Ensures a clearly defined state during the whole system update procedure.
   To be called optionally before starting the procedure with "ActivateFlashloader()".

   * Can be called before "Init()" as all required information is passed as parameters.

   * All referenced files must be present.
   * Validity is not checked (will be done at UpdateSystem()).
   * Will erase a pre-existing folder completely
   * Will create the target folder (recursively if it needs to be)
   * Files for address based targets and NVM files will be renamed to prevent possible conflicts
   * Flash files for file based devices will not be renamed (as they need to be unique anyway)
   * Does not report any progress via m_ReportProgress().

   \param[in]     orc_Nodes               List of nodes (part of system definition)
   \param[in]     orc_ActiveNodes         Vector of flags for each node in orc_Nodes
                                          - true: file path(s) for the target are expected to be present
                                          - false: file path(s) for the target are not expected to be present
   \param[in]     orc_TargetPath          Path to place files in (with trailing path separator)
   \param[in,out] orc_ApplicationsToWrite in: Paths to files to copy
                                          out: Paths to resulting files copied and renamed by this function
                                          if the function returns an error this parameter will not be modified
   \param[in,out] opc_ErrorPath           Optional pointer to store path error details (current: which file did fail)

   \return
   Errc::success     files copied
   Errc::overflow    size of orc_ApplicationsToWrite is not the same as the size of nodes in orc_Nodes
                     size of orc_ActiveNodes is not the same as the size of nodes in orc_Nodes
   Errc::noact       orc_ApplicationsToWrite has non-empty list of files for node that was not set as active
                     in orc_ActiveNodes
                     size of files in orc_ApplicationsToWrite[node] differs from the
                     number of applications of the node in orc_Nodes (for an active and address based node)
   Errc::range       Flash or NVM file referenced by orc_ApplicationsToWrite does not exist
                     orc_TargetPath does not end in "\" or "/"
   Errc::busy        could not erase pre-existing target path (note: can result in partially erased target path)
   Errc::rd_wr       could not copy file
   Errc::timeout     could not create target directory
   Errc::config      at least one file based node has at least two identical named files (independent of character case)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::h_CreateTemporaryFolder(const std::vector<C_OscNode> & orc_Nodes,
                                                          const std::vector<uint8_t> & orc_ActiveNodes,
                                                          const std::string & orc_TargetPath,
                                                          std::vector<C_DoFlash> & orc_ApplicationsToWrite,
                                                          std::string * const opc_ErrorPath)
{
   std::error_code c_Return = Errc::success;

   std::vector<C_DoFlash> c_NodesToFlashNewPaths = orc_ApplicationsToWrite;
   std::vector<std::string> c_NodeTargetPaths;

   const char_t cn_LastCharacter = orc_TargetPath[orc_TargetPath.length()];

   if ((cn_LastCharacter != '\\') && (cn_LastCharacter != '/'))
   {
      c_Return = Errc::range;

      if (opc_ErrorPath != nullptr)
      {
         *opc_ErrorPath = orc_TargetPath;
      }
   }
   else
   {
      if ((orc_ApplicationsToWrite.size() != orc_Nodes.size()) || (orc_ActiveNodes.size() != orc_Nodes.size()))
      {
         c_Return = Errc::overflow;
      }
   }
   if (c_Return == Errc::success)
   {
      //consistent configuration ?
      for (uint16_t u16_Node = 0U; u16_Node < orc_Nodes.size(); u16_Node++)
      {
         tgl_assert(orc_Nodes[u16_Node].pc_DeviceDefinition != nullptr);

         //node inactive but files defined ?
         if ((orc_ActiveNodes[u16_Node] == 0U) &&
             ((orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size() != 0) ||
              (orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm.size() != 0) ||
              (orc_ApplicationsToWrite[u16_Node].c_PemFile != "")))
         {
            //file(s) defined for inactive node -> cry
            c_Return = Errc::noact;
         }

         if (c_Return == Errc::success)
         {
            //do all files exist ?
            if (orc_ActiveNodes[u16_Node] == 1U)
            {
               //files for flash:
               for (uint16_t u16_File = 0U; u16_File < orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size();
                    u16_File++)
               {
                  const std::string c_File = orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u16_File];
                  if (TglFileExists(c_File) == false)
                  {
                     c_Return = Errc::range;
                     if (opc_ErrorPath != nullptr)
                     {
                        *opc_ErrorPath = c_File;
                     }
                     break;
                  }
               }
               //files for NVM:
               for (uint16_t u16_File = 0U; u16_File < orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm.size();
                    u16_File++)
               {
                  const std::string c_File = orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u16_File];
                  if (TglFileExists(c_File) == false)
                  {
                     c_Return = Errc::range;
                     if (opc_ErrorPath != nullptr)
                     {
                        *opc_ErrorPath = c_File;
                     }
                     break;
                  }
               }
               // PEM file
               if (orc_ApplicationsToWrite[u16_Node].c_PemFile != "")
               {
                  const std::string c_File = orc_ApplicationsToWrite[u16_Node].c_PemFile;
                  if (TglFileExists(c_File) == false)
                  {
                     c_Return = Errc::range;
                     if (opc_ErrorPath != nullptr)
                     {
                        *opc_ErrorPath = c_File;
                     }
                  }
               }
            }
         }

         if (c_Return == Errc::success)
         {
            // Special case: File based nodes shall have unique file names
            tgl_assert(
               orc_Nodes[u16_Node].u32_SubDeviceIndex <
               orc_Nodes[u16_Node].pc_DeviceDefinition->c_SubDevices.size());
            if (orc_Nodes[u16_Node].pc_DeviceDefinition->c_SubDevices[orc_Nodes[u16_Node].u32_SubDeviceIndex].
                q_FlashloaderOpenSydeIsFileBased == true)
            {
               std::vector<std::string> c_Files = orc_ApplicationsToWrite[u16_Node].c_FilesToFlash;
               //convert all file names to lower case to detect conflicts in the file system
               //also: remove paths
               for (uint16_t u16_File = 0U; u16_File < c_Files.size(); u16_File++)
               {
                  c_Files[u16_File] = LowerCaseCompat(TglExtractFileName(c_Files[u16_File]));
               }
               //get same names next to each other:
               std::sort(c_Files.begin(), c_Files.end());
               //remove duplicates:
               c_Files.erase(std::unique(c_Files.begin(), c_Files.end()), c_Files.end());
               if (c_Files.size() != orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size())
               {
                  //we have less files than originally; so there must have been dupes
                  c_Return = Errc::config;
               }
            }
         }

         if (c_Return != Errc::success)
         {
            break;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //erase target path if it exists:
      if (TglDirectoryExists(orc_TargetPath) == true)
      {
         //TglRemoveDirectory reports 0/non-zero, not the STW convention
         const int32_t s32_FileApiReturn = TglRemoveDirectory(orc_TargetPath, false);
         if (s32_FileApiReturn != 0)
         {
            c_Return = Errc::busy;
            if (opc_ErrorPath != nullptr)
            {
               *opc_ErrorPath = orc_TargetPath;
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //create target folder (from bottom-up if required):
      c_Return = C_OscUtils::h_CreateFolderRecursively(orc_TargetPath);
      if (c_Return != Errc::success)
      {
         if (opc_ErrorPath != nullptr)
         {
            *opc_ErrorPath = orc_TargetPath;
         }
         c_Return = Errc::timeout;
      }

      if (c_Return == Errc::success)
      {
         // Creating sub folders for each node in the target path
         c_NodeTargetPaths.resize(orc_Nodes.size(), "");

         for (uint16_t u16_Node = 0U; u16_Node < orc_Nodes.size(); u16_Node++)
         {
            if (orc_ActiveNodes[u16_Node] == 1U)
            {
               c_NodeTargetPaths[u16_Node] =
                  TglFileIncludeTrailingDelimiter(orc_TargetPath + C_OscUtils::h_NiceifyStringForFileName(
                                                     orc_Nodes[u16_Node].c_Properties.c_Name));

               //TglCreateDirectory reports 0/-1, not the STW convention
               const int32_t s32_FileApiReturn = TglCreateDirectory(c_NodeTargetPaths[u16_Node]);
               if (s32_FileApiReturn != 0)
               {
                  if (opc_ErrorPath != nullptr)
                  {
                     *opc_ErrorPath = c_NodeTargetPaths[u16_Node];
                  }
                  c_Return = Errc::timeout;
                  break;
               }
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //copy files
      for (uint16_t u16_Node = 0U; u16_Node < orc_Nodes.size(); u16_Node++)
      {
         if (orc_ActiveNodes[u16_Node] == 1U)
         {
            //flash files
            for (uint16_t u16_File = 0U; u16_File < orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size();
                 u16_File++)
            {
               //get source file name
               const std::string c_SourceFileName = orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u16_File];
               //compose target file name
               std::string c_TargetFileName;

               tgl_assert(
                  orc_Nodes[u16_Node].u32_SubDeviceIndex <
                  orc_Nodes[u16_Node].pc_DeviceDefinition->c_SubDevices.size());
               if (orc_Nodes[u16_Node].pc_DeviceDefinition->c_SubDevices[orc_Nodes[u16_Node].u32_SubDeviceIndex].
                   q_FlashloaderOpenSydeIsFileBased == true)
               {
                  // File based nodes need the unchanged file name and must be unique
                  c_TargetFileName = c_NodeTargetPaths[u16_Node] +
                                     TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u16_File]);
               }
               else
               {
                  // The original source file name is not relevant and does not need to be unique
                  // Add a counter to the target file name
                  c_TargetFileName = c_NodeTargetPaths[u16_Node] +
                                     std::to_string(static_cast<uint32_t>(u16_File) + 1U) + "_" +
                                     TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u16_File]);
               }

               //copy file
               c_Return = C_OscUtils::h_CopyFile(c_SourceFileName, c_TargetFileName, opc_ErrorPath);
               if (c_Return == Errc::success)
               {
                  c_NodesToFlashNewPaths[u16_Node].c_FilesToFlash[u16_File] = c_TargetFileName;
               }
               else
               {
                  break;
               }
            }
            //NVM files
            for (uint16_t u16_File = 0U; u16_File < orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm.size();
                 u16_File++)
            {
               //get source file name
               const std::string c_SourceFileName = orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u16_File];
               //compose target file name
               // The original file name is not relevant does not need to be unique
               // Add a counter to the target file name
               const std::string c_TargetFileName =
                  c_NodeTargetPaths[u16_Node] +
                  std::to_string(static_cast<uint32_t>(u16_File) + 1U) + "_" +
                  TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u16_File]);

               //copy file
               c_Return = C_OscUtils::h_CopyFile(c_SourceFileName, c_TargetFileName, opc_ErrorPath);
               if (c_Return == Errc::success)
               {
                  c_NodesToFlashNewPaths[u16_Node].c_FilesToWriteToNvm[u16_File] = c_TargetFileName;
               }
               else
               {
                  break;
               }
            }

            //PEM file
            if (c_NodesToFlashNewPaths[u16_Node].c_PemFile != "")
            {
               //get source file name
               const std::string c_SourceFileName = orc_ApplicationsToWrite[u16_Node].c_PemFile;
               //compose target file name
               const std::string c_TargetFileName =
                  c_NodeTargetPaths[u16_Node] +
                  TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_PemFile);

               //copy file
               c_Return = C_OscUtils::h_CopyFile(c_SourceFileName, c_TargetFileName, opc_ErrorPath);
               if (c_Return == Errc::success)
               {
                  c_NodesToFlashNewPaths[u16_Node].c_PemFile = c_TargetFileName;
               }
               else
               {
                  break;
               }
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      //copy over new paths to return to caller:
      orc_ApplicationsToWrite = c_NodesToFlashNewPaths;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether application on client side match applications on devices

   Compares a list of application properties to a second list of application properties.
   The function will report which elements of the first list are present in the second list.

   Use case:
   Check which applications need to be flashed.

   Workflow:
   * Read application properties from servers (e.g. using ReadDeviceInformation())
   * Parse application properties from client side hex files
        (e.g. using C_OscHexFile::ScanApplicationInformationBlockFromHexFile())
   * call this function
   * set up parameter for UpdateSystem() based on the results of this function

   \param[in]     orc_ClientSideApplications       properties of client side applications
   \param[in]     orc_ServerSideApplications       properties of server side applications
   \param[out]    orc_ApplicationsPresentOnServer   size matches size of orc_ClientSideApplications
                                                    0: application not contained in orc_ServerSideApplications
                                                    1: application contained in orc_ServerSideApplications
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSuSequences::h_CheckForChangedApplications(
   const std::vector<C_ApplicationProperties> & orc_ClientSideApplications,
   const std::vector<C_ApplicationProperties> & orc_ServerSideApplications,
   std::vector<uint8_t> & orc_ApplicationsPresentOnServer)
{
   orc_ApplicationsPresentOnServer.resize(orc_ClientSideApplications.size());
   for (uint32_t u32_ClientApplIndex = 0U; u32_ClientApplIndex < orc_ClientSideApplications.size();
        u32_ClientApplIndex++)
   {
      orc_ApplicationsPresentOnServer[u32_ClientApplIndex] = 0U;
      for (uint32_t u32_ServerApplIndex = 0U; u32_ServerApplIndex < orc_ServerSideApplications.size();
           u32_ServerApplIndex++)
      {
         if (orc_ServerSideApplications[u32_ServerApplIndex] == orc_ClientSideApplications[u32_ClientApplIndex])
         {
            orc_ApplicationsPresentOnServer[u32_ClientApplIndex] = 1U;
            break;
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialization of the protocol

   Enter Flashloader for all nodes
   Note: STW Flashloader nodes can only be end-points, not routers

   Prerequisite:
   * class members and communication has been set up with Init()

   Rough sequence:
   For local bus:
   * openSYDE nodes:
   ** send "RequestProgramming" to all nodes
   ** send "EcuReset" to all nodes
   ** if connected via CAN: send "EnterPreProgrammingSession" as broadcast (for a few seconds in short intervals)
   * STW Flashloader nodes (CAN only):
   ** send all configured reset request messages
   ** send "FLASH" (for a few seconds in short intervals;
      in parallel to the openSYDE "EnterPreProgrammingSession" broadcasts
   For confirmation:
   * use simple read service for all nodes that are expected to be present (as "ping")

   Then recurse for all buses that can be reached through the first bus:
   * set up routing and perform sequence bus-by-bus

   Result:
   * all nodes defined to be present on all buses are in flashloader mode

   \param[in]   oq_FailOnFirstError   true: abort all further communication if connecting to one device fails
                                      false: try to continue with other devices in this case

   \return
   Errc::success    flashloaders on all nodes activated
   Errc::config     mpc_SystemDefinition is NULL (Init() not called)
   Errc::com        communication driver reported problem (details will be written to log file)
   Errc::warn       activation for at least one device failed (see log for details)
   Errc::checksum   Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::ActivateFlashloader(const bool oq_FailOnFirstError)
{
   std::error_code c_Return = Errc::success;
   bool q_AtLeastOneError = false;
   const uint32_t u32_SCAN_TIME_MS = 5000U;
   const uint32_t u32_INTERVAL_TESTER_PRESENT = 1000U;

   if (this->mpc_SystemDefinition == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      uint8_t u8_NrCode;

      // Prepare only nodes on the local bus in the first step ...

      //check prerequisites (should have been checked at Init() already)
      tgl_assert(this->mpc_SystemDefinition->c_Nodes.size() == this->mc_ActiveNodes.size());
      tgl_assert(this->mu32_ActiveBusIndex < this->mpc_SystemDefinition->c_Buses.size());

      // send openSYDE CAN-TP "RequestProgramming" to each node
      (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_REQUEST_PROGRAMMING_START, C_NO_ERR, 0U,
                             "Sending openSYDE Flashloader activation requests ...");

      this->mpc_ComDriver->ClearDispatcherQueue();

      // Reset timeout flags
      this->mc_TimeoutNodes.clear();
      this->mc_TimeoutNodes.resize(this->mc_ActiveNodes.size(), 0U);

      // Reset states
      this->mc_ConnectStatesNodes.clear();
      this->mc_ConnectStatesNodes.resize(this->mc_ActiveNodes.size());

      // First set the request programming flag
      for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
      {
         C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;
         const bool q_IsActive = m_IsNodeActive(u16_Node, this->mu32_ActiveBusIndex, e_ProtocolType, mc_CurrentNode);

         if (q_IsActive == true)
         {
            c_Return = this->mpc_ComDriver->IsRoutingNecessary(u16_Node);

            // Continue with nodes without routing
            if (c_Return == Errc::noact)
            {
               (void)e_ProtocolType;
               c_Return = this->mpc_ComDriver->ReConnectNode(mc_CurrentNode);

               if (c_Return != Errc::success)
               {
                  // In this case, the node did not connect.
                  // Errors in case of not reachable nodes will be handled when
                  // the state eACTIVATE_FLASHLOADER_OSY_BC_PING_START was set the first time to have
                  // the same process sequence like using CAN.
                  // Therefore do not abort and send only a warning as progress information.
                  // This can only happen in case of Ethernet. The node is not connected, so the next
                  // request did not make any sense to send and will be skipped till the
                  // "EnterPreProgrammingSession" broadcasts. By skipping the other request,
                  // further sending errors will be avoided which would occur a "real" sending error.
                  (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_RECONNECT_WARNING, c_Return.value(),
                                         10U, mc_CurrentNode,
                                         "Device does not respond to RequestProgramming request.");

                  c_Return = Errc::success;
               }
               else
               {
                  // Set the request programming flag to bring the server into flashloader
                  c_Return = this->mpc_ComDriver->SendOsyRequestProgramming(mc_CurrentNode);

                  this->mpc_ComDriver->DisconnectNode(mc_CurrentNode);

                  if (c_Return != Errc::success)
                  {
                     // In this case, the node does not respond and maybe the next step can help to bring
                     // it into the flashloader by sending EnterPreProgrammingSession as broadcast
                     // Therefore do not abort and send only a warning as progress information.
                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_REQUEST_PROGRAMMING_WARNING, c_Return.value(),
                                            10U, mc_CurrentNode,
                                            "Device does not respond to RequestProgramming request.");

                     c_Return = Errc::success;
                  }
               }
            }
         }
      }

      if (c_Return == Errc::success)
      {
         // Sending the reset commandos
         for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
         {
            C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;
            const bool q_IsActive =
               m_IsNodeActive(u16_Node, this->mu32_ActiveBusIndex, e_ProtocolType, mc_CurrentNode);

            if (q_IsActive == true)
            {
               c_Return = this->mpc_ComDriver->IsRoutingNecessary(u16_Node);

               // Continue with nodes without routing
               if (c_Return == Errc::noact)
               {
                  (void)e_ProtocolType;
                  c_Return = this->mpc_ComDriver->ReConnectNode(mc_CurrentNode);

                  if (c_Return != Errc::success)
                  {
                     // In this case, the node did not connect.
                     // Errors in case of not reachable nodes will be handled when
                     // the state eACTIVATE_FLASHLOADER_OSY_BC_PING_START was set the first time to have
                     // the same process sequence like using CAN.
                     // Therefore do not abort and send only a warning as progress information.
                     // This can only happen in case of Ethernet. The node is not connected, so the next
                     // request did not make any sense to send and will be skipped till the
                     // "EnterPreProgrammingSession" broadcasts. By skipping the other request,
                     // further sending errors will be avoided which would occur a "real" sending error.
                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_RECONNECT_WARNING, c_Return.value(),
                                            15U, mc_CurrentNode,
                                            "Device does not respond to RequestProgramming request.");

                     c_Return = Errc::success;
                  }
                  else
                  {
                     c_Return = this->mpc_ComDriver->SendOsyEcuReset(
                        mc_CurrentNode,
                        C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER);

                     this->mpc_ComDriver->DisconnectNode(mc_CurrentNode);

                     if (c_Return != Errc::success)
                     {
                        // In this case, the node will not get the request, maybe the next step can help to bring
                        // it into the flashloader by sending EnterPreProgrammingSession as broadcast
                        // Therefore do not abort and send only a warning as progress information.
                        (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_ECU_RESET_WARNING, c_Return.value(),
                                               10U, mc_CurrentNode,
                                               "EcuReset request for device failed.");

                        c_Return = Errc::success;
                     }
                  }
               }
            }
         }
      }

      //send "EnterPreProgrammingSession" as broadcast (for a few seconds in short intervals)
      if (c_Return == Errc::success)
      {
         //but only if we are communicating via CAN bus; keeping the target in the flashloader via the time window
         // is not possible via Ethernet; it really has to react to RequestProgramming
         if (this->mpc_SystemDefinition->c_Buses[this->mu32_ActiveBusIndex].e_Type == C_OscSystemBus::eCAN)
         {
            const uint32_t u32_StartTime = stw::tgl::TglGetTickCount();
            uint32_t u32_WaitTime = this->GetMinimumFlashloaderResetWaitTime(C_OscComDriverFlash::eNO_CHANGES_CAN);

            if (u32_WaitTime < u32_SCAN_TIME_MS)
            {
               // The scan time is necessary for the manual triggering of the nodes
               u32_WaitTime = u32_SCAN_TIME_MS;
            }

            (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_BC_ENTER_FLASHLOADER_START, C_NO_ERR, 20U,
                                   "Broadcasting enter Flashloader request ...");
            do
            {
               if (this->mq_OpenSydeDevicesActive == true)
               {
                  // openSYDE "DiagnosticSessionControl(PreProgramming)" broadcast
                  c_Return = this->mpc_ComDriver->SendOsyCanBroadcastEnterPreProgrammingSession();
                  if (c_Return != Errc::success)
                  {
                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_BC_ENTER_PRE_PROGRAMMING_ERROR, c_Return.value(),
                                            20U,
                                            "EnterPreProgramming broadcast failed.");
                     c_Return = Errc::com;
                  }
               }

               if (c_Return != Errc::success)
               {
                  break;
               }

               stw::tgl::TglSleep(5);
            }
            while (stw::tgl::TglGetTickCount() < (u32_WaitTime + u32_StartTime));
         }
         else
         {
            //Ethernet. Give the targets the minimum reset time to reset and initialize their Ethernet interfaces ...
            TglSleep(this->GetMinimumFlashloaderResetWaitTime(C_OscComDriverFlash::eNO_CHANGES_ETHERNET));
         }
      }

      //Previous broadcasts might have caused responses placed in the receive queues of the device
      // specific driver instances. Dump them.
      this->mpc_ComDriver->ClearDispatcherQueue();

      //use simple read service for all nodes that are expected to be present (as "ping")
      if (c_Return == Errc::success)
      {
         (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_BC_PING_START, C_NO_ERR, 30U,
                                "Checking node states ...");

         for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
         {
            C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;

            const bool q_Return =
               m_IsNodeActive(u16_Node, this->mu32_ActiveBusIndex, e_ProtocolType, mc_CurrentNode);

            if (q_Return == true)
            {
               (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_BC_PING_START, C_NO_ERR, 30U, mc_CurrentNode,
                                      "Checking node state ...");

               c_Return = this->mpc_ComDriver->IsRoutingNecessary(u16_Node);

               // Continue with nodes without routing
               if (c_Return == Errc::noact)
               {
                  (void)e_ProtocolType;
                  //if connected via Ethernet we need to reconnect as the reset will break the active TCP
                  // connection
                  c_Return = this->mpc_ComDriver->ReConnectNode(mc_CurrentNode);

                  if (c_Return != Errc::success)
                  {
                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_RECONNECT_ERROR, c_Return.value(), 30U,
                                            mc_CurrentNode,
                                            "Could not reconnect to node");

                     // Node is not reachable
                     this->mc_TimeoutNodes[u16_Node] = static_cast<uint8_t>(c_Return == Errc::busy);

                     c_Return = Errc::com;
                  }
                  if (c_Return == Errc::success)
                  {
                     //If we are on Ethernet the node is in flashloader but not in programming mode yet
                     // we need to send one EnterPreProgramming request to get it there
                     //If we're on CAN one more of this services does not hurt as well :-)
                     //We want to confirm the device is in flashloader anyway.
                     //So we use this service (it will fail if the node is in the application
                     // as there is no "PreProgramming" session there.
                     c_Return =
                        this->mpc_ComDriver->SendOsySetPreProgrammingMode(mc_CurrentNode, true,
                                                                          &u8_NrCode);
                     if (c_Return != Errc::success)
                     {
                        (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_SET_SESSION_ERROR, c_Return.value(), 30U,
                                               mc_CurrentNode, "Request to set active session failed. Details:" +
                                               C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(
                                                  c_Return, u8_NrCode));

                        // Node is not reachable
                        this->mc_TimeoutNodes[u16_Node] = static_cast<uint8_t>(c_Return == Errc::timeout);

                        if (c_Return != Errc::checksum)
                        {
                           c_Return = Errc::com;
                        }
                        else
                        {
                           this->mc_ConnectStatesNodes[u16_Node].q_SecurityAccessError = true;
                        }
                     }
                     else
                     {
                        //looks good ...
                        //node is active and flashable directly on the bus that the client is connected to
                     }
                  }

                  this->mpc_ComDriver->DisconnectNode(mc_CurrentNode);
                  if (c_Return != Errc::success)
                  {
                     q_AtLeastOneError = true;
                     if (oq_FailOnFirstError == true)
                     {
                        break;
                     }
                     c_Return = Errc::success;
                  }
               }
            }

            if (this->mc_TimeoutNodes[u16_Node] == 1U)
            {
               this->mc_ConnectStatesNodes[u16_Node].q_Timeout = true;
            }
         }
      }

      // Activate all nodes with routing
      if (c_Return == Errc::success)
      {
         for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
         {
            C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;
            uint32_t u32_BusIndex;

            if (this->mpc_ComDriver->GetBusIndexOfRoutingNode(u16_Node, u32_BusIndex) == Errc::success)
            {
               const bool q_IsActive = m_IsNodeActive(u16_Node, u32_BusIndex, e_ProtocolType, mc_CurrentNode);

               if (q_IsActive == true)
               {
                  const bool q_IsNodeReachable = this->m_IsNodeReachable(u16_Node);

                  (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_BC_PING_START, C_NO_ERR, 30U, mc_CurrentNode,
                                         "Checking node state ...");

                  if (q_IsNodeReachable == true)
                  {
                     uint32_t u32_ErrorIndex = 0U;

                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_ROUTING_START, C_NO_ERR, 40U, mc_CurrentNode,
                                            "Starting routing for node ...");

                     c_Return = this->mpc_ComDriver->StartRouting(u16_Node, &u32_ErrorIndex);

                     if (c_Return == Errc::success)
                     {
                        (void)e_ProtocolType;
                        // Set the request programming flag to bring the server into flashloader
                        c_Return = this->mpc_ComDriver->SendOsyRequestProgramming(mc_CurrentNode);

                        // Reset the server
                        if (c_Return == Errc::success)
                        {
                           c_Return = this->mpc_ComDriver->SendOsyEcuReset(
                              mc_CurrentNode,
                              C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER);

                           (void)this->m_DisconnectFromTargetServer();

                           if (c_Return != Errc::success)
                           {
                              (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_ECU_RESET_ERROR, c_Return.value(),
                                                     50U, mc_CurrentNode,
                                                     "EcuReset request for routing device failed.");
                              c_Return = Errc::com;
                           }
                        }
                        else
                        {
                           (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_REQUEST_PROGRAMMING_ERROR, c_Return.value(),
                                                  50U, mc_CurrentNode,
                                                  "RequestProgramming request for routing device failed.");

                           // Node is not reachable
                           this->mc_TimeoutNodes[u16_Node] = static_cast<uint8_t>(c_Return == Errc::timeout);

                           c_Return = Errc::com;
                        }

                        if (c_Return == Errc::success)
                        {
                           uint32_t u32_StartTime;
                           uint32_t u32_CurrentTime;
                           uint32_t u32_LastSentTesterPresent;
                           uint32_t u32_WaitTime;
                           C_OscSystemBus::E_Type e_TargetInterfaceType = C_OscSystemBus::eCAN;
                           C_OscComDriverFlash::E_MinimumFlashloaderResetWaitTimeType e_WaitType;
                           stw::opensyde_core::C_OscProtocolDriverOsyNode c_LastRouter;

                           // Check what interface is used of target (not the local bus)
                           tgl_assert(this->mpc_ComDriver->GetRoutingTargetInterfaceType(
                                         u16_Node, e_TargetInterfaceType) == Errc::success);
                           if (e_TargetInterfaceType == C_OscSystemBus::eCAN)
                           {
                              e_WaitType = C_OscComDriverFlash::eNO_CHANGES_CAN;
                           }
                           else
                           {
                              e_WaitType = C_OscComDriverFlash::eNO_CHANGES_ETHERNET;
                           }

                           tgl_assert(this->GetMinimumFlashloaderResetWaitTime(e_WaitType,
                                                                              this->mc_CurrentNode,
                                                                              u32_WaitTime) == Errc::success);

                           tgl_assert(this->mpc_ComDriver->GetServerIdOfLastRouter(u16_Node,
                                                                                  c_LastRouter) ==
                                      Errc::success);

                           u32_StartTime = stw::tgl::TglGetTickCount();
                           u32_LastSentTesterPresent = u32_StartTime;

                           // Give the targets some time to reset and initialize their interfaces
                           // and wait the minimum time
                           do
                           {
                              u32_CurrentTime = stw::tgl::TglGetTickCount();

                              // Tester present is only necessary in case of CAN. In case of Ethernet the entire
                              // routing will be restarted anyway by m_ReconnectToTargetServer
                              if ((e_TargetInterfaceType == C_OscSystemBus::eCAN) &&
                                  (u32_CurrentTime > (u32_LastSentTesterPresent + u32_INTERVAL_TESTER_PRESENT)))
                              {
                                 // We need TesterPresent to keep the routing connection alive
                                 // Send it to the last routing point
                                 c_Return = this->mpc_ComDriver->SendTesterPresent(c_LastRouter);

                                 if (c_Return != Errc::success)
                                 {
                                    (void)m_ReportProgress(eACTIVATE_FLASHLOADER_OSY_RECONNECT_ERROR, c_Return.value(),
                                                           50U,
                                                           this->mc_CurrentNode,
                                                           "Sending Tester Present to router node failed.");
                                    c_Return = Errc::com;
                                    break;
                                 }

                                 u32_LastSentTesterPresent = u32_CurrentTime;
                              }

                              TglSleep(5);
                           }
                           while (u32_CurrentTime < (u32_WaitTime + u32_StartTime));

                           if (c_Return == Errc::success)
                           {
                              c_Return = this->m_ReconnectToTargetServer(true, u16_Node);
                           }
                        }

                        if (c_Return == Errc::success)
                        {
                           //If we are on Ethernet the node is in flashloader but not in programming mode yet
                           // we need to send one EnterPreProgramming request to get it there
                           //If we're on CAN one more of this services does not hurt as well :-)
                           //We want to confirm the device is in flashloader anyway.
                           //So we use this service (it will fail if the node is in the application
                           // as there is no "PreProgramming" session there.
                           c_Return =
                              this->mpc_ComDriver->SendOsySetPreProgrammingMode(mc_CurrentNode, true,
                                                                                &u8_NrCode);

                           if (c_Return != Errc::success)
                           {
                              (void)m_ReportProgress(
                                 eACTIVATE_FLASHLOADER_OSY_SET_SESSION_ERROR, c_Return.value(), 50U,
                                 mc_CurrentNode,
                                 "Request to set active session for routing device failed. Details:" +
                                 C_OscProtocolDriverOsy::h_GetOpenSydeServiceErrorDetails(c_Return, u8_NrCode));

                              // Node is not reachable
                              this->mc_TimeoutNodes[u16_Node] = static_cast<uint8_t>(c_Return == Errc::timeout);

                              if (c_Return != Errc::checksum)
                              {
                                 c_Return = Errc::com;
                              }
                              else
                              {
                                 this->mc_ConnectStatesNodes[u16_Node].q_SecurityAccessError = true;
                              }
                           }

                           (void)this->m_DisconnectFromTargetServer(false);
                        }
                     }
                     else
                     {
                        if (c_Return == Errc::noact)
                        {
                           (void)m_ReportProgress(eACTIVATE_FLASHLOADER_ROUTING_AVAILABLE_FEATURE_ERROR,
                                                  c_Return.value(),
                                                  50U, mc_CurrentNode,
                                                  "Starting routing for node failed due to not capable node");
                        }
                        else
                        {
                           (void)m_ReportProgress(eACTIVATE_FLASHLOADER_ROUTING_ERROR, c_Return.value(), 50U,
                                                  mc_CurrentNode,
                                                  "Starting routing for node failed");
                        }
                        osc_write_log_error(
                           "Activate Flashloader",
                           "Activate Flashloader: Start of routing for node (" + std::to_string(
                              u16_Node) + ") failed with error code: " +
                           C_OscLoggingHandler::h_StwError(c_Return.value()));

                        // Routing node and target node is not reachable
                        this->mc_TimeoutNodes[u16_Node] = static_cast<uint8_t>(c_Return == Errc::timeout);
                        this->mc_TimeoutNodes[u32_ErrorIndex] = static_cast<uint8_t>(c_Return == Errc::timeout);

                        this->mc_ConnectStatesNodes[u16_Node].q_CouldNotReachedDueToRoutingError = true;

                        if ((c_Return != Errc::config) &&
                            (c_Return != Errc::range))
                        {
                           this->mc_ConnectStatesNodes[u32_ErrorIndex].q_RoutingNodeError = true;
                        }

                        if (c_Return == Errc::checksum)
                        {
                           this->mc_ConnectStatesNodes[u32_ErrorIndex].q_SecurityAccessError = true;
                        }
                     }

                     // Stop routing always to clean up
                     this->mpc_ComDriver->StopRouting(u16_Node);
                  }
                  else
                  {
                     // At least one node is not reachable to get to the current node
                     c_Return = Errc::timeout;

                     (void)m_ReportProgress(eACTIVATE_FLASHLOADER_ROUTING_ERROR, c_Return.value(), 50U,
                                            mc_CurrentNode, "Starting routing for node failed");
                     osc_write_log_error(
                        "Activate Flashloader",
                        "Activate Flashloader: Start of routing for node (" + std::to_string(
                           u16_Node) + ") failed due to not available node on route with error code: " +
                        C_OscLoggingHandler::h_StwError(c_Return.value()));
                  }

                  if (c_Return != Errc::success)
                  {
                     q_AtLeastOneError = true;
                     if (oq_FailOnFirstError == true)
                     {
                        break;
                     }
                  }
               }
            }

            if (this->mc_TimeoutNodes[u16_Node] == 1U)
            {
               this->mc_ConnectStatesNodes[u16_Node].q_Timeout = true;
            }
         }
      }
   }
   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eACTIVATE_FLASHLOADER_FINISHED, C_NO_ERR, 100U,
                             "Flashloader activated on all devices.");
   }

   if (q_AtLeastOneError == true)
   {
      c_Return = Errc::warn;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read information of server nodes

   Prerequisite:
   All nodes marked as active in mc_ActiveNodes are present and are in flashloader mode
   i.e. the system is in the state after a successful call to ActivateFlashloader()

   For the API it would be better to first read all information from STW Flashloader nodes and return it and then from
   openSYDE nodes in another function or vice versa.
   But for routing it makes sense to set it up sequentially and handle the system bus by bus.

   So this function reports found information via the virtual function
   m_ReportOpenSydeFlashloaderInformationRead.

   The sequence for routing is the same as in "ActivateFlashloader".

   Services for openSYDE:
   * service readDataByIdentifier(DeviceName)
   * service readFlashBlockData
   * one fixed application info block for the flashloader
   * ReadDataById(BootSwIdentification)
   * ReadDataById(ApplicationSwFingerprint)
   * ReadDataById(ECUSerialNumber)
   * ReadDataById(SystemSupplierECUHWNumber)
   * ReadDataById(SystemSupplierECUHWVersion)
   * ReadDataById(ProtocolVersion)
   * ReadDataById(FlashloaderProtocolVersion)
   * ReadDataById(FlashCount)

   \param[in]   oq_FailOnFirstError   true: abort all further communication if communication with one device fails
                                      false: try to continue with other devices in this case

   \return
   Errc::config     mpc_SystemDefinition is NULL (Init() not called)
   Errc::busy       aborted by user
   Errc::warn       reading failed for at least one node
   Errc::success    information read
   Errc::com        communication driver reported error (details will be written to log file)
   Errc::checksum   Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::ReadDeviceInformation(const bool oq_FailOnFirstError)
{
   std::error_code c_Return = Errc::success;
   bool q_AtLeastOneError = false;
   bool q_AtLeastOneAuthenticationError = false;

   if (this->mpc_SystemDefinition == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      if (this->mc_ConnectStatesNodes.size() == 0)
      {
         // In case of not used ActivateFlashloader function
         this->mc_ConnectStatesNodes.resize(this->mc_ActiveNodes.size());
      }

      for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
      {
         //set progress based on node index:
         const uint8_t u8_Progress =
            static_cast<uint8_t>(((static_cast<size_t>(u16_Node) + 1U) * 100U) /
                                 (this->mpc_SystemDefinition->c_Nodes.size() + 1U));
         const bool q_Abort = m_ReportProgress(eREAD_DEVICE_INFO_START, C_NO_ERR, u8_Progress,
                                               "Reading device information from device ...");
         if (q_Abort == true)
         {
            c_Return = Errc::busy;
         }
         else
         {
            const bool q_IsNodeReachable = this->m_IsNodeReachable(u16_Node);

            if (q_IsNodeReachable == true)
            {
               uint32_t u32_BusIndex;
               bool q_RoutingActivated = false;

               c_Return = this->mpc_ComDriver->GetBusIndexOfRoutingNode(u16_Node, u32_BusIndex);

               if (c_Return == Errc::noact)
               {
                  // No routing necessary
                  u32_BusIndex = this->mu32_ActiveBusIndex;
                  c_Return = Errc::success;
               }
               else if (c_Return == Errc::success)
               {
                  // Routing necessary
                  c_Return = this->mpc_ComDriver->StartRouting(u16_Node);
                  q_RoutingActivated = true;
               }
               else
               {
                  // Error case
               }

               if (c_Return == Errc::success)
               {
                  C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;
                  const bool q_Return = m_IsNodeActive(u16_Node, u32_BusIndex, e_ProtocolType, mc_CurrentNode);
                  if (q_Return == true)
                  {
                     (void)e_ProtocolType;
                     (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_START, C_NO_ERR, u8_Progress, mc_CurrentNode,
                                            "Reading openSYDE device information ...");
                     c_Return = this->m_ReadDeviceInformationOpenSyde(u8_Progress, u16_Node,
                                                                        this->mc_ConnectStatesNodes[u16_Node]);

                     if (c_Return == Errc::success)
                     {
                        (void)m_ReportProgress(eREAD_DEVICE_INFO_OSY_FINISHED, C_NO_ERR, u8_Progress,
                                               mc_CurrentNode,
                                               "openSYDE device information read.");
                     }
                  } //end node active
               }

               if (q_RoutingActivated == true)
               {
                  // Stop routing always to clean up
                  this->mpc_ComDriver->StopRouting(u16_Node);
               }
            }
            else
            {
               // At least one node is not reachable to get to the current node
               c_Return = Errc::timeout;

               osc_write_log_warning("Read device information",
                                     "Read device information: Node (" + std::to_string(u16_Node) +
                                     ") is not reachable due to previous timeout.");
            }
         }
         if (c_Return != Errc::success)
         {
            if (c_Return == Errc::checksum)
            {
               q_AtLeastOneAuthenticationError = true;
            }

            q_AtLeastOneError = true;
            if (oq_FailOnFirstError == true)
            {
               break;
            }
            c_Return = Errc::success;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eREAD_DEVICE_INFO_FINISHED, C_NO_ERR, 100U,
                             "Device information read from all devices.");
   }

   if (q_AtLeastOneError == true)
   {
      if (q_AtLeastOneAuthenticationError == true)
      {
         c_Return = Errc::checksum;
      }
      else
      {
         c_Return = Errc::warn;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read information of server nodes

   Prerequisite:
   All nodes marked as active in mc_ActiveNodes are present and are in flashloader mode
   i.e. the system is in the state after a successful call to ActivateFlashloader()

   Flash all configured files to defined devices.

   Sequence:
   * for all nodes on local bus:
   ** flash all configured applications for all nodes on bus
   ** write all defined NVM parameter set files to NVM
   * for all buses that can be reached via routing
   ** set up routing
   ** for all nodes on bus:
   *** flash all configured applications
   *** write all defined NVM parameter set files to NVM

   Progress report:
   * Reports 0..100 for the overall process
   * Reports 0..100 for each file of each individual node being flashed

   \param[in]  orc_ApplicationsToWrite   list of files to flash per node; must have the same size as the system
                                          definition contains nodes
   \param[in]  orc_NodesOrder            Vector with node update order (index is update position, value is node index)

   \return
   Errc::success     flashed all files
   Errc::config      mpc_SystemDefinition is NULL (Init() not called)
                     parameter writing: one of the files contains data for zero or more than one device
                     (expected: data for exactly one device)
   Errc::overflow    size of orc_ApplicationsToWrite is not the same as the size of nodes in mpc_SystemDefinition
                     for address based targets: device name of device does not match name contained in hex file
   Errc::noact       orc_ApplicationsToWrite has non-empty list of flash or NVM files for node that was not
                     set as active
                     orc_ApplicationsToWrite has non-empty list of flash or NVM files for node that has no
                     position in the
                     update order
                     orc_ApplicationsToWrite has non-empty list of flash or NVM files for node that has more
                     than one position
                     in the update order
                     for address based targets: could not extract device name from hex file
   Errc::rd_wr       file referenced by orc_ApplicationsToWrite does not exist
                     for address based targets: one of the flash files is not a valid Intel or Motorola hex file
                     for address based targets: could not split up flash file into individual memory areas
                     parameter writing: one of the files is not a valid .psi_syde file or does not exist
   Errc::config      for address based targets: no signature block found in flash hex file
                     for STW Flashloader targets: NVM files are defined (not supported by STW Flashloader)
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::busy        procedure aborted by user (as returned by m_ReportProgress)
   Errc::default_    parameter writing: one of the files is present but checksum is invalid
   Errc::warn        The file is present but key details of PEM file could not be extracted
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
   Errc::range       At least one feature of the openSYDE Flashloader is not available for NVM writing
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::UpdateSystem(const std::vector<C_OscSuSequences::C_DoFlash> & orc_ApplicationsToWrite,
                                               const std::vector<uint32_t> & orc_NodesOrder)
{
   std::error_code c_Return = Errc::success;

   this->mu32_CurrentNode = 0U;
   this->mu32_CurrentFile = 0U;

   if (this->mpc_SystemDefinition == nullptr)
   {
      c_Return = Errc::config;
   }
   else if (orc_ApplicationsToWrite.size() != this->mpc_SystemDefinition->c_Nodes.size())
   {
      c_Return = Errc::overflow;
   }
   else
   {
      uint32_t u32_NodesToFlash = 0U;

      // Reset states
      this->mc_UpdateStatesNodes.clear();
      this->mc_UpdateStatesNodes.resize(this->mc_ActiveNodes.size());

      //are all nodes supposed to get flashed active ?
      for (uint16_t u16_Node = 0U; u16_Node < this->mpc_SystemDefinition->c_Nodes.size(); u16_Node++)
      {
         C_OscSuSequencesNodeUpdateStates & rc_State = this->mc_UpdateStatesNodes[u16_Node];
         // Prepare the node states for its files
         if (this->mpc_SystemDefinition->c_Nodes[u16_Node].c_Properties.e_FlashLoader ==
             C_OscNodeProperties::eFL_OPEN_SYDE)
         {
            const C_OscDeviceDefinition * const pc_DeviceDefinition =
               this->mpc_SystemDefinition->c_Nodes[u16_Node].pc_DeviceDefinition;
            const uint32_t u32_SubDeviceIndex =
               this->mpc_SystemDefinition->c_Nodes[u16_Node].u32_SubDeviceIndex;

            tgl_assert(pc_DeviceDefinition != nullptr);
            if (pc_DeviceDefinition != nullptr)
            {
               if (pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].q_FlashloaderOpenSydeIsFileBased == false)
               {
                  rc_State.c_StateHexFiles.resize(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size());
               }
               else
               {
                  rc_State.c_StateOtherFiles.resize(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size());
               }
            }

            // PSI files
            rc_State.c_StatePsiFiles.resize(orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm.size());
         }

         if (orc_ApplicationsToWrite[u16_Node].IsAnyActionRequired() == true)
         {
            if (this->mc_ActiveNodes[u16_Node] == false)
            {
               osc_write_log_error("System Update",
                                   "Update packages contains file(s) or setting(s) for node " + std::to_string(
                                      u16_Node) + " which is not marked as active !");
               c_Return = Errc::noact;
            }
            else
            {
               uint32_t u32_PositionCounter;

               ++u32_NodesToFlash;

               // Does the node have a position ?
               c_Return = Errc::noact;
               for (u32_PositionCounter = 0U; u32_PositionCounter < orc_NodesOrder.size(); ++u32_PositionCounter)
               {
                  if (orc_NodesOrder[u32_PositionCounter] == static_cast<uint32_t>(u16_Node))
                  {
                     c_Return = Errc::success;
                     break;
                  }
               }

               if (c_Return == Errc::success)
               {
                  //do all files exist ?
                  //files for flash:
                  for (uint32_t u32_File = 0U; u32_File < orc_ApplicationsToWrite[u16_Node].c_FilesToFlash.size();
                       u32_File++)
                  {
                     // Getting the correct state variable
                     C_OscSuSequencesNodeFileStates * pc_FileState = nullptr;
                     if (rc_State.c_StateHexFiles.size() > 0)
                     {
                        pc_FileState = &rc_State.c_StateHexFiles[u32_File];
                     }
                     else if (rc_State.c_StateOtherFiles.size() > 0)
                     {
                        pc_FileState = &rc_State.c_StateOtherFiles[u32_File];
                     }
                     else
                     {
                        tgl_assert(false);
                     }

                     if (pc_FileState != nullptr)
                     {
                        // Save the file name
                        pc_FileState->c_FileName =
                           TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u32_File]);
                     }

                     if (TglFileExists(orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u32_File]) == false)
                     {
                        osc_write_log_error("System Update", "Could not find file \"" +
                                            orc_ApplicationsToWrite[u16_Node].c_FilesToFlash[u32_File] + "\" !");
                        if (pc_FileState != nullptr)
                        {
                           pc_FileState->e_FileExists = eSUSEQ_STATE_ERROR;
                        }
                        c_Return = Errc::rd_wr;
                        break;
                     }
                     else
                     {
                        if (pc_FileState != nullptr)
                        {
                           pc_FileState->e_FileExists = eSUSEQ_STATE_NO_ERR;
                        }
                     }
                  }
                  //files for Nvm:
                  for (uint32_t u32_File = 0U;
                       u32_File < orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm.size();
                       u32_File++)
                  {
                     // Save the file name
                     rc_State.c_StatePsiFiles[u32_File].c_FileName =
                        TglExtractFileName(orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u32_File]);

                     if (TglFileExists(orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u32_File]) == false)
                     {
                        osc_write_log_error("System Update", "Could not find file \"" +
                                            orc_ApplicationsToWrite[u16_Node].c_FilesToWriteToNvm[u32_File] +
                                            "\" !");
                        rc_State.c_StatePsiFiles[u32_File].e_FileExists = eSUSEQ_STATE_ERROR;
                        c_Return = Errc::rd_wr;
                        break;
                     }
                     else
                     {
                        rc_State.c_StatePsiFiles[u32_File].e_FileExists = eSUSEQ_STATE_NO_ERR;
                     }
                  }
                  // PEM file
                  if (orc_ApplicationsToWrite[u16_Node].c_PemFile != "")
                  {
                     rc_State.c_StateSecuritySettings.c_FileName = TglExtractFileName(
                        orc_ApplicationsToWrite[u16_Node].c_PemFile);
                     if (TglFileExists(orc_ApplicationsToWrite[u16_Node].c_PemFile) == false)
                     {
                        osc_write_log_error("System Update", "Could not find file \"" +
                                            orc_ApplicationsToWrite[u16_Node].c_PemFile + "\" !");
                        rc_State.c_StateSecuritySettings.e_FileExists = eSUSEQ_STATE_ERROR;
                        c_Return = Errc::rd_wr;
                     }
                     else
                     {
                        rc_State.c_StateSecuritySettings.e_FileExists = eSUSEQ_STATE_NO_ERR;
                     }
                  }
                  else
                  {
                     // No PEM file exists; writing not needed; we still do try to write activation flags if set in
                     //  update package
                     rc_State.c_StateSecuritySettings.c_FileName = "";
                     rc_State.c_StateSecuritySettings.e_FileExists = eSUSEQ_STATE_NOT_NEEDED;
                     rc_State.c_StateSecuritySettings.e_FileLoaded = eSUSEQ_STATE_NOT_NEEDED;
                     rc_State.c_StateSecuritySettings.e_PemFileExtracted = eSUSEQ_STATE_NOT_NEEDED;
                  }
               }
            }
            if (c_Return != Errc::success)
            {
               break;
            }
         }
      }

      if (c_Return == Errc::success)
      {
         // Check the order configuration
         // One node shall have maximum one position
         // The number of positions must match with number of nodes to flash
         if (orc_NodesOrder.size() == u32_NodesToFlash)
         {
            uint32_t u32_OrderPos;
            uint32_t u32_OrderPosCompare;

            for (u32_OrderPos = 0U; u32_OrderPos < orc_NodesOrder.size(); ++u32_OrderPos)
            {
               for (u32_OrderPosCompare = 0U; u32_OrderPosCompare < orc_NodesOrder.size(); ++u32_OrderPosCompare)
               {
                  if ((u32_OrderPos != u32_OrderPosCompare) &&
                      (orc_NodesOrder[u32_OrderPos] == orc_NodesOrder[u32_OrderPosCompare]))
                  {
                     // Node has more than one position
                     c_Return = Errc::noact;
                     break;
                  }
               }

               if (c_Return != Errc::success)
               {
                  break;
               }
            }
         }
         else
         {
            c_Return = Errc::noact;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_START, C_NO_ERR, 0U, "Starting System Update ...");
      for (uint32_t u32_Position = 0U; u32_Position < orc_NodesOrder.size(); u32_Position++)
      {
         const uint32_t u32_NodeIndex = orc_NodesOrder[u32_Position];
         C_OscSuSequencesNodeUpdateStates & rc_NodeUpdateStates = this->mc_UpdateStatesNodes[u32_NodeIndex];

         // Save node index
         this->mu32_CurrentNode = u32_NodeIndex;

         //Flash openSYDE nodes.
         //Do we have anything for that node at all?
         if (orc_ApplicationsToWrite[u32_NodeIndex].IsAnyActionRequired() == true)
         {
            uint32_t u32_BusIndex;
            bool q_RoutingActivated = false;

            c_Return = this->mpc_ComDriver->GetBusIndexOfRoutingNode(u32_NodeIndex, u32_BusIndex);

            if (c_Return == Errc::noact)
            {
               // No routing necessary
               u32_BusIndex = this->mu32_ActiveBusIndex;
               c_Return = Errc::success;
            }
            else if (c_Return == Errc::success)
            {
               uint32_t u32_ErrorIndex = 0U;
               // Routing necessary
               c_Return = this->mpc_ComDriver->StartRouting(u32_NodeIndex, &u32_ErrorIndex);

               if (c_Return != Errc::success)
               {
                  // Handle routing specific errors
                  this->mc_ConnectStatesNodes[u32_NodeIndex].q_CouldNotReachedDueToRoutingError = true;
                  if ((c_Return != Errc::config) &&
                      (c_Return != Errc::range))
                  {
                     this->mc_ConnectStatesNodes[u32_ErrorIndex].q_RoutingNodeError = true;
                  }

                  if (c_Return == Errc::checksum)
                  {
                     this->mc_ConnectStatesNodes[u32_ErrorIndex].q_SecurityAccessError = true;
                  }
               }

               q_RoutingActivated = true;
            }
            else
            {
               // Error case
            }

            if (c_Return == Errc::success)
            {
               C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;

               const bool q_Return = m_IsNodeActive(u32_NodeIndex, u32_BusIndex, e_ProtocolType, mc_CurrentNode);
               if (q_Return == true)
               {
                  (void)e_ProtocolType;
                  {
                     const C_OscDeviceDefinition * const pc_DeviceDefinition =
                        this->mpc_SystemDefinition->c_Nodes[u32_NodeIndex].pc_DeviceDefinition;
                     const uint32_t u32_SubDeviceIndex =
                        this->mpc_SystemDefinition->c_Nodes[u32_NodeIndex].u32_SubDeviceIndex;
                     (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_START, C_NO_ERR, 10U, mc_CurrentNode,
                                            "Starting device update ...");
                     tgl_assert(pc_DeviceDefinition != nullptr);
                     if (pc_DeviceDefinition != nullptr)
                     {
                        C_OscProtocolDriverOsy::C_ListOfFeatures c_AvailableFeatures;
                        bool q_SetProgrammingMode = true;

                        //if connected via Ethernet we might need to reconnect (in case we ran into the session timeout)
                        c_Return = this->m_ReconnectToTargetServer();
                        if (c_Return != Errc::success)
                        {
                           (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_RECONNECT_ERROR, c_Return.value(), 10U,
                                                  mc_CurrentNode,
                                                  "Could not reconnect to node");
                           if (c_Return != Errc::checksum)
                           {
                              c_Return = Errc::com;
                           }
                        }

                        if (c_Return == Errc::success)
                        {
                           //check which protocol features are available
                           c_Return = this->mpc_ComDriver->SendOsyReadListOfFeatures(this->mc_CurrentNode,
                                                                                       c_AvailableFeatures);
                           if (c_Return != Errc::success)
                           {
                              (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_READ_FEATURE_ERROR, c_Return.value(), 10U,
                                                     mc_CurrentNode,
                                                     "Could not read available openSYDE Flashloader features.");
                              c_Return = Errc::com;
                           }
                        }

                        //files to flash ?
                        if ((c_Return == Errc::success) &&
                            (orc_ApplicationsToWrite[u32_NodeIndex].c_FilesToFlash.size() > 0))
                        {
                           tgl_assert(u32_SubDeviceIndex < pc_DeviceDefinition->c_SubDevices.size());
                           //address based or file based ?
                           if (pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].q_FlashloaderOpenSydeIsFileBased
                               ==
                               false)
                           {
                              c_Return = m_FlashNodeOpenSydeHex(
                                 orc_ApplicationsToWrite[u32_NodeIndex].c_FilesToFlash,
                                 orc_ApplicationsToWrite[u32_NodeIndex].c_OtherAcceptedDeviceNames,
                                 pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].
                                 u32_FlashloaderOpenSydeRequestDownloadTimeout,
                                 pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].
                                 u32_FlashloaderOpenSydeTransferDataTimeout,
                                 q_SetProgrammingMode,
                                 rc_NodeUpdateStates.c_StateHexFiles);
                           }
                           else
                           {
                              c_Return = m_FlashNodeOpenSydeFile(
                                 orc_ApplicationsToWrite[u32_NodeIndex].c_FilesToFlash,
                                 pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].
                                 u32_FlashloaderOpenSydeRequestDownloadTimeout,
                                 pc_DeviceDefinition->c_SubDevices[u32_SubDeviceIndex].
                                 u32_FlashloaderOpenSydeTransferDataTimeout,
                                 c_AvailableFeatures,
                                 q_SetProgrammingMode,
                                 rc_NodeUpdateStates.c_StateOtherFiles);
                           }
                        }

                        //files to write to Nvm ?
                        if ((c_Return == Errc::success) &&
                            (orc_ApplicationsToWrite[u32_NodeIndex].c_FilesToWriteToNvm.size() > 0))
                        {
                           c_Return =
                              m_WriteNvmOpenSyde(orc_ApplicationsToWrite[u32_NodeIndex].c_FilesToWriteToNvm,
                                                 c_AvailableFeatures,
                                                 q_SetProgrammingMode,
                                                 rc_NodeUpdateStates.c_StatePsiFiles);
                        }

                        q_SetProgrammingMode = true;

                        // PEM file to write?
                        if (c_Return == Errc::success)
                        {
                           // Special case: Another security level is necessary for the next steps.
                           // The next step must set the programming mode with the other security level again
                           c_Return = m_WritePemOpenSydeFile(orc_ApplicationsToWrite[u32_NodeIndex].c_PemFile,
                                                               c_AvailableFeatures,
                                                               q_SetProgrammingMode,
                                                               rc_NodeUpdateStates.c_StateSecuritySettings);
                        }

                        // States to write?
                        if (c_Return == Errc::success)
                        {
                           c_Return = m_WriteOpenSydeNodeStates(orc_ApplicationsToWrite[u32_NodeIndex],
                                                                  c_AvailableFeatures,
                                                                  q_SetProgrammingMode,
                                                                  rc_NodeUpdateStates.c_StateSecuritySettings);
                        }

                        (void)this->m_DisconnectFromTargetServer();
                     } //lint !e438 //false positive; valued of q_SetProgrammingMode passed through following calls
                     if (c_Return == Errc::success)
                     {
                        (void)m_ReportProgress(eUPDATE_SYSTEM_OSY_NODE_FINISHED, C_NO_ERR, 100U, mc_CurrentNode,
                                               "Finishing device update ...");
                     }
                  }
               }
            }

            if (q_RoutingActivated == true)
            {
               // Stop routing always to clean up
               this->mpc_ComDriver->StopRouting(u32_NodeIndex);
            }
         }
         if (c_Return != Errc::success)
         {
            switch (c_Return.value())
            {
            case C_TIMEOUT:
               rc_NodeUpdateStates.q_Timeout = true;
               break;
            case C_CHECKSUM:
               rc_NodeUpdateStates.q_SecurityAccessError = true;
               break;
            default:
               // Nothing to do
               break;
            }

            break;
         }
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eUPDATE_SYSTEM_FINISHED, C_NO_ERR, 100U, "Finished System Update.");
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reset all devices in the system

   Reset all nodes backwards from the tiniest twigs up to the mighty trunk
   For STW Flashloader we can send a NET Reset

   Sequence:
   * for all openSYDE nodes on local bus:
   ** for openSYDE nodes: send ECU-Reset requests (do not use broadcast as we cannot route it)
   ** for STW Flashloader nodes: send NET-Reset (broadcast)

   Progress report:
   * Reports 0..100 for the overall process

   \return
   Errc::success     reset requests were sent out to all nodes
   Errc::config      mpc_SystemDefinition is NULL (Init() not called)
   Errc::com         communication driver reported problem (details will be written to log file)
   Errc::checksum    Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::ResetSystem(void)
{
   std::error_code c_Return = Errc::success;

   if (this->mpc_SystemDefinition == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      const uint32_t u32_LongestRoute = this->mpc_ComDriver->GetRoutingPointMaximum();
      int32_t s32_RouteSizeCounter;

      (void)m_ReportProgress(eRESET_SYSTEM_START, C_NO_ERR, 0U, "Resetting System ...");

      // Reset the system beginning with the longest route.
      // Avoiding conflicts with resetting a node, which is necessary for an other route
      for (s32_RouteSizeCounter = static_cast<int32_t>(u32_LongestRoute);
           s32_RouteSizeCounter >= 0; --s32_RouteSizeCounter)
      {
         for (uint32_t u32_Node = 0U; u32_Node < this->mpc_SystemDefinition->c_Nodes.size(); u32_Node++)
         {
            const bool q_IsNodeReachable = this->m_IsNodeReachable(u32_Node);

            if (q_IsNodeReachable == true)
            {
               bool q_Return;
               const uint32_t u32_RoutePointCount = this->mpc_ComDriver->GetRoutingPointCount(u32_Node, q_Return);

               if ((static_cast<int32_t>(u32_RoutePointCount) == s32_RouteSizeCounter) &&
                   (q_Return == true))
               {
                  uint32_t u32_BusIndex;
                  bool q_RoutingActivated = false;

                  c_Return = this->mpc_ComDriver->GetBusIndexOfRoutingNode(u32_Node, u32_BusIndex);

                  if (c_Return == Errc::noact)
                  {
                     // No routing necessary
                     u32_BusIndex = this->mu32_ActiveBusIndex;
                     c_Return = Errc::success;
                  }
                  else if (c_Return == Errc::success)
                  {
                     // Routing necessary
                     c_Return = this->mpc_ComDriver->StartRouting(u32_Node);
                     q_RoutingActivated = true;
                  }
                  else
                  {
                     // Error case
                  }

                  if (c_Return == Errc::success)
                  {
                     //reset nodes
                     C_OscNodeProperties::E_FlashLoaderProtocol e_ProtocolType;
                     q_Return = m_IsNodeActive(u32_Node, u32_BusIndex, e_ProtocolType, mc_CurrentNode);
                     if (q_Return == true)
                     {
                        (void)e_ProtocolType;
                        c_Return = this->m_ReconnectToTargetServer();

                        if (c_Return == Errc::success)
                        {
                           c_Return = this->mpc_ComDriver->SendOsyEcuReset(
                              mc_CurrentNode,
                              C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON);

                           (void)this->m_DisconnectFromTargetServer();
                        }
                     }
                     if (c_Return != Errc::success)
                     {
                        // 0 equals the local bus
                        if (s32_RouteSizeCounter == 0)
                        {
                           (void)m_ReportProgress(eRESET_SYSTEM_OSY_NODE_ERROR, c_Return.value(), 0U, mc_CurrentNode,
                                                  "Could not reset routed node.");
                        }
                        else
                        {
                           (void)m_ReportProgress(eRESET_SYSTEM_OSY_ROUTED_NODE_ERROR, c_Return.value(), 0U,
                                                  mc_CurrentNode,
                                                  "Could not reset routed node.");
                        }
                        c_Return = Errc::com;
                        break;
                     }
                  }

                  if (q_RoutingActivated == true)
                  {
                     // Reset command does not send a response. Need a little wait time to guarantee that
                     // the routing nodes really send the reset command
                     TglSleep(20);

                     // Stop routing always because of cleaning up the tp and legacy routing dispatcher
                     this->mpc_ComDriver->StopRouting(u32_Node);
                  }
               }
            }
         }
      }
   }

   if (c_Return == Errc::success)
   {
      (void)m_ReportProgress(eRESET_SYSTEM_FINISHED, C_NO_ERR, 100U, "System was reset.");
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the node states of the connect process

   The connect process involves the both steps:
   * ActivateFlashloader
   * ReadDeviceInformation

   \param[out]      orc_ConnectStatesNodes   Detailed output parameter description

   \retval   Errc::success States returned
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::GetConnectStates(
   std::vector<C_OscSuSequencesNodeConnectStates> & orc_ConnectStatesNodes) const
{
   orc_ConnectStatesNodes = this->mc_ConnectStatesNodes;
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the node states of the connect process

   The connect process involves the both steps:
   * ActivateFlashloader
   * ReadDeviceInformation

   \param[out]      orc_UpdateStatesNodes   Detailed output parameter description

   \retval   Errc::success States returned
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::GetUpdateStates(
   std::vector<C_OscSuSequencesNodeUpdateStates> & orc_UpdateStatesNodes) const
{
   orc_UpdateStatesNodes = this->mc_UpdateStatesNodes;
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Fill flash data structure with security settings

   \param[in]   oe_StateAuthentication   Setting for secure authentication
   \param[in]   oe_StateEncryption       Setting for traffic enctyption
   \param[in]   oe_StateDebugger         Setting for debugger activation
   \param[out]  orc_DoFlash              Flash data structure to fill information into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSuSequences::h_FillDoFlashWithSecurityOptions(
   const C_OscViewNodeUpdate::E_StateSecureAuthentication oe_StateAuthentication,
   const C_OscViewNodeUpdate::E_StateTrafficEncryption oe_StateEncryption,
   const C_OscViewNodeUpdate::E_StateDebugger oe_StateDebugger, C_OscSuSequences::C_DoFlash & orc_DoFlash)
{
   switch (oe_StateAuthentication)
   {
   case C_OscViewNodeUpdate::eST_SEC_NO_CHANGE:
      orc_DoFlash.q_SendSecureAuthenticationEnabledState = false;
      break;
   case C_OscViewNodeUpdate::eST_SEC_ACTIVATE:
      orc_DoFlash.q_SendSecureAuthenticationEnabledState = true;
      orc_DoFlash.q_SecureAuthenticationEnabled = true;
      break;
   case C_OscViewNodeUpdate::eST_SEC_DEACTIVATE:
      orc_DoFlash.q_SendSecureAuthenticationEnabledState = true;
      orc_DoFlash.q_SecureAuthenticationEnabled = false;
      break;
   default:
      break;
   }

   switch (oe_StateEncryption)
   {
   case C_OscViewNodeUpdate::eST_TEN_NO_CHANGE:
      orc_DoFlash.q_SendTrafficEncryptionEnabledState = false;
      break;
   case C_OscViewNodeUpdate::eST_TEN_ACTIVATE:
      orc_DoFlash.q_SendTrafficEncryptionEnabledState = true;
      orc_DoFlash.q_TrafficEncryptionEnabled = true;
      break;
   case C_OscViewNodeUpdate::eST_TEN_DEACTIVATE:
      orc_DoFlash.q_SendTrafficEncryptionEnabledState = true;
      orc_DoFlash.q_TrafficEncryptionEnabled = false;
      break;
   default:
      break;
   }

   switch (oe_StateDebugger)
   {
   case C_OscViewNodeUpdate::eST_DEB_NO_CHANGE:
      orc_DoFlash.q_SendDebuggerEnabledState = false;
      break;
   case C_OscViewNodeUpdate::eST_DEB_ACTIVATE:
      orc_DoFlash.q_SendDebuggerEnabledState = true;
      orc_DoFlash.q_DebuggerEnabled = true;
      break;
   case C_OscViewNodeUpdate::eST_DEB_DEACTIVATE:
      orc_DoFlash.q_SendDebuggerEnabledState = true;
      orc_DoFlash.q_DebuggerEnabled = false;
      break;
   default:
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert information read from openSYDE server node to string list

   Can be used by the application for a no-frills approach to get a textual representation of the information
   reported by m_ReportOpenSydeFlashloaderInformationRead.

   \param[in]     orc_Info         openSYDE server node information
   \param[out]    orc_Text         textual representation of read information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSuSequences::h_OpenSydeFlashloaderInformationToText(const C_OsyDeviceInformation & orc_Info,
                                                              std::vector<std::string> & orc_Text)
{
   const std::vector<std::string> c_MoreInformation = orc_Info.c_MoreInformation.FlashloaderInformationToText();

   orc_Text.clear();
   orc_Text.push_back("Device name: " + orc_Info.c_DeviceName);
   orc_Text.push_back("Number of applications: " + std::to_string(orc_Info.c_Applications.size()));
   for (uint8_t u8_Application = 0U; u8_Application < orc_Info.c_Applications.size(); u8_Application++)
   {
      orc_Text.push_back("");
      orc_Text.push_back("Application " + std::to_string(u8_Application));
      orc_Text.push_back(" Name: " + orc_Info.c_Applications[u8_Application].c_ApplicationName);
      orc_Text.push_back(" Version: " + orc_Info.c_Applications[u8_Application].c_ApplicationVersion);
      orc_Text.push_back(" Build date: " + orc_Info.c_Applications[u8_Application].c_BuildDate);
      orc_Text.push_back(" Build time: " + orc_Info.c_Applications[u8_Application].c_BuildTime);
      orc_Text.push_back(" Block start address: 0x" +
                   IntToHexCompat(static_cast<int64_t>(orc_Info.c_Applications[u8_Application].
                                                              u32_BlockStartAddress), 8U));
      orc_Text.push_back(" Block end address: 0x" +
                   IntToHexCompat(static_cast<int64_t>(orc_Info.c_Applications[u8_Application].
                                                              u32_BlockEndAddress), 8U));
      orc_Text.push_back(static_cast<std::string>(" Signature valid: ") +
                   ((orc_Info.c_Applications[u8_Application].u8_SignatureValid == 0) ? "yes" : "no"));
      orc_Text.push_back(" Additional information: " + orc_Info.c_Applications[u8_Application].c_AdditionalInformation);
   }
   orc_Text.insert(orc_Text.end(), c_MoreInformation.begin(), c_MoreInformation.end());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compare two instances

   \param[in]   orc_Source   instance to compare *this against

   \return
   true:   orc_Source == (*this)
   false:  orc_Source != (*this)
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSuSequences::C_ApplicationProperties::operator ==(const C_ApplicationProperties & orc_Source) const
{
   return((TrimCompat(this->c_Name) == TrimCompat(orc_Source.c_Name)) &&
          (TrimCompat(this->c_Version) == TrimCompat(orc_Source.c_Version)) &&
          (TrimCompat(this->c_BuildDate) == TrimCompat(orc_Source.c_BuildDate)) &&
          (TrimCompat(this->c_BuildTime) == TrimCompat(orc_Source.c_BuildTime)));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reconnects the current server

   Checking if the target node is really connected to Ethernet.
   It is only necessary in case of ETH -> ETH routing and
   not in case of ETH -> CAN routing! In case of ETH -> CAN routing the target and
   restarted device is not connected to Ethernet and the Ethernet routing device must not be
   reconnected to hold the routing configuration active.

   \param[in]     oq_RestartRouting    In case of ETH -> ETH routing the routing must be restarted if necessary
   \param[in]     ou32_NodeIndex       Node index (only used if oq_RestartRouting is true)

   \return
   Errc::success    re-connected or no reconnect necessary
   Errc::busy       could not re-connect to node
   Errc::range      node not found or no openSYDE protocol installed
   Errc::com        communication driver reported error (details will be written to log file)
   Errc::checksum   Security related error (something went wrong while handshaking with the server)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_ReconnectToTargetServer(const bool oq_RestartRouting, const uint32_t ou32_NodeIndex)
{
   uint32_t u32_BusCounter;
   std::error_code c_Return = Errc::success;

   for (u32_BusCounter = 0U; u32_BusCounter < this->mpc_SystemDefinition->c_Buses.size(); ++u32_BusCounter)
   {
      if (this->mpc_SystemDefinition->c_Buses[u32_BusCounter].u8_BusId == this->mc_CurrentNode.u8_BusIdentifier)
      {
         if (this->mpc_SystemDefinition->c_Buses[u32_BusCounter].e_Type == C_OscSystemBus::eETHERNET)
         {
            if (oq_RestartRouting == true)
            {
               if (this->mc_CurrentNode.u8_BusIdentifier != this->mpc_ComDriver->GetClientId().u8_BusIdentifier)
               {
                  // Restart routing when the routing is still necessary. Only a problem when using
                  // Ethernet to Ethernet routing a target is connected to Ethernet and not CAN.
                  // The routing connection on the last router server will be dropped by reseting the target server.
                  c_Return = this->mpc_ComDriver->StartRouting(ou32_NodeIndex);
               }
            }

            if (c_Return == Errc::success)
            {
               c_Return = this->mpc_ComDriver->ReConnectNode(mc_CurrentNode);
            }
         }
         break;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Disconnects the current server

   \param[in]  oq_DisconnectOnIp2IpRouting    Flag if a disconnect in case of Ethernet to Ethernet routing shall be
                                              executed. If the routing is still necessary after this call, the
                                              disconnect shall not be executed.

   \return
   Errc::success    disconnected or no reconnect necessary
   Errc::noact      could not disconnect to node
   Errc::range      node not found or no openSYDE protocol installed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSuSequences::m_DisconnectFromTargetServer(const bool oq_DisconnectOnIp2IpRouting)
{
   uint32_t u32_BusCounter;
   std::error_code c_Return = Errc::success;

   // Disconnect the current server
   for (u32_BusCounter = 0U; u32_BusCounter < this->mpc_SystemDefinition->c_Buses.size(); ++u32_BusCounter)
   {
      if (this->mpc_SystemDefinition->c_Buses[u32_BusCounter].u8_BusId == this->mc_CurrentNode.u8_BusIdentifier)
      {
         if (this->mpc_SystemDefinition->c_Buses[u32_BusCounter].e_Type == C_OscSystemBus::eETHERNET)
         {
            if (this->mc_CurrentNode.u8_BusIdentifier == this->mpc_ComDriver->GetClientId().u8_BusIdentifier)
            {
               c_Return = this->mpc_ComDriver->DisconnectNode(mc_CurrentNode);
            }
            else if (oq_DisconnectOnIp2IpRouting == true)
            {
               uint32_t u32_NodeIndex;

               this->mpc_ComDriver->GetNodeIndex(this->mc_CurrentNode, u32_NodeIndex);
               this->mpc_ComDriver->StopRouting(u32_NodeIndex);
            }
            else
            {
               // Nothing to do
            }
         }
         break;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns an adapted device transfer data timeout time for compensating a potential higher bus load

   See C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout for detailed calculation description.

   In a routing scenario when considering the transfer time we need to consider the slowest possible bus.
   As we support routing from Ethernet to CAN, but not the other way around this means that the last bus
    (i.e. the one connected to the target device) is the slowest (for our purpose we can assume CAN is always slower
     than Ethernet).
    However there is one scenario not covered by this: PC <-> CAN@LowSpeed <-> CAN@HighSpeed <-> Device.
    So there is room for further improvement.

   \param[in]       ou32_DeviceTransferDataTimeout     Device specific timeout time as base
   \param[in]       ou32_MaxBlockLength                Maximum number of bytes of each block
   \param[in]       ou8_BusIdentifier                  Bus identifier of the bus the server is connected to

   \return
   Calculated device transfer data timeout
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscSuSequences::m_GetAdaptedTransferDataTimeout(const uint32_t ou32_DeviceTransferDataTimeout,
                                                           const uint32_t ou32_MaxBlockLength,
                                                           const uint8_t ou8_BusIdentifier) const
{
   uint32_t u32_AdaptedTime = ou32_DeviceTransferDataTimeout;
   uint32_t u32_BusCounter;
   bool q_IsCan = false;
   uint32_t u32_Bitrate = 0U;

   // Search the matching bus for the bus identifier
   for (u32_BusCounter = 0U; u32_BusCounter < this->mpc_SystemDefinition->c_Buses.size(); ++u32_BusCounter)
   {
      const C_OscSystemBus & rc_Bus = this->mpc_SystemDefinition->c_Buses[u32_BusCounter];

      if (ou8_BusIdentifier == rc_Bus.u8_BusId)
      {
         if (rc_Bus.e_Type == C_OscSystemBus::eCAN)
         {
            q_IsCan = true;
            u32_Bitrate = static_cast<uint32_t>(rc_Bus.u64_BitRate / 1000ULL);
         }
         break;
      }
   }

   // In case of Ethernet no offset is necessary
   if (q_IsCan == true)
   {
      u32_AdaptedTime = C_OscUpdateUtil::h_GetAdaptedTransferDataTimeout(ou32_DeviceTransferDataTimeout,
                                                                         ou32_MaxBlockLength, u32_Bitrate);
   }

   return u32_AdaptedTime;
}
