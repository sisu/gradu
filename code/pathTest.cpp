#include "path.hpp"
#include "obstacles.hpp"
#include <cstring>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using namespace std;

TEST(LinkDistance2D, StartEndPointSame) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {1,1}), 0);
}

TEST(LinkDistance2D, StartEndSameLine) {
	ObstacleSet<2> obs = makeObstaclesForPlane({".."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {2,1}), 1);
}

TEST(LinkDistance2D, OneTurn) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"..", ".."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {2,2}), 2);
}

TEST(LinkDistance2D, AroundObstacle) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"...", ".#.", "..."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {3,3}), 2);
}

TEST(LinkDistance2D, AroundObstacle2) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"#..", ".#.", "..."});
	EXPECT_EQ(linkDistance(obs, {2,1}, {1,2}), 4);
}

} // namespace
