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
		} else {
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
	}
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
void mergePlaneResults(Decomposition<D>& result,
		Decomposition<D>& activeCells,
		Decomposition<D-1>& plane,
		int curZ) {
	sort(activeCells.begin(), activeCells.end(),
			[](const Cell<D>& a, const Cell<D>& b) {
				return compare(a.box,b.box) < 0;
			});
	sort(plane.begin(), plane.end(),
			[](const Cell<D-1>& a, const Cell<D-1>& b) {
				return compare(a.box,b.box) < 0;
			});
	Decomposition<D> newCells;
	size_t i=0, j=0;
	while(i < activeCells.size() && j < plane.size()) {
		Cell<D> a = activeCells[i];
		const Cell<D-1>& b = plane[j];
		int x = compare(a.box, b.box);
		if (x<0) {
			a.box[D-1].to = curZ;
			result.push_back(a);
			i++;
		} else {
			Box<D> box;
			for(int k=0; k<D-1; ++k) box[k] = b.box[k];
			int start = curZ;
			if (x==0) {
				start = a.box[D-1].from;
				i++;
			}
			box[D-1] = {start, curZ+1};
			newCells.emplace_back(box);
			j++;
		}
	}
	for(; i<activeCells.size(); ++i) {
		Cell<D> a = activeCells[i];
		a.box[D-1].to = curZ;
		result.push_back(a);
	}
	for(; j<plane.size(); ++j) {
		const Cell<D-1>& b = plane[j];
		Box<D> box;
		for(int k=0; k<D-1; ++k) box[k] = b.box[k];
		box[D-1] = {curZ, curZ+1};
		newCells.emplace_back(box);
	}
	activeCells = move(newCells);
}

void computeLinksBetweenLayers() {
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
	Decomposition<D> result;
	Decomposition<D> activeCells;
	for(int z: depths) {
		ObstacleSet<D-1> crossSection;
		for(const auto& obs: obstacles) {
			if (obs.box[Z_AXIS].contains(z)) {
				crossSection.push_back({obs.box.project(), obs.direction});
			}
		}
		Decomposition<D-1> curPlane = decomposeFreeSpace(crossSection);
		mergePlaneResults(result, activeCells, curPlane, z);
		computeLinksBetweenLayers();
	}
	assert(activeCells.empty());
	return result;
}

template
Decomposition<3> decomposeFreeSpace<3>(const ObstacleSet<3>& obstacles);
