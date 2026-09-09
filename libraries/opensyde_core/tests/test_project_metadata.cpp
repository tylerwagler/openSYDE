//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscProject time handling

   Part of the `project` wave groundwork. h_GetTimeOfString parses the creation
   and modification timestamps stored in every .syde project, and h_GetTimeFormatted
   writes them back, so the pair has to round-trip or a project's history is
   rewritten every time it is opened and saved.

   The leading-zero cases are deliberate. This function parses with std::stoi,
   which is base 10, so "08" is 8. Switching it to base 0 -- an easy-looking
   "fix", and the exact thing ScanBaseCompat in C_SclStringUtil warns about --
   would make "08" an invalid octal literal and throw, silently resetting every
   August and September timestamp to the current time.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include "stwtypes.hpp"
#include "C_OscProject.hpp"
#include "C_SclDateTime.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(ProjectMetadata, TimeStringRoundTrips)
{
   const std::string c_Original = "24.12.2025 18:30";
   const stw::scl::C_SclDateTime c_Parsed = C_OscProject::h_GetTimeOfString(c_Original);

   EXPECT_EQ(24U, c_Parsed.mu16_Day);
   EXPECT_EQ(12U, c_Parsed.mu16_Month);
   EXPECT_EQ(2025U, c_Parsed.mu16_Year);
   EXPECT_EQ(18U, c_Parsed.mu16_Hour);
   EXPECT_EQ(30U, c_Parsed.mu16_Minute);

   EXPECT_EQ(c_Original, C_OscProject::h_GetTimeFormatted(c_Parsed))
      << "a project's timestamp would be rewritten on every save";
}

/// std::stoi is base 10, so "08" is 8. If this ever throws, someone moved the
/// parse to base 0 and every August/September timestamp now resets to now.
TEST(ProjectMetadata, LeadingZerosAreDecimalNotOctal)
{
   const stw::scl::C_SclDateTime c_Parsed = C_OscProject::h_GetTimeOfString("08.09.2025 08:09");

   EXPECT_EQ(8U, c_Parsed.mu16_Day);
   EXPECT_EQ(9U, c_Parsed.mu16_Month);
   EXPECT_EQ(2025U, c_Parsed.mu16_Year);
   EXPECT_EQ(8U, c_Parsed.mu16_Hour);
   EXPECT_EQ(9U, c_Parsed.mu16_Minute);
}

/// The documented contract is "current time if orc_Str is invalid" - it must not
/// propagate an exception out of a project load.
TEST(ProjectMetadata, MalformedStringFallsBackInsteadOfThrowing)
{
   const stw::scl::C_SclDateTime c_Now = stw::scl::C_SclDateTime::Now();

   const char * const apcn_Bad[] =
   {
      "",                       //empty
      "not a date",             //non-numeric
      "24.12.2025",             //too few fields
      "24.12.2025 18:30:45.1",  //too many fields
      "aa.bb.cccc dd:ee"        //right shape, wrong content
   };

   for (const char * const pcn_Bad : apcn_Bad)
   {
      stw::scl::C_SclDateTime c_Parsed;
      ASSERT_NO_THROW(c_Parsed = C_OscProject::h_GetTimeOfString(pcn_Bad)) << "input: " << pcn_Bad;

      //fell back to "now" - the year is the cheap, stable thing to check
      EXPECT_EQ(c_Now.mu16_Year, c_Parsed.mu16_Year) << "input: " << pcn_Bad;
   }
}

/// The separators are documented as interchangeable.
TEST(ProjectMetadata, SeparatorsAreInterchangeable)
{
   const stw::scl::C_SclDateTime c_Dots = C_OscProject::h_GetTimeOfString("01.02.2024 03:04");
   const stw::scl::C_SclDateTime c_Mixed = C_OscProject::h_GetTimeOfString("01 02 2024.03.04");

   EXPECT_EQ(c_Dots.mu16_Day, c_Mixed.mu16_Day);
   EXPECT_EQ(c_Dots.mu16_Month, c_Mixed.mu16_Month);
   EXPECT_EQ(c_Dots.mu16_Year, c_Mixed.mu16_Year);
   EXPECT_EQ(c_Dots.mu16_Hour, c_Mixed.mu16_Hour);
   EXPECT_EQ(c_Dots.mu16_Minute, c_Mixed.mu16_Minute);
}

/// Formatting drops seconds, so a parsed time always has them at zero and the
/// round trip stays stable.
TEST(ProjectMetadata, FormattedTimeHasNoSecondsField)
{
   const stw::scl::C_SclDateTime c_Parsed = C_OscProject::h_GetTimeOfString("24.12.2025 18:30");
   const std::string c_Formatted = C_OscProject::h_GetTimeFormatted(c_Parsed);

   EXPECT_EQ(0U, c_Parsed.mu16_Second);
   EXPECT_EQ(16U, c_Formatted.length()) << "expected exactly DD.MM.YYYY HH:MM";
}
