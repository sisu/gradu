#pragma once

#include "Box.hpp"
#include "TreeStructure.hpp"

#include <array>
#include <vector>
#include <cassert>
#include <iostream>

inline int toPow2(int x) {
	while(x & (x-1)) x+=x&-x;
	return x;
}

template<class T, int D>
class UnifiedTree {
public:
#if 0
	struct Visitor {
		virtual ~Visitor() {}
		virtual bool visitParent(T* children[D]) = 0;
		virtual bool exitParent(T* children[D]) = 0;
		virtual bool visitCanonical(T* children[D]) = 0;
	};
#endif
	using Index = std::array<int, D>;

	UnifiedTree(std::initializer_list<int> sizes) {
		assert((int)sizes.size() == D);
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
		Index ones;
		for(int i=0; i<D; ++i) ones[i]=1;
		removeRec(0,ones,0,0,box);
		postRemove(0,ones,0,0,box);
	}

	Index getSize() const { return size; }

private:
	using Mask = unsigned;

	void addRec(int index, int axis, Mask covered, const Box<D>& box, const T& value) {
		if (axis == D) {
			T& x = data[index];
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
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap) {
				addRec(index + step*ap, axis+1, covered, box, value);
			}
			if (b != bp) {
				addRec(index + step*bp, axis+1, covered, box, value);
			}
			if (a&1) {
//				std::cout<<"add a "<<a<<'\n';
				addRec(index + step*a++, axis+1, covered | (1U << axis), box, value);
			}
			if (!(b&1)) {
//				std::cout<<"add b "<<b<<'\n';
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

	bool checkRec(int index, int axis, Mask covered, const Box<D>& box) const {
		if (axis == D) {
			const T& x = data[index];
//			std::cout<<"check "<<index<<' '<<covered<<' '<<data.size()<<'\n';
			return x.hasData[covered ^ ALL_MASK];
		}
		int s = size[axis];
		int step = stepSize[axis];
		Range range = box[axis];
		if (range.size()==0) return false;
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
//			std::cout<<"check "<<a<<' '<<b<<' '<<ap<<' '<<bp<<'\n';
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

	void removeRec(int totalIndex, Index index, int axis, Mask covered, const Box<D>& box) {
		if (axis == D) {
			std::cout<<"rm "<<totalIndex<<' '<<covered<<' '<<box<<'\n';
			T& x = data[totalIndex];
			if (covered == ALL_MASK) {
				x.hasData.reset();
				return;
			}
			if (x.hasData[ALL_MASK]) {
				int splitAxis = 0;
				while(1 & (covered >> splitAxis)) ++splitAxis;
				int step = stepSize[splitAxis];
				int i = index[splitAxis];
				int baseIndex = totalIndex - step * i;
				assignItem(baseIndex + step * (2*i), x);
				assignItem(baseIndex + step * (2*i+1), x);
			}
			x.hasData.reset();
			return;
		}
		int i = index[axis];
		Range range = rangeForIndex(axis, i);
		std::cout<<"removerec "<<totalIndex<<' '<<axis<<' '<<i<<' '<<range<<" ; "<<box<<'\n';
		if (!box[axis].intersects(range)) return;
		if (range.contains(box[axis])) {
			removeRec(totalIndex + stepSize[axis] * i, index, axis+1, covered | (1U << axis), box);
		} else {
			removeRec(totalIndex + stepSize[axis] * i, index, axis+1, covered, box);
			index[axis] = 2*i;
			removeRec(totalIndex, index, axis, covered, box);
			index[axis] = 2*i+1;
			removeRec(totalIndex, index, axis, covered, box);
		}
	}

	void assignItem(int index, const T& item) {
		if (data[index].hasData[ALL_MASK]) return;
		data[index] = item;
		data[index].hasData.set();
	}

	void postRemove(int totalIndex, Index index, int axis, Mask covered, const Box<D>& box) {
		if (axis == D) {
			return;
		}
		int s = size[axis];
		int step = stepSize[axis];
		Range range = box[axis];
		int a,b,ap,bp;
		for(a=s+range.from, b=s+range.to-1, ap=a, bp=b; a<=b; a/=2, b/=2, ap/=2, bp/=2) {
			if (a != ap) {
				index[axis] = ap;
				postRemove(totalIndex + step*ap, index, axis+1, covered, box);
			}
			if (b != bp) {
				index[axis] = bp;
				postRemove(totalIndex + step*bp, index, axis+1, covered, box);
			}
			if (a&1) {
				index[axis] = a;
				postRemove(totalIndex + step*a++, index, axis+1, covered | (1U<<axis), box);
			}
			if (!(b&1)) {
				index[axis] = b;
				postRemove(totalIndex + step*b--, index, axis+1, covered | (1U<<axis), box);
			}
		}
		for(; ap > 0; ap/=2, bp/=2) {
			index[axis] = ap;
			postRemove(totalIndex + step*ap, index, axis+1, covered, box);
			if (ap != bp) {
				index[axis] = bp;
				postRemove(totalIndex + step*bp, index, axis+1, covered, box);
			}
		}
	}

	Range rangeForIndex(int axis, int index) const {
//		std::cout<<"rangeforindex "<<axis<<' '<<index<<' '<<size[axis]<<'\n';
		return TreeStructure{2*size[axis]}.indexToRange(index);
	}

#if 0
	bool VisitRec(int idx, int endIdx, int axis, T* children[D], Visitor& visitor) {
		if (axis == D) {
			return visitor.VisitCanonical(children);
		}
		int step = stepSize[axis];
		if (idx + step < endIdx) {
		}
	}
#endif
	static constexpr Mask ALL_MASK = (1U<<D)-1;

	Index size = {};
	Index stepSize = {};
	std::vector<T> data;
};
