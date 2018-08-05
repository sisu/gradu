#include "TreeStructure.hpp"
#include <gtest/gtest.h>

TEST(TreeStructureTest, IndexToRange) {
	TreeStructure st{16};
	EXPECT_EQ(st.indexToRange(1), Range(0, 8));
	EXPECT_EQ(st.indexToRange(2), Range(0, 4));
	EXPECT_EQ(st.indexToRange(3), Range(4, 8));
	EXPECT_EQ(st.indexToRange(4), Range(0, 2));
	EXPECT_EQ(st.indexToRange(5), Range(2, 4));
	EXPECT_EQ(st.indexToRange(8), Range(0, 1));
}

TEST(TreeStructureTest, RangeToIndex) {
	TreeStructure st{16};
	EXPECT_EQ(st.rangeToIndex({0, 8}), 1);
	EXPECT_EQ(st.rangeToIndex({0, 4}), 2);
	EXPECT_EQ(st.rangeToIndex({4, 8}), 3);
	EXPECT_EQ(st.rangeToIndex({0, 2}), 4);
	EXPECT_EQ(st.rangeToIndex({2, 4}), 5);
	EXPECT_EQ(st.rangeToIndex({0, 1}), 8);
}
