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
//		clearedIndices.resize(total);
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
//		clearedIndices.clear();
		for(int i=0; i<D; ++i) if (box[i].size()==0) return;
		Index ones;
		for(int i=0; i<D; ++i) ones[i]=1;
		removeRec(ones,0,0,box, visitor);
#if 1
		postRemove2(box);
//		postRemove(ones,0,0,box);
#else
		std::reverse(clearedIndices.begin(), clearedIndices.end());
		for(Index index: clearedIndices) {
			int totalIndex = computeIndex(index);
			Item& t = data[totalIndex];
			t.hasData.reset();
			Mask covered = getCovered(index, box);
			for(int d=0; d<D; ++d) {
				if (index[d] >= size[d]) continue;
				int x = index[d];
				int step = stepSize[d];
				int baseIndex = totalIndex - step*x;
				const auto& a = data[baseIndex + step*(2*x)];
				const auto& b = data[baseIndex + step*(2*x+1)];
				for(Mask i=0; i<1<<D; ++i) {
					if (!(1 & (i>>d)) && (covered | i) != ALL_MASK) {
						std::cout<<"pr "<<d<<' '<<i<<' '<<a.hasData[i]<<' '<<b.hasData[i]<<'\n';
						t.hasData[i] = t.hasData[i] | a.hasData[i] | b.hasData[i];
					}
				}
			}
			std::cout<<"Postremove res for "<<index<<' '<<box<<" : "<<t.hasData.to_ulong()<<'\n';
		}
#endif
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
			std::cout<<"add "<<toIndex(index)<<' '<<covered<<' '<<box<<'\n';
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
			std::cout<<"check "<<toIndex(index)<<' '<<covered<<' '<<x.hasData[covered ^ ALL_MASK]<<'\n';
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

	template<class V>
	void removeRec(Index index, int axis, Mask covered, const Box<D>& box, V&& visitor) {
		if (axis == D) {
//			clearedIndices.set(computeIndex(index));
#if 0
			int totalIndex = computeIndex(index);
			Item& x = data[totalIndex];
			if (covered != ALL_MASK && x.hasData[ALL_MASK]) {
				int splitAxis = 0;
				while(1 & (covered >> splitAxis)) ++splitAxis;
				int step = stepSize[splitAxis];
				int i = index[splitAxis];
				int baseIndex = totalIndex - step * i;
				std::cout<<"split "<<index<<" by "<<splitAxis<<'\n';
				assignItem(baseIndex + step * (2*i), x.data);
				assignItem(baseIndex + step * (2*i+1), x.data);
			}
#endif
			std::cout<<"Clear subtree "<<index<<' '<<covered<<'\n';
			clearSubtree(index, 0, covered, visitor);
			return;
		}
		int i = index[axis];
		Range range = rangeForIndex(axis, i);
		if (!box[axis].intersects(range)) return;
		if (box[axis].contains(range)) {
			removeRec(index, axis+1, covered | (1U << axis), box, visitor);
		} else {
			removeRec(index, axis+1, covered, box, visitor);
			index[axis] = 2*i;
			removeRec(index, axis, covered, box, visitor);
			index[axis] = 2*i+1;
			removeRec(index, axis, covered, box, visitor);
		}
	}

	template<class V>
	void clearSubtree(Index index, int axis, Mask covered, V&& visitor) {
		while(axis<D && !(1&(covered>>axis))) ++axis;
		if (axis == D) {
			int totalIndex = computeIndex(index);
//			clearedIndices.set(totalIndex);
			Item& t = data[totalIndex];
			if (covered != ALL_MASK && t.hasData[ALL_MASK]) {
				int splitAxis = 0;
				while(1 & (covered >> splitAxis)) ++splitAxis;
				int step = stepSize[splitAxis];
				int i = index[splitAxis];
				int baseIndex = totalIndex - step * i;
				std::cout<<"split "<<index<<" by "<<splitAxis<<'\n';
				assignItem(baseIndex + step * (2*i), t.data);
				assignItem(baseIndex + step * (2*i+1), t.data);
			} else if (t.hasData[ALL_MASK]) {
				visitor(index, t.data);
			}
//			bool res = t.hasData[0];
			std::cout<<"rm "<<index<<' '<<covered<<" : "<<t.hasData.to_ulong()<<'\n';
			for(Mask i=0; i<1<<D; ++i) {
				if ((i | covered) == ALL_MASK) {
					t.hasData.reset(i);
				}
			}
			return;
		}
		int i = index[axis];
		clearSubtree(index, axis+1, covered, visitor);
		if (i < size[axis]) {
			index[axis] = 2*i;
			clearSubtree(index, axis, covered, visitor);
			index[axis] = 2*i+1;
			clearSubtree(index, axis, covered, visitor);
		}
	}

	void postRemove(Index index, int axis, Mask covered, const Box<D>& box) {
		if (axis == D) {
			genSubtreeState(index);
			return;
		}
		int s = size[axis];
		Range range = box[axis];
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap) {
				postRemove(withIndex(index, axis, ap), axis+1, covered, box);
			}
			if (b != bp) {
				postRemove(withIndex(index, axis, bp), axis+1, covered, box);
			}
			if (a&1) {
				postRemove(withIndex(index, axis, a++), axis+1, covered | (1U<<axis), box);
			}
			if (!(b&1)) {
				postRemove(withIndex(index, axis, b--), axis+1, covered | (1U<<axis), box);
			}
		}
		for(; ap > 0; ap/=2, bp/=2) {
			postRemove(withIndex(index, axis, ap), axis+1, covered, box);
			if (ap != bp) {
				postRemove(withIndex(index, axis, bp), axis+1, covered, box);
			}
		}
	}

	void postRemove2(const Box<D>& box) {
		int s = size[0];
		Range range = box[0];
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap) fixSubtreeStates(ap);
			if (b != bp) fixSubtreeStates(bp);
			if (a&1) fixSubtreeStates(a++);
			if (!(b&1)) fixSubtreeStates(b--);
		}
		for(; ap > 0; ap/=2, bp/=2) {
			fixSubtreeStates(ap);
			if (ap != bp) fixSubtreeStates(bp);
		}
	}

	void fixSubtreeStates(int firstIndex) {
		Index index;
		for(int i=0; i<D; ++i) index[i]=0;
		index[0] = firstIndex;
		assert(D >= 1);
		fixSubtreeStatesRec(index, 1);
	}
	void fixSubtreeStatesRec(Index index, int axis) {
		if (axis == D) {
			return genSubtreeState(index);
		}
		for(int i=2*size[axis]-1; i>=0; --i) {
			fixSubtreeStatesRec(withIndex(index, axis, i), axis+1);
		}
	}

	void genSubtreeState(Index index) {
		int totalIndex = computeIndex(index);
		Item& t = data[totalIndex];
		if (t.hasData[ALL_MASK]) {
			std::cout<<"skip postremove for node "<<index<<'\n';
			return;
		}
		t.hasData.reset();
		for(int d=0; d<D; ++d) {
			if (index[d] >= size[d]) continue;
			int x = index[d];
			int step = stepSize[d];
			int baseIndex = totalIndex - step*x;
			const auto& a = data[baseIndex + step*(2*x)];
			const auto& b = data[baseIndex + step*(2*x+1)];
			for(Mask i=0; i<1<<D; ++i) {
				if (!(1 & (i>>d))) {
					std::cout<<"pr "<<d<<' '<<i<<' '<<a.hasData[i]<<' '<<b.hasData[i]<<'\n';
					t.hasData[i] = t.hasData[i] | a.hasData[i] | b.hasData[i];
				}
			}
		}
		std::cout<<"Postremove res for "<<index<<" : "<<t.hasData.to_ulong()<<'\n';
	}

	void removeInSubtree(Index index, int axis, const Box<D>& box) {
		Range range = rangeForIndex(axis, index[axis]);
		if (!range.intersects(box[axis])) return;
		int totalIndex = computeIndex(index);
		Item& item = data[totalIndex];
		if (!item.hasData(0)) return;
		if (axis == D) {
			if (item.hasData[ALL_MASK]) {
				visitor(index, item.data);
			}
			item.hasData.reset();
			return;
		}
		bool isParent = !box[axis].contains(range);
		if (isParent) {
		}
		int i = index[axis];
		if (i < size[axis]) {
			removeInSubtree(withIndex(index, axis, 2*i), axis, box);
			removeInSubtree(withIndex(index, axis, 2*i+1), axis, box);
		}
		if (isParent) {
		}
	}

	static Index& withIndex(Index& index, int axis, int x) {
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
//	std::vector<Index> clearedIndices;
};
