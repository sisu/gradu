#pragma once

#include "Box.hpp"
#include "TreeStructure.hpp"
#include "print.hpp"
#include "util.hpp"

#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>

// D-dimensional unified segment tree storing nodes of type T.
//
// A segment tree is a data structures for storing ranges. A D-dimensional
// segment tree allows storing D-dimensional rectangles, and is typically
// implemented as a 1-dimendional segment tree of (D-1)-dimensional inner
// trees. This means that the tree structure depends on the order of the
// coordinates: For example a 2D tree might have outer tree consisting of
// Y-ranges and inner trees consisting of X-ranges or the other way round.
//
// A *unified* D-dimensional segment tree combines the different
// representations into a single "tree". The "tree" is actually a DAG where
// each node represents a D-dimensional subrectange, and the links represent
// the ways to split the rectangle in half along all the different axes.
//
// The structure is efficient, storing all the data as a single flat array.
// Another important advantage of the unified tree over a regular
// multidimensional segment tree is that we can implement the `remove` function
// with good time complexity.
template<class T, int D>
class UnifiedTree {
public:
	// Index identifying a single internal node.
	using Index = std::array<int, D>;

	UnifiedTree(Index sizes) {
		int total = 1;
		for(int i=D-1; i>=0; --i) {
			int s = toPow2(sizes.begin()[i]);
			size[i] = s;
			stepSize[i] = total;
			total *= 2*s;
		}
		data.resize(total);
	}

	// Fills the region of `box` by `value`. Time complexity O(log^D n).
	void add(const Box<D>& box, const T& value) {
		addRec(0, 0, 0, box, value);
	}

	// Find if any added box intersects with the given box. Time complexity O(log^D n).
	bool check(const Box<D>& box) const {
		return checkRec(0, 0, 0, box);
	}

	// Clears the region defines by a given box from the tree. Time complexity
	// O(n^(D-1)*log n+k) where k is the number of cleared internal nodes.
	void remove(Box<D> box) {
		remove(box, [](const Index&, const T&){});
	}

	// Clears the region of `box`, calling `visitor` for each cleared node.
	template<class V>
	void remove(Box<D> box, V&& visitor) {
		for(int i=0; i<D; ++i) if (box[i].size()==0) return;
		Index ones;
		for(int i=0; i<D; ++i) ones[i]=1;
		removeInSubtree(ones, 0, box, visitor);
	}

	Index getSize() const { return size; }

	// Returns the box represented by internal node `index`.
	Box<D> boxForIndex(const Index& index) const {
		Box<D> box;
		for(int i=0; i<D; ++i) box[i] = rangeForIndex(i, index[i]);
		return box;
	}

	Range rangeForIndex(int axis, int index) const {
		return TreeStructure{2*size[axis]}.indexToRange(index);
	}

private:
	// Single internal node.
	struct Item {
		// Whether this node is completely covered by any box in the tree when
		// limited to certain axis. For example if we add rectangle
		// [5,8]x[3,4] to the tree, then node representing range [6,8]x[1,4]
		// would have hasData[0b10]=true, because the range [6,8] is contained
		// by the added range [5,8], but hasData[0b01]=hasData[0b11]=false
		// because the range [1,4] is not contained by [3,4].
		std::bitset<1<<D> hasData;
		T data;
	};
	// Bitmask representing dimensions where the query rectangle covers another rectangle.
	using Mask = unsigned;

	Mask getCovered(const Index& index, const Box<D>& box) const {
		Mask res = 0;
		for(int i=0; i<D; ++i) {
			res |= box[i].contains(rangeForIndex(i, index[i])) << i;
		}
		return res;
	}
	Index toIndex(int idx) const {
		Index res = {};
		for(int i=D-1; i>=0; --i) {
			if (!size[i]) continue;
			res[i] = idx % (2*size[i]);
			 idx /= 2*size[i];
		}
		return res;
	}

	// Recursively add box to the tree. The recursion is by the dimension: On
	// each recursion level we perform 1-dimensional segment tree traversal and
	// recurse into each subtree intersecting the added box on the current
	// `axis`.
	void addRec(int index, int axis, Mask covered, const Box<D>& box, const T& value) {
		if (axis == D) {
			Item& x = data[index];
			if (covered == ALL_MASK && !x.hasData[ALL_MASK]) {
				assignItem(index, value);
			} else {
				for(Mask i=0; i<1<<D; ++i) {
					if (i == (i & covered)) {
						x.hasData.set(i);
					}
				}
			}
			return;
		}
		int s = size[axis];
		int step = stepSize[axis];
		Range range = box[axis];
		if (range.size()==0) return;
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap) {
				addRec(index + step*ap, axis+1, covered, box, value);
			}
			if (b != bp) {
				addRec(index + step*bp, axis+1, covered, box, value);
			}
			if (a&1) {
				addRec(index + step*a++, axis+1, covered | (1U << axis), box, value);
			}
			if (!(b&1)) {
				addRec(index + step*b--, axis+1, covered | (1U << axis), box, value);
			}
		}
		for(; ap > 0; ap/=2, bp/=2) {
			addRec(index + step*ap, axis+1, covered, box, value);
			if (ap != bp) {
				addRec(index + step*bp, axis+1, covered, box, value);
			}
		}
	}

	void assignItem(int index, const T& item) {
		if (data[index].hasData[ALL_MASK]) return;
		data[index].data = item;
		data[index].hasData.set();
	}

	// Recursively check if `box` intersects any added boxes. The recursion
	// proceeds by the dimension: On each level we perform 1-dimensional
	// segment tree search, recursing into each subtree intersected by `box` in
	// the current `axis`.
	bool checkRec(int index, int axis, Mask covered, const Box<D>& box) const {
		if (axis == D) {
			const Item& x = data[index];
			return x.hasData[covered ^ ALL_MASK];
		}
		int s = size[axis];
		int step = stepSize[axis];
		Range range = box[axis];
		if (range.size()==0) return false;
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap && checkRec(index + step*ap, axis+1, covered, box)) return true;
			if (b != bp && ap!=bp && checkRec(index + step*bp, axis+1, covered, box)) return true;
			if ((a&1) && checkRec(index + step*a++, axis+1, covered | (1U << axis), box)) return true;
			if (!(b&1) && checkRec(index + step*b--, axis+1, covered | (1U << axis), box)) return true;
		}
		for(; ap > 0; ap/=2, bp/=2) {
			if (checkRec(index + step*ap, axis+1, covered, box)) return true;
			if (ap != bp && checkRec(index + step*bp, axis+1, covered, box)) return true;
		}
		return false;
	}

	// Recursively clears the region `box` from the tree. We clear all nodes
	// fully contained by box, and propagate data in nodes partially touched by
	// `box` into child nodes until all the nodes are either fully covered by
	// or fully outside `box`.
	//
	// The recursion proceeds in 2 dimensions: By `axis` and by `index[axis]`:
	// For each internal node we first descent to clear the lower-dimensional
	// subtree obtained by fixing index[axis]. Then we try splitting the
	// current node along `axis` and recursively clear the half-sized subtrees.
	//
	// Complexity: O(n^(D-axis-1)*log n+k), where k is the number of cleared nodes.
	template<class V>
	void removeInSubtree(Index index, int axis, const Box<D>& box, V&& visitor) {
		int totalIndex = computeIndex(index);
		Item& item = data[totalIndex];
		if (!item.hasData[0]) return;
		if (axis == D) {
			if (item.hasData[ALL_MASK]) {
				visitor(index, item.data);
			}
			item.hasData.reset();
			return;
		}
		Range range = rangeForIndex(axis, index[axis]);
		if (!range.intersects(box[axis])) return;
		bool isParent = !box[axis].contains(range);
		if (isParent) {
			propagateInSubtree(index, axis+1, box, axis);
		}
		removeInSubtree(index, axis+1, box, visitor);
		int i = index[axis];
		if (i < size[axis]) {
			removeInSubtree(withIndex(index, axis, 2*i), axis, box, visitor);
			removeInSubtree(withIndex(index, axis, 2*i+1), axis, box, visitor);
		}
		if (isParent) {
			computeChildData(index, axis+1, box);
		}
	}

	// Split all nodes in subtree partially intersected by the removed `box`
	// along `splitAxis`. Complexity O(n^(D-axis)).
	void propagateInSubtree(Index index, int axis, const Box<D>& box, int splitAxis) {
		if (axis == D) {
			int totalIndex = computeIndex(index);
			Item& t = data[totalIndex];
			if (!t.hasData[0]) return;
			int step = stepSize[splitAxis];
			int i = index[splitAxis];
			int baseIndex = totalIndex - step * i;
			if (t.hasData[ALL_MASK]) {
				// This node is contained by some stored rectangle and we split
				// it.
				assignItem(baseIndex + step * (2*i), t.data);
				assignItem(baseIndex + step * (2*i+1), t.data);
				t.hasData.reset(ALL_MASK);
			} else if (t.hasData[1 << splitAxis]) {
				// This node is not fully contained by any stored rectangle,
				// but it intersects some stored rectangle and we propagate
				// that info to the children.
				Item& left = data[baseIndex + step * (2*i)];
				Item& right = data[baseIndex + step * (2*i+1)];
				left.hasData = left.hasData | t.hasData;
				right.hasData = right.hasData | t.hasData;
			}
			return;
		}
		Range range = rangeForIndex(axis, index[axis]);
		if (!range.intersects(box[axis])) return;
		propagateInSubtree(index, axis+1, box, splitAxis);
		int i = index[axis];
		if (i < size[axis]) {
			propagateInSubtree(withIndex(index, axis, 2*i), axis, box, splitAxis);
			propagateInSubtree(withIndex(index, axis, 2*i+1), axis, box, splitAxis);
		}
	}

	// Recursively compute the `hasData` bitmasks in the subtree of `index`
	// after clearing `box`. Complexity O(n^(D-axis)).
	void computeChildData(Index index, int axis, const Box<D>& box) {
		if (axis == D) {
			Mask covered = 0;
			for(int i=0; i<D; ++i) {
				Range r = rangeForIndex(i, index[i]);
				covered |= box[i].contains(r) << i;
			}
			return genSubtreeState(index, covered);
		}
		int i = index[axis];
		if (i < size[axis]) {
			computeChildData(withIndex(index, axis, 2*i), axis, box);
			computeChildData(withIndex(index, axis, 2*i+1), axis, box);
		}
		computeChildData(withIndex(index, axis, i), axis+1, box);
	}

	// Refill the `hasData` bitmask for node in `index` based on child nodes
	// along all axes.
	void genSubtreeState(Index index, Mask covered = 0) {
		int totalIndex = computeIndex(index);
		Item& t = data[totalIndex];
		if (t.hasData[ALL_MASK]) {
			return;
		}
		t.hasData.reset();
		for(int d=0; d<D; ++d) {
			if (index[d] >= size[d]) continue;
			if (1 & (covered >> d)) continue;
			int x = index[d];
			int step = stepSize[d];
			int baseIndex = totalIndex - step*x;
			const auto& a = data[baseIndex + step*(2*x)];
			const auto& b = data[baseIndex + step*(2*x+1)];

			std::bitset<1<<D> dirMask = 0;
			for(Mask i=0; i<1<<D; ++i) if (!(1 & i>>d)) dirMask.set(i);
			t.hasData = t.hasData | ((a.hasData | b.hasData) & dirMask);
		}
	}

	static Index withIndex(Index index, int axis, int x) {
		index[axis] = x;
		return index;
	}

	int computeIndex(const Index& index) const {
		int r=0;
		for(int i=0; i<D; ++i) r += stepSize[i] * index[i];
		return r;
	}

	static constexpr Mask ALL_MASK = (1U<<D)-1;

	Index size = {};
	Index stepSize = {};
	std::vector<Item> data;
};
