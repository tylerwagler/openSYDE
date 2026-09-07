//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscNodeDataPoolContent

   Groundwork for the `project` error-handling migration -- 213 functions across
   roughly 640 caller files, the largest remaining wave by caller fan-out.

   C_OscNodeDataPoolContent is the tagged-union value type behind every datapool
   value in the tool, and its two blob setters turn raw bytes read off an ECU
   into a typed value. An endianness or sizing mistake there does not crash; it
   silently reports the wrong number for a live parameter, which is why the
   endianness cases below assert on exact byte order rather than round-trips
   alone.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <system_error>
#include <vector>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscNodeDataPoolContent.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

namespace
{
C_OscNodeDataPoolContent h_MakeScalar(const C_OscNodeDataPoolContent::E_Type oe_Type)
{
   C_OscNodeDataPoolContent c_Content;

   c_Content.SetArray(false);
   c_Content.SetType(oe_Type);
   return c_Content;
}
}

// -- structure ---------------------------------------------------------------

TEST(NodeDataPoolContent, SizeByteMatchesDeclaredType)
{
   EXPECT_EQ(1U, h_MakeScalar(C_OscNodeDataPoolContent::eUINT8).GetSizeByte());
   EXPECT_EQ(2U, h_MakeScalar(C_OscNodeDataPoolContent::eUINT16).GetSizeByte());
   EXPECT_EQ(4U, h_MakeScalar(C_OscNodeDataPoolContent::eUINT32).GetSizeByte());
   EXPECT_EQ(8U, h_MakeScalar(C_OscNodeDataPoolContent::eUINT64).GetSizeByte());
   EXPECT_EQ(1U, h_MakeScalar(C_OscNodeDataPoolContent::eSINT8).GetSizeByte());
   EXPECT_EQ(2U, h_MakeScalar(C_OscNodeDataPoolContent::eSINT16).GetSizeByte());
   EXPECT_EQ(4U, h_MakeScalar(C_OscNodeDataPoolContent::eSINT32).GetSizeByte());
   EXPECT_EQ(8U, h_MakeScalar(C_OscNodeDataPoolContent::eSINT64).GetSizeByte());
   EXPECT_EQ(4U, h_MakeScalar(C_OscNodeDataPoolContent::eFLOAT32).GetSizeByte());
   EXPECT_EQ(8U, h_MakeScalar(C_OscNodeDataPoolContent::eFLOAT64).GetSizeByte());
}

TEST(NodeDataPoolContent, ArraySizeScalesTheByteSize)
{
   C_OscNodeDataPoolContent c_Content;

   c_Content.SetArray(true);
   c_Content.SetType(C_OscNodeDataPoolContent::eUINT32);
   c_Content.SetArraySize(4U);

   EXPECT_TRUE(c_Content.GetArray());
   EXPECT_EQ(4U, c_Content.GetArraySize());
   EXPECT_EQ(16U, c_Content.GetSizeByte());
}

TEST(NodeDataPoolContent, ScalarRoundTripsPerType)
{
   C_OscNodeDataPoolContent c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT8);

   c_Content.SetValueU8(0xABU);
   EXPECT_EQ(0xABU, c_Content.GetValueU8());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eSINT16);
   c_Content.SetValueS16(-1234);
   EXPECT_EQ(-1234, c_Content.GetValueS16());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT64);
   c_Content.SetValueU64(0x0123456789ABCDEFULL);
   EXPECT_EQ(0x0123456789ABCDEFULL, c_Content.GetValueU64());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eFLOAT64);
   c_Content.SetValueF64(-2.5);
   EXPECT_DOUBLE_EQ(-2.5, c_Content.GetValueF64());
}

// -- blob decoding: the ECU-facing part --------------------------------------

/// The byte order has to be exactly this way round. A round-trip test alone
/// would pass even if both directions were swapped consistently.
TEST(NodeDataPoolContent, BigEndianBlobUsesMostSignificantByteFirst)
{
   C_OscNodeDataPoolContent c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT16);

   ASSERT_EQ(Errc::success, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x01U, 0x02U})));
   EXPECT_EQ(0x0102U, c_Content.GetValueU16());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   ASSERT_EQ(Errc::success, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x01U, 0x02U, 0x03U, 0x04U})));
   EXPECT_EQ(0x01020304U, c_Content.GetValueU32());
}

TEST(NodeDataPoolContent, LittleEndianBlobUsesLeastSignificantByteFirst)
{
   C_OscNodeDataPoolContent c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT16);

   ASSERT_EQ(Errc::success, c_Content.SetValueFromLittleEndianBlob(std::vector<uint8_t>({0x01U, 0x02U})));
   EXPECT_EQ(0x0201U, c_Content.GetValueU16());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   ASSERT_EQ(Errc::success, c_Content.SetValueFromLittleEndianBlob(std::vector<uint8_t>({0x01U, 0x02U, 0x03U, 0x04U})));
   EXPECT_EQ(0x04030201U, c_Content.GetValueU32());
}

/// If these ever agree on a multi-byte type, one of the two decoders is wrong.
TEST(NodeDataPoolContent, EndiannessDecodersDisagreeOnMultiByteTypes)
{
   C_OscNodeDataPoolContent c_Big = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   C_OscNodeDataPoolContent c_Little = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   const std::vector<uint8_t> c_Data({0xAAU, 0xBBU, 0xCCU, 0xDDU});

   ASSERT_EQ(Errc::success, c_Big.SetValueFromBigEndianBlob(c_Data));
   ASSERT_EQ(Errc::success, c_Little.SetValueFromLittleEndianBlob(c_Data));

   EXPECT_NE(c_Big.GetValueU32(), c_Little.GetValueU32());
   EXPECT_EQ(0xAABBCCDDU, c_Big.GetValueU32());
   EXPECT_EQ(0xDDCCBBAAU, c_Little.GetValueU32());
}

TEST(NodeDataPoolContent, SignedValuesDecodeAsNegative)
{
   C_OscNodeDataPoolContent c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eSINT16);

   //0xFFFF is -1 in two's complement
   ASSERT_EQ(Errc::success, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0xFFU, 0xFFU})));
   EXPECT_EQ(-1, c_Content.GetValueS16());

   c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eSINT8);
   ASSERT_EQ(Errc::success, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x80U})));
   EXPECT_EQ(-128, c_Content.GetValueS8()) << "sign bit was dropped";
}

/// A blob whose length does not match the declared type must be refused. Reading
/// four bytes into a two-byte value, or two into a four-byte one, is exactly the
/// kind of mismatch a protocol change would introduce.
TEST(NodeDataPoolContent, BlobWithWrongLengthIsRejected)
{
   C_OscNodeDataPoolContent c_Content = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);

   EXPECT_EQ(Errc::config, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x01U, 0x02U})));
   EXPECT_EQ(Errc::config, c_Content.SetValueFromLittleEndianBlob(std::vector<uint8_t>({0x01U, 0x02U})));
   EXPECT_EQ(Errc::config,
             c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x01U, 0x02U, 0x03U, 0x04U, 0x05U})));

   //an empty blob must not reach the &orc_Data[0] in the decoder
   EXPECT_EQ(Errc::config, c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>()));
}

TEST(NodeDataPoolContent, ArrayBlobDecodesEveryElement)
{
   C_OscNodeDataPoolContent c_Content;

   c_Content.SetArray(true);
   c_Content.SetType(C_OscNodeDataPoolContent::eUINT16);
   c_Content.SetArraySize(3U);

   ASSERT_EQ(Errc::success,
             c_Content.SetValueFromBigEndianBlob(std::vector<uint8_t>({0x00U, 0x01U, 0x00U, 0x02U, 0x00U, 0x03U})));

   EXPECT_EQ(1U, c_Content.GetValueArrU16Element(0U));
   EXPECT_EQ(2U, c_Content.GetValueArrU16Element(1U));
   EXPECT_EQ(3U, c_Content.GetValueArrU16Element(2U));
}

// -- comparison --------------------------------------------------------------

TEST(NodeDataPoolContent, ComparisonOperatorsAgreeOnOrdering)
{
   C_OscNodeDataPoolContent c_Low = h_MakeScalar(C_OscNodeDataPoolContent::eSINT32);
   C_OscNodeDataPoolContent c_High = h_MakeScalar(C_OscNodeDataPoolContent::eSINT32);

   c_Low.SetValueS32(-5);
   c_High.SetValueS32(7);

   EXPECT_TRUE(c_Low < c_High);
   EXPECT_TRUE(c_Low <= c_High);
   EXPECT_TRUE(c_High > c_Low);
   EXPECT_TRUE(c_High >= c_Low);
   EXPECT_FALSE(c_High < c_Low);
   EXPECT_FALSE(c_Low == c_High);
}

/// Signed comparison must not be done on the raw unsigned storage, or a negative
/// value sorts above a positive one.
TEST(NodeDataPoolContent, NegativeValuesCompareBelowPositive)
{
   C_OscNodeDataPoolContent c_Negative = h_MakeScalar(C_OscNodeDataPoolContent::eSINT16);
   C_OscNodeDataPoolContent c_Positive = h_MakeScalar(C_OscNodeDataPoolContent::eSINT16);

   c_Negative.SetValueS16(-1);
   c_Positive.SetValueS16(1);

   EXPECT_TRUE(c_Negative < c_Positive) << "negative value did not sort below positive";
}

TEST(NodeDataPoolContent, EqualityRequiresSameValue)
{
   C_OscNodeDataPoolContent c_First = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   C_OscNodeDataPoolContent c_Second = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);

   c_First.SetValueU32(42U);
   c_Second.SetValueU32(42U);
   EXPECT_TRUE(c_First == c_Second);

   c_Second.SetValueU32(43U);
   EXPECT_FALSE(c_First == c_Second);
}

// -- hashing -----------------------------------------------------------------

/// CalcHash feeds the project change-detection. Two different values must not
/// hash the same, or an edited parameter looks unchanged and is never saved.
TEST(NodeDataPoolContent, HashDiffersForDifferentValues)
{
   C_OscNodeDataPoolContent c_First = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   C_OscNodeDataPoolContent c_Second = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   uint32_t u32_HashFirst = 0xFFFFFFFFU;
   uint32_t u32_HashSecond = 0xFFFFFFFFU;

   c_First.SetValueU32(1U);
   c_Second.SetValueU32(2U);

   c_First.CalcHash(u32_HashFirst);
   c_Second.CalcHash(u32_HashSecond);

   EXPECT_NE(u32_HashFirst, u32_HashSecond);
}

TEST(NodeDataPoolContent, HashIsStableForEqualValues)
{
   C_OscNodeDataPoolContent c_First = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   C_OscNodeDataPoolContent c_Second = h_MakeScalar(C_OscNodeDataPoolContent::eUINT32);
   uint32_t u32_HashFirst = 0xFFFFFFFFU;
   uint32_t u32_HashSecond = 0xFFFFFFFFU;

   c_First.SetValueU32(0xDEADBEEFU);
   c_Second.SetValueU32(0xDEADBEEFU);

   c_First.CalcHash(u32_HashFirst);
   c_Second.CalcHash(u32_HashSecond);

   EXPECT_EQ(u32_HashFirst, u32_HashSecond);
}
