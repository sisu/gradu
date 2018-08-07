#include "decomposition.hpp"
#include "Box.hpp"
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <unordered_map>
#include <cassert>

using namespace std;

struct Event {
	int pos = -1;
	int idx = -1;
	bool startObstacle = false;

	bool operator<(const Event& e) const {
		if (pos==e.pos) return startObstacle>e.startObstacle;
		return pos<e.pos;
	}
};

constexpr int X_AXIS = 0;
constexpr int Y_AXIS = 1;

constexpr int UP = 0;
constexpr int DOWN = 1;

struct DecomposeNode {
	Range xRange;
	int yStart = 0;

	bool operator<(const DecomposeNode& n) const {
		return xRange.from < n.xRange.from;
	}
};
bool operator<(const DecomposeNode& n, int i) {
	return n.xRange.from < i;
}
bool operator<(int i, const DecomposeNode& n) {
	return i < n.xRange.from;
}

template<>
Decomposition<2> decomposeFreeSpace<2>(const ObstacleSet<2>& obstacles) {
	vector<Event> events;
	for(int i=0; i<(int)obstacles.size(); ++i) {
		const auto& obs = obstacles[i];
		cout<<"obs "<<obs<<' '<<obs.box[X_AXIS].size()<<'\n';
		if (obs.box[X_AXIS].size() == 0) continue;
		events.push_back({obs.box[Y_AXIS].from, i, obs.direction == UP});
	}
	sort(events.begin(), events.end());

	set<DecomposeNode, less<>> nodeSet;
	Decomposition<2> decomposition;
	for(Event event : events) {
		const Range range = obstacles[event.idx].box[X_AXIS];
		cout<<"evt "<<event.idx<<' '<<range<<' '<<event.pos<<" : "<<event.startObstacle<<'\n';
		if (event.startObstacle) {
			auto it = nodeSet.upper_bound(range.from);
			assert(it != nodeSet.begin());
			--it;
			Range oldRange = it->xRange;
			Box<2> box{oldRange, {it->yStart, event.pos}};
			decomposition.emplace_back(box);
			it = nodeSet.erase(it);
			if (oldRange.from < range.from) {
				DecomposeNode node{{oldRange.from, range.from}, event.pos};
				nodeSet.insert(node);
			}
			if (oldRange.to > range.to) {
				DecomposeNode node{{range.to, oldRange.to}, event.pos};
				nodeSet.insert(node);
			}
		} else {
			Range totalRange = range;
			auto it = nodeSet.upper_bound(range.from);
			if (it != nodeSet.begin()) {
				--it;
			}
			while(it != nodeSet.end() && it->xRange.from <= totalRange.to) {
				Box<2> box{it->xRange, {it->yStart, event.pos}};
				decomposition.emplace_back(box);
				totalRange = totalRange.union_(it->xRange);
				it = nodeSet.erase(it);
			}
			DecomposeNode node{totalRange, event.pos};
			nodeSet.insert(std::move(node));
		}
	}
	return decomposition;
}

template<int D>
Decomposition<D> decomposeFreeSpace(const ObstacleSet<D>& obstacles) {
	return {};
}

template
Decomposition<3> decomposeFreeSpace<3>(const ObstacleSet<3>& obstacles);
