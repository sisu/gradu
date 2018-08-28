#include "SegmentTree.hpp"
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using testing::UnorderedElementsAre;

TEST(SegmentTreeTest, AddAndLookup) {
  SegmentTree<int> tree(10);
  tree.add({1,5}, 1);
  tree.add({4,5}, 2);
  tree.add({8,9}, 3);
  tree.add({0,10}, 4);
  EXPECT_THAT(tree.find({1,2}), UnorderedElementsAre(1, 4));
  EXPECT_THAT(tree.find({1,5}), UnorderedElementsAre(1, 2, 4));
  EXPECT_THAT(tree.find({0, 10}), UnorderedElementsAre(1, 2, 3, 4));
}

} // namespace
