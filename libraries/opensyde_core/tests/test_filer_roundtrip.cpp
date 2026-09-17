//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Save/load round-trip tests for the project filers

   Every defect found in the September 2026 sweep lived in an integration path rather
   than in a function: C_OscChecksummedXml could not reload the file it had just
   written, and a parameter set's own version did not survive the trip. Unit tests over
   individual functions cannot see that class at all -- only a round trip can.

   Each test here fills an object with values distinct from its defaults, writes it,
   reads it back into a fresh object, and compares. CalcHash is used where available as
   a deep-equality oracle, with the individual fields checked too so a failure says
   which one was lost rather than only that something was.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include <cstdint>
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscSystemBusFiler.hpp"
#include "C_OscProject.hpp"
#include "C_OscProjectFiler.hpp"
#include "C_OscDataLoggerJob.hpp"
#include "C_OscDataLoggerJobFiler.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscNodeDataPoolFiler.hpp"
#include "C_OscXcoManifest.hpp"
#include "C_OscXcoManifestFiler.hpp"
#include "C_OscXappProperties.hpp"
#include "C_OscXappPropertiesFiler.hpp"
#include "C_OscXceManifest.hpp"
#include "C_OscXceManifestFiler.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscHalcConfigStandalone.hpp"
#include "C_OscHalcConfigStandaloneFiler.hpp"
#include "C_OscHalcDef.hpp"
#include "C_OscHalcDefFiler.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscNodeCommFiler.hpp"
#include "C_OscNodeSquad.hpp"
#include "C_OscNodeSquadFiler.hpp"
#include "C_OscParamSetInterpretedNode.hpp"
#include "C_OscParamSetInterpretedNodeFiler.hpp"
#include "C_OscParamSetRawNode.hpp"
#include "C_OscParamSetRawNodeFiler.hpp"
#include "C_OscViewData.hpp"
#include "C_OscViewFiler.hpp"
#include "C_OscNode.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSystemDefinitionFiler.hpp"
#include "C_OscNodeFiler.hpp"
#include "C_OscCanOpenManagerInfo.hpp"
#include "C_OscCanOpenManagerFiler.hpp"
#include "C_OscCanInterfaceId.hpp"
#include "C_OscHalcConfig.hpp"
#include "C_OscHalcConfigFiler.hpp"
#include "C_OscHalcConfigUtil.hpp"
#include "osy_test_models.hpp"

#include <filesystem>
#include <fstream>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A fully populated bus survives a save/load round trip

   Every field is set to something other than its default, so a field that is dropped
   or never written shows up as a mismatch rather than coincidentally matching the
   default-constructed destination.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, SystemBus)
{
   C_OscSystemBus c_Source;

   c_Source.e_Type = C_OscSystemBus::eETHERNET;
   c_Source.c_Name = "Round Trip Bus";
   c_Source.c_Comment = "comment with spaces & an ampersand";
   c_Source.u64_BitRate = 500000ULL;
   c_Source.q_UseCanFd = true;
   c_Source.u64_CanFdBitRate = 2000000ULL;
   c_Source.u8_BusId = 7U;
   c_Source.u16_RxTimeoutOffsetMs = 1234U;
   c_Source.q_UseableForRouting = false;

   C_OscXmlParser c_Xml;
   ASSERT_EQ("bus", c_Xml.CreateAndSelectNodeChild("bus"));
   C_OscSystemBusFiler::h_SaveBus(c_Source, c_Xml);

   C_OscSystemBus c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscSystemBusFiler::h_LoadBus(c_Target, c_Xml)));

   EXPECT_EQ(c_Source.e_Type, c_Target.e_Type);
   EXPECT_EQ(c_Source.c_Name, c_Target.c_Name);
   EXPECT_EQ(c_Source.c_Comment, c_Target.c_Comment);
   EXPECT_EQ(c_Source.u64_BitRate, c_Target.u64_BitRate);
   EXPECT_EQ(c_Source.q_UseCanFd, c_Target.q_UseCanFd);
   EXPECT_EQ(c_Source.u64_CanFdBitRate, c_Target.u64_CanFdBitRate);
   EXPECT_EQ(c_Source.u8_BusId, c_Target.u8_BusId);
   EXPECT_EQ(c_Source.u16_RxTimeoutOffsetMs, c_Target.u16_RxTimeoutOffsetMs);
   EXPECT_EQ(c_Source.q_UseableForRouting, c_Target.q_UseableForRouting);

   uint32_t u32_HashSource = 0xFFFFFFFFUL;
   uint32_t u32_HashTarget = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_HashSource);
   c_Target.CalcHash(u32_HashTarget);
   EXPECT_EQ(u32_HashSource, u32_HashTarget) << "a field is lost that the checks above do not cover";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A CAN bus without CAN-FD also round trips

   The CAN-FD properties are written conditionally, so the disabled case takes a
   different path through both the writer and the reader.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, SystemBusWithoutCanFd)
{
   C_OscSystemBus c_Source;

   c_Source.e_Type = C_OscSystemBus::eCAN;
   c_Source.c_Name = "Plain CAN";
   c_Source.c_Comment = "";
   c_Source.u64_BitRate = 125000ULL;
   c_Source.q_UseCanFd = false;
   c_Source.u8_BusId = 1U;
   c_Source.u16_RxTimeoutOffsetMs = 10U;
   c_Source.q_UseableForRouting = true;

   C_OscXmlParser c_Xml;
   ASSERT_EQ("bus", c_Xml.CreateAndSelectNodeChild("bus"));
   C_OscSystemBusFiler::h_SaveBus(c_Source, c_Xml);

   C_OscSystemBus c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscSystemBusFiler::h_LoadBus(c_Target, c_Xml)));

   uint32_t u32_HashSource = 0xFFFFFFFFUL;
   uint32_t u32_HashTarget = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_HashSource);
   c_Target.CalcHash(u32_HashTarget);
   EXPECT_EQ(u32_HashSource, u32_HashTarget);
   EXPECT_EQ(c_Source.q_UseCanFd, c_Target.q_UseCanFd);
   EXPECT_EQ(c_Source.q_UseableForRouting, c_Target.q_UseableForRouting);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A project survives a save/load round trip through an actual file

   File-level rather than parser-level on purpose: that is the path the paramset CRC
   bug lived in, and it is only reachable by writing and reading a real file.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, Project)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_project.syde";

   (void)std::filesystem::remove(c_Path);

   C_OscProject c_Source;
   c_Source.c_Author = "Round Tripper";
   c_Source.c_Editor = "Second Person";
   c_Source.c_Template = "a template name";
   c_Source.c_Version = "4.5.6";

   ASSERT_FALSE(static_cast<bool>(C_OscProjectFiler::h_Save(c_Source, c_Path.string(), "1.2.3")));

   C_OscProject c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscProjectFiler::h_Load(c_Target, c_Path.string())));

   EXPECT_EQ(c_Source.c_Author, c_Target.c_Author);
   EXPECT_EQ(c_Source.c_Template, c_Target.c_Template);
   EXPECT_EQ(c_Source.c_Version, c_Target.c_Version);
   //h_Save stamps the editor and the openSYDE version rather than preserving them
   EXPECT_EQ("1.2.3", c_Target.c_OpenSydeVersion);

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A data logger job survives a save/load round trip through an actual file
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, DataLoggerJob)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_datalogger.xml";

   (void)std::filesystem::remove(c_Path);

   std::vector<C_OscDataLoggerJob> c_Source;
   C_OscDataLoggerJob c_Job;
   c_Job.q_IsEnabled = true;
   c_Job.c_Properties.c_Name = "Logger One";
   c_Job.c_Properties.c_Comment = "logs things";
   c_Job.c_Properties.u32_MaxLogEntries = 4321U;
   c_Job.c_Properties.u32_MaxLogDurationSec = 600U;
   c_Job.c_Properties.u32_LogIntervalMs = 250U;
   c_Job.c_Properties.c_LogDestinationDirectory = "some/dir";
   c_Source.push_back(c_Job);

   ASSERT_FALSE(static_cast<bool>(C_OscDataLoggerJobFiler::h_SaveFile(c_Source, c_Path.string())));

   std::vector<C_OscDataLoggerJob> c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscDataLoggerJobFiler::h_LoadFile(c_Target, c_Path.string())));

   ASSERT_EQ(c_Source.size(), c_Target.size());

   uint32_t u32_HashSource = 0xFFFFFFFFUL;
   uint32_t u32_HashTarget = 0xFFFFFFFFUL;
   c_Source[0].CalcHash(u32_HashSource);
   c_Target[0].CalcHash(u32_HashTarget);
   EXPECT_EQ(u32_HashSource, u32_HashTarget);

   EXPECT_EQ(c_Source[0].q_IsEnabled, c_Target[0].q_IsEnabled);
   EXPECT_EQ(c_Source[0].c_Properties.c_Name, c_Target[0].c_Properties.c_Name);
   EXPECT_EQ(c_Source[0].c_Properties.c_Comment, c_Target[0].c_Properties.c_Comment);
   EXPECT_EQ(c_Source[0].c_Properties.u32_MaxLogEntries, c_Target[0].c_Properties.u32_MaxLogEntries);
   EXPECT_EQ(c_Source[0].c_Properties.u32_MaxLogDurationSec, c_Target[0].c_Properties.u32_MaxLogDurationSec);
   EXPECT_EQ(c_Source[0].c_Properties.u32_LogIntervalMs, c_Target[0].c_Properties.u32_LogIntervalMs);
   EXPECT_EQ(c_Source[0].c_Properties.c_LogDestinationDirectory,
             c_Target[0].c_Properties.c_LogDestinationDirectory);

   (void)std::filesystem::remove(c_Path);
}

/* -- Helpers ------------------------------------------------------------------------------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Build a datapool of the given type with every field away from its default

   Two lists, three elements, one data set. The float element carries a factor and offset
   that are not round numbers, because that is the pair the locale bug corrupted.

   The filer persists some element fields only for some pool types: diag-event-call only
   for eDIAG, nvm-start-address only for eNVM/eHALC_NVM. So the type-specific fields are
   set only where they will be written, or CalcHash would report a difference that is by
   design rather than a defect. Both branches get their own test below.
*/
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save/load a datapool through a real file and compare, localising any mismatch
*/
//----------------------------------------------------------------------------------------------------------------------
void h_RoundTripDataPool(const C_OscNodeDataPool & orc_Source, const std::string & orc_FileName)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / orc_FileName;
   (void)std::filesystem::remove(c_Path);

   ASSERT_FALSE(static_cast<bool>(C_OscNodeDataPoolFiler::h_SaveDataPoolFile(orc_Source, c_Path.string())));
   C_OscNodeDataPool c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscNodeDataPoolFiler::h_LoadDataPoolFile(c_Target, c_Path.string())));

   uint32_t u32_HashSource = 0xFFFFFFFFUL;
   uint32_t u32_HashTarget = 0xFFFFFFFFUL;
   orc_Source.CalcHash(u32_HashSource);
   c_Target.CalcHash(u32_HashTarget);
   EXPECT_EQ(u32_HashSource, u32_HashTarget) << "a field is lost that the checks below do not cover";

   ASSERT_EQ(orc_Source.c_Lists.size(), c_Target.c_Lists.size());
   for (uint32_t u32_L = 0U; u32_L < orc_Source.c_Lists.size(); ++u32_L)
   {
      ASSERT_EQ(orc_Source.c_Lists[u32_L].c_Elements.size(), c_Target.c_Lists[u32_L].c_Elements.size());
      for (uint32_t u32_E = 0U; u32_E < orc_Source.c_Lists[u32_L].c_Elements.size(); ++u32_E)
      {
         uint32_t u32_A = 0xFFFFFFFFUL;
         uint32_t u32_B = 0xFFFFFFFFUL;
         orc_Source.c_Lists[u32_L].c_Elements[u32_E].CalcHash(u32_A);
         c_Target.c_Lists[u32_L].c_Elements[u32_E].CalcHash(u32_B);
         EXPECT_EQ(u32_A, u32_B) << "element " << u32_L << "/" << u32_E << " ("
                                 << orc_Source.c_Lists[u32_L].c_Elements[u32_E].c_Name << ") differs";
      }
   }

   EXPECT_EQ(orc_Source.e_Type, c_Target.e_Type);
   EXPECT_EQ(orc_Source.c_Name, c_Target.c_Name);
   EXPECT_EQ(orc_Source.q_IsSafety, c_Target.q_IsSafety);
   EXPECT_EQ(orc_Source.u32_NvmStartAddress, c_Target.u32_NvmStartAddress);
   ASSERT_EQ(2U, c_Target.c_Lists.size());
   ASSERT_EQ(3U, c_Target.c_Lists[0].c_Elements.size());
   EXPECT_EQ(1U, c_Target.c_Lists[0].c_DataSets.size());

   const C_OscNodeDataPoolListElement & rc_F64 = c_Target.c_Lists[0].c_Elements[0];
   EXPECT_DOUBLE_EQ(0.001, rc_F64.f64_Factor);
   EXPECT_DOUBLE_EQ(-2.5, rc_F64.f64_Offset);
   EXPECT_DOUBLE_EQ(-40.25, rc_F64.c_MinValue.GetValueF64());
   EXPECT_DOUBLE_EQ(125.75, rc_F64.c_MaxValue.GetValueF64());
   ASSERT_EQ(1U, rc_F64.c_DataSetValues.size());
   EXPECT_DOUBLE_EQ(21.5, rc_F64.c_DataSetValues[0].GetValueF64());
   EXPECT_EQ("C", rc_F64.c_Unit);
   EXPECT_EQ(2U, rc_F64.c_ValueDescription.size());

   const C_OscNodeDataPoolListElement & rc_U32 = c_Target.c_Lists[0].c_Elements[1];
   EXPECT_EQ(0xFFFFFFFFU, rc_U32.c_MaxValue.GetValueU32());
   EXPECT_EQ(C_OscNodeDataPoolListElement::eACCESS_RO, rc_U32.e_Access);

   const C_OscNodeDataPoolListElement & rc_Arr = c_Target.c_Lists[0].c_Elements[2];
   EXPECT_TRUE(rc_Arr.q_InterpretAsString);
   EXPECT_TRUE(rc_Arr.c_Value.GetArray());
   EXPECT_EQ(4U, rc_Arr.c_Value.GetArraySize());

   (void)std::filesystem::remove(c_Path);
}
}

/* -- Datapool ------------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A DIAG datapool survives a save/load round trip through a real file

   The float element with a non-round factor/offset and float min/max/data-set value is
   the important one: that is the data the comma-separator locale bug silently truncated.
   DIAG exercises the diag-event-call attribute.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, NodeDataPoolDiag)
{
   h_RoundTripDataPool(h_MakeDataPool(C_OscNodeDataPool::eDIAG), "osy_rt_datapool_diag.xml");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An NVM datapool survives the same round trip -- the branch that writes NVM addresses and CRC
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, NodeDataPoolNvm)
{
   h_RoundTripDataPool(h_MakeDataPool(C_OscNodeDataPool::eNVM), "osy_rt_datapool_nvm.xml");
}

/* -- Small file-pair filers ---------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An X-config manifest survives a save/load round trip through a real file
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, XcoManifest)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_xco_manifest.xml";
   (void)std::filesystem::remove(c_Path);

   C_OscXcoManifest c_Source;
   c_Source.c_NodeName = "Node With Spaces & Ampersand";
   ASSERT_FALSE(static_cast<bool>(C_OscXcoManifestFiler::h_SaveFile(c_Source, c_Path.string())));

   C_OscXcoManifest c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscXcoManifestFiler::h_LoadFile(c_Target, c_Path.string())));
   EXPECT_EQ(c_Source.c_NodeName, c_Target.c_NodeName);

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   X-app properties survive a save/load round trip through a real file
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, XappProperties)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_xapp_properties.xml";
   (void)std::filesystem::remove(c_Path);

   C_OscXappProperties c_Source;
   c_Source.u32_PollingIntervalMs = 250U;
   c_Source.u32_DataRequestIntervalMs = 1500U;
   c_Source.u8_ConnectedInterfaceNumber = 3U;
   ASSERT_FALSE(static_cast<bool>(C_OscXappPropertiesFiler::h_SaveXappPropertiesFile(c_Source, c_Path.string())));

   C_OscXappProperties c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscXappPropertiesFiler::h_LoadXappPropertiesFile(c_Target, c_Path.string())));

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B);
   EXPECT_EQ(250U, c_Target.u32_PollingIntervalMs);
   EXPECT_EQ(1500U, c_Target.u32_DataRequestIntervalMs);
   EXPECT_EQ(3U, c_Target.u8_ConnectedInterfaceNumber);

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An X-certificates manifest survives a save/load round trip through a real file
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, XceManifest)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_xce_manifest.xml";
   (void)std::filesystem::remove(c_Path);

   C_OscXceManifest c_Source;
   c_Source.c_CertificatesPath = "certs/sub dir";
   C_OscXceUpdatePackageParameters c_Param;
   c_Param.c_Password = "p@ss w0rd <&>";
   c_Param.c_AuthenticationKeyPath = "keys/auth.pem";
   c_Source.c_UpdatePackageParameters.push_back(c_Param);
   c_Param.c_Password = "";
   c_Param.c_AuthenticationKeyPath = "keys/second.pem";
   c_Source.c_UpdatePackageParameters.push_back(c_Param);
   ASSERT_FALSE(static_cast<bool>(C_OscXceManifestFiler::h_SaveFile(c_Source, c_Path.string())));

   C_OscXceManifest c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscXceManifestFiler::h_LoadFile(c_Target, c_Path.string())));
   EXPECT_EQ(c_Source.c_CertificatesPath, c_Target.c_CertificatesPath);
   ASSERT_EQ(2U, c_Target.c_UpdatePackageParameters.size());
   EXPECT_EQ("p@ss w0rd <&>", c_Target.c_UpdatePackageParameters[0].c_Password);
   EXPECT_EQ("keys/auth.pem", c_Target.c_UpdatePackageParameters[0].c_AuthenticationKeyPath);
   EXPECT_EQ("", c_Target.c_UpdatePackageParameters[1].c_Password);
   EXPECT_EQ("keys/second.pem", c_Target.c_UpdatePackageParameters[1].c_AuthenticationKeyPath);

   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A device definition survives a save/load round trip through a real file

   No CalcHash on this class, so every scalar and the numeric vectors are compared by hand.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, DeviceDefinition)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / "osy_rt_device.syde_devdef";
   (void)std::filesystem::remove(c_Path);

   //the loader resolves image, toolbox icon and company logo against the definition's directory
   //with a realpath-style expansion that yields "" for anything not on disk -- so give it real
   //files, and expect the canonical absolute path back
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path();
   for (const char * const pcn_Name : {"osy_rt_dev_image.png", "osy_rt_dev_icon.svg", "osy_rt_dev_logo.png"})
   {
      std::ofstream c_Touch(c_Dir / pcn_Name, std::ios::binary);
      c_Touch << "x";
   }
   //the loader expands the paths against the file's directory; on Windows that can come back in 8.3 form
   //(RUNNER~1 for runneradmin), so compare by identity rather than by spelling
   const auto h_SameFile = [&c_Dir](const char * const pcn_Name, const std::string & orc_Loaded) -> bool
   {
      std::error_code c_Err;
      return std::filesystem::equivalent(c_Dir / pcn_Name, std::filesystem::path(orc_Loaded), c_Err);
   };

   C_OscDeviceDefinition c_Source;
   c_Source.c_DeviceName = "RT-Device 1";
   c_Source.c_DeviceNameAlias = "rtdev";
   c_Source.c_DeviceDescription = "a device with <markup> & symbols";
   c_Source.c_ImagePath = "osy_rt_dev_image.png";
   c_Source.u8_NumCanBusses = 3U;
   c_Source.u8_NumEthernetBusses = 1U;
   c_Source.c_SupportedBitrates = {125U, 250U, 500U, 1000U};
   c_Source.c_SupportedCanFdDataBitrates = {2000U, 5000U};
   c_Source.u8_ManufacturerId = 7U;
   c_Source.c_ManufacturerDisplayValue = "ACME";
   c_Source.c_CompanyLogoLink = "osy_rt_dev_logo.png";
   c_Source.c_ProductPageLink = "https://example.invalid/product";
   c_Source.c_ToolboxIcon = "osy_rt_dev_icon.svg";

   //the loader requires at least one sub-device, as every real definition has -- the device itself
   C_OscSubDeviceDefinition c_Sub;
   c_Sub.c_SubDeviceName = "RT-Device 1";
   c_Sub.c_OtherAcceptedNames = {"RTD1", "rt-device-one"};
   c_Sub.q_ProgrammingSupport = true;
   c_Sub.q_DiagnosticProtocolOpenSydeCan = true;
   c_Sub.q_DiagnosticProtocolOpenSydeEthernet = false;
   c_Sub.q_FlashloaderOpenSydeCan = true;
   c_Sub.q_FlashloaderOpenSydeEthernet = true;
   c_Sub.q_FlashloaderOpenSydeIsFileBased = true;
   c_Sub.u32_FlashloaderResetWaitTimeNoChangesCan = 1100U;
   c_Sub.u32_FlashloaderResetWaitTimeNoChangesEthernet = 2200U;
   c_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesCan = 3300U;
   c_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet = 4400U;
   c_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesCan = 5500U;
   c_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesEthernet = 6600U;
   c_Sub.u32_FlashloaderOpenSydeRequestDownloadTimeout = 7700U;
   c_Sub.u32_FlashloaderOpenSydeTransferDataTimeout = 8800U;
   c_Sub.u32_UserEepromSizeBytes = 65536U;
   //with a single sub-device the loader auto-fills every interface implied by the bus counts as
   //connected (mh_HandleConnectedInterfaces); saved flags only matter with two or more sub-devices
   c_Sub.c_ConnectedInterfaces = {{"can1", true}, {"can2", true}, {"can3", true}, {"eth1", true}};
   c_Source.c_SubDevices.push_back(c_Sub);
   ASSERT_FALSE(static_cast<bool>(C_OscDeviceDefinitionFiler::h_Save(c_Source, c_Path.string())));

   C_OscDeviceDefinition c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscDeviceDefinitionFiler::h_Load(c_Target, c_Path.string())));
   EXPECT_EQ(c_Source.c_DeviceName, c_Target.c_DeviceName);
   EXPECT_EQ(c_Source.c_DeviceNameAlias, c_Target.c_DeviceNameAlias);
   EXPECT_EQ(c_Source.c_DeviceDescription, c_Target.c_DeviceDescription);
   EXPECT_TRUE(h_SameFile("osy_rt_dev_image.png", c_Target.c_ImagePath)) << c_Target.c_ImagePath;
   EXPECT_EQ(c_Source.u8_NumCanBusses, c_Target.u8_NumCanBusses);
   EXPECT_EQ(c_Source.u8_NumEthernetBusses, c_Target.u8_NumEthernetBusses);
   EXPECT_EQ(c_Source.c_SupportedBitrates, c_Target.c_SupportedBitrates);
   EXPECT_EQ(c_Source.c_SupportedCanFdDataBitrates, c_Target.c_SupportedCanFdDataBitrates);
   EXPECT_EQ(c_Source.u8_ManufacturerId, c_Target.u8_ManufacturerId);
   EXPECT_EQ(c_Source.c_ManufacturerDisplayValue, c_Target.c_ManufacturerDisplayValue);
   EXPECT_TRUE(h_SameFile("osy_rt_dev_logo.png", c_Target.c_CompanyLogoLink)) << c_Target.c_CompanyLogoLink;
   EXPECT_EQ(c_Source.c_ProductPageLink, c_Target.c_ProductPageLink);
   EXPECT_TRUE(h_SameFile("osy_rt_dev_icon.svg", c_Target.c_ToolboxIcon)) << c_Target.c_ToolboxIcon;
   ASSERT_EQ(1U, c_Target.c_SubDevices.size());
   const C_OscSubDeviceDefinition & rc_Sub = c_Target.c_SubDevices[0];
   EXPECT_EQ(c_Sub.c_SubDeviceName, rc_Sub.c_SubDeviceName);
   EXPECT_EQ(c_Sub.c_OtherAcceptedNames, rc_Sub.c_OtherAcceptedNames);
   EXPECT_EQ(c_Sub.q_ProgrammingSupport, rc_Sub.q_ProgrammingSupport);
   EXPECT_EQ(c_Sub.q_DiagnosticProtocolOpenSydeCan, rc_Sub.q_DiagnosticProtocolOpenSydeCan);
   EXPECT_EQ(c_Sub.q_DiagnosticProtocolOpenSydeEthernet, rc_Sub.q_DiagnosticProtocolOpenSydeEthernet);
   EXPECT_EQ(c_Sub.q_FlashloaderOpenSydeCan, rc_Sub.q_FlashloaderOpenSydeCan);
   EXPECT_EQ(c_Sub.q_FlashloaderOpenSydeEthernet, rc_Sub.q_FlashloaderOpenSydeEthernet);
   EXPECT_EQ(c_Sub.q_FlashloaderOpenSydeIsFileBased, rc_Sub.q_FlashloaderOpenSydeIsFileBased);
   EXPECT_EQ(1100U, rc_Sub.u32_FlashloaderResetWaitTimeNoChangesCan);
   EXPECT_EQ(2200U, rc_Sub.u32_FlashloaderResetWaitTimeNoChangesEthernet);
   EXPECT_EQ(3300U, rc_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesCan);
   EXPECT_EQ(4400U, rc_Sub.u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet);
   EXPECT_EQ(5500U, rc_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesCan);
   EXPECT_EQ(6600U, rc_Sub.u32_FlashloaderResetWaitTimeFundamentalChangesEthernet);
   EXPECT_EQ(7700U, rc_Sub.u32_FlashloaderOpenSydeRequestDownloadTimeout);
   EXPECT_EQ(8800U, rc_Sub.u32_FlashloaderOpenSydeTransferDataTimeout);
   EXPECT_EQ(65536U, rc_Sub.u32_UserEepromSizeBytes);
   EXPECT_EQ(c_Sub.c_ConnectedInterfaces, rc_Sub.c_ConnectedInterfaces);

   (void)std::filesystem::remove(c_Path);
   for (const char * const pcn_Name : {"osy_rt_dev_image.png", "osy_rt_dev_icon.svg", "osy_rt_dev_logo.png"})
   {
      (void)std::filesystem::remove(c_Dir / pcn_Name);
   }
}


/* -- CAN communication protocol ------------------------------------------------------------------------------------ */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Build a protocol of the given type with messages and signals, setting only what that type persists

   The message and signal filers write the CANopen-manager fields only for eCAN_OPEN and the
   J1939 part only for eJ1939, so a source that sets both would hash differently from what
   comes back for reasons that are by design. One protocol per type below.
*/
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save/load a protocol through a real file and compare
*/
//----------------------------------------------------------------------------------------------------------------------
void h_RoundTripProtocol(const C_OscCanProtocol & orc_Source, const std::string & orc_FileName)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / orc_FileName;
   (void)std::filesystem::remove(c_Path);

   ASSERT_FALSE(static_cast<bool>(C_OscNodeCommFiler::h_SaveNodeComProtocolFile(orc_Source, c_Path.string(), "ComPool")));

   //the loader resolves the saved datapool name against these
   std::vector<C_OscNodeDataPool> c_Pools(1);
   c_Pools[0].c_Name = "ComPool";
   c_Pools[0].e_Type = C_OscNodeDataPool::eCOM;

   C_OscCanProtocol c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscNodeCommFiler::h_LoadNodeComProtocolFile(c_Target, c_Path.string(), c_Pools)));

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   orc_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";

   EXPECT_EQ(orc_Source.e_Type, c_Target.e_Type);
   EXPECT_EQ(0U, c_Target.u32_DataPoolIndex);
   ASSERT_EQ(1U, c_Target.c_ComMessages.size());
   const C_OscCanMessageContainer & rc_C = c_Target.c_ComMessages[0];
   EXPECT_TRUE(rc_C.q_IsComProtocolUsedByInterface);
   ASSERT_EQ(1U, rc_C.c_TxMessages.size());
   ASSERT_EQ(1U, rc_C.c_RxMessages.size());
   const C_OscCanMessage & rc_Tx = rc_C.c_TxMessages[0];
   const C_OscCanMessage & rc_SrcTx = orc_Source.c_ComMessages[0].c_TxMessages[0];
   EXPECT_EQ(rc_SrcTx.c_Name, rc_Tx.c_Name);
   EXPECT_EQ(rc_SrcTx.u32_CanId, rc_Tx.u32_CanId);
   EXPECT_EQ(rc_SrcTx.q_IsExtended, rc_Tx.q_IsExtended);
   EXPECT_EQ(rc_SrcTx.u16_Dlc, rc_Tx.u16_Dlc);
   EXPECT_EQ(rc_SrcTx.e_TxMethod, rc_Tx.e_TxMethod);
   EXPECT_EQ(250U, rc_Tx.u32_CycleTimeMs);
   EXPECT_EQ(15U, rc_Tx.u16_DelayTimeMs);
   EXPECT_EQ(1250U, rc_Tx.u32_TimeoutMs);
   EXPECT_EQ(rc_SrcTx.c_CanOpenManagerOwnerNodeIndex.u32_NodeIndex, rc_Tx.c_CanOpenManagerOwnerNodeIndex.u32_NodeIndex);
   EXPECT_EQ(rc_SrcTx.u32_CanOpenManagerCobIdOffset, rc_Tx.u32_CanOpenManagerCobIdOffset);
   ASSERT_EQ(2U, rc_Tx.c_Signals.size());
   EXPECT_EQ(C_OscCanSignal::eBYTE_ORDER_INTEL, rc_Tx.c_Signals[0].e_ComByteOrder);
   EXPECT_EQ(C_OscCanSignal::eMUX_MULTIPLEXER_SIGNAL, rc_Tx.c_Signals[0].e_MultiplexerType);
   EXPECT_EQ(C_OscCanSignal::eBYTE_ORDER_MOTOROLA, rc_Tx.c_Signals[1].e_ComByteOrder);
   EXPECT_EQ(12U, rc_Tx.c_Signals[1].u16_ComBitStart);
   EXPECT_EQ(16U, rc_Tx.c_Signals[1].u16_ComBitLength);
   EXPECT_EQ(7U, rc_Tx.c_Signals[1].u16_MultiplexValue);
   EXPECT_EQ(rc_SrcTx.c_Signals[1].u16_CanOpenManagerObjectDictionaryIndex, rc_Tx.c_Signals[1].u16_CanOpenManagerObjectDictionaryIndex);
   EXPECT_EQ(rc_SrcTx.c_Signals[1].u32_J1939SuspectParameterNumber, rc_Tx.c_Signals[1].u32_J1939SuspectParameterNumber);
   EXPECT_EQ("Command", rc_C.c_RxMessages[0].c_Name);

   (void)std::filesystem::remove(c_Path);
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A J1939 protocol survives a save/load round trip -- the branch that writes the SPN
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, NodeComProtocolJ1939)
{
   h_RoundTripProtocol(h_MakeProtocol(C_OscCanProtocol::eJ1939), "osy_rt_com_j1939.xml");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A CANopen protocol survives the same round trip -- the branch that writes the manager fields
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, NodeComProtocolCanOpen)
{
   h_RoundTripProtocol(h_MakeProtocol(C_OscCanProtocol::eCAN_OPEN), "osy_rt_com_canopen.xml");
}

/* -- Node squads --------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Node squads (multi-CPU devices) survive a parser-level save/load round trip

   h_SaveNodeGroups creates "node-groups" under the selected node and h_LoadNodeGroups
   selects it from the same position, so the parser is returned to the root between the two.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, NodeSquads)
{
   std::vector<C_OscNodeSquad> c_Source(2);
   c_Source[0].c_BaseName = "Gateway";
   c_Source[0].c_SubNodeIndexes = {1U, 3U, 7U};
   c_Source[1].c_BaseName = "Sensor Cluster";
   c_Source[1].c_SubNodeIndexes = {12U};

   C_OscXmlParser c_Xml;
   ASSERT_EQ("root", c_Xml.CreateAndSelectNodeChild("root"));
   C_OscNodeSquadFiler::h_SaveNodeGroups(c_Source, c_Xml);

   ASSERT_EQ("root", c_Xml.SelectRoot());
   std::vector<C_OscNodeSquad> c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscNodeSquadFiler::h_LoadNodeGroups(c_Target, c_Xml)));

   ASSERT_EQ(2U, c_Target.size());
   EXPECT_EQ("Gateway", c_Target[0].c_BaseName);
   EXPECT_EQ((std::vector<uint32_t>{1U, 3U, 7U}), c_Target[0].c_SubNodeIndexes);
   EXPECT_EQ("Sensor Cluster", c_Target[1].c_BaseName);
   EXPECT_EQ((std::vector<uint32_t>{12U}), c_Target[1].c_SubNodeIndexes);
}

/* -- Parameter sets ------------------------------------------------------------------------------------------------ */

namespace
{
C_OscParamSetDataPoolInfo h_MakeDataPoolInfo(void)
{
   C_OscParamSetDataPoolInfo c_Info;
   c_Info.c_Name = "NvmPool";
   c_Info.u32_DataPoolCrc = 0xDEADBEEFU;
   c_Info.u32_NvmStartAddress = 0x2000U;
   c_Info.u32_NvmSize = 512U;
   c_Info.au8_Version[0] = 1U;
   c_Info.au8_Version[1] = 7U;
   c_Info.au8_Version[2] = 3U;
   return c_Info;
}

void h_ExpectDataPoolInfo(const C_OscParamSetDataPoolInfo & orc_Info)
{
   EXPECT_EQ("NvmPool", orc_Info.c_Name);
   EXPECT_EQ(0xDEADBEEFU, orc_Info.u32_DataPoolCrc);
   EXPECT_EQ(0x2000U, orc_Info.u32_NvmStartAddress);
   EXPECT_EQ(512U, orc_Info.u32_NvmSize);
   EXPECT_EQ(1U, orc_Info.au8_Version[0]);
   EXPECT_EQ(7U, orc_Info.au8_Version[1]);
   EXPECT_EQ(3U, orc_Info.au8_Version[2]);
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   An interpreted parameter-set node survives a parser-level save/load round trip

   Typed NVM values are the point here: a float64 and a uint16 array go through the same
   content serialisation the datapool filer uses.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, ParamSetInterpretedNode)
{
   C_OscParamSetInterpretedNode c_Source;
   c_Source.c_Name = "Node A";

   C_OscParamSetInterpretedDataPool c_Pool;
   c_Pool.c_DataPoolInfo = h_MakeDataPoolInfo();
   C_OscParamSetInterpretedList c_List;
   c_List.c_Name = "Calibration";

   C_OscParamSetInterpretedElement c_F64;
   c_F64.c_Name = "Gain";
   c_F64.c_NvmValue.SetType(C_OscNodeDataPoolContent::eFLOAT64);
   c_F64.c_NvmValue.SetValueF64(1.0625);
   c_List.c_Elements.push_back(c_F64);

   C_OscParamSetInterpretedElement c_Arr;
   c_Arr.c_Name = "Table";
   c_Arr.c_NvmValue.SetType(C_OscNodeDataPoolContent::eUINT16);
   c_Arr.c_NvmValue.SetArray(true);
   c_Arr.c_NvmValue.SetArraySize(3U);
   c_Arr.c_NvmValue.SetValueArrU16Element(100U, 0U);
   c_Arr.c_NvmValue.SetValueArrU16Element(200U, 1U);
   c_Arr.c_NvmValue.SetValueArrU16Element(65535U, 2U);
   c_List.c_Elements.push_back(c_Arr);

   c_Pool.c_Lists.push_back(c_List);
   c_Source.c_DataPools.push_back(c_Pool);

   C_OscXmlParser c_Xml;
   ASSERT_EQ("node", c_Xml.CreateAndSelectNodeChild("node"));
   C_OscParamSetInterpretedNodeFiler::h_SaveInterpretedNode(c_Source, c_Xml);

   ASSERT_EQ("node", c_Xml.SelectRoot());
   C_OscParamSetInterpretedNode c_Target;
   bool q_MissingOptional = false;
   ASSERT_FALSE(static_cast<bool>(C_OscParamSetInterpretedNodeFiler::h_LoadInterpretedNode(c_Target, c_Xml, q_MissingOptional)));

   EXPECT_EQ("Node A", c_Target.c_Name);
   ASSERT_EQ(1U, c_Target.c_DataPools.size());
   h_ExpectDataPoolInfo(c_Target.c_DataPools[0].c_DataPoolInfo);
   ASSERT_EQ(1U, c_Target.c_DataPools[0].c_Lists.size());
   const C_OscParamSetInterpretedList & rc_List = c_Target.c_DataPools[0].c_Lists[0];
   EXPECT_EQ("Calibration", rc_List.c_Name);
   ASSERT_EQ(2U, rc_List.c_Elements.size());
   EXPECT_EQ("Gain", rc_List.c_Elements[0].c_Name);
   EXPECT_EQ(C_OscNodeDataPoolContent::eFLOAT64, rc_List.c_Elements[0].c_NvmValue.GetType());
   EXPECT_DOUBLE_EQ(1.0625, rc_List.c_Elements[0].c_NvmValue.GetValueF64());
   EXPECT_EQ("Table", rc_List.c_Elements[1].c_Name);
   EXPECT_TRUE(rc_List.c_Elements[1].c_NvmValue.GetArray());
   ASSERT_EQ(3U, rc_List.c_Elements[1].c_NvmValue.GetArraySize());
   EXPECT_EQ(100U, rc_List.c_Elements[1].c_NvmValue.GetValueArrU16Element(0U));
   EXPECT_EQ(200U, rc_List.c_Elements[1].c_NvmValue.GetValueArrU16Element(1U));
   EXPECT_EQ(65535U, rc_List.c_Elements[1].c_NvmValue.GetValueArrU16Element(2U));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A raw parameter-set node survives a parser-level save/load round trip
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, ParamSetRawNode)
{
   C_OscParamSetRawNode c_Source;
   c_Source.c_Name = "Node B";
   c_Source.c_DataPools.push_back(h_MakeDataPoolInfo());
   C_OscParamSetRawEntry c_Entry;
   c_Entry.u32_StartAddress = 0x2000U;
   c_Entry.c_Bytes = {0x00U, 0xFFU, 0x7FU, 0x80U, 0x01U};
   c_Source.c_Entries.push_back(c_Entry);
   c_Entry.u32_StartAddress = 0x2010U;
   c_Entry.c_Bytes = {0xAAU};
   c_Source.c_Entries.push_back(c_Entry);

   C_OscXmlParser c_Xml;
   ASSERT_EQ("node", c_Xml.CreateAndSelectNodeChild("node"));
   C_OscParamSetRawNodeFiler::h_SaveRawNode(c_Source, c_Xml);

   ASSERT_EQ("node", c_Xml.SelectRoot());
   C_OscParamSetRawNode c_Target;
   bool q_MissingOptional = false;
   ASSERT_FALSE(static_cast<bool>(C_OscParamSetRawNodeFiler::h_LoadRawNode(c_Target, c_Xml, q_MissingOptional)));

   EXPECT_EQ("Node B", c_Target.c_Name);
   ASSERT_EQ(1U, c_Target.c_DataPools.size());
   h_ExpectDataPoolInfo(c_Target.c_DataPools[0]);
   ASSERT_EQ(2U, c_Target.c_Entries.size());
   EXPECT_EQ(0x2000U, c_Target.c_Entries[0].u32_StartAddress);
   EXPECT_EQ((std::vector<uint8_t>{0x00U, 0xFFU, 0x7FU, 0x80U, 0x01U}), c_Target.c_Entries[0].c_Bytes);
   EXPECT_EQ(0x2010U, c_Target.c_Entries[1].u32_StartAddress);
   EXPECT_EQ((std::vector<uint8_t>{0xAAU}), c_Target.c_Entries[1].c_Bytes);
}

/* -- System view (core part) --------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The core part of a system view survives a parser-level save/load round trip

   Core has no whole-view saver -- the GUI composes the "name" node with the three core
   savers below -- so this test composes the same layout by hand and loads it with the core
   loader. The loader sizes the per-node update information from the node list it is given,
   so three placeholder nodes accompany three active-flag entries.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, SystemViewCore)
{
   const std::vector<C_OscNode> c_Nodes(3);

   C_OscViewData c_Source;
   c_Source.SetName("Commissioning View");
   c_Source.SetNodeActiveFlags({1U, 0U, 1U});
   c_Source.SetPcConnected(true, 2U);

   std::vector<C_OscViewNodeUpdate> c_Updates(3);
   c_Updates[0].u32_NodeUpdatePosition = 2U;
   c_Updates[0].SetPaths({"fw/app_a.hex", "fw/app_b.hex"}, C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
   c_Updates[0].SetSkipUpdateOfPathsFlags({false, true}, C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
   c_Updates[0].SetPemFilePath("keys/node0.pem");
   C_OscViewNodeUpdateParamInfo c_Param;
   c_Param.SetContent("params/set1.syde_psi", 0x12345678U);
   c_Updates[0].AddParamInfo(c_Param);
   c_Updates[0].SetSkipUpdateOfParamInfosFlags({false});
   c_Updates[2].u32_NodeUpdatePosition = 0U;
   c_Updates[2].SetPaths({"fs/image.bin"}, C_OscViewNodeUpdate::eFTP_FILE_BASED);
   c_Updates[2].SetSkipUpdateOfPathsFlags({false}, C_OscViewNodeUpdate::eFTP_FILE_BASED);
   c_Source.SetNodeUpdateInformation(c_Updates);

   C_OscXmlParser c_Xml;
   ASSERT_EQ("opensyde-system-view", c_Xml.CreateAndSelectNodeChild("opensyde-system-view"));
   c_Xml.CreateNodeChild("name", c_Source.GetName());
   C_OscViewFiler::h_SaveNodeActiveFlags(c_Source.GetNodeActiveFlags(), c_Xml);
   C_OscViewFiler::h_SaveNodeUpdateInformation(c_Source.GetAllNodeUpdateInformation(), c_Xml);
   //h_SavePc writes attributes onto whatever node is selected; the GUI creates "pc" around it exactly like this
   ASSERT_EQ("pc", c_Xml.CreateAndSelectNodeChild("pc"));
   C_OscViewFiler::h_SavePc(c_Source.GetOscPcData(), c_Xml);
   ASSERT_EQ("opensyde-system-view", c_Xml.SelectNodeParent());

   ASSERT_EQ("opensyde-system-view", c_Xml.SelectRoot());
   C_OscViewData c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_LoadViewOsc(c_Target, c_Xml, c_Nodes)));

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";

   EXPECT_EQ("Commissioning View", c_Target.GetName());
   EXPECT_EQ((std::vector<uint8_t>{1U, 0U, 1U}), c_Target.GetNodeActiveFlags());
   EXPECT_TRUE(c_Target.GetOscPcData().GetConnected());
   EXPECT_EQ(2U, c_Target.GetOscPcData().GetBusIndex());
   ASSERT_EQ(3U, c_Target.GetAllNodeUpdateInformation().size());
   const C_OscViewNodeUpdate & rc_U0 = c_Target.GetAllNodeUpdateInformation()[0];
   EXPECT_EQ(2U, rc_U0.u32_NodeUpdatePosition);
   EXPECT_EQ((std::vector<std::string>{"fw/app_a.hex", "fw/app_b.hex"}), rc_U0.GetPaths(C_OscViewNodeUpdate::eFTP_DATA_BLOCK));
   EXPECT_EQ((std::vector<bool>{false, true}), rc_U0.GetSkipUpdateOfPathsFlags(C_OscViewNodeUpdate::eFTP_DATA_BLOCK));
   EXPECT_EQ("keys/node0.pem", rc_U0.GetPemFilePath());
   ASSERT_EQ(1U, rc_U0.GetParamInfos().size());
   EXPECT_EQ("params/set1.syde_psi", rc_U0.GetParamInfos()[0].GetPath());
   EXPECT_EQ(0x12345678U, rc_U0.GetParamInfos()[0].GetLastKnownCrc());
   EXPECT_EQ((std::vector<std::string>{"fs/image.bin"}), c_Target.GetAllNodeUpdateInformation()[2].GetPaths(C_OscViewNodeUpdate::eFTP_FILE_BASED));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   System definition file round-trip, which is also the node file round-trip

   h_SaveSystemDefinitionFile writes every node into its own folder through C_OscNodeFiler::h_SaveNodeFile, and
   the sub-filers with a base path (datapools, comm protocols) each get a file of their own, so one save/load of a
   two-node system exercises the whole file-level tree. Loaded without device definitions, the way the CLI tools
   that only need the topology do it.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, SystemDefinitionFile)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_rt_sysdef";
   const std::filesystem::path c_Path = c_Dir / "system_definition.syde_sysdef";

   (void)std::filesystem::remove_all(c_Dir);

   C_OscSystemDefinition c_Source;
   c_Source.u32_NameMaxCharLimit = 42U;
   {
      C_OscSystemBus c_Can;
      c_Can.c_Name = "Drive CAN";
      c_Can.c_Comment = "first bus";
      c_Can.e_Type = C_OscSystemBus::eCAN;
      c_Can.u64_BitRate = 500000ULL;
      c_Can.u8_BusId = 0U;
      c_Can.q_UseableForRouting = true;
      c_Source.c_Buses.push_back(c_Can);
      C_OscSystemBus c_Eth;
      c_Eth.c_Name = "Service ETH";
      c_Eth.e_Type = C_OscSystemBus::eETHERNET;
      c_Eth.u8_BusId = 1U;
      c_Source.c_Buses.push_back(c_Eth);
   }
   c_Source.c_Nodes.push_back(h_MakeNode("NodeA", 7U, 0U));
   c_Source.c_Nodes.push_back(h_MakeNode("NodeB", 9U, 0U));
   {
      C_OscNodeSquad c_Squad;
      c_Squad.c_BaseName = "Pair";
      c_Squad.c_SubNodeIndexes.push_back(0U);
      c_Squad.c_SubNodeIndexes.push_back(1U);
      c_Source.c_NodeSquads.push_back(c_Squad);
   }

   std::vector<std::string> c_Created;
   ASSERT_FALSE(static_cast<bool>(C_OscSystemDefinitionFiler::h_SaveSystemDefinitionFile(c_Source, c_Path.string(),
                                                                                         &c_Created)));
   //every node folder and every sub-file is reported back, relative to the system definition
   EXPECT_TRUE(std::filesystem::exists(c_Dir / C_OscNodeFiler::h_GetFolderName("NodeA") /
                                       C_OscNodeFiler::h_GetFileName())) << "node folder layout changed";
   for (const std::string & rc_File : c_Created)
   {
      EXPECT_TRUE(std::filesystem::exists(c_Dir / rc_File)) << "created-files list names \"" << rc_File <<
         "\" which was not written";
   }

   C_OscSystemDefinition c_Target;
   uint16_t u16_Version = 0U;
   ASSERT_FALSE(static_cast<bool>(C_OscSystemDefinitionFiler::h_LoadSystemDefinitionFile(c_Target, c_Path.string(),
                                                                                         "", false, &u16_Version,
                                                                                         nullptr, false, nullptr,
                                                                                         nullptr)));

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";
   ASSERT_EQ(c_Source.c_Nodes.size(), c_Target.c_Nodes.size());
   for (uint32_t u32_It = 0U; u32_It < c_Source.c_Nodes.size(); ++u32_It)
   {
      const C_OscNode & rc_S = c_Source.c_Nodes[u32_It];
      const C_OscNode & rc_T = c_Target.c_Nodes[u32_It];
      const std::string c_N = "node " + std::to_string(u32_It) + " ";
      h_ExpectSameHash(rc_S.c_Properties, rc_T.c_Properties, c_N + "properties");
      ASSERT_EQ(rc_S.c_Applications.size(), rc_T.c_Applications.size());
      for (uint32_t u32_A2 = 0U; u32_A2 < rc_S.c_Applications.size(); ++u32_A2)
      {
         h_ExpectSameHash(rc_S.c_Applications[u32_A2], rc_T.c_Applications[u32_A2],
                          c_N + "application " + std::to_string(u32_A2));
      }
      ASSERT_EQ(rc_S.c_DataPools.size(), rc_T.c_DataPools.size());
      for (uint32_t u32_D = 0U; u32_D < rc_S.c_DataPools.size(); ++u32_D)
      {
         h_ExpectSameHash(rc_S.c_DataPools[u32_D], rc_T.c_DataPools[u32_D], c_N + "datapool " + std::to_string(u32_D));
      }
      ASSERT_EQ(rc_S.c_ComProtocols.size(), rc_T.c_ComProtocols.size());
      for (uint32_t u32_P = 0U; u32_P < rc_S.c_ComProtocols.size(); ++u32_P)
      {
         h_ExpectSameHash(rc_S.c_ComProtocols[u32_P], rc_T.c_ComProtocols[u32_P],
                          c_N + "protocol " + std::to_string(u32_P));
      }
      h_ExpectSameHash(rc_S.c_HalcConfig, rc_T.c_HalcConfig, c_N + "halc");
      h_ExpectSameHash(rc_S.c_XappProperties, rc_T.c_XappProperties, c_N + "x-app properties");
      EXPECT_EQ(rc_S.c_CanOpenManagers.size(), rc_T.c_CanOpenManagers.size()) << c_N << "canopen managers";
      EXPECT_EQ(rc_S.c_DataLoggerJobs.size(), rc_T.c_DataLoggerJobs.size()) << c_N << "data logger jobs";
   }
   ASSERT_EQ(c_Source.c_Buses.size(), c_Target.c_Buses.size());
   for (uint32_t u32_It = 0U; u32_It < c_Source.c_Buses.size(); ++u32_It)
   {
      h_ExpectSameHash(c_Source.c_Buses[u32_It], c_Target.c_Buses[u32_It], "bus " + std::to_string(u32_It));
   }

   //the cast keeps the in-class constant from being odr-used (gtest takes its arguments by reference)
   EXPECT_EQ(static_cast<uint16_t>(C_OscSystemDefinitionFiler::hu16_FILE_VERSION_LATEST), u16_Version);
   EXPECT_EQ(42U, c_Target.u32_NameMaxCharLimit);
   ASSERT_EQ(2U, c_Target.c_Buses.size());
   EXPECT_EQ("Service ETH", c_Target.c_Buses[1].c_Name);
   ASSERT_EQ(2U, c_Target.c_Nodes.size());
   const C_OscNode & rc_Node = c_Target.c_Nodes[1];
   EXPECT_EQ("NodeB", rc_Node.c_Properties.c_Name);
   EXPECT_EQ("RoundTrip Controller", rc_Node.c_DeviceType);
   EXPECT_EQ(nullptr, rc_Node.pc_DeviceDefinition) << "loaded without device definitions";
   ASSERT_EQ(2U, rc_Node.c_Properties.c_ComInterfaces.size());
   for (uint32_t u32_If = 0U; u32_If < 2U; ++u32_If)
   {
      const C_OscNodeComInterfaceSettings & rc_S = c_Source.c_Nodes[1].c_Properties.c_ComInterfaces[u32_If];
      const C_OscNodeComInterfaceSettings & rc_T = rc_Node.c_Properties.c_ComInterfaces[u32_If];
      const std::string c_W = "interface " + std::to_string(u32_If) + " ";
      EXPECT_EQ(rc_S.e_InterfaceType, rc_T.e_InterfaceType) << c_W << "type";
      EXPECT_EQ(rc_S.u8_InterfaceNumber, rc_T.u8_InterfaceNumber) << c_W << "number";
      EXPECT_EQ(rc_S.u8_NodeId, rc_T.u8_NodeId) << c_W << "node id";
      EXPECT_EQ(rc_S.q_IsUpdateEnabled, rc_T.q_IsUpdateEnabled) << c_W << "update flag";
      EXPECT_EQ(rc_S.q_IsRoutingEnabled, rc_T.q_IsRoutingEnabled) << c_W << "routing flag";
      EXPECT_EQ(rc_S.q_IsDiagnosisEnabled, rc_T.q_IsDiagnosisEnabled) << c_W << "diagnosis flag";
      EXPECT_EQ(rc_S.GetBusConnectedRawValue(), rc_T.GetBusConnectedRawValue()) << c_W << "bus connected";
      EXPECT_EQ(rc_S.u32_BusIndex, rc_T.u32_BusIndex) << c_W << "bus index";
      EXPECT_EQ(rc_S.e_DbcProtocol, rc_T.e_DbcProtocol) << c_W << "dbc protocol";
      EXPECT_EQ(rc_S.c_LastSyncedDbcSha256, rc_T.c_LastSyncedDbcSha256) << c_W << "dbc sha";
      EXPECT_EQ(rc_S.c_LastSyncedProjectMsgHash, rc_T.c_LastSyncedProjectMsgHash) << c_W << "dbc msg hash";
      for (uint32_t u32_B = 0U; u32_B < 4U; ++u32_B)
      {
         EXPECT_EQ(rc_S.c_Ip.au8_IpAddress[u32_B], rc_T.c_Ip.au8_IpAddress[u32_B]) << c_W << "ip byte " << u32_B;
         EXPECT_EQ(rc_S.c_Ip.au8_NetMask[u32_B], rc_T.c_Ip.au8_NetMask[u32_B]) << c_W << "mask byte " << u32_B;
         EXPECT_EQ(rc_S.c_Ip.au8_DefaultGateway[u32_B], rc_T.c_Ip.au8_DefaultGateway[u32_B]) << c_W << "gw byte " <<
            u32_B;
      }
   }
   EXPECT_EQ(9U, rc_Node.c_Properties.c_ComInterfaces[0].u8_NodeId);
   EXPECT_TRUE(rc_Node.c_Properties.c_ComInterfaces[0].GetBusConnected());
   EXPECT_EQ(0U, rc_Node.c_Properties.c_ComInterfaces[0].u32_BusIndex);
   EXPECT_FALSE(rc_Node.c_Properties.c_ComInterfaces[1].GetBusConnected());
   EXPECT_EQ(9U, rc_Node.c_Properties.c_ComInterfaces[1].c_Ip.au8_IpAddress[3]);
   EXPECT_EQ(1024U, rc_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxServiceSizeByte);
   ASSERT_EQ(2U, rc_Node.c_Applications.size());
   EXPECT_EQ(C_OscNodeApplication::ePROGRAMMABLE_APPLICATION, rc_Node.c_Applications[1].e_Type);
   EXPECT_EQ("make -C proj/app", rc_Node.c_Applications[1].c_IdeCall);
   EXPECT_EQ((std::vector<std::string>{"out/app.hex", "out/app.syde_hex"}), rc_Node.c_Applications[1].c_ResultPaths);
   ASSERT_EQ(3U, rc_Node.c_DataPools.size());
   EXPECT_EQ("NvmPool", rc_Node.c_DataPools[1].c_Name);
   EXPECT_EQ(2U, rc_Node.c_DataPools[1].c_Lists.size());
   ASSERT_EQ(1U, rc_Node.c_ComProtocols.size());
   EXPECT_EQ(2U, rc_Node.c_ComProtocols[0].u32_DataPoolIndex);
   ASSERT_EQ(1U, c_Target.c_NodeSquads.size());
   EXPECT_EQ("Pair", c_Target.c_NodeSquads[0].c_BaseName);

   (void)std::filesystem::remove_all(c_Dir);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   CANopen manager file round-trip

   The manager filer copies each device's EDS next to the manager file (prefixed with node name and interface) and
   only remembers the path on load, so the target has to be asked for its EDS content before the hashes can be
   compared: C_OscCanOpenManagerDeviceInfo::CalcHash covers the parsed dictionary. The EDS itself is a minimal
   one written here; every FileInfo/DeviceInfo key has a default in the parser, only the object lists must agree.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, CanOpenManagerFile)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_rt_canopen";
   const std::filesystem::path c_Eds = c_Dir / "source.eds";
   const std::filesystem::path c_Path = c_Dir / "canopen_manager.xml";

   (void)std::filesystem::remove_all(c_Dir);
   ASSERT_TRUE(std::filesystem::create_directories(c_Dir));
   {
      std::ofstream c_File(c_Eds);
      c_File << "[FileInfo]\nFileName=device.eds\nFileVersion=1\nFileRevision=2\nDescription=round-trip device\n"
             << "[DeviceInfo]\nVendorName=RoundTrip\nProductName=RT Sensor\nBaudRate_250=1\nBaudRate_500=1\n"
             << "NrOfRxPDO=4\nNrOfTxPDO=4\nGranularity=8\n"
             << "[MandatoryObjects]\nSupportedObjects=2\n1=0x1000\n2=0x1001\n"
             << "[1000]\nParameterName=Device Type\nObjectType=0x7\nDataType=0x0007\nAccessType=ro\n"
             << "DefaultValue=0x00000000\nPDOMapping=0\n"
             << "[1001]\nParameterName=Error Register\nObjectType=0x7\nDataType=0x0005\nAccessType=ro\nPDOMapping=0\n"
             << "[OptionalObjects]\nSupportedObjects=0\n[ManufacturerObjects]\nSupportedObjects=0\n";
   }

   std::map<uint8_t, C_OscCanOpenManagerInfo> c_Source;
   {
      C_OscCanOpenManagerInfo c_M;
      c_M.q_UseOpenSydeNodeId = false;
      c_M.u8_NodeIdValue = 42U;
      c_M.u16_GlobalSdoTimeoutMs = 750U;
      c_M.q_AutostartCanOpenManager = true;
      c_M.q_StartDevices = true;
      c_M.q_NmtStartAll = false;
      c_M.e_NmtErrorBehaviour = C_OscCanOpenManagerInfo::eRESTART_FAILURE_DEVICE;
      c_M.q_EnableHeartbeatProducing = true;
      c_M.u16_HeartbeatProducerTimeMs = 300U;
      c_M.q_ProduceSyncMessage = true;
      c_M.u32_SyncCyclePeriodUs = 10000U;
      c_M.u32_SyncWindowLengthUs = 2500U;

      C_OscCanOpenManagerDeviceInfo c_Dev;
      c_Dev.c_ProjectEdsFilePath = c_Eds.string();
      c_Dev.c_OriginalEdsFileName = "device.eds";
      c_Dev.q_DeviceOptional = true;
      c_Dev.q_NoInitialization = false;
      c_Dev.q_FactorySettingsActive = true;
      c_Dev.u8_ResetNodeObjectDictionarySubIndex = 3U;
      c_Dev.q_EnableHeartbeatProducing = true;
      c_Dev.u16_HeartbeatProducerTimeMs = 400U;
      c_Dev.q_UseOpenSydeNodeId = false;
      c_Dev.u8_NodeIdValue = 17U;
      c_Dev.q_EnableHeartbeatConsuming = true;
      c_Dev.u16_HeartbeatConsumerTimeMs = 600U;
      c_Dev.q_EnableHeartbeatConsumingAutoCalculation = false;
      {
         C_OscCanOpenManagerMappableSignal c_Sig;
         c_Sig.q_AutoMinMaxUsed = true;
         c_Sig.c_SignalData.e_ComByteOrder = C_OscCanSignal::eBYTE_ORDER_INTEL;
         c_Sig.c_SignalData.u16_ComBitStart = 8U;
         c_Sig.c_SignalData.u16_ComBitLength = 16U;
         c_Sig.c_SignalData.u32_ComDataElementIndex = 0U;
         c_Sig.c_SignalData.e_MultiplexerType = C_OscCanSignal::eMUX_DEFAULT;
         c_Sig.c_SignalData.u16_CanOpenManagerObjectDictionaryIndex = 0x6000U;
         c_Sig.c_SignalData.u8_CanOpenManagerObjectDictionarySubIndex = 1U;
         c_Sig.c_DatapoolData.c_Name = "Speed";
         c_Sig.c_DatapoolData.c_Comment = "mapped from the EDS";
         c_Sig.c_DatapoolData.c_Unit = "rpm";
         c_Sig.c_DatapoolData.e_Access = C_OscNodeDataPoolListElement::eACCESS_RO;
         c_Sig.c_DatapoolData.f64_Factor = 0.125;
         c_Sig.c_DatapoolData.f64_Offset = -10.0;
         c_Sig.c_DatapoolData.c_MinValue.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_Sig.c_DatapoolData.c_MinValue.SetValueU16(0U);
         c_Sig.c_DatapoolData.c_MaxValue.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_Sig.c_DatapoolData.c_MaxValue.SetValueU16(8000U);
         c_Sig.c_DatapoolData.c_Value.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_Sig.c_DatapoolData.c_Value.SetValueU16(1500U);
         c_Sig.c_DatapoolData.c_DataSetValues.push_back(c_Sig.c_DatapoolData.c_Value);
         c_Dev.c_EdsFileMappableSignals.push_back(c_Sig);
      }
      c_M.c_CanOpenDevices[C_OscCanInterfaceId(3U, 1U)] = c_Dev;
      c_Source[1U] = c_M;
   }
   const std::map<uint32_t, std::string> c_Names = {{3U, "SensorNode"}};

   //"base path" is the convention of the node filers: the path of the file being written, whose directory the
   //side files go next to (h_SaveNodeFile hands its own file path down as the base path)
   std::vector<std::string> c_Created;
   ASSERT_FALSE(static_cast<bool>(C_OscCanOpenManagerFiler::h_SaveFile(c_Source, c_Path.string(), c_Path.string(),
                                                                       &c_Created, c_Names)));
   ASSERT_EQ(1U, c_Created.size()) << "one EDS copy per device";
   EXPECT_TRUE(std::filesystem::exists(c_Dir / c_Created[0])) << c_Created[0];
   EXPECT_NE(std::string::npos, c_Created[0].find("device.eds")) << "copy keeps the original EDS name as suffix";

   std::map<uint8_t, C_OscCanOpenManagerInfo> c_Target;
   ASSERT_FALSE(static_cast<bool>(C_OscCanOpenManagerFiler::h_LoadFile(c_Target, c_Path.string(), c_Path.string())));
   ASSERT_EQ(1U, c_Target.size());
   ASSERT_EQ(1U, c_Target.count(1U));
   const C_OscCanOpenManagerInfo & rc_M = c_Target.at(1U);
   ASSERT_EQ(1U, rc_M.c_CanOpenDevices.size());
   const C_OscCanOpenManagerDeviceInfo & rc_Dev = rc_M.c_CanOpenDevices.begin()->second;
   EXPECT_EQ(3U, rc_M.c_CanOpenDevices.begin()->first.u32_NodeIndex);
   EXPECT_EQ(1U, rc_M.c_CanOpenDevices.begin()->first.u8_InterfaceNumber);
   EXPECT_EQ((c_Dir / c_Created[0]).string(), rc_Dev.c_ProjectEdsFilePath) << "load remembers the copied EDS";
   //the dictionary is loaded on demand: pull it in on the target so both sides hash the parsed EDS
   EXPECT_EQ(2U, rc_Dev.GetEdsFileContent().c_OdObjects.size()) << "the copied EDS did not parse back";

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.at(1U).CalcHash(u32_A);
   rc_M.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";

   EXPECT_EQ(42U, rc_M.u8_NodeIdValue);
   EXPECT_EQ(750U, rc_M.u16_GlobalSdoTimeoutMs);
   EXPECT_EQ(C_OscCanOpenManagerInfo::eRESTART_FAILURE_DEVICE, rc_M.e_NmtErrorBehaviour);
   EXPECT_EQ(2500U, rc_M.u32_SyncWindowLengthUs);
   EXPECT_EQ("device.eds", rc_Dev.c_OriginalEdsFileName);
   EXPECT_EQ(3U, rc_Dev.u8_ResetNodeObjectDictionarySubIndex);
   EXPECT_EQ(600U, rc_Dev.u16_HeartbeatConsumerTimeMs);
   ASSERT_EQ(1U, rc_Dev.c_EdsFileMappableSignals.size());
   EXPECT_TRUE(rc_Dev.c_EdsFileMappableSignals[0].q_AutoMinMaxUsed);
   EXPECT_EQ(0x6000U, rc_Dev.c_EdsFileMappableSignals[0].c_SignalData.u16_CanOpenManagerObjectDictionaryIndex);
   EXPECT_EQ("Speed", rc_Dev.c_EdsFileMappableSignals[0].c_DatapoolData.c_Name);
   EXPECT_EQ(8000U, rc_Dev.c_EdsFileMappableSignals[0].c_DatapoolData.c_MaxValue.GetValueU16());

   (void)std::filesystem::remove_all(c_Dir);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   HALC definition and configuration round-trips

   The definition is the device-side description (domains, channels, use-cases, typed parameters incl. enum and
   bitmask); the configuration wraps it with per-channel user settings and, with an empty base path, embeds the
   definition inline. Both go through their in-memory savers so the same model can drive the definition, the
   configuration and (below) the standalone export.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(FilerRoundTrip, HalcDef)
{
   C_OscHalcDef c_Source;

   h_FillHalcBase(c_Source);
   c_Source.AddDomain(h_MakeHalcDomain());

   C_OscXmlParser c_Xml;
   c_Xml.CreateAndSelectNodeChild("opensyde-HALC-description");
   ASSERT_FALSE(static_cast<bool>(C_OscHalcDefFiler::h_SaveData(c_Source, c_Xml)));
   //a copy on disk to look at when the load below fails: the definition loader is picky and not always vocal
   const std::filesystem::path c_Dump = std::filesystem::temp_directory_path() / "osy_rt_halc_def.xml";
   (void)c_Xml.SaveToFile(c_Dump.string());

   C_OscHalcDef c_Target;
   ASSERT_EQ("opensyde-HALC-description", c_Xml.SelectRoot());
   ASSERT_FALSE(static_cast<bool>(C_OscHalcDefFiler::h_LoadData(c_Target, c_Xml)));

   //h_LoadFile fills these two from the file; the in-memory saver does not carry them, nor should it
   EXPECT_TRUE(c_Target.c_FileString.empty());

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";

   //C_OscHalcDef::CalcHash covers the base fields only, so the domains get their own comparison
   ASSERT_EQ(c_Source.GetDomainSize(), c_Target.GetDomainSize());
   for (uint32_t u32_It = 0U; u32_It < c_Source.GetDomainSize(); ++u32_It)
   {
      h_ExpectSameHash(*c_Source.GetDomainDefDataConst(u32_It), *c_Target.GetDomainDefDataConst(u32_It),
                       "domain " + std::to_string(u32_It));
   }

   EXPECT_EQ(3U, c_Target.u32_ContentVersion);
   EXPECT_EQ("RT-IO", c_Target.c_DeviceName);
   EXPECT_EQ(C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING, c_Target.e_SafetyMode);
   EXPECT_EQ(2U, c_Target.u8_NumConfigCopies);
   EXPECT_EQ((std::vector<uint32_t>{0x300U, 0x400U}), c_Target.c_NvmNonSafeAddressOffset);
   EXPECT_EQ(13U, c_Target.u32_NvmReservedListSizeStatusValues);
   ASSERT_EQ(1U, c_Target.GetDomainSize());
   const C_OscHalcDefDomain * const pc_Dom = c_Target.GetDomainDefDataConst(0U);
   ASSERT_NE(nullptr, pc_Dom);
   EXPECT_EQ("Digital Input", pc_Dom->c_SingularName);
   EXPECT_EQ(C_OscHalcDefDomain::eCA_INPUT, pc_Dom->e_Category);
   ASSERT_EQ(3U, pc_Dom->c_Channels.size());
   EXPECT_EQ("DI_3", pc_Dom->c_Channels[2].c_Name);
   ASSERT_EQ(2U, pc_Dom->c_ChannelUseCases.size());
   EXPECT_EQ((std::vector<uint32_t>{2U}), pc_Dom->c_ChannelUseCases[1].c_DefaultChannels);
   ASSERT_EQ(2U, pc_Dom->c_ChannelUseCases[1].c_Availability.size());
   EXPECT_EQ(1U, pc_Dom->c_ChannelUseCases[1].c_Availability[0].u32_ValueIndex);
   ASSERT_EQ(2U, pc_Dom->c_ChannelValues.c_Parameters.size());
   const C_OscHalcDefStruct & rc_Filter = pc_Dom->c_ChannelValues.c_Parameters[0];
   ASSERT_EQ(2U, rc_Filter.c_StructElements.size());
   EXPECT_EQ(C_OscHalcDefContent::eCT_ENUM, rc_Filter.c_StructElements[0].GetComplexType());
   EXPECT_EQ(C_OscHalcDefContent::eCT_BIT_MASK, rc_Filter.c_StructElements[1].GetComplexType());
   EXPECT_EQ((std::vector<uint32_t>{1U}), rc_Filter.c_StructElements[1].c_UseCaseAvailabilities);
   EXPECT_EQ(5U, pc_Dom->c_ChannelValues.c_Parameters[1].c_InitialValue.GetValueU16());
   EXPECT_EQ(1000U, pc_Dom->c_DomainValues.c_Parameters[0].c_MaxValue.GetValueU16());

   (void)std::filesystem::remove(c_Dump);
}

TEST(FilerRoundTrip, HalcConfig)
{
   C_OscHalcConfig c_Source;

   h_FillHalcBase(c_Source);
   //the config filer persists these two itself (as attributes of the inline definition)
   c_Source.c_FileString = "<original definition text/>";
   c_Source.c_OriginalFileName = "rt_io.syde_halc_def";
   c_Source.AddDomain(h_MakeHalcDomain());
   ASSERT_FALSE(static_cast<bool>(c_Source.SetDomainChannelConfig(0U, 0U, true, "Ignition", "key switch", true, 0U)));
   ASSERT_FALSE(static_cast<bool>(c_Source.SetDomainChannelConfigUseCase(0U, 2U, true, 0U)));
   ASSERT_FALSE(static_cast<bool>(c_Source.SetDomainChannelParameterConfigElementEnum(0U, 1U, 0U, 0U, true, "Edge")));
   ASSERT_FALSE(static_cast<bool>(c_Source.SetDomainChannelParameterConfigElementBitmask(0U, 1U, 0U, 1U, true,
                                                                                         "Pullup", true)));
   {
      //plain parameters and the domain-level config have no element-wise setter: edit a copy and put it back
      const C_OscHalcConfigDomain * const pc_Dom = c_Source.GetDomainConfigDataConst(0U);
      ASSERT_NE(nullptr, pc_Dom);
      C_OscHalcConfigDomain c_Dom = *pc_Dom;
      ASSERT_EQ(3U, c_Dom.c_ChannelConfigs.size());
      ASSERT_EQ(2U, c_Dom.c_ChannelConfigs[1].c_Parameters.size());
      c_Dom.c_ChannelConfigs[1].c_Parameters[1].c_Value.SetValueU16(55U);
      c_Dom.c_ChannelConfigs[1].c_Parameters[1].c_Comment = "longer debounce";
      ASSERT_EQ(1U, c_Dom.c_DomainConfig.c_Parameters.size());
      c_Dom.c_DomainConfig.c_Parameters[0].c_Value.SetValueU16(500U);
      c_Dom.c_DomainConfig.c_Comment = "domain wide";
      ASSERT_FALSE(static_cast<bool>(c_Source.SetDomainConfig(0U, c_Dom)));
   }

   C_OscXmlParser c_Xml;
   c_Xml.CreateAndSelectNodeChild("opensyde-node-io-config");
   ASSERT_FALSE(static_cast<bool>(C_OscHalcConfigFiler::h_SaveData(c_Source, c_Xml, "", nullptr)));

   C_OscHalcConfig c_Target;
   ASSERT_EQ("opensyde-node-io-config", c_Xml.SelectRoot());
   ASSERT_FALSE(static_cast<bool>(C_OscHalcConfigFiler::h_LoadData(c_Target, c_Xml, "")));

   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;
   c_Source.CalcHash(u32_A);
   c_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << "a field is lost that the checks below do not cover";
   {
      //CalcHash is virtual, so a base reference still hashes the whole config: qualify the call to get the base part
      uint32_t u32_S = 0xFFFFFFFFUL;
      uint32_t u32_T = 0xFFFFFFFFUL;
      c_Source.C_OscHalcDefBase::CalcHash(u32_S);
      c_Target.C_OscHalcDefBase::CalcHash(u32_T);
      EXPECT_EQ(u32_S, u32_T) << "definition base does not round-trip";
   }
   ASSERT_EQ(c_Source.GetDomainSize(), c_Target.GetDomainSize());
   for (uint32_t u32_It = 0U; u32_It < c_Source.GetDomainSize(); ++u32_It)
   {
      const C_OscHalcConfigDomain & rc_S = *c_Source.GetDomainConfigDataConst(u32_It);
      const C_OscHalcConfigDomain & rc_T = *c_Target.GetDomainConfigDataConst(u32_It);
      const std::string c_D = "domain " + std::to_string(u32_It) + " ";
      {
         uint32_t u32_S = 0xFFFFFFFFUL;
         uint32_t u32_T = 0xFFFFFFFFUL;
         rc_S.C_OscHalcDefDomain::CalcHash(u32_S);
         rc_T.C_OscHalcDefDomain::CalcHash(u32_T);
         EXPECT_EQ(u32_S, u32_T) << c_D << "definition part does not round-trip";
      }
      EXPECT_EQ(rc_S.c_Id, rc_T.c_Id) << c_D << "id";
      EXPECT_EQ(rc_S.c_Name, rc_T.c_Name) << c_D << "name";
      EXPECT_EQ(rc_S.c_SingularName, rc_T.c_SingularName) << c_D << "singular name";
      EXPECT_EQ(rc_S.c_Comment, rc_T.c_Comment) << c_D << "def comment";
      EXPECT_EQ(rc_S.e_Category, rc_T.e_Category) << c_D << "category";
      ASSERT_EQ(rc_S.c_Channels.size(), rc_T.c_Channels.size());
      ASSERT_EQ(rc_S.c_ChannelUseCases.size(), rc_T.c_ChannelUseCases.size());
      for (uint32_t u32_U = 0U; u32_U < rc_S.c_ChannelUseCases.size(); ++u32_U)
      {
         h_ExpectSameHash(rc_S.c_ChannelUseCases[u32_U], rc_T.c_ChannelUseCases[u32_U],
                          c_D + "use-case " + std::to_string(u32_U));
      }
      h_ExpectSameHash(rc_S.c_DomainValues, rc_T.c_DomainValues, c_D + "domain values");
      h_ExpectSameHash(rc_S.c_ChannelValues, rc_T.c_ChannelValues, c_D + "channel values");
      h_ExpectSameChannelConfig(rc_S.c_DomainConfig, rc_T.c_DomainConfig, c_D + "domain config");
      ASSERT_EQ(rc_S.c_ChannelConfigs.size(), rc_T.c_ChannelConfigs.size());
      for (uint32_t u32_Ch = 0U; u32_Ch < rc_S.c_ChannelConfigs.size(); ++u32_Ch)
      {
         h_ExpectSameChannelConfig(rc_S.c_ChannelConfigs[u32_Ch], rc_T.c_ChannelConfigs[u32_Ch],
                                   c_D + "channel config " + std::to_string(u32_Ch));
      }
   }

   EXPECT_EQ("<original definition text/>", c_Target.c_FileString);
   EXPECT_EQ("rt_io.syde_halc_def", c_Target.c_OriginalFileName);
   ASSERT_EQ(1U, c_Target.GetDomainSize());
   const C_OscHalcConfigDomain * const pc_Dom = c_Target.GetDomainConfigDataConst(0U);
   ASSERT_NE(nullptr, pc_Dom);
   ASSERT_EQ(3U, pc_Dom->c_ChannelConfigs.size());
   EXPECT_EQ("Ignition", pc_Dom->c_ChannelConfigs[0].c_Name);
   EXPECT_EQ("key switch", pc_Dom->c_ChannelConfigs[0].c_Comment);
   EXPECT_TRUE(pc_Dom->c_ChannelConfigs[0].q_SafetyRelevant);
   EXPECT_EQ(0U, pc_Dom->c_ChannelConfigs[1].u32_UseCaseIndex) << "DI_2 keeps its default use-case (Off)";
   EXPECT_EQ(0U, pc_Dom->c_ChannelConfigs[2].u32_UseCaseIndex) << "DI_3 was switched to Off";
   ASSERT_EQ(2U, pc_Dom->c_ChannelConfigs[1].c_Parameters.size());
   ASSERT_EQ(2U, pc_Dom->c_ChannelConfigs[1].c_Parameters[0].c_ParameterElements.size());
   {
      std::string c_Enum;
      C_OscHalcDefContent c_Mode = pc_Dom->c_ChannelConfigs[1].c_Parameters[0].c_ParameterElements[0].c_Value;
      ASSERT_FALSE(static_cast<bool>(c_Mode.GetEnumValue(c_Enum)));
      EXPECT_EQ("Edge", c_Enum);
      bool q_Pullup = false;
      ASSERT_FALSE(static_cast<bool>(pc_Dom->c_ChannelConfigs[1].c_Parameters[0].c_ParameterElements[1].c_Value.
                                     GetBitmask("Pullup", q_Pullup)));
      EXPECT_TRUE(q_Pullup);
   }
   EXPECT_EQ(55U, pc_Dom->c_ChannelConfigs[1].c_Parameters[1].c_Value.GetValueU16());
   EXPECT_EQ("longer debounce", pc_Dom->c_ChannelConfigs[1].c_Parameters[1].c_Comment);
   ASSERT_EQ(1U, pc_Dom->c_DomainConfig.c_Parameters.size());
   EXPECT_EQ(500U, pc_Dom->c_DomainConfig.c_Parameters[0].c_Value.GetValueU16());
   EXPECT_EQ("domain wide", pc_Dom->c_DomainConfig.c_Comment);
}

TEST(FilerRoundTrip, HalcConfigStandalone)
{
   //the standalone export is derived from a full configuration, never built by hand: its domains inherit the
   //definition's channel list and the filer asserts that the channel configs line up with it
   C_OscHalcConfig c_Config;

   h_FillHalcBase(c_Config);
   c_Config.AddDomain(h_MakeHalcDomain());
   ASSERT_FALSE(static_cast<bool>(c_Config.SetDomainChannelConfig(0U, 0U, true, "Ignition", "key switch", true, 0U)));
   ASSERT_FALSE(static_cast<bool>(c_Config.SetDomainChannelParameterConfigElementEnum(0U, 1U, 0U, 0U, true, "Edge")));

   C_OscHalcConfigStandalone c_Source;
   C_OscHalcConfigUtil::h_GetConfigStandalone(c_Config, c_Source);
   ASSERT_EQ(1U, c_Source.c_Domains.size());
   ASSERT_EQ(3U, c_Source.c_Domains[0].c_StandaloneChannels.size());

   C_OscXmlParser c_Xml;
   c_Xml.CreateAndSelectNodeChild("opensyde-halc-standalone");
   ASSERT_FALSE(static_cast<bool>(C_OscHalcConfigStandaloneFiler::h_SaveDataStandalone(c_Source, c_Xml)));

   C_OscHalcConfigStandalone c_Target;
   ASSERT_EQ("opensyde-halc-standalone", c_Xml.SelectRoot());
   ASSERT_FALSE(static_cast<bool>(C_OscHalcConfigStandaloneFiler::h_LoadDataStandalone(c_Target, c_Xml)));

   EXPECT_EQ(c_Source.c_DeviceType, c_Target.c_DeviceType);
   EXPECT_EQ(c_Source.u32_DefinitionContentVersion, c_Target.u32_DefinitionContentVersion);
   ASSERT_EQ(c_Source.c_Domains.size(), c_Target.c_Domains.size());
   for (uint32_t u32_It = 0U; u32_It < c_Source.c_Domains.size(); ++u32_It)
   {
      const C_OscHalcConfigStandaloneDomain & rc_S = c_Source.c_Domains[u32_It];
      const C_OscHalcConfigStandaloneDomain & rc_T = c_Target.c_Domains[u32_It];
      //the standalone format carries the configuration half only: domain id, channel names, parameter ids and
      //the channel/domain configs. The definition part (use-cases, values) is deliberately not in the file, so the
      //inherited domain hash is not the oracle here; the config pieces have hashes of their own
      const std::string c_D = "standalone domain " + std::to_string(u32_It) + " ";
      EXPECT_EQ(rc_S.c_Id, rc_T.c_Id) << c_D << "id";
      ASSERT_EQ(rc_S.c_Channels.size(), rc_T.c_Channels.size());
      for (uint32_t u32_Ch = 0U; u32_Ch < rc_S.c_Channels.size(); ++u32_Ch)
      {
         EXPECT_EQ(rc_S.c_Channels[u32_Ch].c_Name, rc_T.c_Channels[u32_Ch].c_Name) << c_D << "channel " << u32_Ch;
      }
      h_ExpectSameHash(rc_S.c_DomainConfig, rc_T.c_DomainConfig, c_D + "domain config");
      ASSERT_EQ(rc_S.c_ChannelConfigs.size(), rc_T.c_ChannelConfigs.size());
      for (uint32_t u32_Ch = 0U; u32_Ch < rc_S.c_ChannelConfigs.size(); ++u32_Ch)
      {
         h_ExpectSameChannelConfig(rc_S.c_ChannelConfigs[u32_Ch], rc_T.c_ChannelConfigs[u32_Ch],
                                   c_D + "channel config " + std::to_string(u32_Ch));
      }
      ASSERT_EQ(rc_S.c_StandaloneChannels.size(), rc_T.c_StandaloneChannels.size());
      for (uint32_t u32_Ch = 0U; u32_Ch < rc_S.c_StandaloneChannels.size(); ++u32_Ch)
      {
         EXPECT_EQ(rc_S.c_StandaloneChannels[u32_Ch].c_ParameterIds, rc_T.c_StandaloneChannels[u32_Ch].c_ParameterIds) <<
            "channel " << u32_Ch << " parameter ids";
      }
   }
   EXPECT_EQ("Ignition", c_Target.c_Domains[0].c_ChannelConfigs[0].c_Name);
   EXPECT_TRUE(c_Target.c_Domains[0].c_ChannelConfigs[0].q_SafetyRelevant);
   EXPECT_EQ((std::vector<std::string>{"filter", "debounce"}), c_Target.c_Domains[0].c_StandaloneChannels[2].c_ParameterIds);
}
