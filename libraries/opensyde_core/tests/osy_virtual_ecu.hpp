//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       A virtual openSYDE flashloader for tests: answers the UDS services the sequences use, records the rest

   Shared by the transport-level suites (Ethernet, CAN) so the same device sits behind either bus.
   One request payload in, at most one response payload out; the transport double does the framing.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef OSY_VIRTUAL_ECU_HPP
#define OSY_VIRTUAL_ECU_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "C_OscEndian.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_SclChecksums.hpp"
#include "C_OscSuSequences.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace osy_virtual_ecu
{
/* -- Types --------------------------------------------------------------------------------------------------------- */
using T_Bytes = std::vector<uint8_t>;

constexpr uint8_t mhau8_ECU_IP[4] = {192U, 168U, 7U, 5U};
constexpr uint8_t mhu8_ECU_NODE_ID = 5U;
constexpr uint8_t mhu8_BUS_ID = 0U;
constexpr uint32_t mhu32_APPLICATION_BASE = 0x00100000U;
constexpr uint32_t mhu32_MAX_BLOCK_LENGTH = 256U; //what the ECU allows per TransferData, header included

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A flashloader with a memory: answers the UDS services the update sequences use, records the rest

   One request in, at most one response out. Everything it is asked and everything it is given
   is kept so the tests can check it afterwards.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_VirtualEcu
{
public:
   struct T_FlashBlock
   {
      uint32_t u32_Start;
      uint32_t u32_End;
      std::string c_Name;
      std::string c_Version;
   };

   //identity
   std::string c_DeviceName;
   uint8_t au8_SerialNumber[6];
   uint32_t u32_HardwareNumber;
   std::string c_HardwareVersion;
   uint32_t u32_FlashCount;
   std::vector<T_FlashBlock> c_ExistingBlocks;
   bool q_Silent;             ///< true: never answers, like a device that is not there
   bool q_RefuseFlashMemory;  ///< true: CheckFlashMemoryAvailable is refused with "conditions not correct"
   bool q_RejectFileCrc;      ///< true: the file-based transfer exit fails with "general programming failure"
   uint8_t u8_FeatureByte7;   ///< ListOfFeatures byte 7: 0x01 NVM write, 0x02 max block length, 0x08 exit result,
                              ///< 0x20 secure authentication, 0x40 debugger off, 0x80 debugger on
   uint8_t u8_FeatureByte6;   ///< ListOfFeatures byte 6: 0x01 traffic encryption

   //what happened
   std::vector<T_Bytes> c_Requests;
   std::vector<uint8_t> c_Sessions;
   std::vector<uint8_t> c_SecurityLevelsUnlocked;
   std::vector<uint8_t> c_ResetTypes;
   std::map<uint32_t, T_Bytes> c_Flash;         ///< start address -> bytes written through download/transfer/exit
   std::vector<uint32_t> c_SignatureAddresses;  ///< what the final transfer exit of each file carried
   std::string c_FingerprintUser;
   uint8_t au8_FingerprintDate[3];
   std::vector<std::pair<uint32_t, uint32_t> > c_MemoryChecks; ///< (address, size) of every availability check
   std::map<std::string, T_Bytes> c_Files;                      ///< file name -> bytes received through file transfer
   std::vector<std::pair<std::string, uint32_t> > c_FileRequests; ///< (name, announced size) of every RequestFileTransfer
   std::vector<uint32_t> c_FileCrcsReceived;                    ///< the CRC the client sent with each file's exit
   std::vector<uint32_t> c_FileCrcsComputed;                    ///< what the device computed over what it got
   std::map<uint32_t, uint8_t> c_Nvm;                           ///< address -> byte, written by WriteMemoryByAddress
   std::vector<std::pair<uint32_t, uint32_t> > c_NvmWrites;     ///< (address, size) of every WriteMemoryByAddress
   std::vector<std::pair<bool, uint8_t> > c_AuthenticationActivations; ///< (on, algorithm) written to 0xA821
   std::vector<std::pair<bool, uint8_t> > c_EncryptionActivations;     ///< (on, algorithm) written to 0xA825
   std::vector<bool> c_DebuggerActivations;                            ///< written to 0xA822
   std::vector<T_Bytes> c_AuthenticationKeysWritten;                   ///< modulus + exponent + serial, as sent to 0xA823

   C_VirtualEcu(void) :
      c_DeviceName("VIRTUAL-ECU"),
      au8_SerialNumber{0x12U, 0x34U, 0x56U, 0x78U, 0x9AU, 0xBCU},
      u32_HardwareNumber(4711U),
      c_HardwareVersion("V1.03"),
      u32_FlashCount(12U),
      q_Silent(false),
      q_RefuseFlashMemory(false),
      q_RejectFileCrc(false),
      u8_FeatureByte7(0x02U),
      u8_FeatureByte6(0x00U),
      au8_FingerprintDate{0U, 0U, 0U},
      mu32_DownloadAddress(0U),
      mq_DownloadOpen(false),
      mq_FileOpen(false)
   {
      c_ExistingBlocks.push_back({0x00000000U, 0x0000FFFFU, "Flashloader", "V2.11r0"});
      c_ExistingBlocks.push_back({mhu32_APPLICATION_BASE, 0x0013FFFFU, "OldApp", "V0.9"});
   }

   ///Response payload for one request payload; nothing for services that do not answer
   std::optional<T_Bytes> Handle(const T_Bytes & orc_Request)
   {
      c_Requests.push_back(orc_Request);
      if (q_Silent || orc_Request.empty())
      {
         return std::nullopt;
      }
      const uint8_t u8_Sid = orc_Request[0];
      switch (u8_Sid)
      {
      case 0x10U: //DiagnosticSessionControl
         c_Sessions.push_back(orc_Request[1]);
         return T_Bytes{0x50U, orc_Request[1], 0x00U, 0x32U, 0x01U, 0xF4U};
      case 0x11U: //EcuReset: no answer, the device is gone
         c_ResetTypes.push_back(orc_Request[1]);
         mq_DownloadOpen = false;
         mq_FileOpen = false;
         return std::nullopt;
      case 0x3EU: //TesterPresent
         return ((orc_Request[1] & 0x80U) != 0U) ? std::nullopt : std::optional<T_Bytes>(T_Bytes{0x7EU, 0x00U});
      case 0x22U: //ReadDataByIdentifier
         return m_ReadDataByIdentifier(stw::opensyde_core::C_OscEndian::h_GetU16Big(&orc_Request[1]));
      case 0x2EU: //WriteDataByIdentifier
         return m_WriteDataByIdentifier(stw::opensyde_core::C_OscEndian::h_GetU16Big(&orc_Request[1]), orc_Request);
      case 0x27U: //SecurityAccess: odd = seed request, even = key
         if ((orc_Request[1] % 2U) == 1U)
         {
            return T_Bytes{0x67U, orc_Request[1], 0x0AU, 0x0BU, 0x0CU, 0x0DU};
         }
         c_SecurityLevelsUnlocked.push_back(static_cast<uint8_t>(orc_Request[1] - 1U));
         return T_Bytes{0x67U, orc_Request[1]};
      case 0xBBU: //ReadDataPoolDataById: echo the packed identifier, then a value of the identifier's low byte
         return T_Bytes{0xFBU, orc_Request[1], orc_Request[2], orc_Request[3], orc_Request[3], 0x00U};
      case 0xBCU: //WriteDataPoolDataById: echo the packed identifier
         return T_Bytes{0xFCU, orc_Request[1], orc_Request[2], orc_Request[3]};
      case 0x31U: //RoutineControl
         return m_RoutineControl(orc_Request);
      case 0x34U: //RequestDownload
         mu32_DownloadAddress = stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[3]);
         mc_DownloadData.clear();
         mq_DownloadOpen = true;
         return T_Bytes{0x74U, 0x20U, static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH >> 8U),
                        static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH)};
      case 0x36U: //TransferData, into whichever transfer is open
         if ((mq_DownloadOpen == false) && (mq_FileOpen == false))
         {
            return mh_Negative(u8_Sid, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_SEQUENCE_ERROR);
         }
         mc_DownloadData.insert(mc_DownloadData.end(), orc_Request.begin() + 2, orc_Request.end());
         return T_Bytes{0x76U, orc_Request[1]};
      case 0x37U: //RequestTransferExit: 1 or 5 bytes for an address based transfer, 9 for a file
         if (mq_FileOpen)
         {
            return m_FileTransferExit(orc_Request);
         }
         if (mq_DownloadOpen == false)
         {
            return mh_Negative(u8_Sid, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_SEQUENCE_ERROR);
         }
         c_Flash[mu32_DownloadAddress] = mc_DownloadData;
         mq_DownloadOpen = false;
         if (orc_Request.size() == 5U)
         {
            c_SignatureAddresses.push_back(stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[1]));
         }
         return T_Bytes{0x77U};
      case 0x38U: //RequestFileTransfer: mode, path length, path, compression, size format, size
         return m_RequestFileTransfer(orc_Request);
      case 0x23U: //ReadMemoryByAddress: format nibbles say how many bytes address and size take
         return m_ReadMemory(orc_Request);
      case 0x3DU: //WriteMemoryByAddress: same header, then the data; echoes the header
         return m_WriteMemory(orc_Request);
      default:
         return mh_Negative(u8_Sid, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_SERVICE_NOT_SUPPORTED);
      }
   }

   ///Requests of one service id, in order
   std::vector<T_Bytes> RequestsFor(const uint8_t ou8_Sid) const
   {
      std::vector<T_Bytes> c_Result;
      for (const T_Bytes & rc_Request : c_Requests)
      {
         if ((rc_Request.empty() == false) && (rc_Request[0] == ou8_Sid))
         {
            c_Result.push_back(rc_Request);
         }
      }
      return c_Result;
   }

private:
   uint32_t mu32_DownloadAddress;
   T_Bytes mc_DownloadData;
   bool mq_DownloadOpen;
   bool mq_FileOpen;
   std::string mc_FileName;

   std::optional<T_Bytes> m_RequestFileTransfer(const T_Bytes & orc_Request)
   {
      if ((orc_Request.size() < 10U) || (orc_Request[1] != 0x03U))
      {
         return mh_Negative(0x38U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_INCORRECT_MESSAGE_LENGTH_OR_FORMAT);
      }
      const uint16_t u16_PathLength = stw::opensyde_core::C_OscEndian::h_GetU16Big(&orc_Request[2]);
      if (orc_Request.size() != (10U + static_cast<size_t>(u16_PathLength)))
      {
         return mh_Negative(0x38U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_INCORRECT_MESSAGE_LENGTH_OR_FORMAT);
      }
      mc_FileName.assign(orc_Request.begin() + 4, orc_Request.begin() + 4 + u16_PathLength);
      const uint32_t u32_Size = stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[6U + u16_PathLength]);
      c_FileRequests.emplace_back(mc_FileName, u32_Size);
      mc_DownloadData.clear();
      mq_FileOpen = true;
      //mode echoed, two bytes of max block length, data format "plain"
      return T_Bytes{0x78U, 0x03U, 0x02U, static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH >> 8U),
                     static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH), 0x00U};
   }

   std::optional<T_Bytes> m_FileTransferExit(const T_Bytes & orc_Request)
   {
      if (orc_Request.size() != 9U)
      {
         return mh_Negative(0x37U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_INCORRECT_MESSAGE_LENGTH_OR_FORMAT);
      }
      mq_FileOpen = false;
      c_Files[mc_FileName] = mc_DownloadData;
      const uint32_t u32_Claimed = stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[1]);
      uint32_t u32_Crc = 0xFFFFFFFFU;
      stw::scl::C_SclChecksums::CalcCRC32(mc_DownloadData.data(), static_cast<uint32_t>(mc_DownloadData.size()),
                                          u32_Crc);
      u32_Crc ^= 0xFFFFFFFFU;
      c_FileCrcsReceived.push_back(u32_Claimed);
      c_FileCrcsComputed.push_back(u32_Crc);
      if (q_RejectFileCrc || (u32_Claimed != u32_Crc))
      {
         mc_TransferExitResult = "CRC mismatch for " + mc_FileName;
         return mh_Negative(0x37U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_GENERAL_PROGRAMMING_FAILURE);
      }
      mc_TransferExitResult = "OK: " + mc_FileName + " stored";
      return T_Bytes{0x77U};
   }

   ///the address/size header both memory services share; returns false when the request is too short
   static bool mh_ParseMemoryHeader(const T_Bytes & orc_Request, uint32_t & oru32_Address, uint32_t & oru32_Size,
                                    size_t & orx_DataStart)
   {
      if (orc_Request.size() < 2U)
      {
         return false;
      }
      const size_t x_AddressBytes = orc_Request[1] & 0x0FU;
      const size_t x_SizeBytes = orc_Request[1] >> 4U;
      if (orc_Request.size() < (2U + x_AddressBytes + x_SizeBytes))
      {
         return false;
      }
      oru32_Address = 0U;
      for (size_t x_Index = 0U; x_Index < x_AddressBytes; ++x_Index)
      {
         oru32_Address = (oru32_Address << 8U) | orc_Request[2U + x_Index];
      }
      oru32_Size = 0U;
      for (size_t x_Index = 0U; x_Index < x_SizeBytes; ++x_Index)
      {
         oru32_Size = (oru32_Size << 8U) | orc_Request[2U + x_AddressBytes + x_Index];
      }
      orx_DataStart = 2U + x_AddressBytes + x_SizeBytes;
      return true;
   }

   std::optional<T_Bytes> m_ReadMemory(const T_Bytes & orc_Request) const
   {
      uint32_t u32_Address = 0U;
      uint32_t u32_Size = 0U;
      size_t x_DataStart = 0U;
      if (mh_ParseMemoryHeader(orc_Request, u32_Address, u32_Size, x_DataStart) == false)
      {
         return mh_Negative(0x23U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_INCORRECT_MESSAGE_LENGTH_OR_FORMAT);
      }
      T_Bytes c_Response{0x63U};
      for (uint32_t u32_Index = 0U; u32_Index < u32_Size; ++u32_Index)
      {
         const auto c_It = c_Nvm.find(u32_Address + u32_Index);
         c_Response.push_back((c_It == c_Nvm.end()) ? 0xFFU : c_It->second); //erased flash reads as 0xFF
      }
      return c_Response;
   }

   std::optional<T_Bytes> m_WriteMemory(const T_Bytes & orc_Request)
   {
      uint32_t u32_Address = 0U;
      uint32_t u32_Size = 0U;
      size_t x_DataStart = 0U;
      if ((mh_ParseMemoryHeader(orc_Request, u32_Address, u32_Size, x_DataStart) == false) ||
          (orc_Request.size() != (x_DataStart + u32_Size)))
      {
         return mh_Negative(0x3DU, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_INCORRECT_MESSAGE_LENGTH_OR_FORMAT);
      }
      for (uint32_t u32_Index = 0U; u32_Index < u32_Size; ++u32_Index)
      {
         c_Nvm[u32_Address + u32_Index] = orc_Request[x_DataStart + u32_Index];
      }
      c_NvmWrites.emplace_back(u32_Address, u32_Size);
      //the positive response repeats everything up to the data
      T_Bytes c_Response(orc_Request.begin(), orc_Request.begin() + static_cast<long>(x_DataStart));
      c_Response[0] = 0x7DU;
      return c_Response;
   }

   std::string mc_TransferExitResult;

   static T_Bytes mh_Negative(const uint8_t ou8_Sid, const uint8_t ou8_Code)
   {
      return T_Bytes{0x7FU, ou8_Sid, ou8_Code};
   }

   static void mh_Append(T_Bytes & orc_To, const std::string & orc_Text)
   {
      for (const char cn_Char : orc_Text)
      {
         orc_To.push_back(static_cast<uint8_t>(cn_Char));
      }
   }

   std::optional<T_Bytes> m_ReadDataByIdentifier(const uint16_t ou16_Did) const
   {
      T_Bytes c_Response{0x62U, static_cast<uint8_t>(ou16_Did >> 8U), static_cast<uint8_t>(ou16_Did)};
      switch (ou16_Did)
      {
      case 0xA81AU: //device name
         mh_Append(c_Response, c_DeviceName);
         break;
      case 0xF18CU: //serial number, POS format
         c_Response.insert(c_Response.end(), &au8_SerialNumber[0], &au8_SerialNumber[6]);
         break;
      case 0xF192U: //hardware number
         c_Response.resize(7U);
         stw::opensyde_core::C_OscEndian::h_SetU32Big(u32_HardwareNumber, &c_Response[3]);
         break;
      case 0xF193U: //hardware version
         mh_Append(c_Response, c_HardwareVersion);
         break;
      case 0xF180U: //boot software identification: count, then M.m.r
         c_Response.insert(c_Response.end(), {1U, 2U, 11U, 0U});
         break;
      case 0xF184U: //fingerprint: date, time, length, user
         c_Response.insert(c_Response.end(), {25U, 1U, 2U, 3U, 4U, 5U, 3U, 'o', 'l', 'd'});
         break;
      case 0xA813U: //protocol version
      case 0xA815U: //flashloader protocol version
         c_Response.insert(c_Response.end(), {1U, 0U, 0U});
         break;
      case 0xA819U: //flash count
         c_Response.resize(7U);
         stw::opensyde_core::C_OscEndian::h_SetU32Big(u32_FlashCount, &c_Response[3]);
         break;
      case 0xA800U: //list of features: two configurable flag bytes, the rest is zero
         c_Response.insert(c_Response.end(), {0U, 0U, 0U, 0U, 0U, 0U, u8_FeatureByte6, u8_FeatureByte7});
         break;
      case 0xA81DU: //what the device has to say about the last file transfer
         mh_Append(c_Response, mc_TransferExitResult);
         break;
      case 0xA801U: //max number of block length
         c_Response.insert(c_Response.end(), {static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH >> 8U),
                                              static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH)});
         break;
      default:
         return mh_Negative(0x22U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
      }
      return c_Response;
   }

   std::optional<T_Bytes> m_WriteDataByIdentifier(const uint16_t ou16_Did, const T_Bytes & orc_Request)
   {
      const T_Bytes c_Ack{0x6EU, static_cast<uint8_t>(ou16_Did >> 8U), static_cast<uint8_t>(ou16_Did)};
      switch (ou16_Did)
      {
      case 0xF184U: //fingerprint: date(3) time(3) length user
         (void)std::memcpy(&au8_FingerprintDate[0], &orc_Request[3], 3U);
         c_FingerprintUser.assign(orc_Request.begin() + 10, orc_Request.end());
         return c_Ack;
      case 0xA821U: //security authentication activation: on/off, algorithm
         c_AuthenticationActivations.emplace_back((orc_Request[3] & 0x01U) != 0U, orc_Request[4]);
         return c_Ack;
      case 0xA825U: //traffic encryption activation: on/off, algorithm
         c_EncryptionActivations.emplace_back((orc_Request[3] & 0x01U) != 0U, orc_Request[4]);
         return c_Ack;
      case 0xA822U: //debugger activation
         c_DebuggerActivations.push_back((orc_Request[3] & 0x01U) != 0U);
         return c_Ack;
      case 0xA823U: //security authentication key: 128 byte modulus, 4 byte exponent, up to 20 byte serial
         c_AuthenticationKeysWritten.emplace_back(orc_Request.begin() + 3, orc_Request.end());
         return c_Ack;
      default:
         return mh_Negative(0x2EU, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
      }
   }

   std::optional<T_Bytes> m_RoutineControl(const T_Bytes & orc_Request)
   {
      const uint16_t u16_Routine = stw::opensyde_core::C_OscEndian::h_GetU16Big(&orc_Request[2]);
      T_Bytes c_Response{0x71U, orc_Request[1], orc_Request[2], orc_Request[3]};
      switch (u16_Routine)
      {
      case 0x0206U: //RequestProgramming
         return c_Response;
      case 0x0208U: //CheckFlashMemoryAvailable
         c_MemoryChecks.emplace_back(stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[4]),
                                     stw::opensyde_core::C_OscEndian::h_GetU32Big(&orc_Request[8]));
         if (q_RefuseFlashMemory)
         {
            return mh_Negative(0x31U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_CONDITIONS_NOT_CORRECT);
         }
         return c_Response;
      case 0x0209U: //ReadFlashBlockData: TLVs 1 (addresses), 2 (signature), 3 (version), 5 (name)
      {
         const uint8_t u8_Block = orc_Request[4];
         if (u8_Block >= c_ExistingBlocks.size())
         {
            return mh_Negative(0x31U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
         }
         const T_FlashBlock & rc_Block = c_ExistingBlocks[u8_Block];
         c_Response.push_back(1U);
         c_Response.resize(c_Response.size() + 8U);
         stw::opensyde_core::C_OscEndian::h_SetU32Big(rc_Block.u32_Start, &c_Response[c_Response.size() - 8U]);
         stw::opensyde_core::C_OscEndian::h_SetU32Big(rc_Block.u32_End, &c_Response[c_Response.size() - 4U]);
         c_Response.push_back(2U);
         c_Response.push_back(0U);
         c_Response.push_back(3U);
         c_Response.push_back(static_cast<uint8_t>(rc_Block.c_Version.size()));
         mh_Append(c_Response, rc_Block.c_Version);
         c_Response.push_back(5U);
         c_Response.push_back(static_cast<uint8_t>(rc_Block.c_Name.size()));
         mh_Append(c_Response, rc_Block.c_Name);
         return c_Response;
      }
      default:
         return mh_Negative(0x31U, stw::opensyde_core::C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
      }
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sequences that keep every progress report and the device information they read
*/
//----------------------------------------------------------------------------------------------------------------------
class C_RecordingSequences :
   public stw::opensyde_core::C_OscSuSequences
{
public:
   std::vector<E_ProgressStep> c_Steps;
   std::vector<std::string> c_Errors; ///< texts reported with a non-zero result
   std::map<uint32_t, C_OsyDeviceInformation> c_DeviceInformation;

   ///How long a service may go unanswered; the default second is a long time for a test
   void SetPollingTimeout(const stw::opensyde_core::C_OscProtocolDriverOsyNode & orc_Node, const uint32_t ou32_Ms)
   {
      (void)mpc_ComDriver->OsySetPollingTimeout(orc_Node, ou32_Ms);
   }

   std::string ErrorsAsText(void) const
   {
      std::string c_Text;
      for (const std::string & rc_Error : c_Errors)
      {
         c_Text += rc_Error + "\n";
      }
      return c_Text;
   }

   bool Saw(const E_ProgressStep oe_Step) const
   {
      for (const E_ProgressStep e_Step : c_Steps)
      {
         if (e_Step == oe_Step)
         {
            return true;
         }
      }
      return false;
   }

protected:
   bool m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result, const uint8_t,
                         const std::string & orc_Information) override
   {
      c_Steps.push_back(oe_Step);
      if (os32_Result != 0)
      {
         c_Errors.push_back(orc_Information);
      }
      return false;
   }

   bool m_ReportProgress(const E_ProgressStep oe_Step, const int32_t os32_Result, const uint8_t,
                         const stw::opensyde_core::C_OscProtocolDriverOsyNode &, const std::string & orc_Information) override
   {
      c_Steps.push_back(oe_Step);
      if (os32_Result != 0)
      {
         c_Errors.push_back(orc_Information);
      }
      return false;
   }

   void m_ReportOpenSydeFlashloaderInformationRead(const C_OsyDeviceInformation & orc_Info,
                                                   const uint32_t ou32_NodeIndex) override
   {
      c_DeviceInformation[ou32_NodeIndex] = orc_Info;
   }
};

}

#endif
