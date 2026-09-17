//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Service update package: create a package and process it back

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscSupServiceUpdatePackageCreate.hpp"
#include "C_OscSupServiceUpdatePackageLoad.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
void h_Touch(const std::filesystem::path & orc_Path, const std::string & orc_Content)
{
   std::ofstream c_File(orc_Path, std::ios::binary);

   c_File << orc_Content;
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A package created from a two-node system comes back with the same topology, flags and files

   This is the round-trip the zip-container filers deserve: C_OscSupServiceUpdatePackageCreate writes the system
   definition (through the node file filers), one device manifest per device (copied from the definition's own
   file), the package definition and every application/NVM file into a zip; C_OscSupServiceUpdatePackageLoad
   unpacks it and re-reads the system definition WITH device definitions rooted at the unpack directory.

   That last step goes through the process-global C_OscSystemDefinition::hc_Devices, which is why this lives in its
   own test binary: it only loads the package's device root if nothing loaded device definitions before.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(SupPackage, CreateThenProcessRoundTrip)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_sup_rt";
   const std::filesystem::path c_DevDir = c_Dir / "devices" / "RT-Device 1";
   const std::filesystem::path c_Files = c_Dir / "files";
   const std::filesystem::path c_Unzip = c_Dir / "unzip";
   const std::filesystem::path c_Package = c_Dir / "round_trip.syde_sup";

   (void)std::filesystem::remove_all(c_Dir);
   ASSERT_TRUE(std::filesystem::create_directories(c_DevDir));
   ASSERT_TRUE(std::filesystem::create_directories(c_Files));

   //the device definition has to exist on disk: the package copies its manifest by path
   C_OscDeviceDefinition c_Device;
   c_Device.c_DeviceName = "RT-Device 1";
   c_Device.c_DeviceNameAlias = "rtdev";
   c_Device.u8_NumCanBusses = 1U;
   c_Device.u8_NumEthernetBusses = 0U;
   c_Device.c_SupportedBitrates = {250U, 500U};
   {
      C_OscSubDeviceDefinition c_Sub;
      c_Sub.c_SubDeviceName = "RT-Device 1";
      c_Sub.c_OtherAcceptedNames = {"RTD1", "rt-device-one"};
      c_Sub.q_ProgrammingSupport = true;
      c_Sub.q_FlashloaderOpenSydeCan = true;
      c_Sub.q_DiagnosticProtocolOpenSydeCan = true;
      c_Sub.c_ConnectedInterfaces = {{"can1", true}};
      c_Device.c_SubDevices.push_back(c_Sub);
   }
   c_Device.c_FilePath = (c_DevDir / "device.syd").string();
   ASSERT_FALSE(static_cast<bool>(C_OscDeviceDefinitionFiler::h_Save(c_Device, c_Device.c_FilePath)));

   C_OscSystemDefinition c_Source;
   {
      C_OscSystemBus c_Bus;
      c_Bus.c_Name = "Service CAN";
      c_Bus.e_Type = C_OscSystemBus::eCAN;
      c_Bus.u64_BitRate = 500000ULL;
      c_Bus.u8_BusId = 0U;
      c_Source.c_Buses.push_back(c_Bus);
   }
   for (const char * const pcn_Name : {"Gateway", "Sensor"})
   {
      C_OscNode c_Node;
      c_Node.c_DeviceType = c_Device.c_DeviceName;
      c_Node.pc_DeviceDefinition = &c_Device;
      c_Node.c_Properties.c_Name = pcn_Name;
      c_Node.c_Properties.e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
      c_Node.c_Properties.e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;
      C_OscNodeComInterfaceSettings c_If;
      c_If.e_InterfaceType = C_OscSystemBus::eCAN;
      c_If.u8_InterfaceNumber = 0U;
      c_If.u8_NodeId = static_cast<uint8_t>(10U + c_Source.c_Nodes.size());
      c_If.q_IsUpdateEnabled = true;
      c_If.AddConnection(0U);
      c_Node.c_Properties.c_ComInterfaces.push_back(c_If);
      c_Source.c_Nodes.push_back(c_Node);
   }

   //files to ship: two applications and one parameter set for the gateway, nothing for the sensor
   h_Touch(c_Files / "app_a.hex", ":00000001FF\n");
   h_Touch(c_Files / "app_b.hex", ":00000001FF\n");
   h_Touch(c_Files / "params.syde_psi", "<not really a parameter set/>");
   std::vector<C_OscSuSequences::C_DoFlash> c_Apps(2);
   c_Apps[0].c_FilesToFlash = {(c_Files / "app_a.hex").string(), (c_Files / "app_b.hex").string()};
   c_Apps[0].c_FilesToWriteToNvm = {(c_Files / "params.syde_psi").string()};
   c_Apps[0].q_SendSecureAuthenticationEnabledState = true;
   c_Apps[0].q_SecureAuthenticationEnabled = true;
   c_Apps[0].q_SendTrafficEncryptionEnabledState = true;
   c_Apps[0].q_TrafficEncryptionEnabled = false;
   c_Apps[0].q_SendDebuggerEnabledState = true;
   c_Apps[0].q_DebuggerEnabled = false;
   const std::vector<uint8_t> c_Active = {1U, 0U};
   const std::vector<uint32_t> c_Order = {0U};

   std::vector<std::string> c_Warnings;
   std::string c_Error;
   const std::error_code c_Created = C_OscSupServiceUpdatePackageCreate::h_CreatePackage(
      c_Package.string(), c_Source, 0U, c_Active, c_Order, c_Apps, c_Warnings, c_Error);
   ASSERT_FALSE(static_cast<bool>(c_Created)) << c_Created.message() << ": " << c_Error;
   EXPECT_TRUE(c_Warnings.empty()) << c_Warnings[0];
   ASSERT_TRUE(std::filesystem::exists(c_Package));

   C_OscSystemDefinition c_Target;
   uint32_t u32_Bus = 99U;
   std::vector<uint8_t> c_ActiveBack;
   std::vector<uint32_t> c_OrderBack;
   std::vector<C_OscSuSequences::C_DoFlash> c_AppsBack;
   c_Warnings.clear();
   c_Error.clear();
   const std::error_code c_Processed = C_OscSupServiceUpdatePackageLoad::h_ProcessPackage(
      c_Package.string(), c_Unzip.string(), c_Target, u32_Bus, c_ActiveBack, c_OrderBack, c_AppsBack, c_Warnings,
      c_Error, true);
   ASSERT_FALSE(static_cast<bool>(c_Processed)) << c_Processed.message() << ": " << c_Error;
   EXPECT_TRUE(c_Warnings.empty()) << c_Warnings[0];

   EXPECT_EQ(0U, u32_Bus);
   EXPECT_EQ(c_Active, c_ActiveBack);
   EXPECT_EQ(c_Order, c_OrderBack);
   ASSERT_EQ(1U, c_Target.c_Buses.size());
   EXPECT_EQ("Service CAN", c_Target.c_Buses[0].c_Name);
   ASSERT_EQ(2U, c_Target.c_Nodes.size());
   //only the active nodes are loaded; an inactive one stays a named placeholder without a device
   EXPECT_EQ("Gateway", c_Target.c_Nodes[0].c_Properties.c_Name);
   EXPECT_EQ("RT-Device 1", c_Target.c_Nodes[0].c_DeviceType);
   ASSERT_NE(nullptr, c_Target.c_Nodes[0].pc_DeviceDefinition) << "the package's own device manifest was not picked up";
   EXPECT_EQ("rtdev", c_Target.c_Nodes[0].pc_DeviceDefinition->c_DeviceNameAlias);
   EXPECT_EQ("UnloadedNodeWithNodeIndex1", c_Target.c_Nodes[1].c_Properties.c_Name);
   EXPECT_EQ(nullptr, c_Target.c_Nodes[1].pc_DeviceDefinition);

   ASSERT_EQ(2U, c_AppsBack.size());
   const C_OscSuSequences::C_DoFlash & rc_Gw = c_AppsBack[0];
   ASSERT_EQ(2U, rc_Gw.c_FilesToFlash.size());
   for (const std::string & rc_File : rc_Gw.c_FilesToFlash)
   {
      EXPECT_TRUE(std::filesystem::exists(rc_File)) << rc_File;
      EXPECT_NE(std::string::npos, rc_File.find(c_Unzip.filename().string())) << "not from the unpacked package";
   }
   //copied files carry their update position as a prefix ("1_app_a.hex"): the order is what the updater needs
   EXPECT_TRUE(rc_Gw.c_FilesToFlash[0].ends_with("1_app_a.hex")) << rc_Gw.c_FilesToFlash[0];
   EXPECT_TRUE(rc_Gw.c_FilesToFlash[1].ends_with("2_app_b.hex")) << rc_Gw.c_FilesToFlash[1];
   ASSERT_EQ(1U, rc_Gw.c_FilesToWriteToNvm.size());
   EXPECT_TRUE(rc_Gw.c_FilesToWriteToNvm[0].ends_with("1_params.syde_psi")) << rc_Gw.c_FilesToWriteToNvm[0];
   EXPECT_TRUE(rc_Gw.q_SendSecureAuthenticationEnabledState);
   EXPECT_TRUE(rc_Gw.q_SecureAuthenticationEnabled);
   EXPECT_TRUE(rc_Gw.q_SendTrafficEncryptionEnabledState);
   EXPECT_FALSE(rc_Gw.q_TrafficEncryptionEnabled);
   EXPECT_TRUE(rc_Gw.q_SendDebuggerEnabledState);
   EXPECT_FALSE(rc_Gw.q_DebuggerEnabled);
   EXPECT_EQ((std::vector<std::string>{"RTD1", "rt-device-one"}), rc_Gw.c_OtherAcceptedDeviceNames)
      << "filled from the device manifest inside the package";
   EXPECT_TRUE(c_AppsBack[1].c_FilesToFlash.empty()) << "the inactive node ships nothing";

   (void)std::filesystem::remove_all(c_Dir);
}
