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
#include <fstream>
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
#include "C_OscParamSetHandler.hpp"
#include "C_OscParamSetInterpretedNode.hpp"
#include "C_OscParamSetRawNode.hpp"
#include "C_OscSystemDefinition.hpp"
#include "osy_virtual_ecu.hpp"

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
using namespace osy_virtual_ecu;

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
   mc_Sequences.SetPollingTimeout(C_OscProtocolDriverOsyNode(mhu8_BUS_ID, mhu8_ECU_NODE_ID), 30U); //silence costs milliseconds here, not the device's second

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

/* -- The other flashloader paths: file based, and parameter set images into NVM ------------------------------------- */

namespace
{
std::string mh_WriteBinaryFile(const std::string & orc_Name, const std::vector<uint8_t> & orc_Bytes)
{
   const std::string c_Path = (std::filesystem::temp_directory_path() / orc_Name).string();
   std::ofstream c_Stream(c_Path, std::ofstream::binary | std::ofstream::trunc);
   c_Stream.write(reinterpret_cast<const char *>(orc_Bytes.data()), static_cast<std::streamsize>(orc_Bytes.size()));
   return c_Path;
}

std::vector<uint8_t> mh_Pattern(const uint32_t ou32_Count, const uint32_t ou32_Seed)
{
   std::vector<uint8_t> c_Bytes(ou32_Count);
   for (uint32_t u32_Index = 0U; u32_Index < ou32_Count; ++u32_Index)
   {
      c_Bytes[u32_Index] = static_cast<uint8_t>((u32_Index * 31U + ou32_Seed) & 0xFFU);
   }
   return c_Bytes;
}

///A parameter set image for the node: two raw entries at 0x2000 and 0x2100, and the interpreted twin the
///format requires. Written without CRC and then stamped, the way the GUI's "create image" ends.
std::string mh_WriteParameterSetImage(const std::string & orc_Name, const std::vector<uint8_t> & orc_Small,
                                      const std::vector<uint8_t> & orc_Large)
{
   const std::string c_Path = (std::filesystem::temp_directory_path() / orc_Name).string();
   (void)std::remove(c_Path.c_str());

   C_OscParamSetDataPoolInfo c_Info;
   c_Info.c_Name = "NvmPool";
   c_Info.u32_DataPoolCrc = 0x1234U;
   c_Info.u32_NvmStartAddress = 0x2000U;
   c_Info.u32_NvmSize = 0x200U;
   c_Info.au8_Version[0] = 1U;
   c_Info.au8_Version[1] = 0U;
   c_Info.au8_Version[2] = 0U;

   C_OscParamSetRawNode c_Raw;
   c_Raw.c_Name = "Node1";
   c_Raw.c_DataPools.push_back(c_Info);
   C_OscParamSetRawEntry c_Entry;
   c_Entry.u32_StartAddress = 0x2000U;
   c_Entry.c_Bytes = orc_Small;
   c_Raw.c_Entries.push_back(c_Entry);
   c_Entry.u32_StartAddress = 0x2100U;
   c_Entry.c_Bytes = orc_Large;
   c_Raw.c_Entries.push_back(c_Entry);

   C_OscParamSetInterpretedNode c_Interpreted;
   c_Interpreted.c_Name = "Node1";
   C_OscParamSetInterpretedDataPool c_Pool;
   c_Pool.c_DataPoolInfo = c_Info;
   C_OscParamSetInterpretedList c_List;
   c_List.c_Name = "Persisted";
   C_OscParamSetInterpretedElement c_Element;
   c_Element.c_Name = "Speed";
   c_Element.c_NvmValue.SetType(C_OscNodeDataPoolContent::eUINT16);
   c_Element.c_NvmValue.SetValueU16(1200U);
   c_List.c_Elements.push_back(c_Element);
   c_Pool.c_Lists.push_back(c_List);
   c_Interpreted.c_DataPools.push_back(c_Pool);

   C_OscParamSetHandler c_Handler;
   EXPECT_FALSE(static_cast<bool>(c_Handler.AddRawDataForNode(c_Raw)));
   EXPECT_FALSE(static_cast<bool>(c_Handler.AddInterpretedDataForNode(c_Interpreted)));
   EXPECT_FALSE(static_cast<bool>(c_Handler.CreateCleanFileWithoutCrc(c_Path)));
   EXPECT_FALSE(static_cast<bool>(C_OscParamSetHandler::h_UpdateCrcForFile(c_Path)));
   return c_Path;
}
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_FileBasedFlashloader_TransfersTheFileAndItsCrc)
{
   mc_DeviceDefinition.c_SubDevices[0].q_FlashloaderOpenSydeIsFileBased = true;
   mc_Ecu.u8_FeatureByte7 = 0x02U | 0x08U; //max block length readable, transfer exit result readable
   ASSERT_EQ(Errc::success, m_Init());
   const std::vector<uint8_t> c_Bytes = mh_Pattern(700U, 3U);
   const std::string c_Path = mh_WriteBinaryFile("osy_vecu_app.bin", c_Bytes);

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_Path);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();

   //the device was told the file's name and size, got every byte, and the CRC the client sent matches
   ASSERT_EQ(1U, mc_Ecu.c_FileRequests.size());
   EXPECT_EQ("osy_vecu_app.bin", mc_Ecu.c_FileRequests[0].first);
   EXPECT_EQ(700U, mc_Ecu.c_FileRequests[0].second);
   ASSERT_EQ(1U, mc_Ecu.c_Files.count("osy_vecu_app.bin"));
   EXPECT_EQ(c_Bytes, mc_Ecu.c_Files["osy_vecu_app.bin"]);
   ASSERT_EQ(1U, mc_Ecu.c_FileCrcsReceived.size());
   EXPECT_EQ(mc_Ecu.c_FileCrcsComputed[0], mc_Ecu.c_FileCrcsReceived[0]);
   EXPECT_TRUE(mc_Ecu.c_Flash.empty()); //nothing address based happened

   //and the device's verdict on the transfer was read back and reported
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_RESULT_STRING));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_FINISHED));
   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   ASSERT_EQ(1U, c_States[0].c_StateOtherFiles.size());
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateOtherFiles[0].e_RequestFileTransferSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateOtherFiles[0].e_AllTransferDataSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateOtherFiles[0].e_RequestTransferFileExitSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateOtherFiles[0].e_RequestTransferFileExitResultSent);

   (void)std::remove(c_Path.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_FileBased_DeviceRejectsTheTransfer_ResultStringIsStillRead)
{
   mc_DeviceDefinition.c_SubDevices[0].q_FlashloaderOpenSydeIsFileBased = true;
   mc_Ecu.u8_FeatureByte7 = 0x02U | 0x08U;
   mc_Ecu.q_RejectFileCrc = true;
   ASSERT_EQ(Errc::success, m_Init());
   const std::string c_Path = mh_WriteBinaryFile("osy_vecu_bad.bin", mh_Pattern(100U, 9U));

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToFlash.push_back(c_Path);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::com, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_EXIT_ERROR));
   //"general programming failure" means the target layer had an opinion, and the sequence asks for it
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_RESULT_STRING));
   EXPECT_FALSE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_FLASH_FILE_FINISHED));

   (void)std::remove(c_Path.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_WritesAParameterSetImageIntoNvmEntryByEntry)
{
   mc_Ecu.u8_FeatureByte7 = 0x01U | 0x02U; //flashloader can write NVM, max block length readable
   ASSERT_EQ(Errc::success, m_Init());
   const std::vector<uint8_t> c_Small = mh_Pattern(8U, 1U);
   const std::vector<uint8_t> c_Large = mh_Pattern(300U, 2U); //more than one WriteMemoryByAddress at 256 - 10
   const std::string c_Path = mh_WriteParameterSetImage("osy_vecu_params.syde_psi", c_Small, c_Large);

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToWriteToNvm.push_back(c_Path);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();

   //both entries landed where the image said, in three writes: 8, then 246 + 54
   ASSERT_EQ(3U, mc_Ecu.c_NvmWrites.size());
   EXPECT_EQ(std::make_pair(0x2000U, 8U), mc_Ecu.c_NvmWrites[0]);
   EXPECT_EQ(std::make_pair(0x2100U, 246U), mc_Ecu.c_NvmWrites[1]);
   EXPECT_EQ(std::make_pair(0x2100U + 246U, 54U), mc_Ecu.c_NvmWrites[2]);
   for (uint32_t u32_Index = 0U; u32_Index < c_Small.size(); ++u32_Index)
   {
      EXPECT_EQ(c_Small[u32_Index], mc_Ecu.c_Nvm[0x2000U + u32_Index]) << u32_Index;
   }
   for (uint32_t u32_Index = 0U; u32_Index < c_Large.size(); ++u32_Index)
   {
      EXPECT_EQ(c_Large[u32_Index], mc_Ecu.c_Nvm[0x2100U + u32_Index]) << u32_Index;
   }
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_FINISHED));
   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   ASSERT_EQ(1U, c_States[0].c_StatePsiFiles.size());
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StatePsiFiles[0].e_FileLoaded);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StatePsiFiles[0].e_PsiFileWrote);

   (void)std::remove(c_Path.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_NvmWriteNeedsTheFlashloaderFeature)
{
   mc_Ecu.u8_FeatureByte7 = 0x02U; //no "can write NVM"
   ASSERT_EQ(Errc::success, m_Init());
   const std::string c_Path = mh_WriteParameterSetImage("osy_vecu_params2.syde_psi", mh_Pattern(8U, 1U),
                                                        mh_Pattern(16U, 2U));

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_FilesToWriteToNvm.push_back(c_Path);
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::range, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_NVM_WRITE_AVAILABLE_FEATURE_ERROR));
   EXPECT_TRUE(mc_Ecu.c_NvmWrites.empty());

   (void)std::remove(c_Path.c_str());
}

/* -- The node state flags: secure authentication, traffic encryption, debugger ------------------------------------- */

TEST_F(SuSequencesVirtualEcu, UpdateSystem_WritesTheSecurityAndDebuggerFlagsTheDeviceSupports)
{
   mc_Ecu.u8_FeatureByte7 = 0x02U | 0x20U | 0x40U | 0x80U; //authentication, debugger off and on
   mc_Ecu.u8_FeatureByte6 = 0x01U;                          //traffic encryption
   ASSERT_EQ(Errc::success, m_Init());

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].q_SendSecureAuthenticationEnabledState = true;
   c_ToFlash[0].q_SecureAuthenticationEnabled = true;
   c_ToFlash[0].q_SendTrafficEncryptionEnabledState = true;
   c_ToFlash[0].q_TrafficEncryptionEnabled = false;
   c_ToFlash[0].q_SendDebuggerEnabledState = true;
   c_ToFlash[0].q_DebuggerEnabled = false;
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();

   ASSERT_EQ(1U, mc_Ecu.c_AuthenticationActivations.size());
   EXPECT_TRUE(mc_Ecu.c_AuthenticationActivations[0].first);
   EXPECT_EQ(0U, mc_Ecu.c_AuthenticationActivations[0].second);
   ASSERT_EQ(1U, mc_Ecu.c_EncryptionActivations.size());
   EXPECT_FALSE(mc_Ecu.c_EncryptionActivations[0].first);
   ASSERT_EQ(1U, mc_Ecu.c_DebuggerActivations.size());
   EXPECT_FALSE(mc_Ecu.c_DebuggerActivations[0]);

   //the flags are written in the programming session at security level 1
   EXPECT_NE(mc_Ecu.c_Sessions.end(), std::find(mc_Ecu.c_Sessions.begin(), mc_Ecu.c_Sessions.end(), 0x02U));
   EXPECT_NE(mc_Ecu.c_SecurityLevelsUnlocked.end(),
             std::find(mc_Ecu.c_SecurityLevelsUnlocked.begin(), mc_Ecu.c_SecurityLevelsUnlocked.end(), 1U));

   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_SecureAuthenticationFlagSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_TrafficEncryptionFlagSent);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_DebuggerFlagSent);
   EXPECT_EQ(eSUSEQ_STATE_NOT_NEEDED, c_States[0].c_StateSecuritySettings.e_SecureAuthenticationKeySent);
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_DebuggerFlagNeedsTheMatchingFeature)
{
   //the device can switch its debugger off but not on
   mc_Ecu.u8_FeatureByte7 = 0x02U | 0x40U;
   ASSERT_EQ(Errc::success, m_Init());

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].q_SendDebuggerEnabledState = true;
   c_ToFlash[0].q_DebuggerEnabled = true;
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::range, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_STATE_DEBUGGER_WRITE_AVAILABLE_FEATURE_ERROR));
   EXPECT_TRUE(mc_Ecu.c_DebuggerActivations.empty());

   //off is supported
   c_ToFlash[0].q_DebuggerEnabled = false;
   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   ASSERT_EQ(1U, mc_Ecu.c_DebuggerActivations.size());
   EXPECT_FALSE(mc_Ecu.c_DebuggerActivations[0]);
}

/* -- The PEM write: a device learns which certificate may authenticate against it ---------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A self-signed RSA-1024 identity as PEM text, plus the two things the service sends: modulus and serial

   The service (`OsyWriteSecurityAuthenticationKey`) takes a 128 byte modulus, which is RSA-1024. The security
   suite's identity builder makes 2048 bit keys, so this is its small sibling rather than a share.
*/
//----------------------------------------------------------------------------------------------------------------------
std::string mh_WriteRsa1024Pem(const std::string & orc_Name, std::vector<uint8_t> & orc_Modulus,
                               std::vector<uint8_t> & orc_Serial)
{
   EVP_PKEY * const pc_Key = EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", static_cast<size_t>(1024));
   X509 * const pc_Cert = X509_new();
   (void)X509_set_version(pc_Cert, 2);
   (void)ASN1_INTEGER_set(X509_get_serialNumber(pc_Cert), 77L);
   (void)X509_gmtime_adj(X509_getm_notBefore(pc_Cert), 0);
   (void)X509_gmtime_adj(X509_getm_notAfter(pc_Cert), 60L * 60L * 24L * 365L);
   X509_NAME * const pc_Subject = X509_get_subject_name(pc_Cert);
   (void)X509_NAME_add_entry_by_txt(pc_Subject, "CN", MBSTRING_ASC,
                                    reinterpret_cast<const unsigned char *>("osy virtual ecu operator"), -1, -1, 0);
   (void)X509_set_issuer_name(pc_Cert, pc_Subject);
   (void)X509_set_pubkey(pc_Cert, pc_Key);
   (void)X509_sign(pc_Cert, pc_Key, EVP_sha256());

   {
      BIGNUM * pc_Modulus = nullptr;
      EXPECT_EQ(1, EVP_PKEY_get_bn_param(pc_Key, OSSL_PKEY_PARAM_RSA_N, &pc_Modulus));
      orc_Modulus.resize(128U);
      EXPECT_EQ(128, BN_bn2binpad(pc_Modulus, orc_Modulus.data(), 128));
      BN_free(pc_Modulus);
   }
   {
      //the loader keeps the INTEGER's content and drops the two byte tag/length header
      unsigned char * pu8_Der = nullptr;
      const int x_Length = i2d_ASN1_INTEGER(X509_get_serialNumber(pc_Cert), &pu8_Der);
      orc_Serial.assign(pu8_Der + 2, pu8_Der + x_Length);
      OPENSSL_free(pu8_Der);
   }

   BIO * const pc_Bio = BIO_new(BIO_s_mem());
   (void)PEM_write_bio_X509(pc_Bio, pc_Cert);
   (void)PEM_write_bio_PKCS8PrivateKey(pc_Bio, pc_Key, nullptr, nullptr, 0, nullptr, nullptr);
   char * pcn_Text = nullptr;
   const long x_TextLength = BIO_get_mem_data(pc_Bio, &pcn_Text);
   const std::string c_Path = (std::filesystem::temp_directory_path() / orc_Name).string();
   {
      std::ofstream c_Stream(c_Path, std::ofstream::binary | std::ofstream::trunc);
      c_Stream.write(pcn_Text, x_TextLength);
   }
   BIO_free(pc_Bio);
   X509_free(pc_Cert);
   EVP_PKEY_free(pc_Key);
   return c_Path;
}
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_WritesThePemsPublicKeyAndSerialAsTheAuthenticationKey)
{
   mc_Ecu.u8_FeatureByte7 = 0x02U | 0x20U; //secure authentication supported
   ASSERT_EQ(Errc::success, m_Init());
   std::vector<uint8_t> c_Modulus;
   std::vector<uint8_t> c_Serial;
   const std::string c_Path = mh_WriteRsa1024Pem("osy_vecu_operator.pem", c_Modulus, c_Serial);

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_PemFile = c_Path;
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::success, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.c_Errors.empty()) << mc_Sequences.ErrorsAsText();

   //modulus (128), exponent right-aligned in four bytes (65537), then the certificate serial
   ASSERT_EQ(1U, mc_Ecu.c_AuthenticationKeysWritten.size());
   const std::vector<uint8_t> & rc_Sent = mc_Ecu.c_AuthenticationKeysWritten[0];
   ASSERT_EQ(128U + 4U + c_Serial.size(), rc_Sent.size());
   EXPECT_EQ(c_Modulus, std::vector<uint8_t>(rc_Sent.begin(), rc_Sent.begin() + 128));
   EXPECT_EQ(std::vector<uint8_t>({0x00U, 0x01U, 0x00U, 0x01U}),
             std::vector<uint8_t>(rc_Sent.begin() + 128, rc_Sent.begin() + 132));
   EXPECT_EQ(c_Serial, std::vector<uint8_t>(rc_Sent.begin() + 132, rc_Sent.end()));

   //written in the programming session at security level 1
   EXPECT_NE(mc_Ecu.c_Sessions.end(), std::find(mc_Ecu.c_Sessions.begin(), mc_Ecu.c_Sessions.end(), 0x02U));
   EXPECT_NE(mc_Ecu.c_SecurityLevelsUnlocked.end(),
             std::find(mc_Ecu.c_SecurityLevelsUnlocked.begin(), mc_Ecu.c_SecurityLevelsUnlocked.end(), 1U));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_FINISHED));

   std::vector<C_OscSuSequencesNodeUpdateStates> c_States;
   ASSERT_EQ(Errc::success, mc_Sequences.GetUpdateStates(c_States));
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_FileLoaded);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_PemFileExtracted);
   EXPECT_EQ(eSUSEQ_STATE_NO_ERR, c_States[0].c_StateSecuritySettings.e_SecureAuthenticationKeySent);

   (void)std::remove(c_Path.c_str());
}

TEST_F(SuSequencesVirtualEcu, UpdateSystem_PemWriteNeedsTheAuthenticationFeature)
{
   mc_Ecu.u8_FeatureByte7 = 0x02U;
   ASSERT_EQ(Errc::success, m_Init());
   std::vector<uint8_t> c_Modulus;
   std::vector<uint8_t> c_Serial;
   const std::string c_Path = mh_WriteRsa1024Pem("osy_vecu_operator2.pem", c_Modulus, c_Serial);

   std::vector<C_OscSuSequences::C_DoFlash> c_ToFlash(1U);
   c_ToFlash[0].c_PemFile = c_Path;
   const std::vector<uint32_t> c_Order(1U, 0U);

   EXPECT_EQ(Errc::range, mc_Sequences.UpdateSystem(c_ToFlash, c_Order));
   EXPECT_TRUE(mc_Sequences.Saw(C_OscSuSequences::eUPDATE_SYSTEM_OSY_NODE_PEM_FILE_WRITE_AVAILABLE_FEATURE_ERROR));
   EXPECT_TRUE(mc_Ecu.c_AuthenticationKeysWritten.empty());

   (void)std::remove(c_Path.c_str());
}
