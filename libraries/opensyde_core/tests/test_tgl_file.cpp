//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Regression tests for the TGL file path helpers

   TglChangeFileExtension had two defects, and the first was destructive.

   When the path had no extension it returned the path unchanged. Two tools derive
   their log file as TglChangeFileExtension(TglGetExePath(), ".log"), which on
   Windows turns "tool.exe" into "tool.log" -- but on Linux and macOS the
   executable has no extension, so the "log file" resolved to the executable
   itself. osy_syde_coder_c overwrote its own 26 MB binary with 591 bytes of log
   on its first --help. syde_x_gen has identical code.

   It also searched the whole path for the last "." rather than only the final
   component, so a dotted directory name -- "/home/j.doe/tool" -- produced
   "/home/j.log".

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include "TglFile.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::tgl;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(TglFile, ChangeExtensionReplacesAnExistingOne)
{
   EXPECT_EQ("/opt/tool/app.log", TglChangeFileExtension("/opt/tool/app.exe", ".log"));
   EXPECT_EQ("app.log", TglChangeFileExtension("app.exe", ".log"));
   EXPECT_EQ("/opt/tool/app", TglChangeFileExtension("/opt/tool/app.exe", ""));
}

/// The destructive case. A path with no extension must gain one, not come back
/// unchanged -- unchanged is what made "<exe>" + ".log" equal the executable.
TEST(TglFile, ChangeExtensionAppendsWhenThereIsNone)
{
   const std::string c_Exe = "/opt/tool/osy_syde_coder_c";
   const std::string c_Log = TglChangeFileExtension(c_Exe, ".log");

   EXPECT_NE(c_Exe, c_Log) << "the log path resolved to the executable itself";
   EXPECT_EQ("/opt/tool/osy_syde_coder_c.log", c_Log);
}

/// Only a dot in the final path component is an extension.
TEST(TglFile, ChangeExtensionIgnoresDotsInDirectoryNames)
{
   EXPECT_EQ("/home/j.doe/tool.log", TglChangeFileExtension("/home/j.doe/tool", ".log"));
   EXPECT_EQ("/home/j.doe/tool.log", TglChangeFileExtension("/home/j.doe/tool.exe", ".log"));
   EXPECT_EQ("/opt/v1.2/app.log", TglChangeFileExtension("/opt/v1.2/app", ".log"));
}

TEST(TglFile, ChangeExtensionHandlesEmptyInput)
{
   EXPECT_EQ(".log", TglChangeFileExtension("", ".log"));
   EXPECT_EQ("", TglChangeFileExtension("", ""));
}
