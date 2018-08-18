#include "decomposition.hpp"
#include "Box.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <map>
#include <set>
#include <vector>

using namespace std;

struct Event {
	int pos = -1;
	int idx = -1;
	bool startObstacle = false;

	bool operator<(const Event& e) const {
		if (pos!=e.pos) return pos<e.pos;
		return startObstacle>e.startObstacle;
	}
};

constexpr int X_AXIS = 0;
constexpr int Y_AXIS = 1;
constexpr int Z_AXIS = 2;

constexpr int UP = 2;
constexpr int DOWN = 3;

struct DecomposeNode {
	Range xRange;
	int yStart = 0;
	mutable vector<int> backLinks;
	mutable vector<int> backObstacles;

	Cell<2> consumeToCell(int yEnd, int obstacle) const {
		Cell<2> res(Box<2>{{xRange, {yStart, yEnd}}});
		res.links[UP] = move(backLinks);
		res.obstacles[UP] = move(backObstacles);
		if (obstacle >= 0) {
			res.obstacles[DOWN].push_back(obstacle);
		}
		cout<<"Creating cell "<<res.box<<' '<<res.links[UP]<<'\n';
		return res;
	}

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

template<class T>
void moveContentsUnordered(vector<T>& to, vector<T>& from) {
	if (from.size() > to.size()) {
		from.swap(to);
	}
	to.insert(to.end(), make_move_iterator(from.begin()), make_move_iterator(from.end()));
}

class Sweepline {
public:
	Sweepline(const ObstacleSet<2>* obstacles): obstacles(*obstacles) {}

	void handleEvent(const Event& event) {
		if (event.startObstacle) {
			addObstacleEvent(event);
		} else {
			endObstacleEvent(event);
		}
	}

	Decomposition<2>& result() { return decomposition; }

private:
	void addObstacleEvent(const Event& event) {
		const Range range = obstacles[event.idx].box[X_AXIS];
		auto it = nodeSet.upper_bound(range.from);
		assert(it != nodeSet.begin());
		--it;
		Range oldRange = it->xRange;
		vector<int> links;
		if (it->yStart < event.pos) {
			links.push_back(decomposition.size());
			decomposition.push_back(it->consumeToCell(event.pos, event.idx));
		} else {
			links = std::move(it->backLinks);
			for(int i: links) {
				decomposition[i].obstacles[DOWN].push_back(event.idx);
			}
		}
		it = nodeSet.erase(it);
		if (oldRange.from < range.from) {
			DecomposeNode node{{oldRange.from, range.from}, event.pos, links, {}};
			nodeSet.insert(node);
		}
		if (oldRange.to > range.to) {
			DecomposeNode node{{range.to, oldRange.to}, event.pos, links, {}};
			nodeSet.insert(node);
		}
	}

	void endObstacleEvent(const Event& event) {
		const Range range = obstacles[event.idx].box[X_AXIS];
		Range totalRange = range;
		auto it = nodeSet.upper_bound(range.from);
		if (it != nodeSet.begin()) {
			--it;
		}
		vector<int> links;
		vector<int> obstacles = {event.idx};
		while(it != nodeSet.end() && it->xRange.from <= totalRange.to) {
			if (it->xRange.to < totalRange.from) {
				++it;
				continue;
			}
			if (it->yStart < event.pos) {
				links.push_back(decomposition.size());
				decomposition.push_back(it->consumeToCell(event.pos, -1));
			} else {
				moveContentsUnordered(links, it->backLinks);
				moveContentsUnordered(obstacles, it->backObstacles);
			}
			totalRange = totalRange.union_(it->xRange);
			it = nodeSet.erase(it);
		}
		DecomposeNode node{totalRange, event.pos, move(links), move(obstacles)};
		cout<<"insert to nodeset "<<totalRange<<' '<<event.idx<<'\n';
		nodeSet.insert(std::move(node));
	}

	const ObstacleSet<2>& obstacles;

	set<DecomposeNode, less<>> nodeSet;
	Decomposition<2> decomposition;
};

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

	Sweepline sweepline(&obstacles);
	for(Event event : events) {
		sweepline.handleEvent(event);
	}
	Decomposition<2> decomposition = move(sweepline.result());
	for(size_t i=0; i<decomposition.size(); ++i) {
		for(int d=0; d<4; ++d) {
			for(int j: decomposition[i].links[d]) {
				decomposition[j].links[d^1].push_back(i);
			}
		}
	}
	for(auto& c: decomposition) {
		for(int d=0; d<4; ++d) {
			sort(c.links[d].begin(), c.links[d].end());
			sort(c.obstacles[d].begin(), c.obstacles[d].end());
		}
	}
	return decomposition;
}

template<int A, int B>
int compare(const Box<A>& a, const Box<B>& b) {
	int n = min(A,B);
	for(int i=0; i<n; ++i) {
		for(int j=0; j<2; ++j) {
			if (a[i][j] != b[i][j]) return a[i][j] - b[i][j];
		}
	}
	return 0;
}

template<int D>
bool operator<(const Box<D>& a, const Box<D>& b) {
	for(int i=0; i<D; ++i) {
		for(int j=0; j<2; ++j) {
			if (a[i][j] != b[i][j]) return a[i][j] < b[i][j];
		}
	}
	return false;
}

template<int D>
vector<pair<int,int>> overlappingBoxes(
		const vector<Box<D>>& bs1,
		const vector<Box<D>>& bs2) {
	// TODO: more efficient impl
	vector<pair<int,int>> conns;
	for(size_t i=0; i<bs1.size(); ++i) {
		for(size_t j=0; j<bs2.size(); ++j) {
			if (bs1[i].intersects(bs2[j])) {
				conns.emplace_back(i,j);
			}
		}
	}
	return conns;
}

template<int D>
class SweepState {
public:
	SweepState(ObstacleSet<D> obstacles): obstacles(obstacles) {}

	void advanceToDepth(int z) {
		ObstacleSet<D-1> crossSection;
		for(const auto& obs: obstacles) {
			if (obs.box[Z_AXIS].contains(z)) {
				crossSection.push_back({obs.box.project(), obs.direction});
			}
		}
		Decomposition<D-1> curPlane = decomposeFreeSpace(crossSection);
		mergePlaneResults(curPlane, z);
		computeLinksBetweenLayers();
	}

	Decomposition<3>& result() { return decomposition; }

private:
	void computeLinksBetweenLayers() {
	}

	void mergePlaneResults(Decomposition<D-1>& plane, int curZ) {
		map<Box<D-1>, int> newMap;
		vector<int> planeIndex(plane.size());
		vector<Box<D-1>> addedBoxes;
		vector<int> addedIndex;
		for(size_t i=0; i<plane.size(); ++i) {
			const Box<D-1>& box = plane[i].box;
			auto it = activeIndex.find(box);
			int index;
			if (it != activeIndex.end()) {
				index = it->second;
				activeIndex.erase(it);
			} else {
				index = decomposition.size();
				decomposition.emplace_back(fromProj(box, curZ));
				addedBoxes.push_back(box);
				addedIndex.push_back(index);
			}
			newMap[box] = index;
			planeIndex[i] = index;
		}
		for(size_t i=0; i<plane.size(); ++i) {
			int from = planeIndex[i];
			for(int j=0; j<2*(D-1); ++j) {
				for(int x : plane[i].links[j]) {
					decomposition[from].links[j].push_back(planeIndex[x]);
				}
			}
		}
		vector<Box<D-1>> removedBoxes;
		vector<int> removedIndex;
		for(auto p : activeIndex) {
			decomposition[p.second].box[D-1].to = curZ;
			removedBoxes.push_back(p.first);
			removedIndex.push_back(p.second);
		}
		activeIndex = move(newMap);
		auto newLinks = overlappingBoxes(removedBoxes, addedBoxes);
		for(auto p: newLinks) {
			int a = removedIndex[p.first];
			int b = addedIndex[p.second];
			decomposition[a].links[2*(D-1)+1].push_back(b);
			decomposition[b].links[2*(D-1)].push_back(a);
		}
	}

	static Box<D> fromProj(const Box<D-1>& from, int start) {
		Box<D> box;
		for(int k=0; k<D-1; ++k) box[k] = from[k];
		box[D-1] = {start, -1};
		return box;
	}

	ObstacleSet<D> obstacles;

	Decomposition<D> decomposition;
	Decomposition<D> activeCells;

	map<Box<D-1>, int> activeIndex;
};

template<class T>
void sortUnique(vector<T>& v) {
	sort(v.begin(), v.end());
	v.erase(unique(v.begin(), v.end()), v.end());
}

template<int D>
Decomposition<D> decomposeFreeSpace(const ObstacleSet<D>& obstacles) {
	vector<int> depths;
	for(const auto& obs: obstacles) {
		if (obs.box[Z_AXIS].size() == 0) {
			depths.push_back(obs.box[Z_AXIS].from);
		}
	}
	sort(depths.begin(), depths.end());
	depths.erase(unique(depths.begin(), depths.end()), depths.end());

	SweepState<D> state(obstacles);
	for(int z: depths) {
		state.advanceToDepth(z);
	}
	Decomposition<D> result = move(state.result());
	for(Cell<D>& cell: result) {
		for(int i=0; i<2*D; ++i) {
			sortUnique(cell.links[i]);
			sortUnique(cell.obstacles[i]);
		}
	}
	return result;
}

template
Decomposition<3> decomposeFreeSpace<3>(const ObstacleSet<3>& obstacles);
