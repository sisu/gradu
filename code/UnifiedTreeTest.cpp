#include "UnifiedTree.hpp"
#include <bitset>
#include <vector>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using namespace std;

template<class T, int D>
class SlowTree {
public:
	using Index = std::array<int, D>;

	SlowTree(Index sz) {
		size = sz;
		int total = 1;
		for(int i=D-1; i>=0; --i) {
			stepSize[i] = total;
			total *= size[i];
		}
		data.resize(total);
	}

	void add(const Box<D>& box, const T& value) {
		addRec(0,0,box,value);
	}

	bool check(const Box<D>& box) const {
		return checkRec(0,0,box);
	}

private:
	void addRec(int index, int axis, const Box<D>& box, const T& value) {
		if (axis == D) {
			T& x = data[index];
			if (!x.hasData[D]) {
				x = value;
				x.hasData.set(D);
			}
			return;
		}
		int step = stepSize[axis];
		Range range = box[axis];
		for(int i=range.from; i<range.to; ++i) {
			addRec(index + step*i, axis+1, box, value);
		}
	}
	bool checkRec(int index, int axis, const Box<D>& box) const {
		if (axis == D) return data[index].hasData[D];
		int step = stepSize[axis];
		Range range = box[axis];
		for(int i=range.from; i<range.to; ++i) {
			if (checkRec(index + step*i, axis+1, box)) return true;
		}
		return false;
	}

	Index size = {};
	Index stepSize = {};
	vector<T> data;
};

template<int D>
struct Item {
	std::bitset<1<<D> hasData;
};

Box<1> box1(int from, int to) {
	return {{Range{from,to}}};
}

#if 0
Box<2> box2(Range x, Range y) {
	return {{x, y}};
}
#endif

enum class OType { ADD, CHECK };

template<int D>
struct Operation {
	OType type;
	Box<D> box;
	Item<D> value;
};

Operation<1> makeOp1(OType type, int from, int to) {
	return {type, box1(from, to), {}};
}

template<int D>
ostream& operator<<(ostream& out, const Operation<D>& op) {
	return out<<"{"<<(int)op.type<<' '<<op.box<<"}";
}

template<int D>
void runOps(UnifiedTree<Item<D>, D>& actual, const vector<Operation<D>>& ops) {
	SlowTree<Item<D>, D> expected(actual.getSize());
	ostringstream oss;
	for(const auto& t: ops) {
		switch(t.type) {
			case OType::ADD:
				expected.add(t.box, t.value);
				actual.add(t.box, t.value);
				oss<<t<<'\n';
				break;
			case OType::CHECK:
				EXPECT_EQ(expected.check(t.box), actual.check(t.box))<<oss.str()<<" ; "<<t;
				break;
		}
	}
}

TEST(UnifiedTreeTest1D, AddCheckUnitTree) {
#if 0
	UnifiedTree<Item<1>, 1> tree{1};
	EXPECT_FALSE(tree.check(box1(0, 1)));
	tree.add(box1(0,1), {});
	EXPECT_TRUE(tree.check(box1(0, 1)));
#else
	UnifiedTree<Item<1>, 1> tree{1};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 1),
		makeOp1(OType::CHECK, 0, 1)};
	runOps(tree, ops);
#endif
}

TEST(UnifiedTreeTest1D, AddCheckTree) {
	UnifiedTree<Item<1>, 1> tree{8};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 5),
		makeOp1(OType::CHECK, 5, 6),
		makeOp1(OType::CHECK, 4, 6),
		makeOp1(OType::ADD, 4, 8),
		makeOp1(OType::CHECK, 5, 6)};
	runOps(tree, ops);
}

} // namespace
