//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the "0x" and UNC prefix checks that survived the std::string migration

   Three call sites inspected a fixed character index to recognise a prefix, and kept the
   1-based index they had under C_SclString when they moved to std::string. Each pairs the
   wrong index with a correctly migrated 1-based SubStringCompat, so the substring that
   follows is right and only the test that guards it is wrong. All three failed silently:
   nothing reported an error, the value simply came back wrong.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include <cstdint>
#include "C_OscUtils.hpp"
#include "C_OscHalcDefContentBitmaskItem.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A "0x" bitmask takes the hex path

   Regression pin. With the wrong index the value fell through to the decimal path, where
   the stream stopped at the 'x' after reading the leading '0'. That yields 0 and does not
   set failbit, so a bitmask of 0x10 silently became 0 and no error was reported.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(StringPrefixParsing, BitmaskHexPrefixIsRecognised)
{
   C_OscHalcDefContentBitmaskItem c_Item;

   EXPECT_FALSE(static_cast<bool>(c_Item.SetValueByString("0x10")));
   EXPECT_EQ(0x10ULL, c_Item.u64_Value);

   EXPECT_FALSE(static_cast<bool>(c_Item.SetValueByString("0xFF")));
   EXPECT_EQ(0xFFULL, c_Item.u64_Value);

   //the decimal path must keep working
   EXPECT_FALSE(static_cast<bool>(c_Item.SetValueByString("16")));
   EXPECT_EQ(16ULL, c_Item.u64_Value);

   //and the boolean spellings
   EXPECT_FALSE(static_cast<bool>(c_Item.SetValueByString("true")));
   EXPECT_EQ(1ULL, c_Item.u64_Value);
   EXPECT_FALSE(static_cast<bool>(c_Item.SetValueByString("false")));
   EXPECT_EQ(0ULL, c_Item.u64_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A UNC path counts as absolute and is not concatenated onto a base directory

   Regression pin. With the wrong index the "//" and "\\\\" prefixes were never recognised,
   so a UNC path was treated as relative and the base directory was prepended to it.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(StringPrefixParsing, UncPathIsTreatedAsAbsolute)
{
   EXPECT_EQ("//server/share/file.syde",
             C_OscUtils::h_ConcatPathIfNecessary("/home/base", "//server/share/file.syde"));
   EXPECT_EQ("\\\\server\\share\\file.syde",
             C_OscUtils::h_ConcatPathIfNecessary("/home/base", "\\\\server\\share\\file.syde"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A genuinely relative path is still joined to the base directory
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(StringPrefixParsing, RelativePathIsStillConcatenated)
{
   EXPECT_EQ("/home/base/sub/file.syde", C_OscUtils::h_ConcatPathIfNecessary("/home/base", "sub/file.syde"));
}
