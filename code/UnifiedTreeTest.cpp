#include "UnifiedTree.hpp"

#include <bitset>
#include <random>
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

	void remove(const Box<D>& box) {
		removeRec(0,0,box);
	}

private:
	struct Item {
		bool hasData = false;
		T data;
	};

	void addRec(int index, int axis, const Box<D>& box, const T& value) {
		if (axis == D) {
			Item& x = data[index];
			if (!x.hasData) {
				x.data = value;
				x.hasData = true;
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
		if (axis == D) return data[index].hasData;
		int step = stepSize[axis];
		Range range = box[axis];
		for(int i=range.from; i<range.to; ++i) {
			if (checkRec(index + step*i, axis+1, box)) return true;
		}
		return false;
	}
	void removeRec(int index, int axis, const Box<D>& box) {
		if (axis == D) {
			Item& x = data[index];
			x.hasData = false;
			return;
		}
		int step = stepSize[axis];
		Range range = box[axis];
		for(int i=range.from; i<range.to; ++i) {
			removeRec(index + step*i, axis+1, box);
		}
	}

	Index size = {};
	Index stepSize = {};
	vector<Item> data;
};

template<int D>
struct Item {};

Box<1> box1(int from, int to) {
	return {{Range{from,to}}};
}

Box<2> box2(Range x, Range y) {
	return {{x, y}};
}

enum class OType { ADD, REMOVE, CHECK };

template<int D>
struct Operation {
	OType type = OType::ADD;
	Box<D> box;
	Item<D> value;
};

Operation<1> makeOp1(OType type, int from, int to) {
	return {type, box1(from, to), {}};
}
Operation<2> makeOp2(OType type, Range x, Range y) {
	return {type, box2(x, y), {}};
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
			case OType::REMOVE:
				expected.remove(t.box);
				actual.remove(t.box);
				oss<<t<<'\n';
				break;
			case OType::CHECK:
				EXPECT_EQ(actual.check(t.box), expected.check(t.box))<<oss.str()<<" ; "<<t;
				break;
		}
	}
}

template<int D>
vector<Operation<D>> genRandomOps(int size, int n, vector<OType> otypes, mt19937& rng) {
	vector<Operation<D>> ops;
	for(int i=0; i<n; ++i) {
		Operation<D> op;
		op.type = otypes[rng()%otypes.size()];
		for(int j=0; j<D; ++j) {
			int a = rng()%(size+1), b = rng()%(size+1);
			if (a>b) swap(a,b);
			op.box[j] = {a,b};
		}
		ops.push_back(op);
	}
	return ops;
}

TEST(UnifiedTreeTest1D, AddCheckUnitTree) {
	UnifiedTree<Item<1>, 1> tree{{1}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 1),
		makeOp1(OType::CHECK, 0, 1)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, RemoveUnitTree) {
	UnifiedTree<Item<1>, 1> tree{{1}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 1),
		makeOp1(OType::REMOVE, 0, 1),
		makeOp1(OType::CHECK, 0, 1)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, RemoveSmallPartOfRange) {
	UnifiedTree<Item<1>, 1> tree{{8}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 8),
		makeOp1(OType::REMOVE, 4, 5),
		makeOp1(OType::CHECK, 3, 4),
		makeOp1(OType::CHECK, 4, 5),
		makeOp1(OType::CHECK, 5, 6),
		makeOp1(OType::CHECK, 0, 8)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, RemoveLargeRange) {
	UnifiedTree<Item<1>, 1> tree{{8}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 4, 5),
		makeOp1(OType::REMOVE, 0, 8),
		makeOp1(OType::CHECK, 3, 4),
		makeOp1(OType::CHECK, 4, 5),
		makeOp1(OType::CHECK, 5, 6),
		makeOp1(OType::CHECK, 0, 8)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, AddCheckTree) {
	UnifiedTree<Item<1>, 1> tree{{8}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 0, 5),
		makeOp1(OType::CHECK, 5, 6),
		makeOp1(OType::CHECK, 4, 6),
		makeOp1(OType::ADD, 4, 8),
		makeOp1(OType::CHECK, 5, 6)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, RemoveSmall1) {
	UnifiedTree<Item<1>, 1> tree{{4}};
	vector<Operation<1>> ops = {
		makeOp1(OType::ADD, 1, 3),
		makeOp1(OType::REMOVE, 1, 2),
		makeOp1(OType::CHECK, 0, 1)};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest1D, RandomAddCheck32) {
	constexpr int size = 32;
	for(int i=0; i<100; ++i) {
//		cout<<"\nRun "<<i<<'\n';
		UnifiedTree<Item<1>, 1> tree{{size}};
		mt19937 rng(i);
		runOps(tree, genRandomOps<1>(size, 10, {OType::ADD, OType::CHECK}, rng));
	}
}

TEST(UnifiedTreeTest1D, RandomAddRemove32) {
	constexpr int size = 32;
	for(int i=0; i<1000; ++i) {
//		cout<<"\nRun "<<i<<'\n';
		UnifiedTree<Item<1>, 1> tree{{size}};
		mt19937 rng(i);
		runOps(tree, genRandomOps<1>(size, 4, {OType::ADD, OType::REMOVE, OType::CHECK}, rng));
	}
}

TEST(UnifiedTreeTest2D, AddCheckUnitTree) {
	UnifiedTree<Item<2>, 2> tree{{1, 1}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {0,1}, {0,1}),
		makeOp2(OType::CHECK, {0,1}, {0,1})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, AddCheckRemove1) {
	UnifiedTree<Item<2>, 2> tree{{4, 4}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {2,3}, {0,4}),
		makeOp2(OType::REMOVE, {1,4}, {1,4}),
		makeOp2(OType::CHECK, {0,4}, {0,3})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, AddCheckRemove2) {
	UnifiedTree<Item<2>, 2> tree{{4, 4}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {2,3}, {0,4}),
		makeOp2(OType::REMOVE, {1,4}, {2,3}),
		makeOp2(OType::CHECK, {2,4}, {0,3})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, AddCheckRemove3) {
	UnifiedTree<Item<2>, 2> tree{{4, 4}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {0,4}, {0,1}),
		makeOp2(OType::REMOVE, {1,2}, {0,4}),
		makeOp2(OType::CHECK, {0,2}, {0,3})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, AddCheckRemove4) {
	UnifiedTree<Item<2>, 2> tree{{4, 4}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {0,2}, {1,4}),
		makeOp2(OType::REMOVE, {0,3}, {2,3}),
		makeOp2(OType::CHECK, {0,3}, {0,2})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, AddCheckRemove5) {
	UnifiedTree<Item<2>, 2> tree{{4, 4}};
	vector<Operation<2>> ops = {
		makeOp2(OType::ADD, {0,3}, {1,3}),
		makeOp2(OType::REMOVE, {0,4}, {1,3}),
		makeOp2(OType::CHECK, {1,4}, {2,4})};
	runOps(tree, ops);
}

TEST(UnifiedTreeTest2D, RandomAddCheck32) {
	for(int i=0; i<5; ++i) {
//		cout<<"\nRun "<<i<<'\n';
		UnifiedTree<Item<2>, 2> tree{{32, 32}};
		mt19937 rng(i);
		runOps(tree, genRandomOps<2>(32, 20, {OType::ADD, OType::CHECK}, rng));
	}
}

TEST(UnifiedTreeTest2D, RandomAddRemove32) {
	constexpr int size = 32;
	for(int i=0; i<1000; ++i) {
//		cout<<"\nRun "<<i<<'\n';
		UnifiedTree<Item<2>, 2> tree{{size, size}};
		mt19937 rng(i);
		runOps(tree, genRandomOps<2>(size, 4, {OType::ADD, OType::REMOVE, OType::CHECK}, rng));
	}
}

} // namespace
