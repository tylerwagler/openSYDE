#include <stdexcept>
#include "gtest/gtest.h"
#include "C_SclDynamicArray.hpp"

using stw::scl::C_SclDynamicArray;

TEST(DynamicArray, GetHigh_Empty_ReturnsMinusOne)
{
   C_SclDynamicArray<int32_t> c_Arr;
   EXPECT_EQ(-1, c_Arr.GetHigh());
}

TEST(DynamicArray, GetHigh_SingleElement_ReturnsZero)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(1);
   EXPECT_EQ(0, c_Arr.GetHigh());
}

TEST(DynamicArray, GetHigh_MultipleElements)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(5);
   EXPECT_EQ(4, c_Arr.GetHigh());
}

TEST(DynamicArray, GetHigh_AfterIncLength)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.IncLength(3);
   EXPECT_EQ(2, c_Arr.GetHigh());
}

TEST(DynamicArray, GetLength_MatchesSetLength)
{
   C_SclDynamicArray<int32_t> c_Arr;
   EXPECT_EQ(0, c_Arr.GetLength());
   c_Arr.SetLength(10);
   EXPECT_EQ(10, c_Arr.GetLength());
   c_Arr.SetLength(0);
   EXPECT_EQ(0, c_Arr.GetLength());
}

TEST(DynamicArray, Delete_OutOfRange_Throws)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(3);
   EXPECT_THROW(c_Arr.Delete(5), std::out_of_range);
   EXPECT_THROW(c_Arr.Delete(-1), std::out_of_range);
}

TEST(DynamicArray, Insert_OutOfRange_Throws)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(2);
   EXPECT_THROW(c_Arr.Insert(5, 42), std::out_of_range);
   EXPECT_THROW(c_Arr.Insert(-1, 42), std::out_of_range);
}

TEST(DynamicArray, Insert_AtEnd_Allowed)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(2);
   EXPECT_NO_THROW(c_Arr.Insert(2, 42));
   EXPECT_EQ(3, c_Arr.GetLength());
}

TEST(DynamicArray, InsertAndAccess)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(3);
   c_Arr[0] = 10;
   c_Arr[1] = 20;
   c_Arr[2] = 30;
   EXPECT_EQ(10, c_Arr[0]);
   EXPECT_EQ(20, c_Arr[1]);
   EXPECT_EQ(30, c_Arr[2]);
}

TEST(DynamicArray, Delete_ShrinksArray)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(3);
   c_Arr[0] = 1;
   c_Arr[1] = 2;
   c_Arr[2] = 3;
   c_Arr.Delete(1);
   EXPECT_EQ(2, c_Arr.GetLength());
   EXPECT_EQ(1, c_Arr[0]);
   EXPECT_EQ(3, c_Arr[1]);
}

TEST(DynamicArray, CopyConstructor)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(2);
   c_Arr[0] = 7;
   c_Arr[1] = 8;
   C_SclDynamicArray<int32_t> c_Copy(c_Arr);
   EXPECT_EQ(2, c_Copy.GetLength());
   EXPECT_EQ(7, c_Copy[0]);
   EXPECT_EQ(8, c_Copy[1]);
}

TEST(DynamicArray, AssignmentOperator)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(2);
   c_Arr[0] = 1;
   c_Arr[1] = 2;
   C_SclDynamicArray<int32_t> c_Copy;
   c_Copy = c_Arr;
   EXPECT_EQ(2, c_Copy.GetLength());
   EXPECT_EQ(1, c_Copy[0]);
   EXPECT_EQ(2, c_Copy[1]);
}

TEST(DynamicArray, SelfAssignment)
{
   C_SclDynamicArray<int32_t> c_Arr;
   c_Arr.SetLength(2);
   c_Arr[0] = 99;
   // Self-assignment must not corrupt
   c_Arr = c_Arr;
   EXPECT_EQ(2, c_Arr.GetLength());
   EXPECT_EQ(99, c_Arr[0]);
}
