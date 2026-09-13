//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unit tests for the TGL time functions

   TglGetDateTimeNow caches the local-time breakdown for one second at a time, so
   the tests that matter are the ones that would catch a stale cache: the result is
   compared against an independent conversion on every call, across at least one
   second boundary.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <ctime>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "TglTime.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::tgl;

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
//Independent reference conversion, deliberately not sharing code with the implementation
std::tm mh_ReferenceLocalTime(void)
{
   const std::time_t x_Now = std::time(NULL);
   std::tm c_Result;

#ifdef _WIN32
   localtime_s(&c_Result, &x_Now);
#else
   localtime_r(&x_Now, &c_Result);
#endif
   return c_Result;
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(TglTime, GetDateTimeNow_FieldsAreInRange)
{
   C_TglDateTime c_DateTime;

   TglGetDateTimeNow(c_DateTime);

   EXPECT_GE(c_DateTime.mu16_Year, 2020U); //the clock is not set to the last century
   EXPECT_GE(c_DateTime.mu8_Month, 1U);
   EXPECT_LE(c_DateTime.mu8_Month, 12U);
   EXPECT_GE(c_DateTime.mu8_Day, 1U);
   EXPECT_LE(c_DateTime.mu8_Day, 31U);
   EXPECT_LE(c_DateTime.mu8_Hour, 23U);
   EXPECT_LE(c_DateTime.mu8_Minute, 59U);
   EXPECT_LE(c_DateTime.mu8_Second, 59U); //never 60, even on a leap second
   EXPECT_LE(c_DateTime.mu16_MilliSeconds, 999U);
}

TEST(TglTime, GetDateTimeNow_MatchesAnIndependentConversionAcrossASecondBoundary)
{
   //The point of the test: the implementation caches the local-time breakdown per
   //second, so a cache that failed to invalidate would pass any single-call check and
   //fail here. Runs for over a second so at least one boundary is crossed.
   const std::chrono::steady_clock::time_point c_Deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(1200);
   bool q_SawSecondChange = false;
   uint8_t u8_PreviousSecond = 0xFFU;
   uint32_t u32_Calls = 0U;

   while (std::chrono::steady_clock::now() < c_Deadline)
   {
      C_TglDateTime c_DateTime;
      TglGetDateTimeNow(c_DateTime);
      const std::tm c_Reference = mh_ReferenceLocalTime();

      //Read both as close together as possible; if they straddle a second boundary the
      //seconds can legitimately differ by one, so only a larger gap is a failure.
      const int32_t s32_Delta =
         static_cast<int32_t>(c_DateTime.mu8_Second) - static_cast<int32_t>(c_Reference.tm_sec);
      const bool q_SecondsAgree = (s32_Delta == 0) || (s32_Delta == 1) || (s32_Delta == -1) ||
                                  (s32_Delta == 59) || (s32_Delta == -59); //wrap at the minute

      EXPECT_TRUE(q_SecondsAgree) << "cached " << static_cast<uint32_t>(c_DateTime.mu8_Second)
                                  << " vs reference " << c_Reference.tm_sec;
      EXPECT_EQ(c_DateTime.mu16_Year, static_cast<uint16_t>(c_Reference.tm_year + 1900));
      EXPECT_EQ(c_DateTime.mu8_Month, static_cast<uint8_t>(c_Reference.tm_mon + 1));
      EXPECT_EQ(c_DateTime.mu8_Day, static_cast<uint8_t>(c_Reference.tm_mday));
      EXPECT_EQ(c_DateTime.mu8_Hour, static_cast<uint8_t>(c_Reference.tm_hour));

      if ((u8_PreviousSecond != 0xFFU) && (c_DateTime.mu8_Second != u8_PreviousSecond))
      {
         q_SawSecondChange = true;
      }
      u8_PreviousSecond = c_DateTime.mu8_Second;
      u32_Calls++;
   }

   //Proves the loop actually exercised an invalidation rather than sitting in one second
   EXPECT_TRUE(q_SawSecondChange);
   EXPECT_GT(u32_Calls, 1U);
}

TEST(TglTime, GetDateTimeNow_MillisecondsAdvanceWithinOneSecond)
{
   //Milliseconds are not cached -- they come from the clock on every call -- so they must
   //still move while the cached second stands still.
   C_TglDateTime c_First;
   C_TglDateTime c_Second;

   TglGetDateTimeNow(c_First);
   std::this_thread::sleep_for(std::chrono::milliseconds(50));
   TglGetDateTimeNow(c_Second);

   const int32_t s32_FirstTotal = (static_cast<int32_t>(c_First.mu8_Second) * 1000) +
                                  static_cast<int32_t>(c_First.mu16_MilliSeconds);
   const int32_t s32_SecondTotal = (static_cast<int32_t>(c_Second.mu8_Second) * 1000) +
                                   static_cast<int32_t>(c_Second.mu16_MilliSeconds);

   //Allow for the minute wrapping the second field back to zero
   const int32_t s32_Elapsed = (s32_SecondTotal >= s32_FirstTotal) ?
                               (s32_SecondTotal - s32_FirstTotal) :
                               ((s32_SecondTotal + 60000) - s32_FirstTotal);

   EXPECT_GE(s32_Elapsed, 40);
   EXPECT_LT(s32_Elapsed, 5000);
}

TEST(TglTime, GetDateTimeNow_IsStableWhenCalledRepeatedlyInOneSecond)
{
   //A cache hit must return the same breakdown, not a partially updated one
   C_TglDateTime c_Reference;

   TglGetDateTimeNow(c_Reference);

   for (uint32_t u32_Index = 0U; u32_Index < 1000U; u32_Index++)
   {
      C_TglDateTime c_Current;
      TglGetDateTimeNow(c_Current);

      if (c_Current.mu8_Second == c_Reference.mu8_Second)
      {
         EXPECT_EQ(c_Current.mu16_Year, c_Reference.mu16_Year);
         EXPECT_EQ(c_Current.mu8_Month, c_Reference.mu8_Month);
         EXPECT_EQ(c_Current.mu8_Day, c_Reference.mu8_Day);
         EXPECT_EQ(c_Current.mu8_Hour, c_Reference.mu8_Hour);
         EXPECT_EQ(c_Current.mu8_Minute, c_Reference.mu8_Minute);
      }
   }
}

TEST(TglTime, GetDateTimeNow_EachThreadKeepsItsOwnCache)
{
   //The cache is thread_local; a thread starting cold must still get the right answer
   C_TglDateTime c_Main;

   TglGetDateTimeNow(c_Main);

   C_TglDateTime c_Worker;
   std::thread c_Thread([&c_Worker]() { TglGetDateTimeNow(c_Worker); });
   c_Thread.join();

   EXPECT_EQ(c_Worker.mu16_Year, c_Main.mu16_Year);
   EXPECT_EQ(c_Worker.mu8_Month, c_Main.mu8_Month);
   EXPECT_EQ(c_Worker.mu8_Day, c_Main.mu8_Day);
   EXPECT_EQ(c_Worker.mu8_Hour, c_Main.mu8_Hour);
}
