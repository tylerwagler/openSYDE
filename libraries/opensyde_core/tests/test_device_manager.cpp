//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscDeviceManager's scan result reporting

   LoadFromPaths used to return Errc::success unconditionally: it logged unreadable roots and
   unparseable manifests and then told the caller everything was fine. openSYDE could therefore
   start with an empty device list and say nothing about it. These tests pin the contract that
   replaced that, because it is what the startup dialog keys off.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "C_OscDeviceManager.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

namespace
{
/* -- Module Global Functions --------------------------------------------------------------------------------------- */

std::string mh_MakeScratchDir(const std::string & orc_Name)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / orc_Name;

   std::error_code c_Ec;
   std::filesystem::remove_all(c_Dir, c_Ec);
   std::filesystem::create_directories(c_Dir, c_Ec);
   return c_Dir.string();
}

void mh_WriteManifest(const std::string & orc_Dir, const std::string & orc_Content)
{
   std::error_code c_Ec;
   std::filesystem::create_directories(orc_Dir, c_Ec);
   std::ofstream c_Stream((std::filesystem::path(orc_Dir) / "device.syd").string().c_str());
   c_Stream << orc_Content;
   c_Stream.close();
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(OscDeviceManager, LoadFromPaths_NoRootsAtAllReportsNoact)
{
   C_OscDeviceManager c_Manager;
   const std::vector<std::string> c_Empty;

   //This is the case the startup dialog exists for: nothing to scan, so nothing loaded
   EXPECT_EQ(Errc::noact, c_Manager.LoadFromPaths(c_Empty));
   EXPECT_TRUE(c_Manager.GetDeviceGroups().empty());
}

TEST(OscDeviceManager, LoadFromPaths_NonexistentRootReportsNoact)
{
   C_OscDeviceManager c_Manager;
   std::vector<std::string> c_Roots;

   c_Roots.push_back((std::filesystem::temp_directory_path() / "osy_does_not_exist_42").string());

   //An unreadable root is logged and skipped; with nothing else to scan the result is still
   //"no devices", which used to be reported as success.
   EXPECT_EQ(Errc::noact, c_Manager.LoadFromPaths(c_Roots));
}

TEST(OscDeviceManager, LoadFromPaths_EmptyDirectoryReportsNoact)
{
   const std::string c_Root = mh_MakeScratchDir("osy_devmgr_empty");
   C_OscDeviceManager c_Manager;
   std::vector<std::string> c_Roots;

   c_Roots.push_back(c_Root);

   //A perfectly readable root that simply holds no manifests is the same outcome for the user
   EXPECT_EQ(Errc::noact, c_Manager.LoadFromPaths(c_Roots));
}

TEST(OscDeviceManager, LoadFromPaths_UnparseableManifestAloneReportsNoact)
{
   const std::string c_Root = mh_MakeScratchDir("osy_devmgr_bad");
   C_OscDeviceManager c_Manager;
   std::vector<std::string> c_Roots;

   mh_WriteManifest(c_Root + "/some_device", "this is not xml at all");
   c_Roots.push_back(c_Root);

   //The manifest is found and fails to parse. Nothing registered, so the caller is told the
   //scan produced nothing rather than that it succeeded.
   EXPECT_EQ(Errc::noact, c_Manager.LoadFromPaths(c_Roots));
   EXPECT_TRUE(c_Manager.GetDeviceGroups().empty());
}

TEST(OscDeviceManager, LoadFromPaths_LeavesWasLoadedSetEvenWhenNothingWasFound)
{
   C_OscDeviceManager c_Manager;
   const std::vector<std::string> c_Empty;

   //WasLoaded means "a scan has run", and C_OscSystemDefinitionFiler gates on it. The scan
   //reporting no devices must not turn that back off.
   EXPECT_TRUE(static_cast<bool>(c_Manager.LoadFromPaths(c_Empty)));
   EXPECT_TRUE(c_Manager.WasLoaded());
}
