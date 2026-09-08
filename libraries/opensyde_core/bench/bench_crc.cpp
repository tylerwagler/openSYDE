//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE core benchmark: CRC32 vs CRC-32C (hardware-accelerated)

   Phase 7.2 target. Compares the core's table-driven software CRC32
   (C_SclChecksums::CalcCRC32, poly 0xEDB88320) against the new
   C_SclChecksums::CalcCRC32C, which auto-selects an SSE4.2 _mm_crc32 fast path at
   runtime when the CPU supports it. Both produce identical bus traffic for their
   own polynomial; note the polynomials DIFFER (CalcCRC32C is Castagnoli / CRC-32C
   per its documentation), so the numbers here are the cost comparison, not proof
   of interchangeable results.
*/
//----------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <vector>

#include "benchmark/benchmark.h"

#include "C_SclChecksums.hpp"

namespace
{
std::vector<uint8_t> mh_MakeBuffer(benchmark::State & orc_State)
{
   const std::size_t u32_Size = static_cast<std::size_t>(orc_State.range(0));
   std::vector<uint8_t> c_Buffer(u32_Size);
   for (std::size_t u32_Index = 0U; u32_Index < u32_Size; u32_Index++)
   {
      c_Buffer[u32_Index] = static_cast<uint8_t>((u32_Index * 131U) & 0xFFU);
   }
   return c_Buffer;
}

void mh_ReportRate(benchmark::State & orc_State)
{
   orc_State.SetBytesProcessed(static_cast<int64_t>(orc_State.iterations()) *
                               static_cast<int64_t>(orc_State.range(0)));
}
}

static void BM_CRC32_Software(benchmark::State & orc_State)
{
   const std::vector<uint8_t> c_Buffer = mh_MakeBuffer(orc_State);
   uint32_t u32_Crc = 0xFFFFFFFFU;

   for (auto _ : orc_State)
   {
      stw::scl::C_SclChecksums::CalcCRC32(c_Buffer.data(), static_cast<uint32_t>(c_Buffer.size()), u32_Crc);
      benchmark::DoNotOptimize(u32_Crc);
   }
   mh_ReportRate(orc_State);
}
BENCHMARK(BM_CRC32_Software)
   ->RangeMultiplier(4)
   ->Range(1 << 10, 1 << 20);

static void BM_CRC32C_Auto(benchmark::State & orc_State)
{
   const std::vector<uint8_t> c_Buffer = mh_MakeBuffer(orc_State);
   uint32_t u32_Crc = 0xFFFFFFFFU;

   for (auto _ : orc_State)
   {
      stw::scl::C_SclChecksums::CalcCRC32C(c_Buffer.data(), static_cast<uint32_t>(c_Buffer.size()), u32_Crc);
      benchmark::DoNotOptimize(u32_Crc);
   }
   mh_ReportRate(orc_State);
}
BENCHMARK(BM_CRC32C_Auto)
   ->RangeMultiplier(4)
   ->Range(1 << 10, 1 << 20);
