#include "gtest/gtest.h"
#include <string>
#include "C_SclStringCompat.hpp"

using namespace stw::scl;

TEST(SclString, IntToHex_Zero)
{
   EXPECT_EQ("0", IntToHexCompat(0x0U, 1));
}

TEST(SclString, IntToHex_OneDigit)
{
   EXPECT_EQ("F", IntToHexCompat(0xFU, 1));
}

TEST(SclString, IntToHex_Padded)
{
   EXPECT_EQ("0010", IntToHexCompat(0x10U, 4));
}

TEST(SclString, IntToHex_LargeValue)
{
   EXPECT_EQ("ABCD", IntToHexCompat(0xABCDU, 4));
}

TEST(SclString, IntToHex_32Bit)
{
   EXPECT_EQ("DEADBEEF", IntToHexCompat(0xDEADBEEFU, 8));
}

TEST(SclString, IntToHex_Max32)
{
   EXPECT_EQ("FFFFFFFF", IntToHexCompat(0xFFFFFFFFU, 8));
}

TEST(SclString, IntToHex_MoreDigitsThanNeeded)
{
   EXPECT_EQ("0012", IntToHexCompat(0x12U, 4));
}

TEST(SclString, IntToHex_ZeroDigits)
{
   EXPECT_EQ("0", IntToHexCompat(0x0U, 0));
}

TEST(SclString, IntToStr_Integer)
{
   EXPECT_EQ("42", std::to_string(42));
   EXPECT_EQ("-1", std::to_string(-1));
   EXPECT_EQ("0", std::to_string(0));
}

TEST(SclString, SubString)
{
   std::string s("abcdef");
   EXPECT_EQ("bcd", SubStringCompat(s, 2, 3));
}

TEST(SclString, Pos)
{
   std::string s("hello world");
   EXPECT_EQ(3U, PosCompat(s, "llo"));
   EXPECT_EQ(0U, PosCompat(s, "xyz"));
}

TEST(SclString, Trim)
{
   std::string s("  hello  ");
   EXPECT_EQ("hello", TrimCompat(s));
}

TEST(SclString, UpperCase)
{
   std::string s("Hello");
   EXPECT_EQ("HELLO", UpperCaseCompat(s));
}

TEST(SclString, LowerCase)
{
   std::string s("Hello");
   EXPECT_EQ("hello", LowerCaseCompat(s));
}

TEST(SclString, IsEmpty)
{
   std::string empty;
   EXPECT_TRUE(empty.empty());
   std::string non_empty("x");
   EXPECT_FALSE(non_empty.empty());
}

TEST(SclString, Length)
{
   std::string s("hello");
   EXPECT_EQ(5U, s.length());
   EXPECT_EQ(0U, std::string().length());
}
