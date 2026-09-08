//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE core benchmark: CRC32 software vs SSE4.2 hardware

   Phase 7.2 target. Compares the core's software table-based CRC32
   (C_SclChecksums::CalcCRC32, reflected poly 0xEDB88320) against a bench-local
   SSE4.2 _mm_crc32 reference, so the hardware acceleration claim can be measured
   before anything is landed in the core.

   NOTE for when 7.2 lands: _mm_crc32_u32/_u64 implement CRC-32C (Castagnoli,
   poly 0x1EDC6F41), which produces DIFFERENT values than the STW software CRC.
   The throughput win shown here is the motivation; the fix must either switch the
   checksum semantics or accept a new CRC (and invalidate stored values).
*/
//----------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <cstring>
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
   orc_State.SetBytesProcessed(static_cast<int64_t>(orc_State.iterations()) * static_cast<int64_t>(orc_State.range(0)));
}
BENCHMARK(BM_CRC32_Software)
   ->RangeMultiplier(4)
   ->Range(1 << 10, 1 << 20);

#ifdef __SSE4_2__
#include <nmmintrin.h>

static void BM_CRC32_SSE4_2(benchmark::State & orc_State)
{
   const std::vector<uint8_t> c_Buffer = mh_MakeBuffer(orc_State);

   for (auto _ : orc_State)
   {
      uint32_t u32_Crc = 0xFFFFFFFFU;
      const uint8_t * pu8_Pos = c_Buffer.data();
      const uint8_t * const pu8_End = pu8_Pos + c_Buffer.size();

      // 8 bytes per _mm_crc32_u64 (unaligned load via memcpy to stay well-defined).
      while ((pu8_Pos + 8) <= pu8_End)
      {
         uint64_t u64_Chunk;
         std::memcpy(&u64_Chunk, pu8_Pos, 8U);
         u32_Crc = _mm_crc32_u64(u32_Crc, u64_Chunk);
         pu8_Pos += 8U;
      }
      while (pu8_Pos < pu8_End)
      {
         u32_Crc = _mm_crc32_u8(u32_Crc, *pu8_Pos);
         pu8_Pos++;
      }
      benchmark::DoNotOptimize(u32_Crc);
   }
   orc_State.SetBytesProcessed(static_cast<int64_t>(orc_State.iterations()) * static_cast<int64_t>(orc_State.range(0)));
}
BENCHMARK(BM_CRC32_SSE4_2)
   ->RangeMultiplier(4)
   ->Range(1 << 10, 1 << 20);
#endif
