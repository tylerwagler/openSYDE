//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE core benchmark: string formatting idiom

   Phase 7.1 prerequisite: confirms std::format (the C++23 substitute chosen for
   the logging hot path) actually beats std::stringstream. snprintf is shown for
   reference.
*/
//----------------------------------------------------------------------------------------------------------------------

#include <cstdio>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>

#include "benchmark/benchmark.h"

 namespace
 {
struct C_Item
{
   int32_t s32_Id;
   std::string c_Name;
   double f64_Value;
};

const C_Item hc_Item = {42, "sensor_feed", 1234.5678};
}

static void BM_Format_StringStream(benchmark::State & orc_State)
{
   for (auto _ : orc_State)
   {
      std::ostringstream oss;
      oss << hc_Item.s32_Id << " " << hc_Item.c_Name << " " << std::fixed << std::setprecision(2) << hc_Item.f64_Value;
      benchmark::DoNotOptimize(oss.str());
   }
}
BENCHMARK(BM_Format_StringStream);

static void BM_Format_StdFormat(benchmark::State & orc_State)
{
   for (auto _ : orc_State)
   {
      const std::string c_Result = std::format("{} {} {:.2f}", hc_Item.s32_Id, hc_Item.c_Name, hc_Item.f64_Value);
      benchmark::DoNotOptimize(c_Result);
   }
}
BENCHMARK(BM_Format_StdFormat);

static void BM_Format_Snprintf(benchmark::State & orc_State)
{
   char acn_Buffer[128];
   for (auto _ : orc_State)
   {
      std::snprintf(acn_Buffer, sizeof(acn_Buffer), "%d %s %.2f", hc_Item.s32_Id, hc_Item.c_Name.c_str(),
                    hc_Item.f64_Value);
      benchmark::DoNotOptimize(acn_Buffer);
   }
}
BENCHMARK(BM_Format_Snprintf);
