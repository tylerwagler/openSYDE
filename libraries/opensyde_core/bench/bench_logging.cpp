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
