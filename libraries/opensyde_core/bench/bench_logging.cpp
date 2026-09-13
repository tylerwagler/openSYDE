//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE core benchmark: logging hot path

   Phase 7.1 target. Measures C_OscLoggingHandler::h_WriteLogInfo with the heavy
   sinks (file/console) disabled, so the number reflects the message-formatting
   and dispatch path that 7.1 addresses (std::stringstream -> std::format).
*/
//----------------------------------------------------------------------------------------------------------------------

#include <string>

#include "benchmark/benchmark.h"

#include "C_OscLoggingHandler.hpp"
#include "TglTime.hpp"

static void BM_WriteLogInfo(benchmark::State & orc_State)
{
   stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToConsoleActive(false);
   stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToFileActive(false);

   const std::string c_Activity = "BENCH";
   const std::string c_Message = "sample message with id=42 and value=1.2345";

   for (auto _ : orc_State)
   {
      stw::opensyde_core::C_OscLoggingHandler::h_WriteLogInfo(c_Activity, c_Message);
      benchmark::DoNotOptimize(c_Message);
   }
   orc_State.SetItemsProcessed(orc_State.iterations());
}

BENCHMARK(BM_WriteLogInfo);

/* Attribution probe: how much of BM_WriteLogInfo is the clock/timezone call
   rather than the formatting 7.1 targets. */
static void BM_GetDateTimeNow(benchmark::State & orc_State)
{
   for (auto _ : orc_State)
   {
      stw::tgl::C_TglDateTime c_DateTime;
      stw::tgl::TglGetDateTimeNow(c_DateTime);
      benchmark::DoNotOptimize(c_DateTime);
   }
}
BENCHMARK(BM_GetDateTimeNow);

static void BM_ConvertDateTimeToString(benchmark::State & orc_State)
{
   stw::tgl::C_TglDateTime c_DateTime;
   stw::tgl::TglGetDateTimeNow(c_DateTime);
   for (auto _ : orc_State)
   {
      benchmark::DoNotOptimize(
         stw::opensyde_core::C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_DateTime));
   }
}
BENCHMARK(BM_ConvertDateTimeToString);
