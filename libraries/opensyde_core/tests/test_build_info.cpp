//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for C_OscBuildInfo -- the stamp that ties a binary to a commit
///
/// The values come from osy_build_info.hpp, generated from the git checkout by
/// osy_build_info.cmake before every build of the core. CI builds from a checkout, so
/// there the stamp must be a real describe string and a real ISO 8601 date; a build from
/// an exported tarball legitimately gets "unknown" for both, which is the only other
/// accepted shape.
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
///            Copyright 2026 Elytron Defense. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <cctype>
#include <cstring>
#include <regex>
#include <string>
#include "gtest/gtest.h"
#include "C_OscBuildInfo.hpp"

using stw::opensyde_core::C_OscBuildInfo;

namespace
{
bool mh_IsUnknown(const char * const opcn_Value)
{
   return std::strcmp(opcn_Value, "unknown") == 0;
}
}

TEST(BuildInfo, VersionControlIdIsDescribeOutputOrUnknown)
{
   const std::string c_Id = C_OscBuildInfo::h_GetVersionControlId();

   ASSERT_FALSE(c_Id.empty());
   if (mh_IsUnknown(c_Id.c_str()) == false)
   {
      // a tag, "tag-N-gHASH", or a bare short hash; optionally "-dirty". No whitespace, ever.
      const std::regex c_Shape("^[A-Za-z0-9._][A-Za-z0-9._/-]*(-dirty)?$");
      EXPECT_TRUE(std::regex_match(c_Id, c_Shape)) << c_Id;
      for (const char cn_Char : c_Id)
      {
         EXPECT_FALSE(std::isspace(static_cast<unsigned char>(cn_Char))) << c_Id;
      }
   }
}

TEST(BuildInfo, SourceDateIsStrictIso8601OrUnknown)
{
   const std::string c_Date = C_OscBuildInfo::h_GetSourceDate();

   ASSERT_FALSE(c_Date.empty());
   if (mh_IsUnknown(c_Date.c_str()) == false)
   {
      // git log --format=%cI: 2026-09-18T12:14:00+02:00 (or Z)
      const std::regex c_Shape("^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(Z|[+-][0-9]{2}:[0-9]{2})$");
      EXPECT_TRUE(std::regex_match(c_Date, c_Shape)) << c_Date;
   }
}

TEST(BuildInfo, BothValuesKnownOrBothUnknown)
{
   // A checkout yields both; a tarball yields neither. One without the other means the
   // generator half-failed, which is worth knowing about.
   EXPECT_EQ(mh_IsUnknown(C_OscBuildInfo::h_GetVersionControlId()),
             mh_IsUnknown(C_OscBuildInfo::h_GetSourceDate()));
}

TEST(BuildInfo, SummaryCarriesBothValues)
{
   const std::string c_Summary = C_OscBuildInfo::h_GetSummary();

   EXPECT_EQ(std::string(C_OscBuildInfo::h_GetVersionControlId()) + ", source dated " +
             C_OscBuildInfo::h_GetSourceDate(),
             c_Summary);
   EXPECT_NE(std::string::npos, c_Summary.find(C_OscBuildInfo::h_GetVersionControlId()));
   EXPECT_NE(std::string::npos, c_Summary.find(C_OscBuildInfo::h_GetSourceDate()));
}
