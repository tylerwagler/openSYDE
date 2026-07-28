#include "gtest/gtest.h"
#include "C_SclString.hpp"

using stw::scl::C_SclString;

TEST(SclString, IntToHex_Zero)
{
   EXPECT_EQ("0", std::string(C_SclString::IntToHex(0x0U, 1).c_str()));
}

TEST(SclString, IntToHex_OneDigit)
{
   EXPECT_EQ("F", std::string(C_SclString::IntToHex(0xFU, 1).c_str()));
}

TEST(SclString, IntToHex_Padded)
{
   EXPECT_EQ("0010", std::string(C_SclString::IntToHex(0x10U, 4).c_str()));
}

TEST(SclString, IntToHex_LargeValue)
{
   EXPECT_EQ("ABCD", std::string(C_SclString::IntToHex(0xABCDU, 4).c_str()));
}

TEST(SclString, IntToHex_32Bit)
{
   EXPECT_EQ("DEADBEEF", std::string(C_SclString::IntToHex(0xDEADBEEFU, 8).c_str()));
}

TEST(SclString, IntToHex_Max32)
{
   EXPECT_EQ("FFFFFFFF", std::string(C_SclString::IntToHex(0xFFFFFFFFU, 8).c_str()));
}

TEST(SclString, IntToHex_MoreDigitsThanNeeded)
{
   EXPECT_EQ("0012", std::string(C_SclString::IntToHex(0x12U, 4).c_str()));
}

TEST(SclString, IntToHex_ZeroDigits)
{
   // ou32_Digits=0: setw(0) means no minimum width, no padding
   EXPECT_EQ("0", std::string(C_SclString::IntToHex(0x0U, 0).c_str()));
}

TEST(SclString, IntToStr_Integer)
{
   EXPECT_EQ("42", std::string(C_SclString::IntToStr(42).c_str()));
   EXPECT_EQ("-1", std::string(C_SclString::IntToStr(-1).c_str()));
   EXPECT_EQ("0", std::string(C_SclString::IntToStr(0).c_str()));
}

TEST(SclString, IntToStr_Large)
{
   EXPECT_EQ("2147483647", std::string(C_SclString::IntToStr(2147483647).c_str()));
   EXPECT_EQ("-2147483648", std::string(C_SclString::IntToStr(-2147483647 - 1).c_str()));
}

TEST(SclString, Construction_FromCString)
{
   C_SclString s("hello");
   EXPECT_EQ("hello", std::string(s.c_str()));
}

TEST(SclString, Construction_FromInt)
{
   C_SclString s(123);
   EXPECT_EQ("123", std::string(s.c_str()));
}

TEST(SclString, Construction_FromUint8)
{
   // uint8_t should be treated as a number, not a character
   C_SclString s(static_cast<uint8_t>(65));
   EXPECT_EQ("65", std::string(s.c_str()));
}

TEST(SclString, Concatenation)
{
   C_SclString a("hello ");
   C_SclString b("world");
   C_SclString c = a + b;
   EXPECT_EQ("hello world", std::string(c.c_str()));
}

TEST(SclString, SubString)
{
   C_SclString s("abcdef");
   EXPECT_EQ("bcd", std::string(s.SubString(2, 3).c_str()));
}

TEST(SclString, Pos)
{
   C_SclString s("hello world");
   EXPECT_EQ(3U, s.Pos("llo"));
   EXPECT_EQ(0U, s.Pos("xyz"));
}

TEST(SclString, Trim)
{
   C_SclString s("  hello  ");
   EXPECT_EQ("hello", std::string(s.Trim().c_str()));
}

TEST(SclString, UpperCase)
{
   C_SclString s("Hello");
   EXPECT_EQ("HELLO", std::string(s.UpperCase().c_str()));
}

TEST(SclString, LowerCase)
{
   C_SclString s("Hello");
   EXPECT_EQ("hello", std::string(s.LowerCase().c_str()));
}

TEST(SclString, ToInt)
{
   C_SclString s("42");
   EXPECT_EQ(42, s.ToInt());
}

TEST(SclString, ToInt_Negative)
{
   C_SclString s("-42");
   EXPECT_EQ(-42, s.ToInt());
}

TEST(SclString, ToDouble)
{
   C_SclString s("3.14");
   EXPECT_DOUBLE_EQ(3.14, s.ToDouble());
}

TEST(SclString, IsEmpty)
{
   C_SclString empty;
   EXPECT_TRUE(empty.IsEmpty());
   C_SclString non_empty("x");
   EXPECT_FALSE(non_empty.IsEmpty());
}

TEST(SclString, Length)
{
   C_SclString s("hello");
   EXPECT_EQ(5U, s.Length());
   EXPECT_EQ(0U, C_SclString().Length());
}
