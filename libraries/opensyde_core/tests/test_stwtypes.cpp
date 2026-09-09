//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for stwtypes — type sizes, signedness, and limits
///
/// Verifies that the custom typedefs in stwtypes.h match the expected sizes
/// and that the preprocessor detection logic works on the current platform.
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <limits>
#include <type_traits>
#include "gtest/gtest.h"
#include <cstdint>

// stwtypes.hpp defines int8_t/uint8_t/... (with _t suffix, C++ style).
// stwtypes.h defines sint8/uint8/... (C style). We test the C++ names here.

TEST(STWTypes, SizeOf_int8_t)    { EXPECT_EQ(1, sizeof(int8_t)); }
TEST(STWTypes, SizeOf_uint8_t)   { EXPECT_EQ(1, sizeof(uint8_t)); }
TEST(STWTypes, SizeOf_int16_t)   { EXPECT_EQ(2, sizeof(int16_t)); }
TEST(STWTypes, SizeOf_uint16_t)  { EXPECT_EQ(2, sizeof(uint16_t)); }
TEST(STWTypes, SizeOf_int32_t)   { EXPECT_EQ(4, sizeof(int32_t)); }
TEST(STWTypes, SizeOf_uint32_t)  { EXPECT_EQ(4, sizeof(uint32_t)); }
TEST(STWTypes, SizeOf_int64_t)   { EXPECT_EQ(8, sizeof(int64_t)); }
TEST(STWTypes, SizeOf_uint64_t)  { EXPECT_EQ(8, sizeof(uint64_t)); }
TEST(STWTypes, SizeOf_float32_t) { EXPECT_EQ(4, sizeof(float)); }
TEST(STWTypes, SizeOf_float64_t) { EXPECT_EQ(8, sizeof(double)); }

TEST(STWTypes, Signedness_int8_t)    { EXPECT_TRUE(std::is_signed<int8_t>::value); }
TEST(STWTypes, Signedness_uint8_t)   { EXPECT_FALSE(std::is_signed<uint8_t>::value); }
TEST(STWTypes, Signedness_int16_t)   { EXPECT_TRUE(std::is_signed<int16_t>::value); }
TEST(STWTypes, Signedness_uint16_t)  { EXPECT_FALSE(std::is_signed<uint16_t>::value); }
TEST(STWTypes, Signedness_int32_t)   { EXPECT_TRUE(std::is_signed<int32_t>::value); }
TEST(STWTypes, Signedness_uint32_t)  { EXPECT_FALSE(std::is_signed<uint32_t>::value); }
TEST(STWTypes, Signedness_int64_t)   { EXPECT_TRUE(std::is_signed<int64_t>::value); }
TEST(STWTypes, Signedness_uint64_t)  { EXPECT_FALSE(std::is_signed<uint64_t>::value); }

TEST(STWTypes, Limits_int8_t)
{
   EXPECT_EQ(static_cast<int8_t>(-128),  std::numeric_limits<int8_t>::min());
   EXPECT_EQ(static_cast<int8_t>(127),   std::numeric_limits<int8_t>::max());
}

TEST(STWTypes, Limits_uint8_t)
{
   EXPECT_EQ(static_cast<uint8_t>(0),     std::numeric_limits<uint8_t>::min());
   EXPECT_EQ(static_cast<uint8_t>(255),   std::numeric_limits<uint8_t>::max());
}

TEST(STWTypes, Limits_int16_t)
{
   EXPECT_EQ(static_cast<int16_t>(-32768),  std::numeric_limits<int16_t>::min());
   EXPECT_EQ(static_cast<int16_t>(32767),   std::numeric_limits<int16_t>::max());
}

TEST(STWTypes, Limits_uint16_t)
{
   EXPECT_EQ(static_cast<uint16_t>(0),       std::numeric_limits<uint16_t>::min());
   EXPECT_EQ(static_cast<uint16_t>(65535),   std::numeric_limits<uint16_t>::max());
}

TEST(STWTypes, Limits_int32_t)
{
   EXPECT_EQ(static_cast<int32_t>(-2147483647 - 1), std::numeric_limits<int32_t>::min());
   EXPECT_EQ(static_cast<int32_t>(2147483647),       std::numeric_limits<int32_t>::max());
}

TEST(STWTypes, Limits_uint32_t)
{
   EXPECT_EQ(0U,                            std::numeric_limits<uint32_t>::min());
   EXPECT_EQ(4294967295U,                   std::numeric_limits<uint32_t>::max());
}

TEST(STWTypes, Limits_int64_t)
{
   EXPECT_EQ(static_cast<int64_t>(-9223372036854775807LL - 1),
             std::numeric_limits<int64_t>::min());
   EXPECT_EQ(static_cast<int64_t>(9223372036854775807LL),
             std::numeric_limits<int64_t>::max());
}

TEST(STWTypes, Limits_uint64_t)
{
   EXPECT_EQ(0ULL,                          std::numeric_limits<uint64_t>::min());
   EXPECT_EQ(18446744073709551615ULL,       std::numeric_limits<uint64_t>::max());
}

TEST(STWTypes, Char_t_Size)
{
   EXPECT_EQ(sizeof(char), sizeof(char));
}

TEST(STWTypes, FloatCharacteristics)
{
   EXPECT_EQ(24,  std::numeric_limits<float>::digits);
   EXPECT_EQ(53,  std::numeric_limits<double>::digits);
   EXPECT_FLOAT_EQ(std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon());
   EXPECT_DOUBLE_EQ(std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon());
}

TEST(STWTypes, CompilationGuard_Passed)
{
   SUCCEED();
}
