#include "overlap.hpp"
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using ::testing::UnorderedElementsAre;
using namespace std;

Box<2> box2(Range x, Range y) {
  return {{x,y}};
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

} // namespace
