//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscNodeDataPoolContentUtil

   More groundwork for the `project` migration. h_SetValueInMinMaxRange decides
   what happens to a parameter value that falls outside its declared limits --
   clamp to min, clamp to max, zero it, or leave it. That decision reaches an ECU,
   so both the resulting value and the reported reason matter: callers act on the
   E_ValueChangedTo flag to tell the user what was adjusted.

   Its two documented failure modes are also worth pinning, because both describe
   a malformed project rather than a malformed value: mismatched datatypes, and
   limits that are the wrong way round.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscNodeDataPoolContentUtil.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

namespace
{
C_OscNodeDataPoolContent h_MakeS32(const int32_t os32_Value)
{
   C_OscNodeDataPoolContent c_Content;

   c_Content.SetArray(false);
   c_Content.SetType(C_OscNodeDataPoolContent::eSINT32);
   c_Content.SetValueS32(os32_Value);
   return c_Content;
}
}

TEST(DataPoolContentUtil, ValueInsideRangeIsLeftAlone)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(-10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(5);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eMIN;

   ASSERT_EQ(C_NO_ERR, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed));

   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eNO_CHANGE, e_Changed);
   EXPECT_EQ(5, c_Value.GetValueS32());
}

TEST(DataPoolContentUtil, ValueBelowMinimumIsClampedAndReported)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(-10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(-50);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   ASSERT_EQ(C_NO_ERR, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed));

   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eMIN, e_Changed) << "clamping was not reported to the caller";
   EXPECT_EQ(-10, c_Value.GetValueS32());
}

TEST(DataPoolContentUtil, ValueAboveMaximumIsClampedAndReported)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(-10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(50);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   ASSERT_EQ(C_NO_ERR, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed));

   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eMAX, e_Changed);
   EXPECT_EQ(10, c_Value.GetValueS32());
}

/// The boundaries themselves are inside the range; clamping them would silently
/// move a legitimately configured limit value.
TEST(DataPoolContentUtil, BoundaryValuesAreInsideTheRange)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(-10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eMAX;

   C_OscNodeDataPoolContent c_AtMin = h_MakeS32(-10);
   ASSERT_EQ(C_NO_ERR, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_AtMin, e_Changed));
   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eNO_CHANGE, e_Changed);
   EXPECT_EQ(-10, c_AtMin.GetValueS32());

   C_OscNodeDataPoolContent c_AtMax = h_MakeS32(10);
   ASSERT_EQ(C_NO_ERR, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_AtMax, e_Changed));
   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eNO_CHANGE, e_Changed);
   EXPECT_EQ(10, c_AtMax.GetValueS32());
}

/// eTO_ZERO asks for an in-range value to be zeroed rather than kept.
TEST(DataPoolContentUtil, SetToZeroOptionZeroesAnInRangeValue)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(-10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(5);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   ASSERT_EQ(C_NO_ERR,
             C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed,
                                                                   C_OscNodeDataPoolContentUtil::eTO_ZERO));

   EXPECT_EQ(C_OscNodeDataPoolContentUtil::eZERO, e_Changed);
   EXPECT_EQ(0, c_Value.GetValueS32());
}

/// Zero is not always a legal value. When the range excludes it, the clamp has to
/// win over the zeroing request rather than writing an out-of-range value.
TEST(DataPoolContentUtil, SetToZeroClampsWhenZeroIsOutsideTheRange)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(5);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(7);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   ASSERT_EQ(C_NO_ERR,
             C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed,
                                                                   C_OscNodeDataPoolContentUtil::eTO_ZERO));

   EXPECT_GE(c_Value.GetValueS32(), 5) << "value was zeroed outside its own range";
   EXPECT_LE(c_Value.GetValueS32(), 10);
   EXPECT_NE(C_OscNodeDataPoolContentUtil::eZERO, e_Changed);
}

/// Documented C_CONFIG: min, max and value must share a datatype.
TEST(DataPoolContentUtil, MismatchedDataTypesAreRejected)
{
   C_OscNodeDataPoolContent c_Min;
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(5);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   c_Min.SetArray(false);
   c_Min.SetType(C_OscNodeDataPoolContent::eUINT8);
   c_Min.SetValueU8(0U);

   EXPECT_EQ(C_CONFIG, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed));
}

/// Documented C_RANGE: limits the wrong way round describe a broken project, and
/// clamping against them would produce an arbitrary result.
TEST(DataPoolContentUtil, InvertedLimitsAreRejected)
{
   const C_OscNodeDataPoolContent c_Min = h_MakeS32(10);
   const C_OscNodeDataPoolContent c_Max = h_MakeS32(-10);
   C_OscNodeDataPoolContent c_Value = h_MakeS32(0);
   C_OscNodeDataPoolContentUtil::E_ValueChangedTo e_Changed = C_OscNodeDataPoolContentUtil::eNO_CHANGE;

   EXPECT_EQ(C_RANGE, C_OscNodeDataPoolContentUtil::h_SetValueInMinMaxRange(c_Min, c_Max, c_Value, e_Changed));
}

TEST(DataPoolContentUtil, ZeroContentClearsTheValue)
{
   C_OscNodeDataPoolContent c_Content = h_MakeS32(1234);

   C_OscNodeDataPoolContentUtil::h_ZeroContent(c_Content);

   EXPECT_EQ(0, c_Content.GetValueS32());
   EXPECT_EQ(C_OscNodeDataPoolContent::eSINT32, c_Content.GetType()) << "zeroing changed the declared type";
}

TEST(DataPoolContentUtil, DataTypeSizeMatchesTheContentType)
{
   EXPECT_EQ(1U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eUINT8));
   EXPECT_EQ(2U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eUINT16));
   EXPECT_EQ(4U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eUINT32));
   EXPECT_EQ(8U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eUINT64));
   EXPECT_EQ(4U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eFLOAT32));
   EXPECT_EQ(8U, C_OscNodeDataPoolContentUtil::h_GetDataTypeSizeInByte(C_OscNodeDataPoolContent::eFLOAT64));
}

TEST(DataPoolContentUtil, SetValueInContentWritesTheRequestedNumber)
{
   C_OscNodeDataPoolContent c_Content = h_MakeS32(0);

   C_OscNodeDataPoolContentUtil::h_SetValueInContent(42.0, c_Content);

   EXPECT_EQ(42, c_Content.GetValueS32());
}
