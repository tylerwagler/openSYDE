//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for TglStartProcessDetached

   Starts a helper script that records its working directory and arguments, and checks the launcher's contract:
   the process runs in the folder containing the binary, a relative binary path is resolved against the caller's
   working directory, and quoted parameters arrive as single arguments. POSIX only; skipped on Windows.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include "gtest/gtest.h"
#include "stwerrors.hpp"
#include "TglTasks.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::tgl;

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
#ifndef _WIN32
/// Waits up to fifteen seconds for the helper to write its complete report (terminated by an "end" line).
/// The child is forked and detached, so on a saturated CI runner (hundreds of parallel test
/// binaries) it can sit in the run queue for several seconds before the shell script runs; a
/// five-second cap flakes there.
std::vector<std::string> h_WaitForReport(const std::filesystem::path & orc_Path)
{
   std::vector<std::string> c_Lines;

   for (uint32_t u32_Try = 0U; u32_Try < 300U; ++u32_Try)
   {
      std::ifstream c_File(orc_Path);
      std::string c_Line;
      c_Lines.clear();
      while (std::getline(c_File, c_Line))
      {
         c_Lines.push_back(c_Line);
      }
      if ((c_Lines.empty() == false) && (c_Lines.back() == "end"))
      {
         break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   }
   return c_Lines;
}

/// Temporary folder with an executable helper script in a subfolder; the working directory is restored on exit.
class C_TglTasksFixture :
   public ::testing::Test
{
protected:
   std::filesystem::path mc_Root;
   std::filesystem::path mc_PreviousCwd;

   void SetUp() override
   {
      mc_PreviousCwd = std::filesystem::current_path();
      mc_Root = std::filesystem::canonical(std::filesystem::temp_directory_path()) / "osy_tgl_tasks";
      (void)std::filesystem::remove_all(mc_Root);
      ASSERT_TRUE(std::filesystem::create_directories(mc_Root / "bin"));
      {
         std::ofstream c_Script(mc_Root / "bin" / "report.sh");
         c_Script << "#!/bin/sh\n"
                     "out=\"$1\"\n"
                     "shift\n"
                     "pwd > \"$out\"\n"
                     "for a in \"$@\"; do echo \"$a\" >> \"$out\"; done\n"
                     "echo end >> \"$out\"\n";
      }
      std::filesystem::permissions(mc_Root / "bin" / "report.sh", std::filesystem::perms::owner_all);
   }

   void TearDown() override
   {
      std::filesystem::current_path(mc_PreviousCwd);
      (void)std::filesystem::remove_all(mc_Root);
   }
};
#else
class C_TglTasksFixture :
   public ::testing::Test
{
};
#endif
}

/* -- Tests --------------------------------------------------------------------------------------------------------- */

TEST_F(C_TglTasksFixture, RelativeBinaryRunsInItsFolderWithQuotedArguments)
{
#ifdef _WIN32
   GTEST_SKIP() << "helper is a POSIX shell script";
#else
   const std::filesystem::path c_Report = mc_Root / "report.txt";

   //relative to the working directory; the child changes into bin/ before exec, which must not break the lookup
   std::filesystem::current_path(mc_Root);
   ASSERT_EQ(C_NO_ERR, TglStartProcessDetached("bin/report.sh",
                                               "\"" + c_Report.string() + "\" plain \"two words\" 'single quoted'"));

   const std::vector<std::string> c_Lines = h_WaitForReport(c_Report);
   ASSERT_EQ(5U, c_Lines.size());
   EXPECT_EQ((mc_Root / "bin").string(), c_Lines[0]);
   EXPECT_EQ("plain", c_Lines[1]);
   EXPECT_EQ("two words", c_Lines[2]);
   EXPECT_EQ("single quoted", c_Lines[3]);
#endif
}

TEST_F(C_TglTasksFixture, MissingBinaryIsNoact)
{
#ifdef _WIN32
   GTEST_SKIP() << "helper is a POSIX shell script";
#else
   EXPECT_EQ(C_NOACT, TglStartProcessDetached((mc_Root / "bin" / "does_not_exist").string(), ""));
#endif
}
