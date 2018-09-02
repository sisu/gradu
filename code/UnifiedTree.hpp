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

template<class T, int D>
class UnifiedTree {
public:
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

	void add(const Box<D>& box, const T& value) {
		addRec(0, 0, 0, box, value);
	}

	bool check(const Box<D>& box) const {
		return checkRec(0, 0, 0, box);
	}

	void remove(Box<D> box) {
		remove(box, [](const Index&, const T&){});
	}

	template<class V>
	void remove(Box<D> box, V&& visitor) {
		for(int i=0; i<D; ++i) if (box[i].size()==0) return;
		Index ones;
		for(int i=0; i<D; ++i) ones[i]=1;
		removeInSubtree(ones, 0, box, visitor);
	}

	Index getSize() const { return size; }

	Box<D> boxForIndex(const Index& index) const {
		Box<D> box;
		for(int i=0; i<D; ++i) box[i] = rangeForIndex(i, index[i]);
		return box;
	}

	Range rangeForIndex(int axis, int index) const {
		return TreeStructure{2*size[axis]}.indexToRange(index);
	}

private:
	struct Item {
		std::bitset<1<<D> hasData;
		T data;
	};
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

	void addRec(int index, int axis, Mask covered, const Box<D>& box, const T& value) {
		if (axis == D) {
//			std::cout<<"add "<<toIndex(index)<<' '<<covered<<' '<<box<<'\n';
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

	bool checkRec(int index, int axis, Mask covered, const Box<D>& box) const {
		if (axis == D) {
			const Item& x = data[index];
//			std::cout<<"check "<<toIndex(index)<<' '<<covered<<' '<<x.hasData[covered ^ ALL_MASK]<<'\n';
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

	void genSubtreeState(Index index, Mask covered = 0) {
		int totalIndex = computeIndex(index);
		Item& t = data[totalIndex];
		if (t.hasData[ALL_MASK]) {
//			std::cout<<"skip postremove for node "<<index<<'\n';
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
//		std::cout<<"Postremove res for "<<index<<' '<<covered<<" : "<<t.hasData.to_ulong()<<'\n';
	}

	template<class V>
	void removeInSubtree(Index index, int axis, const Box<D>& box, V&& visitor) {
		int totalIndex = computeIndex(index);
		Item& item = data[totalIndex];
		if (!item.hasData[0]) return;
		if (axis == D) {
			if (item.hasData[ALL_MASK]) {
				visitor(index, item.data);
			}
//			std::cout<<"Clear "<<index<<'\n';
			item.hasData.reset();
			return;
		}
		Range range = rangeForIndex(axis, index[axis]);
		if (!range.intersects(box[axis])) return;
//		std::cout<<" Enter remove "<<index<<' '<<axis<<'\n';
		bool isParent = !box[axis].contains(range);
		if (isParent) {
//			std::cout<<"splitting subtree "<<index<<' '<<axis<<'\n';
			propagateInSubtree(index, axis+1, box, axis);
		}
		removeInSubtree(index, axis+1, box, visitor);
		int i = index[axis];
		if (i < size[axis]) {
			removeInSubtree(withIndex(index, axis, 2*i), axis, box, visitor);
			removeInSubtree(withIndex(index, axis, 2*i+1), axis, box, visitor);
		}
		if (isParent) {
//			std::cout<<"computing child data for "<<index<<' '<<axis<<'\n';
			computeChildData(index, axis+1, box);
		}
//		std::cout<<" Exit remove "<<index<<' '<<axis<<'\n';
	}

	void propagateInSubtree(Index index, int axis, const Box<D>& box, int splitAxis) {
		if (axis == D) {
			int totalIndex = computeIndex(index);
			Item& t = data[totalIndex];
			if (!t.hasData[0]) return;
//			Mask covered = getCovered(index, box);
			int step = stepSize[splitAxis];
			int i = index[splitAxis];
			int baseIndex = totalIndex - step * i;
//			std::cout<<"   SPLIT "<<index<<" by "<<splitAxis<<' '<<t.hasData[ALL_MASK]<<'\n';
			if (t.hasData[ALL_MASK]) {
				assignItem(baseIndex + step * (2*i), t.data);
				assignItem(baseIndex + step * (2*i+1), t.data);
				t.hasData.reset(ALL_MASK);
			} else if (t.hasData[1 << splitAxis]) {
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

	void computeChildData(Index index, int axis, const Box<D>& box) {
		if (axis == D) {
			Mask covered = 0;
			for(int i=0; i<D; ++i) {
				Range r = rangeForIndex(i, index[i]);
				covered |= box[i].contains(r) << i;
			}
			return genSubtreeState(index, covered);
		}
//		Range range = rangeForIndex(axis, index[axis]);
//		if (!range.intersects(box[axis])) return;
		int i = index[axis];
		if (i < size[axis]) {
			computeChildData(withIndex(index, axis, 2*i), axis, box);
			computeChildData(withIndex(index, axis, 2*i+1), axis, box);
		}
		computeChildData(withIndex(index, axis, i), axis+1, box);
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
