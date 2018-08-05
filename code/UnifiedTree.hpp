#pragma once

#include "TreeStructure.hpp"
#include <vector>

template<int D, class T>
struct UnifiedTree {
	struct Visitor {
		virtual ~Visitor() {}
		virtual bool visitParent(T* children[D]) = 0;
		virtual bool exitParent(T* children[D]) = 0;
		virtual bool visitCanonical(T* children[D]) = 0;
	};
	using Index = std::array<D, int>;

	UnifiedTree(Index index): size(index) {
		int total = 1;
		for(int i: index) total *= i;
		data.resize(total);
	}

	void Visit(Index from, Index to) {
	}

private:
	bool VisitRec(int idx, int endIdx, int axis, T* children[D], Visitor& visitor) {
		if (axis == D) {
			return visitor.VisitCanonical(children);
		}
		int step = stepSize[axis];
		if (idx + step < endIdx) {
		}
	}

	Index size;
	Index stepSize;
	std::vector<T> data;
};
