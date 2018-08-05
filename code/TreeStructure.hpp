#pragma once

#include <iostream>
#include "Range.hpp"

struct TreeStructure {
	Range indexToRange(int i) const {
		int bits = 8*sizeof(int) - __builtin_clz(i);
		int rs = size >> bits;
		int st = i & ((1<<(bits-1))-1);
		return {st*rs, (st+1)*rs};
	}

	int rangeToIndex(Range r) const {
		int bits = __builtin_ffs(r.size()) - 1;
		return ((size>>1) | r.from) >> bits;
	}

	int size = 0;
};
