#include <set>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "C_OscLoggingHandler.hpp"
#include "TglTime.hpp"

TEST(Logging, StartPerformanceTimer_ReturnsNonZeroId)
{
   const uint16_t u16_Id = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
   // Any uint16_t value is valid; just verify it doesn't crash
   EXPECT_NO_FATAL_FAILURE((void)u16_Id);
}

TEST(Logging, StartPerformanceTimer_IdsAreUnique)
{
   std::set<uint16_t> c_Ids;
   for (int32_t i = 0; i < 100; ++i)
   {
      const uint16_t u16_Id = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
      const auto c_Result = c_Ids.insert(u16_Id);
      EXPECT_TRUE(c_Result.second) << "Duplicate timer ID: " << u16_Id;
   }
}

TEST(Logging, StartPerformanceTimer_IdsIncreaseMonotonically)
{
   uint16_t u16_Prev = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
   for (int32_t i = 0; i < 50; ++i)
   {
      const uint16_t u16_Curr = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
      // With atomic increment, IDs should be strictly increasing (modulo wraparound)
      if (u16_Curr > u16_Prev)
      {
         const uint16_t u16_Diff = u16_Curr - u16_Prev;
         EXPECT_EQ(1U, u16_Diff);
      }
      // If wraparound occurred, just note it
      u16_Prev = u16_Curr;
   }
}

TEST(Logging, StwError_Consistency)
{
   // Verify that every defined error code maps to its own name
   EXPECT_EQ("C_NO_ERR",   stw::opensyde_core::C_OscLoggingHandler::h_StwError(0));
   EXPECT_EQ("C_UNKNOWN_ERR", stw::opensyde_core::C_OscLoggingHandler::h_StwError(-1));
   EXPECT_EQ("C_WARN",     stw::opensyde_core::C_OscLoggingHandler::h_StwError(-2));
   EXPECT_EQ("C_DEFAULT",  stw::opensyde_core::C_OscLoggingHandler::h_StwError(-3));
   EXPECT_EQ("C_BUSY",     stw::opensyde_core::C_OscLoggingHandler::h_StwError(-4));
   EXPECT_EQ("C_RANGE",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(-5));
   EXPECT_EQ("C_OVERFLOW", stw::opensyde_core::C_OscLoggingHandler::h_StwError(-6));
   EXPECT_EQ("C_RD_WR",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(-7));
   EXPECT_EQ("C_NOACT",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(-8));
   EXPECT_EQ("C_COM",      stw::opensyde_core::C_OscLoggingHandler::h_StwError(-9));
   EXPECT_EQ("C_CONFIG",   stw::opensyde_core::C_OscLoggingHandler::h_StwError(-10));
   EXPECT_EQ("C_CHECKSUM", stw::opensyde_core::C_OscLoggingHandler::h_StwError(-11));
   EXPECT_EQ("C_TIMEOUT",  stw::opensyde_core::C_OscLoggingHandler::h_StwError(-12));
}

/* -- Date/time formatting ------------------------------------------------------------------------------------------ */
/* Pins the exact log timestamp format. It is the prefix of every log line, is parsed
   back out of log file names, and was reimplemented with std::format in phase 7.1 --
   previously nothing covered its output at all. */

namespace
{
stw::tgl::C_TglDateTime mh_MakeDateTime(const uint16_t ou16_Year, const uint8_t ou8_Month, const uint8_t ou8_Day,
                                        const uint8_t ou8_Hour, const uint8_t ou8_Minute, const uint8_t ou8_Second,
                                        const uint16_t ou16_MilliSeconds)
{
   stw::tgl::C_TglDateTime c_Retval;

   c_Retval.mu16_Year = ou16_Year;
   c_Retval.mu8_Month = ou8_Month;
   c_Retval.mu8_Day = ou8_Day;
   c_Retval.mu8_Hour = ou8_Hour;
   c_Retval.mu8_Minute = ou8_Minute;
   c_Retval.mu8_Second = ou8_Second;
   c_Retval.mu16_MilliSeconds = ou16_MilliSeconds;
   return c_Retval;
}
}

TEST(Logging, ConvertDateTimeToString_DocumentedExample)
{
   //The format documented on the function itself
   const stw::tgl::C_TglDateTime c_DateTime = mh_MakeDateTime(2017U, 8U, 29U, 7U, 32U, 19U, 123U);

   EXPECT_EQ("2017-08-29 07:32:19.123",
             stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_DateTime));
}

TEST(Logging, ConvertDateTimeToString_ZeroPadsEveryField)
{
   //Single digit fields must pad, and milliseconds pad to three digits
   const stw::tgl::C_TglDateTime c_DateTime = mh_MakeDateTime(2026U, 1U, 2U, 3U, 4U, 5U, 6U);

   EXPECT_EQ("2026-01-02 03:04:05.006",
             stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_DateTime));
}

TEST(Logging, ConvertDateTimeToString_PadsYearAndKeepsFixedWidth)
{
   const stw::tgl::C_TglDateTime c_Early = mh_MakeDateTime(7U, 12U, 31U, 23U, 59U, 59U, 999U);

   EXPECT_EQ("0007-12-31 23:59:59.999",
             stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_Early));
   //Every rendering is the same width, which the fixed-width log columns rely on
   EXPECT_EQ(23U, stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_Early).size());
}

/* -- Log line assembly --------------------------------------------------------------------------------------------- */
/* The full line format (column widths, and how __FILE__/__func__ become "Class::Function")
   was untested; phase 7.1 rewrote both, so pin them here. Capture is via cout redirection
   because the console sink is the observable output. */

namespace
{
std::string mh_CaptureLogLine(const std::string & orc_Activity, const std::string & orc_Message,
                              const char * const opcn_Class, const char * const opcn_Function)
{
   std::ostringstream c_Captured;
   std::streambuf * const pc_Original = std::cout.rdbuf(c_Captured.rdbuf());

   stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToFileActive(false);
   stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToConsoleActive(true);
   stw::opensyde_core::C_OscLoggingHandler::h_WriteLogInfo(orc_Activity, orc_Message, opcn_Class, opcn_Function);
   std::cout.rdbuf(pc_Original);
   stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToConsoleActive(false);
   return c_Captured.str();
}
}

TEST(Logging, LogLine_DerivesClassFromFilePathAndKeepsColumns)
{
   const std::string c_Line = mh_CaptureLogLine("Startup", "Application started.",
                                                "/some/dir/C_OscFoo.cpp", "DoWork");

   //Directories and the extension are stripped, and the function is qualified with the class
   EXPECT_NE(std::string::npos, c_Line.find("C_OscFoo::DoWork"));
   EXPECT_NE(std::string::npos, c_Line.find("INFO"));
   EXPECT_NE(std::string::npos, c_Line.find("Startup"));
   EXPECT_NE(std::string::npos, c_Line.find("Application started."));
   //The columns after the timestamp are pinned byte for byte: the widths are what keeps
   //existing log files readable, and 7.1 rewrote the assembly.
   EXPECT_EQ(0U, c_Line.find("20")); //starts with the timestamp
   EXPECT_EQ("  "                                                       //timestamp padding to 25
             "INFO     "                                                //type, width 7, + 2
             "Startup                     "                             //activity, width 26, + 2
             "C_OscFoo::DoWork                                      "    //class::function, 52, + 2
             "Application started.\n",
             c_Line.substr(23U));
}

TEST(Logging, LogLine_DoesNotDuplicateClassNameFromMsvcFunctionMacro)
{
   //Older MSVC __FUNCTION__ expands to Class::Function; the class must not appear twice
   const std::string c_Line = mh_CaptureLogLine("Act", "Msg", "C_OscFoo.cpp", "C_OscFoo::DoWork");

   EXPECT_NE(std::string::npos, c_Line.find("C_OscFoo::DoWork"));
   EXPECT_EQ(std::string::npos, c_Line.find("C_OscFoo::C_OscFoo"));
}

TEST(Logging, LogLine_FallsBackWhenClassAndFunctionMissing)
{
   const std::string c_Line = mh_CaptureLogLine("Act", "Msg", NULL, NULL);

   EXPECT_NE(std::string::npos, c_Line.find("UNKNOWN_CLASS::UNKNOWN_FUNCTION"));
}

TEST(Logging, ConvertDateTimeToString_WidensRatherThanTruncatingAFiveDigitYear)
{
   //The year field is a uint16_t, so a value past 9999 is representable. Losing a digit
   //would silently misdate the log, so the rendering widens instead.
   const stw::tgl::C_TglDateTime c_Far = mh_MakeDateTime(10000U, 6U, 15U, 12U, 0U, 0U, 500U);

   EXPECT_EQ("10000-06-15 12:00:00.500",
             stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_Far));
}
