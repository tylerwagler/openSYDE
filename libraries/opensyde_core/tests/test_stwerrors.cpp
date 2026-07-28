//----------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief  Unit tests for stwerrors — constant values and h_StwError string mapping
///
/// \copyright Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "stwerrors.hpp"
#include "C_OscLoggingHandler.hpp"

using namespace stw::errors;

TEST(STWErrors, Constants_Values)
{
   EXPECT_EQ(0,   C_NO_ERR);
   EXPECT_EQ(-1,  C_UNKNOWN_ERR);
   EXPECT_EQ(-2,  C_WARN);
   EXPECT_EQ(-3,  C_DEFAULT);
   EXPECT_EQ(-4,  C_BUSY);
   EXPECT_EQ(-5,  C_RANGE);
   EXPECT_EQ(-6,  C_OVERFLOW);
   EXPECT_EQ(-7,  C_RD_WR);
   EXPECT_EQ(-8,  C_NOACT);
   EXPECT_EQ(-9,  C_COM);
   EXPECT_EQ(-10, C_CONFIG);
   EXPECT_EQ(-11, C_CHECKSUM);
   EXPECT_EQ(-12, C_TIMEOUT);
}

TEST(STWErrors, Constants_Type)
{
   EXPECT_TRUE((std::is_same<const int32_t, decltype(C_NO_ERR)>::value));
   EXPECT_TRUE((std::is_same<const int32_t, decltype(C_COM)>::value));
}

TEST(STWErrors, StwError_KnownCodes)
{
   EXPECT_EQ("C_NO_ERR",   stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_NO_ERR));
   EXPECT_EQ("C_WARN",     stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_WARN));
   EXPECT_EQ("C_DEFAULT",  stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_DEFAULT));
   EXPECT_EQ("C_BUSY",     stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_BUSY));
   EXPECT_EQ("C_RANGE",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_RANGE));
   EXPECT_EQ("C_OVERFLOW", stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_OVERFLOW));
   EXPECT_EQ("C_NOACT",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_NOACT));
   EXPECT_EQ("C_COM",      stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_COM));
   EXPECT_EQ("C_RD_WR",    stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_RD_WR));
   EXPECT_EQ("C_CONFIG",   stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_CONFIG));
   EXPECT_EQ("C_CHECKSUM", stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_CHECKSUM));
   EXPECT_EQ("C_TIMEOUT",  stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_TIMEOUT));
}

TEST(STWErrors, StwError_UnknownCode_ReturnsUnknown)
{
   EXPECT_EQ("C_UNKNOWN_ERR",
             stw::opensyde_core::C_OscLoggingHandler::h_StwError(-999));
   EXPECT_EQ("C_UNKNOWN_ERR",
             stw::opensyde_core::C_OscLoggingHandler::h_StwError(42));
}

TEST(STWErrors, StwError_EdgeCases)
{
   EXPECT_EQ("C_UNKNOWN_ERR",
             stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_UNKNOWN_ERR));
   EXPECT_EQ("C_UNKNOWN_ERR",
             stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_TIMEOUT - 1));
   EXPECT_EQ("C_UNKNOWN_ERR",
             stw::opensyde_core::C_OscLoggingHandler::h_StwError(C_NO_ERR + 1));
}

TEST(STWErrors, CompilationGuard_Passed)
{
   SUCCEED();
}
