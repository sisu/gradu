#include "decomposition.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock-more-matchers.h>

using testing::IsEmpty;

TEST(DecompositionTest, Decompose2Empty) {
	ObstacleSet<3> obs;
	EXPECT_THAT(decomposeFreeSpace<3>(obs), IsEmpty());
}

TEST(DecompositionTest, Decompose2SingleCell) {
	ObstacleSet<3> obs;
	EXPECT_THAT(decomposeFreeSpace<3>(obs), IsEmpty());
}
