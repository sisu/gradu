#pragma once

#include <algorithm>
#include <vector>

template<class T>
void sortUnique(std::vector<T>& v) {
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());
}

