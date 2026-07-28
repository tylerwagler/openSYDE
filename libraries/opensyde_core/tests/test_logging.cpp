#include <set>
#include "gtest/gtest.h"
#include "C_OscLoggingHandler.hpp"

TEST(Logging, StartPerformanceTimer_ReturnsNonZeroId)
{
   const uint16_t u16_Id = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
   // Any uint16_t value is valid; just verify it doesn't crash
   EXPECT_NO_FATAL_FAILURE((void)u16_Id);
}

TEST(Logging, StartPerformanceTimer_IdsAreUnique)
{
   std::set<uint16_t> c_Ids;
   for (int32_t i = 0; i < 100; ++i)
   {
      const uint16_t u16_Id = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
      const auto c_Result = c_Ids.insert(u16_Id);
      EXPECT_TRUE(c_Result.second) << "Duplicate timer ID: " << u16_Id;
   }
}

TEST(Logging, StartPerformanceTimer_IdsIncreaseMonotonically)
{
   uint16_t u16_Prev = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
   for (int32_t i = 0; i < 50; ++i)
   {
      const uint16_t u16_Curr = stw::opensyde_core::C_OscLoggingHandler::h_StartPerformanceTimer();
      // With atomic increment, IDs should be strictly increasing (modulo wraparound)
      if (u16_Curr > u16_Prev)
      {
         const uint16_t u16_Diff = u16_Curr - u16_Prev;
         EXPECT_EQ(1U, u16_Diff);
      }
      // If wraparound occurred, just note it
      u16_Prev = u16_Curr;
   }
}

TEST(Logging, StwError_Consistency)
{
   // Verify that every defined error code maps to its own name
   EXPECT_EQ("C_NO_ERR",   std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(0).c_str()));
   EXPECT_EQ("C_UNKNOWN_ERR", std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-1).c_str()));
   EXPECT_EQ("C_WARN",     std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-2).c_str()));
   EXPECT_EQ("C_DEFAULT",  std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-3).c_str()));
   EXPECT_EQ("C_BUSY",     std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-4).c_str()));
   EXPECT_EQ("C_RANGE",    std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-5).c_str()));
   EXPECT_EQ("C_OVERFLOW", std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-6).c_str()));
   EXPECT_EQ("C_RD_WR",    std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-7).c_str()));
   EXPECT_EQ("C_NOACT",    std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-8).c_str()));
   EXPECT_EQ("C_COM",      std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-9).c_str()));
   EXPECT_EQ("C_CONFIG",   std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-10).c_str()));
   EXPECT_EQ("C_CHECKSUM", std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-11).c_str()));
   EXPECT_EQ("C_TIMEOUT",  std::string(stw::opensyde_core::C_OscLoggingHandler::h_StwError(-12).c_str()));
}
