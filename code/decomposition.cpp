#include "decomposition.hpp"
#include "Box.hpp"
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <unordered_map>

using namespace std;

struct Event {
	int pos = -1;
	int idx = -1;
	bool add = false;

	bool operator<(const Event& e) const {
		if (pos==e.pos) return add>e.add;
		return pos<e.pos;
	}
};

template<>
Decomposition<2> decomposeFreeSpace<2>(const ObstacleSet<2>& obstacles) {
	vector<Event> events;
	return events;
}

template<int D>
Decomposition<D> decomposeFreeSpace(const ObstacleSet<D>& obstacles) {
	return {};
}

template
Decomposition<3> decomposeFreeSpace<3>(const ObstacleSet<3>& obstacles);
