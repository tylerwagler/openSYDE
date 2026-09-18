//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The system update sequences against a virtual ECU on Ethernet: no hardware, every byte on the wire

   C_OscSuSequences drives a whole update: activate the flashloader, read the device out, flash the
   hex files, reset. Below it sit the flash com driver, the UDS driver and the DoIP transport; the
   transport talks to the world through the abstract C_OscIpDispatcher. A dispatcher double that
   owns a small UDS server ("the virtual ECU") therefore runs the entire stack in-process, and the
   ECU records what was asked of it and what got flashed.

   This is the layer the round-trip tests could not reach, and the first thing it caught was the
   flash sequence dereferencing a vector of null unique_ptrs (m_FlashNodeOpenSydeHex, since the
   ownership sweep): flashing any hex file crashed before a single byte was sent.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "C_OscApplicationInfoBlock.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscEndian.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscHexFile.hpp"
#include "C_OscIpDispatcher.hpp"
#include "C_OscNode.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscSecurityPemDatabase.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscSystemDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
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

   C_VirtualEcu(void) :
      c_DeviceName("VIRTUAL-ECU"),
      au8_SerialNumber{0x12U, 0x34U, 0x56U, 0x78U, 0x9AU, 0xBCU},
      u32_HardwareNumber(4711U),
      c_HardwareVersion("V1.03"),
      u32_FlashCount(12U),
      q_Silent(false),
      q_RefuseFlashMemory(false),
      au8_FingerprintDate{0U, 0U, 0U},
      mu32_DownloadAddress(0U),
      mq_DownloadOpen(false)
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
         return std::nullopt;
      case 0x3EU: //TesterPresent
         return ((orc_Request[1] & 0x80U) != 0U) ? std::nullopt : std::optional<T_Bytes>(T_Bytes{0x7EU, 0x00U});
      case 0x22U: //ReadDataByIdentifier
         return m_ReadDataByIdentifier(C_OscEndian::h_GetU16Big(&orc_Request[1]));
      case 0x2EU: //WriteDataByIdentifier
         return m_WriteDataByIdentifier(C_OscEndian::h_GetU16Big(&orc_Request[1]), orc_Request);
      case 0x27U: //SecurityAccess: odd = seed request, even = key
         if ((orc_Request[1] % 2U) == 1U)
         {
            return T_Bytes{0x67U, orc_Request[1], 0x0AU, 0x0BU, 0x0CU, 0x0DU};
         }
         c_SecurityLevelsUnlocked.push_back(static_cast<uint8_t>(orc_Request[1] - 1U));
         return T_Bytes{0x67U, orc_Request[1]};
      case 0x31U: //RoutineControl
         return m_RoutineControl(orc_Request);
      case 0x34U: //RequestDownload
         mu32_DownloadAddress = C_OscEndian::h_GetU32Big(&orc_Request[3]);
         mc_DownloadData.clear();
         mq_DownloadOpen = true;
         return T_Bytes{0x74U, 0x20U, static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH >> 8U),
                        static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH)};
      case 0x36U: //TransferData
         if (mq_DownloadOpen == false)
         {
            return mh_Negative(u8_Sid, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_SEQUENCE_ERROR);
         }
         mc_DownloadData.insert(mc_DownloadData.end(), orc_Request.begin() + 2, orc_Request.end());
         return T_Bytes{0x76U, orc_Request[1]};
      case 0x37U: //RequestTransferExit, with the signature address when it is the last area of a file
         if (mq_DownloadOpen == false)
         {
            return mh_Negative(u8_Sid, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_SEQUENCE_ERROR);
         }
         c_Flash[mu32_DownloadAddress] = mc_DownloadData;
         mq_DownloadOpen = false;
         if (orc_Request.size() == 5U)
         {
            c_SignatureAddresses.push_back(C_OscEndian::h_GetU32Big(&orc_Request[1]));
         }
         return T_Bytes{0x77U};
      default:
         return mh_Negative(u8_Sid, C_OscProtocolDriverOsy::hu8_NR_CODE_SERVICE_NOT_SUPPORTED);
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
         C_OscEndian::h_SetU32Big(u32_HardwareNumber, &c_Response[3]);
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
         C_OscEndian::h_SetU32Big(u32_FlashCount, &c_Response[3]);
         break;
      case 0xA800U: //list of features: byte 7 = MaxNumberOfBlockLength readable (0x02)
         c_Response.insert(c_Response.end(), {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0x02U});
         break;
      case 0xA801U: //max number of block length
         c_Response.insert(c_Response.end(), {static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH >> 8U),
                                              static_cast<uint8_t>(mhu32_MAX_BLOCK_LENGTH)});
         break;
      default:
         return mh_Negative(0x22U, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
      }
      return c_Response;
   }

   std::optional<T_Bytes> m_WriteDataByIdentifier(const uint16_t ou16_Did, const T_Bytes & orc_Request)
   {
      if (ou16_Did == 0xF184U) //fingerprint: date(3) time(3) length user
      {
         (void)std::memcpy(&au8_FingerprintDate[0], &orc_Request[3], 3U);
         c_FingerprintUser.assign(orc_Request.begin() + 10, orc_Request.end());
         return T_Bytes{0x6EU, static_cast<uint8_t>(ou16_Did >> 8U), static_cast<uint8_t>(ou16_Did)};
      }
      return mh_Negative(0x2EU, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
   }

   std::optional<T_Bytes> m_RoutineControl(const T_Bytes & orc_Request)
   {
      const uint16_t u16_Routine = C_OscEndian::h_GetU16Big(&orc_Request[2]);
      T_Bytes c_Response{0x71U, orc_Request[1], orc_Request[2], orc_Request[3]};
      switch (u16_Routine)
      {
      case 0x0206U: //RequestProgramming
         return c_Response;
      case 0x0208U: //CheckFlashMemoryAvailable
         c_MemoryChecks.emplace_back(C_OscEndian::h_GetU32Big(&orc_Request[4]),
                                     C_OscEndian::h_GetU32Big(&orc_Request[8]));
         if (q_RefuseFlashMemory)
         {
            return mh_Negative(0x31U, C_OscProtocolDriverOsy::hu8_NR_CODE_CONDITIONS_NOT_CORRECT);
         }
         return c_Response;
      case 0x0209U: //ReadFlashBlockData: TLVs 1 (addresses), 2 (signature), 3 (version), 5 (name)
      {
         const uint8_t u8_Block = orc_Request[4];
         if (u8_Block >= c_ExistingBlocks.size())
         {
            return mh_Negative(0x31U, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
         }
         const T_FlashBlock & rc_Block = c_ExistingBlocks[u8_Block];
         c_Response.push_back(1U);
         c_Response.resize(c_Response.size() + 8U);
         C_OscEndian::h_SetU32Big(rc_Block.u32_Start, &c_Response[c_Response.size() - 8U]);
         C_OscEndian::h_SetU32Big(rc_Block.u32_End, &c_Response[c_Response.size() - 4U]);
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
         return mh_Negative(0x31U, C_OscProtocolDriverOsy::hu8_NR_CODE_REQUEST_OUT_OF_RANGE);
      }
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An Ethernet with virtual ECUs on it, behind the IP dispatcher interface

   TCP is a byte stream per connection handle. A request is one DoIP frame (8 byte header, then
   source and target logical addresses, then the UDS payload); the ECU behind the connection's IP
   gets the payload and its answer is framed the same way back into the connection's receive
   buffer. UDP broadcasts are accepted and nothing answers them.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_VirtualEthernet :
   public C_OscIpDispatcher
{
public:
   std::map<uint32_t, C_VirtualEcu *> c_EcusByIp; ///< key: IP as big endian u32
   std::vector<T_Bytes> c_Broadcasts;
   uint32_t u32_TcpConnectsAttempted;

   C_VirtualEthernet(void) :
      C_OscIpDispatcher(1U),
      u32_TcpConnectsAttempted(0U)
   {
   }

   void Attach(const uint8_t (&orau8_Ip)[4], C_VirtualEcu & orc_Ecu)
   {
      c_EcusByIp[C_OscEndian::h_GetU32Big(orau8_Ip)] = &orc_Ecu;
   }

   std::error_code InitTcp(const uint8_t (&orau8_Ip)[4], uint32_t & oru32_Handle) override
   {
      ++u32_TcpConnectsAttempted;
      const uint32_t u32_Ip = C_OscEndian::h_GetU32Big(orau8_Ip);
      if (c_EcusByIp.count(u32_Ip) == 0U)
      {
         return Errc::noact;
      }
      oru32_Handle = static_cast<uint32_t>(mc_Connections.size()) + 1U; //0 is the broadcast pseudo handle
      mc_Connections.push_back({u32_Ip, true, T_Bytes()});
      return Errc::success;
   }

   std::error_code IsTcpConnected(const uint32_t ou32_Handle) override
   {
      const T_Connection * const pc_Connection = m_Get(ou32_Handle);
      if (pc_Connection == nullptr)
      {
         return Errc::range;
      }
      return pc_Connection->q_Connected ? Errc::success : Errc::noact;
   }

   std::error_code ReConnectTcp(const uint32_t ou32_Handle) override
   {
      T_Connection * const pc_Connection = m_Get(ou32_Handle);
      if (pc_Connection == nullptr)
      {
         return Errc::range;
      }
      pc_Connection->q_Connected = true;
      pc_Connection->c_Rx.clear();
      return Errc::success;
   }

   std::error_code CloseTcp(const uint32_t ou32_Handle) override
   {
      T_Connection * const pc_Connection = m_Get(ou32_Handle);
      if (pc_Connection == nullptr)
      {
         return Errc::range;
      }
      pc_Connection->q_Connected = false;
      pc_Connection->c_Rx.clear();
      return Errc::success;
   }

   std::error_code InitUdp(void) override
   {
      return Errc::success;
   }

   std::error_code CloseUdp(void) override
   {
      return Errc::success;
   }

   std::error_code SendTcp(const uint32_t ou32_Handle, const std::vector<uint8_t> & orc_Data) override
   {
      T_Connection * const pc_Connection = m_Get(ou32_Handle);
      if (pc_Connection == nullptr)
      {
         return Errc::range;
      }
      if (pc_Connection->q_Connected == false)
      {
         return Errc::rd_wr;
      }
      //DoIP: 02 FD, payload type, payload size, then source and target logical address, then UDS
      if ((orc_Data.size() < 12U) || (orc_Data[0] != 0x02U) || (orc_Data[1] != 0xFDU))
      {
         return Errc::rd_wr;
      }
      const uint16_t u16_Type = C_OscEndian::h_GetU16Big(&orc_Data[2]);
      const uint32_t u32_Size = C_OscEndian::h_GetU32Big(&orc_Data[4]);
      if ((u16_Type != 0x8001U) || (u32_Size != (orc_Data.size() - 8U)))
      {
         return Errc::rd_wr;
      }
      C_VirtualEcu * const pc_Ecu = c_EcusByIp[pc_Connection->u32_Ip];
      const T_Bytes c_Payload(orc_Data.begin() + 12, orc_Data.end());
      const std::optional<T_Bytes> c_Answer = pc_Ecu->Handle(c_Payload);
      if (c_Answer.has_value())
      {
         T_Bytes c_Frame(12U);
         c_Frame[0] = 0x02U;
         c_Frame[1] = 0xFDU;
         C_OscEndian::h_SetU16Big(0x8001U, &c_Frame[2]);
         C_OscEndian::h_SetU32Big(static_cast<uint32_t>(4U + c_Answer->size()), &c_Frame[4]);
         //answered from the target, to the source
         c_Frame[8] = orc_Data[10];
         c_Frame[9] = orc_Data[11];
         c_Frame[10] = orc_Data[8];
         c_Frame[11] = orc_Data[9];
         c_Frame.insert(c_Frame.end(), c_Answer->begin(), c_Answer->end());
         pc_Connection->c_Rx.insert(pc_Connection->c_Rx.end(), c_Frame.begin(), c_Frame.end());
      }
      return Errc::success;
   }

   std::error_code ReadTcp(const uint32_t ou32_Handle, std::vector<uint8_t> & orc_Data) override
   {
      T_Connection * const pc_Connection = m_Get(ou32_Handle);
      if (pc_Connection == nullptr)
      {
         return Errc::range;
      }
      if (pc_Connection->c_Rx.size() < orc_Data.size())
      {
         return Errc::noact;
      }
      std::copy(pc_Connection->c_Rx.begin(), pc_Connection->c_Rx.begin() + static_cast<long>(orc_Data.size()),
                orc_Data.begin());
      pc_Connection->c_Rx.erase(pc_Connection->c_Rx.begin(),
                                pc_Connection->c_Rx.begin() + static_cast<long>(orc_Data.size()));
      return Errc::success;
   }

   std::error_code ReadTcp(const uint32_t ou32_Handle, const uint8_t ou8_ClientBusIdentifier,
                           const uint8_t ou8_ClientNodeIdentifier, const uint8_t ou8_ServerBusIdentifier,
                           const uint8_t ou8_ServerNodeIdentifier, std::vector<uint8_t> & orc_Data) override
   {
      const std::error_code c_Result = ReadTcp(ou32_Handle, orc_Data);
      if (!c_Result)
      {
         const uint16_t u16_Server =
            static_cast<uint16_t>((static_cast<uint16_t>(ou8_ServerBusIdentifier) << 7U) + ou8_ServerNodeIdentifier + 1U);
         const uint16_t u16_Client =
            static_cast<uint16_t>((static_cast<uint16_t>(ou8_ClientBusIdentifier) << 7U) + ou8_ClientNodeIdentifier + 1U);
         if ((C_OscEndian::h_GetU16Big(&orc_Data[0]) != u16_Server) ||
             (C_OscEndian::h_GetU16Big(&orc_Data[2]) != u16_Client))
         {
            return Errc::warn;
         }
      }
      return c_Result;
   }

   std::error_code ReadTcpBuffer(const uint8_t, const uint8_t, const uint8_t, const uint8_t,
                                 std::vector<uint8_t> &) override
   {
      return Errc::noact;
   }

   std::error_code SendUdp(const std::vector<uint8_t> & orc_Data) override
   {
      c_Broadcasts.push_back(orc_Data);
      return Errc::success;
   }

   std::error_code ReadUdp(std::vector<uint8_t> &, uint8_t (&)[4]) override
   {
      return Errc::noact;
   }

private:
   struct T_Connection
   {
      uint32_t u32_Ip;
      bool q_Connected;
      T_Bytes c_Rx;
   };
   std::vector<T_Connection> mc_Connections;

   T_Connection * m_Get(const uint32_t ou32_Handle)
   {
      if ((ou32_Handle == 0U) || (ou32_Handle > mc_Connections.size()))
      {
         return nullptr;
      }
      return &mc_Connections[ou32_Handle - 1U];
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sequences that keep every progress report and the device information they read
*/
//----------------------------------------------------------------------------------------------------------------------
class C_RecordingSequences :
   public C_OscSuSequences
{
public:
   std::vector<E_ProgressStep> c_Steps;
   std::vector<std::string> c_Errors; ///< texts reported with a non-zero result
   std::map<uint32_t, C_OsyDeviceInformation> c_DeviceInformation;

   ///How long a service may go unanswered; the default second is a long time for a test
   void SetPollingTimeout(const uint32_t ou32_Ms)
   {
      (void)mpc_ComDriver->OsySetPollingTimeout(C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_ECU_NODE_ID), ou32_Ms);
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
                         const C_OscProtocolDriverOsyNode &, const std::string & orc_Information) override
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

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The image to flash: an application information block naming the device, filler, the signature magic

   Long enough that TransferData has to split it, and with the application block and the
   signature at known offsets so the test can check what the ECU received.
*/
//----------------------------------------------------------------------------------------------------------------------
T_Bytes mh_MakeImage(const std::string & orc_DeviceId, uint32_t & oru32_SignatureOffset)
{
   T_Bytes c_Image;
   for (uint8_t u8_Index = 0U; u8_Index < APPLICATION_INFO_MAGIC_LENGTH_V2; ++u8_Index)
   {
      c_Image.push_back(static_cast<uint8_t>(APPLICATION_INFO_MAGIC_V2[u8_Index]));
   }
   c_Image.push_back(2U);    //structure version
   c_Image.push_back(0x01U); //content map: device ID only
   for (uint8_t u8_Index = 0U; u8_Index < 17U; ++u8_Index) //V2 device ID is 17 characters, space padded
   {
      c_Image.push_back((u8_Index < orc_DeviceId.size()) ? static_cast<uint8_t>(orc_DeviceId[u8_Index]) : 0x20U);
   }
   while (c_Image.size() < 600U)
   {
      c_Image.push_back(static_cast<uint8_t>((c_Image.size() * 7U) & 0xFFU));
   }
   oru32_SignatureOffset = static_cast<uint32_t>(c_Image.size());
   for (const char cn_Char : std::string(";zwm2KgUZ!"))
   {
      c_Image.push_back(static_cast<uint8_t>(cn_Char));
   }
   while (c_Image.size() < 700U)
   {
      c_Image.push_back(0xA5U);
   }
   return c_Image;
}

std::string mh_WriteHexFile(const std::string & orc_Name, const T_Bytes & orc_Image)
{
   stw::hex_file::C_HexDataDump c_Dump;
   stw::hex_file::C_HexDataDumpBlock c_Block;
   c_Block.u32_AddressOffset = mhu32_APPLICATION_BASE;
   c_Block.au8_Data = orc_Image;
   c_Dump.at_Blocks.push_back(c_Block);

   const std::string c_Path = (std::filesystem::temp_directory_path() / orc_Name).string();
   C_OscHexFile c_File;
   EXPECT_FALSE(static_cast<bool>(c_File.CreateHexFile(c_Dump, 32U)));
   EXPECT_FALSE(static_cast<bool>(c_File.SaveToFile(c_Path.c_str())));
   return c_Path;
}

class SuSequencesVirtualEcu :
   public ::testing::Test
{
protected:
   void SetUp(void) override
   {
      //one Ethernet bus
      C_OscSystemBus c_Bus;
      c_Bus.e_Type = C_OscSystemBus::eETHERNET;
      c_Bus.c_Name = "ETH1";
      c_Bus.u8_BusId = mhu8_BUS_ID;
      mc_SystemDefinition.c_Buses.push_back(c_Bus);

      //the device the node is an instance of: openSYDE flashloader over Ethernet, address based, short waits
      C_OscSubDeviceDefinition c_Sub;
      c_Sub.c_SubDeviceName = "VIRTUAL-ECU";
      c_Sub.q_ProgrammingSupport = true;
      c_Sub.q_DiagnosticProtocolOpenSydeEthernet = true;
      c_Sub.q_FlashloaderOpenSydeEthernet = true;
      c_Sub.q_FlashloaderOpenSydeIsFileBased = false;
      c_Sub.u32_FlashloaderResetWaitTimeNoChangesEthernet = 10U;
      c_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet = 10U;
      c_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesEthernet = 10U;
      c_Sub.u32_FlashloaderOpenSydeRequestDownloadTimeout = 200U;
      c_Sub.u32_FlashloaderOpenSydeTransferDataTimeout = 200U;
      mc_DeviceDefinition.c_DeviceName = "VIRTUAL-ECU";
      mc_DeviceDefinition.u8_NumEthernetBusses = 1U;
      mc_DeviceDefinition.c_SubDevices.push_back(c_Sub);

      //one node with one Ethernet interface on that bus
      C_OscNode c_Node;
      c_Node.c_DeviceType = "VIRTUAL-ECU";
      c_Node.pc_DeviceDefinition = &mc_DeviceDefinition;
      c_Node.u32_SubDeviceIndex = 0U;
      c_Node.c_Properties.c_Name = "Node1";
      c_Node.c_Properties.e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
      c_Node.c_Properties.e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;
      C_OscNodeComInterfaceSettings c_Eth;
      c_Eth.e_InterfaceType = C_OscSystemBus::eETHERNET;
      c_Eth.u8_InterfaceNumber = 0U;
      c_Eth.u8_NodeId = mhu8_ECU_NODE_ID;
      c_Eth.q_IsUpdateEnabled = true;
      c_Eth.q_IsDiagnosisEnabled = true;
      c_Eth.q_IsRoutingEnabled = false;
      (void)std::memcpy(&c_Eth.c_Ip.au8_IpAddress[0], &mhau8_ECU_IP[0], 4U);
      c_Eth.AddConnection(0U);
      c_Eth.SetInterfaceConnectedInDevice(true);
      c_Node.c_Properties.c_ComInterfaces.push_back(c_Eth);
      mc_SystemDefinition.c_Nodes.push_back(c_Node);

      mc_Ethernet.Attach(mhau8_ECU_IP, mc_Ecu);
   }

   std::error_code m_Init(void)
   {
      const std::vector<uint8_t> c_ActiveNodes(1U, 1U);
      const std::error_code c_Result =
         mc_Sequences.Init(mc_SystemDefinition, 0U, c_ActiveNodes, nullptr, &mc_Ethernet, &mc_PemDatabase);
      return c_Result;
   }

   C_OscSystemDefinition mc_SystemDefinition;
   C_OscDeviceDefinition mc_DeviceDefinition;
   C_OscSecurityPemDatabase mc_PemDatabase;
   C_VirtualEcu mc_Ecu;
   C_VirtualEthernet mc_Ethernet;
   C_RecordingSequences mc_Sequences;
};
}

TEST_F(SuSequencesVirtualEcu, Init_ConnectsToTheNodeOverTcp)
{
   EXPECT_EQ(Errc::success, m_Init());
   EXPECT_TRUE(mc_Sequences.IsInitialized());
   EXPECT_EQ(1U, mc_Ethernet.u32_TcpConnectsAttempted);
}

TEST_F(SuSequencesVirtualEcu, Init_FailsWhenTheNodeIsNotOnTheNetwork)
{
   mc_Ethernet.c_EcusByIp.clear();
   EXPECT_NE(Errc::success, m_Init());
   EXPECT_FALSE(mc_Sequences.IsInitialized());
}

TEST_F(SuSequencesVirtualEcu, ActivateFlashloader_RequestsProgrammingResetsToFlashloaderAndReturnsToPreProgramming)
{
   ASSERT_EQ(Errc::success, m_Init());

   EXPECT_EQ(Errc::success, mc_Sequences.ActivateFlashloader());

   //the device saw: pre-programming session, RequestProgramming, reset to flashloader, pre-programming again
   EXPECT_EQ(std::vector<uint8_t>({0x60U, 0x60U}), mc_Ecu.c_Sessions);
   ASSERT_EQ(1U, mc_Ecu.RequestsFor(0x31U).size());
   EXPECT_EQ(T_Bytes({0x31U, 0x01U, 0x02U, 0x06U}), mc_Ecu.RequestsFor(0x31U)[0]);
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER}),
             mc_Ecu.c_ResetTypes);
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eACTIVATE_FLASHLOADER_FINISHED));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty());

   std::vector<C_OscSuSequencesNodeConnectStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetConnectStates(c_States));
   ASSERT_EQ(1U, c_States.size());
   EXPECT_FALSE(c_States[0].q_Timeout);
}

TEST_F(SuSequencesVirtualEcu, ActivateFlashloader_ASilentNodeIsFlaggedAsTimedOutEvenWhenAbortingOnIt)
{
   //With the default "fail on first error" the loop used to break before it copied the timeout into the
   //node's connect state, so the GUI got an error without the node being marked. Fixed 2026-09-18.
   ASSERT_EQ(Errc::success, m_Init());
   mc_Ecu.q_Silent = true;
   mc_Sequences.SetPollingTimeout(30U); //silence costs milliseconds here, not the device's second

   const std::error_code c_Result = mc_Sequences.ActivateFlashloader();

   EXPECT_NE(Errc::success, c_Result);
   EXPECT_FALSE(mc_Sequences.c_Errors.empty());
   EXPECT_FALSE(mc_Sequences.Saw(C_OscSuSequences::eACTIVATE_FLASHLOADER_FINISHED));
   std::vector<C_OscSuSequencesNodeConnectStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetConnectStates(c_States));
   ASSERT_EQ(1U, c_States.size());
   EXPECT_TRUE(c_States[0].q_Timeout);
}

TEST_F(SuSequencesVirtualEcu, ReadDeviceInformation_CollectsNameBlocksAndFlashloaderDetails)
{
   ASSERT_EQ(Errc::success, m_Init());

   EXPECT_EQ(Errc::success, mc_Sequences.ReadDeviceInformation());
   EXPECT_TRUE(mc_Sequences.c_Errors.empty());

   ASSERT_EQ(1U, mc_Sequences.c_DeviceInformation.count(0U));
   const C_OscSuSequences::C_OsyDeviceInformation & rc_Info = mc_Sequences.c_DeviceInformation[0U];
   EXPECT_EQ("VIRTUAL-ECU", rc_Info.c_DeviceName);
   ASSERT_EQ(2U, rc_Info.c_Applications.size());
   EXPECT_EQ("Flashloader", rc_Info.c_Applications[0].c_ApplicationName);
   EXPECT_EQ("V2.11r0", rc_Info.c_Applications[0].c_ApplicationVersion);
   EXPECT_EQ(mhu32_APPLICATION_BASE, rc_Info.c_Applications[1].u32_BlockStartAddress);
   EXPECT_EQ(0x0013FFFFU, rc_Info.c_Applications[1].u32_BlockEndAddress);
   EXPECT_EQ(12U, rc_Info.c_MoreInformation.u32_FlashCount);
   EXPECT_EQ(4711U, rc_Info.c_MoreInformation.u32_EcuArticleNumber);
   EXPECT_EQ("V1.03", rc_Info.c_MoreInformation.c_EcuHardwareVersionNumber);
   EXPECT_EQ(2U, rc_Info.c_MoreInformation.au8_FlashloaderSoftwareVersion[0]);
   EXPECT_EQ(11U, rc_Info.c_MoreInformation.au8_FlashloaderSoftwareVersion[1]);
   EXPECT_EQ("old", rc_Info.c_MoreInformation.c_FlashFingerprintUserName);
   EXPECT_EQ(mhu32_MAX_BLOCK_LENGTH, rc_Info.c_MoreInformation.u16_MaxNumberOfBlockLength);
   EXPECT_EQ(0x12U, rc_Info.c_MoreInformation.c_SerialNumber.au8_SerialNumber[0]);
   EXPECT_EQ(0xBCU, rc_Info.c_MoreInformation.c_SerialNumber.au8_SerialNumber[5]);

   //the flash blocks were read until the device said "no such block", and security level 1 was unlocked for it
   EXPECT_EQ(3U, mc_Ecu.RequestsFor(0x31U).size()); //blocks 0, 1 and the refused 2
   EXPECT_FALSE(mc_Ecu.c_SecurityLevelsUnlocked.empty());
   EXPECT_EQ(1U, mc_Ecu.c_SecurityLevelsUnlocked[0]);

   std::vector<C_OscSuSequencesNodeConnectStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetConnectStates(c_States));
   ASSERT_EQ(1U, c_States.size());
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].e_InformationRead);
   EXPECT_TRUE(c_States[0].c_AvailableFeatures.q_MaxNumberOfBlockLengthAvailable);
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_FlashesTheHexFileByteForByte)
{
   ASSERT_EQ(Errc::success, m_Init());
   uint32_t u32_SignatureOffset = 0U;
   const T_Bytes c_Image = mh_MakeImage("VIRTUAL-ECU", u32_SignatureOffset);
   const std::string c_HexPath = mh_WriteHexFile("osy_vecu_app.hex", c_Image);

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_HexPath);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_HEX_FINISHED));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_FINISHED));

   //what arrived on the device is the image, at its address, in the right number of pieces
   ASSERT_EQ(1U, mc_Ecu.c_Flash.size());
   ASSERT_EQ(1U, mc_Ecu.c_Flash.count(mhu32_APPLICATION_BASE));
   EXPECT_EQ(c_Image, mc_Ecu.c_Flash[mhu32_APPLICATION_BASE]);
   const std::vector<T_Bytes> c_Transfers = mc_Ecu.RequestsFor(0x36U);
   ASSERT_EQ((c_Image.size() + (mhu32_MAX_BLOCK_LENGTH - 4U) - 1U) / (mhu32_MAX_BLOCK_LENGTH - 4U),
             c_Transfers.size());
   EXPECT_EQ(1U, c_Transfers[0][1]); //block sequence counter starts at one

   //the erase was checked for exactly the image's extent, and the last exit named the signature block
   ASSERT_EQ(1U, mc_Ecu.c_MemoryChecks.size());
   EXPECT_EQ(mhu32_APPLICATION_BASE, mc_Ecu.c_MemoryChecks[0].first);
   EXPECT_EQ(c_Image.size(), mc_Ecu.c_MemoryChecks[0].second);
   ASSERT_EQ(1U, mc_Ecu.c_SignatureAddresses.size());
   EXPECT_EQ(mhu32_APPLICATION_BASE + u32_SignatureOffset, mc_Ecu.c_SignatureAddresses[0]);

   //programming session with security level 3, and a fingerprint with a user name
   EXPECT_NE(mc_Ecu.c_Sessions.end(), std::find(mc_Ecu.c_Sessions.begin(), mc_Ecu.c_Sessions.end(), 0x02U));
   EXPECT_NE(mc_Ecu.c_SecurityLevelsUnlocked.end(),
             std::find(mc_Ecu.c_SecurityLevelsUnlocked.begin(), mc_Ecu.c_SecurityLevelsUnlocked.end(), 3U));
   EXPECT_FALSE(mc_Ecu.c_FingerprintUser.empty());

   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   ASSERT_EQ(1U, c_States.size());
   ASSERT_EQ(1U, c_States[0].c_StateHexFiles.size());
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateHexFiles[0].e_FileLoaded);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateHexFiles[0].e_NodeNameCompared);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateHexFiles[0].e_AllTransferDataSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateHexFiles[0].e_RequestTransferAddressExitSent);

   (void)std::remove(c_HexPath.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_RefusesAHexFileBuiltForAnotherDevice)
{
   ASSERT_EQ(Errc::success, m_Init());
   uint32_t u32_SignatureOffset = 0U;
   const std::string c_HexPath = mh_WriteHexFile("osy_vecu_other.hex", mh_MakeImage("SOME-OTHER-BOX", u32_SignatureOffset));

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_HexPath);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::overflow, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_CHECK_DEVICE_NAME_MATCH_ERROR));
   EXPECT_TRUE(mc_Ecu.c_Flash.empty());
   EXPECT_TRUE(mc_Ecu.RequestsFor(0x34U).empty()); //never got as far as a download

   //but the same file passes when the name is on the accepted list
   c_ToFlash[0].c_OtherAcceptedDeviceNames.push_back("some-other-box");
   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_EQ(1U, mc_Ecu.c_Flash.size());

   (void)std::remove(c_HexPath.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_StopsWhenTheDeviceRefusesTheFlashRange)
{
   ASSERT_EQ(Errc::success, m_Init());
   mc_Ecu.q_RefuseFlashMemory = true;
   uint32_t u32_SignatureOffset = 0U;
   const std::string c_HexPath = mh_WriteHexFile("osy_vecu_refuse.hex", mh_MakeImage("VIRTUAL-ECU", u32_SignatureOffset));

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_HexPath);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::com, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_CHECK_MEMORY_NOT_OK));
   EXPECT_TRUE(mc_Ecu.c_Flash.empty());
   EXPECT_TRUE(mc_Ecu.c_FingerprintUser.empty()); //the fingerprint is written only once the memory is confirmed

   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   ASSERT_EQ(1U, c_States[0].c_StateHexFiles.size());
   EXPECT_EQ(eSUSEQ_STATE_ERROR, c_States[0].c_StateHexFiles[0].e_AvailableFlashMemoryChecked);

   (void)std::remove(c_HexPath.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_MissingFileIsRdWrBeforeAnythingIsSent)
{
   ASSERT_EQ(Errc::success, m_Init());
   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back("/nowhere/osy_vecu_absent.hex");
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::rd_wr, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Ecu.c_Requests.empty());
}

TEST_F(SuSequencesVirtualEcu, ResetSystem_SendsKeyOffOnToEveryNode)
{
   ASSERT_EQ(Errc::success, m_Init());

   EXPECT_EQ(Errc::success, mc_Sequences.ResetSystem());
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON}), mc_Ecu.c_ResetTypes);
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eRESET_SYSTEM_FINISHED));
}

TEST_F(SuSequencesVirtualEcu, WholeUpdate_ActivateReadFlashReset)
{
   ASSERT_EQ(Errc::success, m_Init());
   uint32_t u32_SignatureOffset = 0U;
   const T_Bytes c_Image = mh_MakeImage("VIRTUAL-ECU", u32_SignatureOffset);
   const std::string c_HexPath = mh_WriteHexFile("osy_vecu_whole.hex", c_Image);
   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_HexPath);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.ActivateFlashloader());
   EXPECT_EQ(Errc::success, mc_Sequences.ReadDeviceInformation());
   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_EQ(Errc::success, mc_Sequences.ResetSystem());

   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();
   EXPECT_EQ(c_Image, mc_Ecu.c_Flash[mhu32_APPLICATION_BASE]);
   EXPECT_EQ(std::vector<uint8_t>({C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_RESET_TO_FLASHLOADER,
                                   C_OscProtocolDriverOsyTpBase::hu8_OSY_RESET_TYPE_KEY_OFF_ON}),
             mc_Ecu.c_ResetTypes);

   (void)std::remove(c_HexPath.c_str());
}
