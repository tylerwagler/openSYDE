//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target support package loader

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "C_OscErrorCategory.hpp"
#include "C_OscTargetSupportPackage.hpp"
#include "C_OscTargetSupportPackageFiler.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
std::string h_WriteTsp(const std::string & orc_Name, const std::string & orc_Version)
{
   const std::filesystem::path c_Path = std::filesystem::temp_directory_path() / orc_Name;
   std::ofstream c_Out(c_Path, std::ios::binary);

   c_Out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<opensyde-target-support-package>\n"
            "  <file-version>" << orc_Version << "</file-version>\n"
            "  <device-name>RT-Device 1</device-name>\n"
            "  <tsp-comment>Round-trip &amp; friends</tsp-comment>\n"
            "  <node-definition>node/rt_device_1.syde_node</node-definition>\n"
            "  <template-project>\n"
            "    <template>template/rt_template.zip</template>\n"
            "  </template-project>\n"
            "</opensyde-target-support-package>\n";
   return c_Path.string();
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A well-formed package yields its device, comment, node path and template

   The loader is read-only (packages are authored by STW), so this is a parse pin rather than a round-trip.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(TargetSupportPackage, LoadsAllFields)
{
   const std::string c_Path = h_WriteTsp("osy_tsp_ok.syde_tsp", "3");
   C_OscTargetSupportPackage c_Tsp;
   std::string c_NodePath;
   const std::error_code c_Res = C_OscTargetSupportPackageFiler::h_Load(c_Tsp, c_NodePath, c_Path);

   ASSERT_FALSE(static_cast<bool>(c_Res)) << c_Res.message();
   EXPECT_EQ("RT-Device 1", c_Tsp.c_DeviceName);
   EXPECT_EQ("Round-trip & friends", c_Tsp.c_Comment);
   EXPECT_EQ("node/rt_device_1.syde_node", c_NodePath);
   EXPECT_EQ("template/rt_template.zip", c_Tsp.c_TemplatePath);
   (void)std::filesystem::remove(c_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Versions 1 and 2 are answered with "busy": use the other loader

   The V2 loader lives in opensyde_tsp_convert; the core loader only says which way to go. The core once carried a
   declaration for it too, with no definition behind it -- removed.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(TargetSupportPackage, OlderVersionsAreHandedToTheConverter)
{
   for (const char * const pcn_Version : {"1", "2"})
   {
      const std::string c_Path = h_WriteTsp("osy_tsp_old.syde_tsp", pcn_Version);
      C_OscTargetSupportPackage c_Tsp;
      std::string c_NodePath;
      EXPECT_EQ(Errc::busy, C_OscTargetSupportPackageFiler::h_Load(c_Tsp, c_NodePath, c_Path)) << "version " <<
         pcn_Version;
      (void)std::filesystem::remove(c_Path);
   }
}

TEST(TargetSupportPackage, UnknownVersionIsRefused)
{
   const std::string c_Path = h_WriteTsp("osy_tsp_v9.syde_tsp", "9");
   C_OscTargetSupportPackage c_Tsp;
   std::string c_NodePath;

   EXPECT_TRUE(static_cast<bool>(C_OscTargetSupportPackageFiler::h_Load(c_Tsp, c_NodePath, c_Path)));
   (void)std::filesystem::remove(c_Path);
}
