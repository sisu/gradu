#include "overlap.hpp"
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using ::testing::UnorderedElementsAre;
using namespace std;

Box<2> box2(Range x, Range y) {
	return {{x,y}};
}

Box<3> box3(Range x, Range y, Range z) {
	return {{x,y,z}};
}

TEST(OverlapTest2D, Simple) {
	vector<Box<2>> bs1 = {
		box2({0,2}, {0,1}),
		box2({1,3}, {0,2})};
	vector<Box<2>> bs2 = {
		box2({0,1}, {0,2}),
		box2({1,3}, {0,2})};
	EXPECT_THAT(overlappingBoxes(bs1, bs2),
			UnorderedElementsAre(
				make_pair(0,0), make_pair(0,1), make_pair(1,1)));
}

TEST(OverlapTest3D, Simple) {
	vector<Box<3>> bs1 = {
		box3({0,2}, {0,1}, {0,1}),
		box3({1,3}, {0,2}, {0,2})};
	vector<Box<3>> bs2 = {
		box3({0,1}, {0,2}, {0,1}),
		box3({1,3}, {0,2}, {1,2})};
	EXPECT_THAT(overlappingBoxes(bs1, bs2),
			UnorderedElementsAre(
				make_pair(0,0), make_pair(1,1)));
}

TEST(OverlapTest3D, NoDuplicates) {
	vector<Box<3>> bs1 = {
		box3({0,1}, {0,3}, {0,5}),
		box3({1,2}, {0,3}, {1,6}),
		box3({2,3}, {0,3}, {2,7})};
	vector<Box<3>> bs2 = {
		box3({0,3}, {0,1}, {0,5}),
		box3({0,3}, {1,2}, {1,6}),
		box3({0,3}, {2,3}, {2,7})};
	EXPECT_THAT(overlappingBoxes(bs1, bs2),
			UnorderedElementsAre(
				make_pair(0,0), make_pair(0,1), make_pair(0,2),
				make_pair(1,0), make_pair(1,1), make_pair(1,2),
				make_pair(2,0), make_pair(2,1), make_pair(2,2)));
}

} // namespace
