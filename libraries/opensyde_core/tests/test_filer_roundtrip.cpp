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

#include <filesystem>

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
