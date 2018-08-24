#include "UnifiedTree.hpp"
#include <bitset>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

#if 0
template<class T>
class SlowTree {
public:
	using Index = std::array<int, D>;

	void add(const Box<D>& box, const T& value) {
	}

	bool check(const Box<D>& box) const {
	}

private:
	vector<T> data;
	Index size = {};
};
#endif

template<int D>
struct Item {
	std::bitset<1<<D> hasData;
};

Box<1> box1(int from, int to) {
	return {{Range{from,to}}};
}

Box<2> box2(Range x, Range y) {
	return {{x, y}};
}

TEST(UnifiedTreeTest1D, AddCheckUnitTree) {
	UnifiedTree<Item<1>, 1> tree{1};
	EXPECT_FALSE(tree.check(box1(0, 1)));
	tree.add(box1(0,1), {});
	EXPECT_TRUE(tree.check(box1(0, 1)));
}

#if 0
TEST(UnifiedTreeTest1D, AddCheckTree) {
	UnifiedTree<Item<1>, 1> tree{10};
	EXPECT_FALSE(tree.check(box1(0, 1)));
	tree.add(box1(0,1), {});
	EXPECT_TRUE(tree.check(box1(0, 1)));
}
#endif

} // namespace
