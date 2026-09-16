//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the std::expected pilot on C_OscNodeDataPoolFiler::h_StringToDataPool

   This converter is the phase 5 follow-up pilot: it used to return std::error_code and write the
   parsed value through an out parameter that it left untouched on failure. These tests pin the
   replacement, whose point is that a failed parse carries no value to read by mistake.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <expected>
#include <string>
#include <system_error>

#include "gtest/gtest.h"

#include "C_OscNodeDataPoolFiler.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(DataPoolTypeString, ParsesEveryKnownType)
{
   EXPECT_EQ(C_OscNodeDataPool::eCOM, C_OscNodeDataPoolFiler::h_StringToDataPool("com").value());
   EXPECT_EQ(C_OscNodeDataPool::eNVM, C_OscNodeDataPoolFiler::h_StringToDataPool("nvm").value());
   EXPECT_EQ(C_OscNodeDataPool::eDIAG, C_OscNodeDataPoolFiler::h_StringToDataPool("diag").value());
   EXPECT_EQ(C_OscNodeDataPool::eHALC, C_OscNodeDataPoolFiler::h_StringToDataPool("halc").value());
   EXPECT_EQ(C_OscNodeDataPool::eHALC_NVM, C_OscNodeDataPoolFiler::h_StringToDataPool("halc-nvm").value());
}

TEST(DataPoolTypeString, UnknownStringCarriesRangeAndNoValue)
{
   const std::expected<C_OscNodeDataPool::E_Type, std::error_code> c_Result =
      C_OscNodeDataPoolFiler::h_StringToDataPool("not-a-datapool-type");

   //The whole point of the pilot: there is no value to read, rather than a stale one
   EXPECT_FALSE(c_Result.has_value());
   EXPECT_EQ(Errc::range, c_Result.error());
}

TEST(DataPoolTypeString, MatchIsExactAndCaseSensitive)
{
   //The strings come from a file format, so "COM" and "com " must not be accepted
   EXPECT_FALSE(C_OscNodeDataPoolFiler::h_StringToDataPool("COM").has_value());
   EXPECT_FALSE(C_OscNodeDataPoolFiler::h_StringToDataPool("com ").has_value());
   EXPECT_FALSE(C_OscNodeDataPoolFiler::h_StringToDataPool("").has_value());
}

TEST(DataPoolTypeString, ValueOrKeepsTheCallersDefaultOnFailure)
{
   //This is how the dashboard filer consumes it, and it replaced a (void) call plus a comment
   const C_OscNodeDataPool::E_Type e_Default = C_OscNodeDataPool::eDIAG;

   EXPECT_EQ(e_Default, C_OscNodeDataPoolFiler::h_StringToDataPool("bogus").value_or(e_Default));
   EXPECT_EQ(C_OscNodeDataPool::eNVM, C_OscNodeDataPoolFiler::h_StringToDataPool("nvm").value_or(e_Default));
}
