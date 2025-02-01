#pragma once

#include "Range.hpp"

// Defines how segment tree nodes are mapped to array indices in order to
// store them efficiently.
// 
// Each segment tree node represents a range: The root node (1) represents
// the largest possible range. Each internal node (i) has children (2*i) and
// (2*i+1) representing its range split in halves.
struct TreeStructure {
	Range indexToRange(int i) const {
		// Highest set bit defines the level in the tree.
		int bits = 8*sizeof(int) - __builtin_clz(i);
		// Each tree level halves the size of the ranges.
		int rangeSize = size >> bits;
		// The lower bits define the starting point of the range.
		int start = i & ((1<<(bits-1))-1);
		return {start*rangeSize, (start+1)*rangeSize};
	}

	int rangeToIndex(Range r) const {
		// Range is assumed to be power of 2.
		int bits = __builtin_ffs(r.size()) - 1;
		return ((size>>1) | r.from) >> bits;
	}

	int size = 0;
};
