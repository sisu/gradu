#pragma once

#include <algorithm>
#include <vector>

template<class T>
void sortUnique(std::vector<T>& v) {
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());
}

inline int toPow2(int x) {
	while(x & (x-1)) x+=x&-x;
	return x;
}
