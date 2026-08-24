//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for C_OscErrorCategory — Errc enum and std::error_category integration
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "C_OscErrorCategory.hpp"

using namespace stw::errors;

TEST(OscErrorCategory, Errc_EnumValues)
{
   EXPECT_EQ(0,   static_cast<int>(Errc::success));
   EXPECT_EQ(-1,  static_cast<int>(Errc::unknown_err));
   EXPECT_EQ(-2,  static_cast<int>(Errc::warn));
   EXPECT_EQ(-3,  static_cast<int>(Errc::default_));
   EXPECT_EQ(-4,  static_cast<int>(Errc::busy));
   EXPECT_EQ(-5,  static_cast<int>(Errc::range));
   EXPECT_EQ(-6,  static_cast<int>(Errc::overflow));
   EXPECT_EQ(-7,  static_cast<int>(Errc::rd_wr));
   EXPECT_EQ(-8,  static_cast<int>(Errc::noact));
   EXPECT_EQ(-9,  static_cast<int>(Errc::com));
   EXPECT_EQ(-10, static_cast<int>(Errc::config));
   EXPECT_EQ(-11, static_cast<int>(Errc::checksum));
   EXPECT_EQ(-12, static_cast<int>(Errc::timeout));
}

TEST(OscErrorCategory, Category_Name)
{
   const std::error_category & rc_Cat = STWErrorCategory::Instance();
   EXPECT_STREQ("STW", rc_Cat.name());
}

TEST(OscErrorCategory, MakeErrorCode_Success)
{
   const std::error_code c_Ec = Errc::success;
   EXPECT_FALSE(c_Ec);
   EXPECT_EQ(0, c_Ec.value());
   EXPECT_STREQ("STW", c_Ec.category().name());
}

TEST(OscErrorCategory, MakeErrorCode_Config)
{
   const std::error_code c_Ec = Errc::config;
   EXPECT_TRUE(c_Ec);
   EXPECT_EQ(-10, c_Ec.value());
   EXPECT_STREQ("STW", c_Ec.category().name());
}

TEST(OscErrorCategory, MakeErrorCode_Checksum)
{
   const std::error_code c_Ec = Errc::checksum;
   EXPECT_TRUE(c_Ec);
   EXPECT_EQ(-11, c_Ec.value());
   EXPECT_STREQ("STW", c_Ec.category().name());
}

TEST(OscErrorCategory, Message_KnownCodes)
{
   const std::error_category & rc_Cat = STWErrorCategory::Instance();
   EXPECT_EQ("C_NO_ERR",   rc_Cat.message(0));
   EXPECT_EQ("C_UNKNOWN_ERR", rc_Cat.message(-1));
   EXPECT_EQ("C_WARN",     rc_Cat.message(-2));
   EXPECT_EQ("C_DEFAULT",  rc_Cat.message(-3));
   EXPECT_EQ("C_BUSY",     rc_Cat.message(-4));
   EXPECT_EQ("C_RANGE",    rc_Cat.message(-5));
   EXPECT_EQ("C_OVERFLOW", rc_Cat.message(-6));
   EXPECT_EQ("C_RD_WR",    rc_Cat.message(-7));
   EXPECT_EQ("C_NOACT",    rc_Cat.message(-8));
   EXPECT_EQ("C_COM",      rc_Cat.message(-9));
   EXPECT_EQ("C_CONFIG",   rc_Cat.message(-10));
   EXPECT_EQ("C_CHECKSUM", rc_Cat.message(-11));
   EXPECT_EQ("C_TIMEOUT",  rc_Cat.message(-12));
}

TEST(OscErrorCategory, Message_UnknownCode_ReturnsUnknown)
{
   const std::error_category & rc_Cat = STWErrorCategory::Instance();
   EXPECT_EQ("C_UNKNOWN_ERR", rc_Cat.message(-999));
   EXPECT_EQ("C_UNKNOWN_ERR", rc_Cat.message(42));
}

TEST(OscErrorCategory, Message_EdgeCases)
{
   const std::error_category & rc_Cat = STWErrorCategory::Instance();
   EXPECT_EQ("C_UNKNOWN_ERR", rc_Cat.message(-13));
   EXPECT_EQ("C_UNKNOWN_ERR", rc_Cat.message(1));
}

TEST(OscErrorCategory, ErrorCodeComparison)
{
   // Verify that std::error_code comparison works with Errc
   const std::error_code c_Ec1 = Errc::success;
   const std::error_code c_Ec2 = Errc::config;

   EXPECT_EQ(c_Ec1, Errc::success);
   EXPECT_NE(c_Ec1, Errc::config);
   EXPECT_EQ(c_Ec2, Errc::config);
}

TEST(OscErrorCategory, IsErrorCodeEnum)
{
   // Verify the trait specialisation works
   EXPECT_TRUE(std::is_error_code_enum<Errc>::value);
}

TEST(OscErrorCategory, ValueRoundTrip)
{
   // Verify that converting Errc -> error_code -> value -> Errc works
   for (int v = -12; v <= 0; ++v)
   {
      const std::error_code c_Ec(v, STWErrorCategory::Instance());
      EXPECT_EQ(v, c_Ec.value());
   }
}
