//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: Time functions

   cf. header for details

   Here: Implementation for Linux.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <unistd.h>
#include <ctime>
#include <cstdint>
#include "TglTime.hpp"
#include "TglUtils.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current date and time with milli seconds

   \param[out] orc_DateTime Current date and time value with milli seconds
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglGetDateTimeNow(C_TglDateTime & orc_DateTime)
{
   //localtime_r is ~290ns of this function's ~366ns, and its answer is by definition the
   //same for every call within one second, so it is evaluated once per second per thread.
   //The cache is thread_local rather than shared: it is two words of state, and a shared
   //one would need the lock this is trying to avoid.
   //
   //The cost is that a change in the local time rules -- a DST transition, or a process
   //calling tzset() after changing TZ -- is picked up on the next second boundary rather
   //than immediately. The staleness is bounded by one second because the cache key is the
   //absolute second the rules are being applied to, not a duration since the last lookup.
   static thread_local time_t hx_CachedSecond = 0;   //lint !e8080 //using type to match library API
   static thread_local bool hq_CacheValid = false;
   static thread_local C_TglDateTime hc_CachedFields;

   struct timespec c_TimeSpec;

   clock_gettime(CLOCK_REALTIME, &c_TimeSpec);

   // Make sure nsec is <1sec
   c_TimeSpec.tv_sec += (c_TimeSpec.tv_nsec / 1000000000);
   c_TimeSpec.tv_nsec = (c_TimeSpec.tv_nsec % 1000000000);

   const time_t x_UnixTime = c_TimeSpec.tv_sec; //lint !e8080 //using type to match library API

   if ((hq_CacheValid == false) || (hx_CachedSecond != x_UnixTime))
   {
      struct std::tm c_Time;
      const struct std::tm * const pc_LocalTime = localtime_r(&x_UnixTime, &c_Time);

      tgl_assert(pc_LocalTime != nullptr);
      if (pc_LocalTime != nullptr)
      {
         //Convert from tm format to TGL format
         hc_CachedFields.mu16_Year  = static_cast<uint16_t>(pc_LocalTime->tm_year + 1900);
         hc_CachedFields.mu8_Month  = static_cast<uint8_t>(pc_LocalTime->tm_mon + 1);
         hc_CachedFields.mu8_Day    = static_cast<uint8_t>(pc_LocalTime->tm_mday);
         hc_CachedFields.mu8_Hour   = static_cast<uint8_t>(pc_LocalTime->tm_hour);
         hc_CachedFields.mu8_Minute = static_cast<uint8_t>(pc_LocalTime->tm_min);
         hc_CachedFields.mu8_Second = static_cast<uint8_t>(pc_LocalTime->tm_sec);

         //Technically tm_sec could be > 59 in some rare cases due to leap seconds
         if (hc_CachedFields.mu8_Second > 59U)
         {
            hc_CachedFields.mu8_Second = 59U;
         }

         hx_CachedSecond = x_UnixTime;
         hq_CacheValid = true;
      }
   }

   if (hq_CacheValid == true)
   {
      orc_DateTime.mu16_Year  = hc_CachedFields.mu16_Year;
      orc_DateTime.mu8_Month  = hc_CachedFields.mu8_Month;
      orc_DateTime.mu8_Day    = hc_CachedFields.mu8_Day;
      orc_DateTime.mu8_Hour   = hc_CachedFields.mu8_Hour;
      orc_DateTime.mu8_Minute = hc_CachedFields.mu8_Minute;
      orc_DateTime.mu8_Second = hc_CachedFields.mu8_Second;
   }

   orc_DateTime.mu16_MilliSeconds = static_cast<uint16_t>(c_TimeSpec.tv_nsec / 1000000);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sleep for a number of milliseconds

   Delay for a number of milliseconds. Thread control shall meanwhile be passed on.
   i.e.: no active, blocking waiting.

   \param[in]    ou32_NumberMs    number of milliseconds to delay
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglSleep(const uint32_t ou32_NumberMs)
{
   usleep(static_cast<useconds_t>(ou32_NumberMs) * 1000);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sleep for a number of milliseconds

   A more use-case oriented sleep:
   Intended to be used while polling for more complex operations to be completed.
   Typical use: poll for completion of an ongoing confirmed communication with an external communication partner without
    actively blocking the CPU. Depending on the architecture an event mechanism might not be available to wait for.

   The goals of the function:
   * keep sleep time low in order for the communication procedure to be able to check for responses often
   * but: do not burden the CPU too much and give other threads CPU time

   The strategy chosen depends on the behavior of the target system. e.g.:
   * under Windows a "Sleep(0)" will provide a good compromise
   * under Linux "usleep(0)" can result in high CPU loads; using "usleep(1000)" should be acceptable
*/
//----------------------------------------------------------------------------------------------------------------------
void stw::tgl::TglSleepPolling()
{
   usleep(1000);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get system time in microseconds

   Return elapsed time since system start in micro-seconds as precise as possible.

   Hint:
   The starting point of CLOCK_MONOTONIC is not specified in the POSIX standard.
   However, for Linux the starting point is at boot time and its the same for all
   processes. Thus it can be used to get the system-up time.

   \return
   System time in micro-seconds.
*/
//----------------------------------------------------------------------------------------------------------------------
uint64_t stw::tgl::TglGetTickCountUs(void)
{
   int32_t s32_Error;
   uint64_t u64_TimeUs = 0;
   struct timespec c_Time;

   s32_Error = clock_gettime(CLOCK_MONOTONIC, &c_Time);
   if (s32_Error == 0)
   {
      u64_TimeUs = (static_cast<uint64_t>(c_Time.tv_sec) * 1000000U) +
                   ((static_cast<uint64_t>(c_Time.tv_nsec) + 500U) / 1000U);
   }
   return u64_TimeUs;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Return elapsed time since system start in ms.

   Return elapsed time since system start in milliseconds.

   Hint:
   The starting point of CLOCK_MONOTONIC is not specified in the POSIX standard.
   However, for Linux the starting point is at boot time and its the same for all
   processes. Thus it can be used to get the system-up time.

   \return
   System time in milliseconds
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t stw::tgl::TglGetTickCount(void)
{
   return static_cast<uint32_t>(TglGetTickCountUs() / 1000U);
}
