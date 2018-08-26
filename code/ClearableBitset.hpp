#pragma once

#include <algorithm>
#include <limits>
#include <vector>

// Vector of booleans that can be reset (set all to zero) in constant time.
class ClearableBitset {
public:
	ClearableBitset(int size): data(size) {}

	void reset() {
		if (current == std::numeric_limits<Item>::max()) {
			std::fill(data.begin(), data.end(), 0);
			current = 0;
		}
		++current;
	}
	bool operator[](int i) const { return data[i] == current; }
	void set(int i) { data[i] = current; }

private:
	using Item = unsigned;
	std::vector<Item> data;
	Item current = 1;
};
