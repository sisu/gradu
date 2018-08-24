#pragma once

//#include "TreeStructure.hpp"
#include "Box.hpp"
#include <array>
#include <vector>
#include <cassert>

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
			int s = sizes.begin()[i];
			size[i] = s;
			stepSize[i] = total;
			total *= 2*s;
		}
		data.resize(total);
	}

	void add(const Box<D>& box, const T& value) {
		addRec(0, 0, box, value);
	}

	bool check(const Box<D>& box) const {
		checkRec(0, 0, box);
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
		int s = size[D];
		int step = stepSize[axis];
		Range range = box[axis];
		for(int a=s+range.from, b = s+range.to; a<=b; a/=2, b/=2) {
			if (a&1) addRec(index + step*a++, axis+1, box, value);
			if (!(b&1)) addRec(index + step*b--, axis+1, box, value);
		}
	}

	bool checkRec(int index, int axis, const Box<D>& box) const {
		if (axis == D) return data[index].hasData[D];
		int s = size[D];
		int step = stepSize[axis];
		Range range = box[axis];
		for(int a=s+range.from, b=s+range.to; a>0; a/=2, b/=2) {
			if (checkRec(index + step*a, axis+1, box)) return true;
			if (a != b && checkRec(index + step*b, axis+1, box)) return true;
		}
		return false;
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

	Index size = {};
	Index stepSize = {};
	std::vector<T> data;
};
