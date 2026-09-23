//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Characterisation tests for C_OscConfFileHandler

   Verifies the .conf parsing contract: comment/blank filtering, the missing-file
   error, and the replace-settings merge (update existing at original position,
   append missing keys).

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <filesystem>
#include "gtest/gtest.h"
#include "C_OscConfFileHandler.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
/// Concrete subclass capturing the settings handed to it after comment filtering
class C_TestConfHandler :
   public C_OscConfFileHandler
{
public:
   std::vector<std::string> c_Loaded;
   std::error_code c_LoadResult = Errc::success;

   std::error_code Replace(const std::string & orc_Path,
                           const std::vector<std::pair<std::string, std::string> > & orc_Configs)
   {
      return mh_ReplaceSettings(orc_Path, orc_Configs);
   }

   std::string GetConfigFilePath(void) const
   {
      return mc_ConfigFilePath;
   }

protected:
   std::error_code m_LoadSettings(const std::vector<std::string> & orc_SettingsWithoutComments) override
   {
      c_Loaded = orc_SettingsWithoutComments;
      return c_LoadResult;
   }
};

std::filesystem::path mh_TempFile(const std::string & orc_Name)
{
   return std::filesystem::temp_directory_path() / orc_Name;
}

void mh_Write(const std::filesystem::path & orc_Path, const std::string & orc_Content)
{
   std::ofstream c_File(orc_Path);
   c_File << orc_Content;
}

std::string mh_Read(const std::filesystem::path & orc_Path)
{
   std::ifstream c_File(orc_Path);
   std::string c_Content;
   std::string c_Line;
   while (std::getline(c_File, c_Line))
   {
      // ListSaveToFile writes CRLF; normalise so line-suffixed searches are stable
      if ((c_Line.empty() == false) && (c_Line.back() == '\r'))
      {
         c_Line.pop_back();
      }
      c_Content += c_Line + "\n";
   }
   return c_Content;
}
}

/* -- Tests --------------------------------------------------------------------------------------------------------- */

TEST(ConfFileHandler, LoadSettings_FiltersCommentsAndBlankLines)
{
   const std::filesystem::path c_Path = mh_TempFile("osy_conf_load.conf");
   mh_Write(c_Path,
            "# a full-line comment\n"
            "\n"
            "  \n"
            "Baudrate=500\n"
            "   \n"
            "NodeId=7\n"
            "# another comment\n"
            "IpAddress=10.0.0.5\n");

   C_TestConfHandler c_Handler;
   const std::error_code c_Result = c_Handler.LoadSettings(c_Path.string());

   EXPECT_EQ(Errc::success, c_Result);
   ASSERT_EQ(3U, c_Handler.c_Loaded.size());
   EXPECT_EQ("Baudrate=500", c_Handler.c_Loaded[0]);
   EXPECT_EQ("NodeId=7", c_Handler.c_Loaded[1]);
   EXPECT_EQ("IpAddress=10.0.0.5", c_Handler.c_Loaded[2]);
   EXPECT_EQ(c_Path.string(), c_Handler.GetConfigFilePath());

   std::filesystem::remove(c_Path);
}

TEST(ConfFileHandler, LoadSettings_MissingFileIsNoact)
{
   C_TestConfHandler c_Handler;
   const std::error_code c_Result = c_Handler.LoadSettings(
      (mh_TempFile("osy_conf_missing_does_not_exist.conf")).string());

   EXPECT_EQ(Errc::noact, c_Result);
   EXPECT_TRUE(c_Handler.c_Loaded.empty());
}

TEST(ConfFileHandler, ReplaceSettings_UpdatesExistingAndAppendsMissing)
{
   const std::filesystem::path c_Path = mh_TempFile("osy_conf_replace.conf");
   mh_Write(c_Path,
            "Baudrate=100\n"
            "NodeId=3\n");

   C_TestConfHandler c_Handler;
   std::vector<std::pair<std::string, std::string> > c_Configs;
   c_Configs.push_back(std::make_pair(std::string("Baudrate"), std::string("250"))); // existing -> replace in place
   c_Configs.push_back(std::make_pair(std::string("NewKey"), std::string("new")));   // missing  -> append

   EXPECT_EQ(Errc::success, c_Handler.Replace(c_Path.string(), c_Configs));

   const std::string c_Content = mh_Read(c_Path);
   // existing key updated at its original position
   EXPECT_NE(std::string::npos, c_Content.find("Baudrate=250\n"));
   EXPECT_NE(std::string::npos, c_Content.find("NodeId=3\n"));
   // missing key appended
   EXPECT_NE(std::string::npos, c_Content.find("NewKey=new\n"));

   std::filesystem::remove(c_Path);
}
