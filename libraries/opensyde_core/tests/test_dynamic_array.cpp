#include <vector>
#include "gtest/gtest.h"

TEST(DynamicArray, Size_MatchesResize)
{
   std::vector<int32_t> c_Arr;
   EXPECT_EQ(0U, c_Arr.size());
   c_Arr.resize(10);
   EXPECT_EQ(10U, c_Arr.size());
   c_Arr.resize(0);
   EXPECT_EQ(0U, c_Arr.size());
}

TEST(DynamicArray, InsertAndAccess)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(3);
   c_Arr[0] = 10;
   c_Arr[1] = 20;
   c_Arr[2] = 30;
   EXPECT_EQ(10, c_Arr[0]);
   EXPECT_EQ(20, c_Arr[1]);
   EXPECT_EQ(30, c_Arr[2]);
}

TEST(DynamicArray, Erase_ShrinksArray)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(3);
   c_Arr[0] = 1;
   c_Arr[1] = 2;
   c_Arr[2] = 3;
   c_Arr.erase(c_Arr.begin() + 1);
   EXPECT_EQ(2U, c_Arr.size());
   EXPECT_EQ(1, c_Arr[0]);
   EXPECT_EQ(3, c_Arr[1]);
}

TEST(DynamicArray, CopyConstructor)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(2);
   c_Arr[0] = 7;
   c_Arr[1] = 8;
   std::vector<int32_t> c_Copy(c_Arr);
   EXPECT_EQ(2U, c_Copy.size());
   EXPECT_EQ(7, c_Copy[0]);
   EXPECT_EQ(8, c_Copy[1]);
}

TEST(DynamicArray, AssignmentOperator)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(2);
   c_Arr[0] = 1;
   c_Arr[1] = 2;
   std::vector<int32_t> c_Copy;
   c_Copy = c_Arr;
   EXPECT_EQ(2U, c_Copy.size());
   EXPECT_EQ(1, c_Copy[0]);
   EXPECT_EQ(2, c_Copy[1]);
}

TEST(DynamicArray, PushBack)
{
   std::vector<int32_t> c_Arr;
   c_Arr.push_back(42);
   EXPECT_EQ(1U, c_Arr.size());
   EXPECT_EQ(42, c_Arr[0]);
}

TEST(DynamicArray, EmplaceBack)
{
   std::vector<int32_t> c_Arr;
   c_Arr.emplace_back(99);
   EXPECT_EQ(1U, c_Arr.size());
   EXPECT_EQ(99, c_Arr[0]);
}

TEST(DynamicArray, Empty_SizeZero)
{
   std::vector<int32_t> c_Arr;
   EXPECT_TRUE(c_Arr.empty());
   EXPECT_EQ(0U, c_Arr.size());
}

TEST(DynamicArray, Clear_RemovesAll)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(5);
   EXPECT_EQ(5U, c_Arr.size());
   c_Arr.clear();
   EXPECT_EQ(0U, c_Arr.size());
   EXPECT_TRUE(c_Arr.empty());
}

TEST(DynamicArray, Iterator_Compatibility)
{
   std::vector<int32_t> c_Arr;
   c_Arr.resize(3);
   c_Arr[0] = 10;
   c_Arr[1] = 20;
   c_Arr[2] = 30;
   int32_t s32_Sum = 0;
   for (auto it = c_Arr.begin(); it != c_Arr.end(); ++it)
   {
      s32_Sum += *it;
   }
   EXPECT_EQ(60, s32_Sum);
}

TEST(DynamicArray, RangeFor_Compatibility)
{
   std::vector<int32_t> c_Arr = { 2, 4, 6 };
   int32_t s32_Sum = 0;
   for (const auto & rc_Val : c_Arr)
   {
      s32_Sum += rc_Val;
   }
   EXPECT_EQ(12, s32_Sum);
}
