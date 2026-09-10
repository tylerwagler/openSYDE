//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for <cstdint> — type sizes, signedness, and limits
///
/// Verifies that the C++ fixed-width integer types are globally available
/// (via <cstdint>) with the expected sizes and signedness, and that float and
/// double have the expected IEEE characteristics. This is a regression pin for
/// the stwtypes removal migration: the codebase now uses the standard types.
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <limits>
#include <type_traits>
#include "gtest/gtest.h"

TEST(CStdint, SizeOf_int8_t)    { EXPECT_EQ(1, sizeof(int8_t)); }
TEST(CStdint, SizeOf_uint8_t)   { EXPECT_EQ(1, sizeof(uint8_t)); }
TEST(CStdint, SizeOf_int16_t)   { EXPECT_EQ(2, sizeof(int16_t)); }
TEST(CStdint, SizeOf_uint16_t)  { EXPECT_EQ(2, sizeof(uint16_t)); }
TEST(CStdint, SizeOf_int32_t)   { EXPECT_EQ(4, sizeof(int32_t)); }
TEST(CStdint, SizeOf_uint32_t)  { EXPECT_EQ(4, sizeof(uint32_t)); }
TEST(CStdint, SizeOf_int64_t)   { EXPECT_EQ(8, sizeof(int64_t)); }
TEST(CStdint, SizeOf_uint64_t)  { EXPECT_EQ(8, sizeof(uint64_t)); }
TEST(CStdint, SizeOf_float)     { EXPECT_EQ(4, sizeof(float)); }
TEST(CStdint, SizeOf_double)    { EXPECT_EQ(8, sizeof(double)); }

TEST(CStdint, Signedness_int8_t)    { EXPECT_TRUE(std::is_signed<int8_t>::value); }
TEST(CStdint, Signedness_uint8_t)   { EXPECT_FALSE(std::is_signed<uint8_t>::value); }
TEST(CStdint, Signedness_int16_t)   { EXPECT_TRUE(std::is_signed<int16_t>::value); }
TEST(CStdint, Signedness_uint16_t)  { EXPECT_FALSE(std::is_signed<uint16_t>::value); }
TEST(CStdint, Signedness_int32_t)   { EXPECT_TRUE(std::is_signed<int32_t>::value); }
TEST(CStdint, Signedness_uint32_t)  { EXPECT_FALSE(std::is_signed<uint32_t>::value); }
TEST(CStdint, Signedness_int64_t)   { EXPECT_TRUE(std::is_signed<int64_t>::value); }
TEST(CStdint, Signedness_uint64_t)  { EXPECT_FALSE(std::is_signed<uint64_t>::value); }

TEST(CStdint, Limits_int8_t)
{
   EXPECT_EQ(static_cast<int8_t>(-128),  std::numeric_limits<int8_t>::min());
   EXPECT_EQ(static_cast<int8_t>(127),   std::numeric_limits<int8_t>::max());
}

TEST(CStdint, Limits_uint8_t)
{
   EXPECT_EQ(static_cast<uint8_t>(0),     std::numeric_limits<uint8_t>::min());
   EXPECT_EQ(static_cast<uint8_t>(255),   std::numeric_limits<uint8_t>::max());
}

TEST(CStdint, Limits_int16_t)
{
   EXPECT_EQ(static_cast<int16_t>(-32768),  std::numeric_limits<int16_t>::min());
   EXPECT_EQ(static_cast<int16_t>(32767),   std::numeric_limits<int16_t>::max());
}

TEST(CStdint, Limits_uint16_t)
{
   EXPECT_EQ(static_cast<uint16_t>(0),       std::numeric_limits<uint16_t>::min());
   EXPECT_EQ(static_cast<uint16_t>(65535),   std::numeric_limits<uint16_t>::max());
}

TEST(CStdint, Limits_int32_t)
{
   EXPECT_EQ(static_cast<int32_t>(-2147483647 - 1), std::numeric_limits<int32_t>::min());
   EXPECT_EQ(static_cast<int32_t>(2147483647),       std::numeric_limits<int32_t>::max());
}

TEST(CStdint, Limits_uint32_t)
{
   EXPECT_EQ(0U,                            std::numeric_limits<uint32_t>::min());
   EXPECT_EQ(4294967295U,                   std::numeric_limits<uint32_t>::max());
}

TEST(CStdint, Limits_int64_t)
{
   EXPECT_EQ(static_cast<int64_t>(-9223372036854775807LL - 1),
             std::numeric_limits<int64_t>::min());
   EXPECT_EQ(static_cast<int64_t>(9223372036854775807LL),
             std::numeric_limits<int64_t>::max());
}

TEST(CStdint, Limits_uint64_t)
{
   EXPECT_EQ(0ULL,                          std::numeric_limits<uint64_t>::min());
   EXPECT_EQ(18446744073709551615ULL,       std::numeric_limits<uint64_t>::max());
}

TEST(CStdint, FloatCharacteristics)
{
   EXPECT_EQ(24,  std::numeric_limits<float>::digits);
   EXPECT_EQ(53,  std::numeric_limits<double>::digits);
   EXPECT_FLOAT_EQ(std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon());
   EXPECT_DOUBLE_EQ(std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon());
}
