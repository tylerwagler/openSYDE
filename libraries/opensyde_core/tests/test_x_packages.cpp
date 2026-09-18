//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       X-config and X-certificates packages: create one, process it back

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
#include "C_OscSystemDefinitionFiler.hpp"
#include "C_OscXcoCreate.hpp"
#include "C_OscXcoLoad.hpp"
#include "C_OscXcoManifest.hpp"
#include "C_OscXceCreate.hpp"
#include "C_OscXceLoad.hpp"
#include "C_OscXceManifest.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
std::string h_Touch(const std::filesystem::path & orc_Path, const std::string & orc_Content)
{
   std::ofstream c_File(orc_Path, std::ios::binary);

   c_File << orc_Content;
   return orc_Path.string();
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An X-config package carries the system definition, the device manifests and the package manifest

   Processing returns the paths of the unpacked system definition and device root; loading the system definition
   from them goes through the process-global device manager, which is why this suite is its own binary.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XPackages, XConfigCreateThenProcess)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_xcfg_rt";
   const std::filesystem::path c_DevDir = c_Dir / "devices" / "RT-Device 1";
   const std::filesystem::path c_Package = c_Dir / "gateway.syde_xcfg";
   const std::filesystem::path c_Unzip = c_Dir / "unzip";

   (void)std::filesystem::remove_all(c_Dir);
   ASSERT_TRUE(std::filesystem::create_directories(c_DevDir));

   C_OscDeviceDefinition c_Device;
   c_Device.c_DeviceName = "RT-Device 1";
   c_Device.c_DeviceNameAlias = "rtdev";
   c_Device.u8_NumCanBusses = 1U;
   {
      C_OscSubDeviceDefinition c_Sub;
      c_Sub.c_SubDeviceName = "RT-Device 1";
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
      c_Source.c_Buses.push_back(c_Bus);
      C_OscNode c_Node;
      c_Node.c_DeviceType = c_Device.c_DeviceName;
      c_Node.pc_DeviceDefinition = &c_Device;
      c_Node.c_Properties.c_Name = "Gateway";
      C_OscNodeComInterfaceSettings c_If;
      c_If.e_InterfaceType = C_OscSystemBus::eCAN;
      c_If.u8_NodeId = 5U;
      c_If.AddConnection(0U);
      c_Node.c_Properties.c_ComInterfaces.push_back(c_If);
      c_Source.c_Nodes.push_back(c_Node);
   }
   C_OscXcoManifest c_Manifest;
   c_Manifest.c_NodeName = "Gateway";

   std::vector<std::string> c_Warnings;
   std::string c_Error;
   const std::error_code c_Created = C_OscXcoCreate::h_CreatePackage(c_Package.string(), c_Source, c_Manifest,
                                                                     c_Warnings, c_Error);
   ASSERT_FALSE(static_cast<bool>(c_Created)) << c_Created.message() << ": " << c_Error;
   EXPECT_TRUE(c_Warnings.empty()) << c_Warnings[0];
   ASSERT_TRUE(std::filesystem::exists(c_Package));

   C_OscXcoManifest c_ManifestBack;
   std::string c_SysDefPath;
   std::string c_DevDefPath;
   c_Warnings.clear();
   c_Error.clear();
   const std::error_code c_Processed = C_OscXcoLoad::h_ProcessPackage(c_Package.string(), c_Unzip.string(),
                                                                      c_ManifestBack, c_SysDefPath, c_DevDefPath,
                                                                      c_Warnings, c_Error);
   ASSERT_FALSE(static_cast<bool>(c_Processed)) << c_Processed.message() << ": " << c_Error;
   EXPECT_EQ("Gateway", c_ManifestBack.c_NodeName);
   EXPECT_TRUE(std::filesystem::exists(c_SysDefPath)) << c_SysDefPath;
   EXPECT_TRUE(c_SysDefPath.ends_with("xcfg_system_definition.syde_sysdef")) << c_SysDefPath;
   EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(c_DevDefPath) / "RT-Device 1" / "device.syd"))
      << c_DevDefPath;

   //the paths are meant to be loaded: with the package's own device root
   C_OscSystemDefinition c_Target;
   const std::error_code c_Loaded = C_OscSystemDefinitionFiler::h_LoadSystemDefinitionFile(
      c_Target, c_SysDefPath, c_DevDefPath, true, nullptr, nullptr, false, nullptr, nullptr);
   ASSERT_FALSE(static_cast<bool>(c_Loaded)) << c_Loaded.message();
   ASSERT_EQ(1U, c_Target.c_Nodes.size());
   EXPECT_EQ("Gateway", c_Target.c_Nodes[0].c_Properties.c_Name);
   ASSERT_NE(nullptr, c_Target.c_Nodes[0].pc_DeviceDefinition);
   EXPECT_EQ("rtdev", c_Target.c_Nodes[0].pc_DeviceDefinition->c_DeviceNameAlias);
   ASSERT_EQ(1U, c_Target.c_Buses.size());
   EXPECT_EQ("Service CAN", c_Target.c_Buses[0].c_Name);

   (void)std::filesystem::remove_all(c_Dir);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An X-certificates package carries the certificates and the per-package authentication keys

   The manifest's key paths are rewritten to the package-relative copies; the files themselves are copied verbatim
   (the creator does not parse them), so plain files stand in for certificates here.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XPackages, XCertificatesCreateThenProcess)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_xcert_rt";
   const std::filesystem::path c_Package = c_Dir / "certs.syde_xcert";
   const std::filesystem::path c_Unzip = c_Dir / "unzip";

   (void)std::filesystem::remove_all(c_Dir);
   ASSERT_TRUE(std::filesystem::create_directories(c_Dir / "in"));
   const std::vector<std::string> c_Certs = {h_Touch(c_Dir / "in" / "root.pem", "-----BEGIN CERTIFICATE-----\nA\n"),
                                             h_Touch(c_Dir / "in" / "device.pem", "-----BEGIN CERTIFICATE-----\nB\n")};
   std::vector<C_OscXceUpdatePackageParameters> c_Params(2);
   c_Params[0].c_Password = "p@ss w0rd <&>";
   c_Params[0].c_AuthenticationKeyPath = h_Touch(c_Dir / "in" / "auth_a.pem", "key A");
   c_Params[1].c_Password = "";
   c_Params[1].c_AuthenticationKeyPath = "";

   std::vector<std::string> c_Warnings;
   std::string c_Error;
   const std::error_code c_Created = C_OscXceCreate::h_CreatePackage(c_Package.string(), c_Certs, c_Params, c_Warnings,
                                                                     c_Error);
   ASSERT_FALSE(static_cast<bool>(c_Created)) << c_Created.message() << ": " << c_Error;
   EXPECT_TRUE(c_Warnings.empty()) << c_Warnings[0];

   C_OscXceManifest c_Manifest;
   c_Warnings.clear();
   c_Error.clear();
   const std::error_code c_Processed = C_OscXceLoad::h_ProcessPackage(c_Package.string(), c_Unzip.string(), c_Manifest,
                                                                      c_Warnings, c_Error);
   ASSERT_FALSE(static_cast<bool>(c_Processed)) << c_Processed.message() << ": " << c_Error;
   EXPECT_EQ("authentication_certificates", c_Manifest.c_CertificatesPath);
   for (const char * const pcn_Name : {"root.pem", "device.pem"})
   {
      EXPECT_TRUE(std::filesystem::exists(c_Unzip / c_Manifest.c_CertificatesPath / pcn_Name)) << pcn_Name;
   }
   ASSERT_EQ(2U, c_Manifest.c_UpdatePackageParameters.size());
   EXPECT_EQ("p@ss w0rd <&>", c_Manifest.c_UpdatePackageParameters[0].c_Password);
   EXPECT_EQ("update_certificates/auth_a.pem", c_Manifest.c_UpdatePackageParameters[0].c_AuthenticationKeyPath)
      << "rewritten to the package-relative copy";
   EXPECT_TRUE(std::filesystem::exists(c_Unzip / "update_certificates" / "auth_a.pem"));
   EXPECT_EQ("", c_Manifest.c_UpdatePackageParameters[1].c_Password);
   EXPECT_EQ("", c_Manifest.c_UpdatePackageParameters[1].c_AuthenticationKeyPath);

   //a certificate that does not exist is refused up front
   std::vector<std::string> c_Missing = c_Certs;
   c_Missing.push_back((c_Dir / "in" / "absent.pem").string());
   (void)std::filesystem::remove(c_Package);
   EXPECT_EQ(Errc::noact, C_OscXceCreate::h_CreatePackage(c_Package.string(), c_Missing, c_Params, c_Warnings,
                                                          c_Error));

   (void)std::filesystem::remove_all(c_Dir);
}
