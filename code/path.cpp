#include "path.hpp"

#include "UnifiedTree.hpp"
#include <algorithm>
#include <cassert>
#include <queue>

using namespace std;

namespace std {
template<class T, size_t N>
ostream& operator<<(ostream& out, const array<T, N>& arr) {
	out<<'[';
	for(size_t i=0; i<N; ++i) {
		if (i) out<<' ';
		out<<arr[i];
	}
	return out<<']';
}
} // namespace std

namespace {

enum class EventType { ADD_RECT, CELL, OBSTACLE };
const string eventTypeNames[] = {"add", "cell", "obstacle"};

template<int D>
struct Event {
	EventType type = EventType::ADD_RECT;
	int cell = -1;
	int position = -1;
	Box<D-1> box;

	bool operator<(const Event& e) const {
		if (position != e.position) return position < e.position;
		return (int)type > (int)e.type;
	}
};

template<int D>
ostream& operator<<(ostream& out, const Event<D>& e) {
	return out<<"{"<<eventTypeNames[(int)e.type]<<' '<<e.cell<<' '<<e.position<<' '<<e.box<<"}";
}

template<int D>
struct EventSet {
	vector<Event<D>> events[2*D];

	void clear() {
		for(auto& v: events) v.clear();
	}
};

struct TreeItem {
	int start = -1;
};

template<int D>
Event<D> cellEvent(const Decomposition<D>& dec, int dir, int cell) {
	Event<D> event;
	event.type = EventType::CELL;
	event.cell = cell;
	event.position = dec[cell].box[dir>>1][dir&1];
	if (dir&1) event.position *= -1;
	return event;
}

template<int D>
Event<D> obstacleEvent(const ObstacleSet<D>& obs, int dir, int obstacle) {
	Event<D> event;
	event.type = EventType::OBSTACLE;
	event.cell = obstacle;
	event.position = obs[obstacle].box[dir>>1][dir&1];
	if (dir&1) event.position *= -1;
	return event;
}

template<int D>
array<int, D-1> buildSize(const Decomposition<D>& dec) {
	int s = 0;
	for(const Cell<D>& c: dec) {
		for(int i=0; i<D; ++i) {
			s = max(s, c.box[i].to);
		}
	}
	array<int, D-1> arr;
	for(int i=0; i<D-1; ++i) arr[i] = s;
	return arr;
}

template<int D>
struct IlluminateState {
	typedef UnifiedTree<TreeItem, D-1> Plane;

	IlluminateState(ObstacleSet<D> obs):
		obstacles(obs), decomposition(decomposeFreeSpace(obstacles)),
	plane(buildSize(decomposition)) {}

	void sweep(int dir) {
		cout<<"sweep "<<dir<<'\n';
		const int axis = dir/2;
		priority_queue<Event<D>> events(curEvents.events[dir].begin(), curEvents.events[dir].end());
		while(!events.empty()) {
			Event<D> event = events.top();
			cout<<"event "<<event<<'\n';
			events.pop();
			int position = dir&1 ? -event.position : event.position;

			if (event.type == EventType::ADD_RECT) {
				plane.add(event.box, {position});
			} else if (event.type == EventType::CELL) {
				const Cell<D>& cell = decomposition[event.cell];
				for(int obs: cell.obstacles[dir]) {
					events.push(obstacleEvent(obstacles, dir, obs));
				}
				for(int nb: cell.links[dir]) {
					Box<D-1> box = decomposition[nb].box.project(axis);
					if (plane.check(box)) {
						events.push(cellEvent(decomposition, dir, nb));
					}
				}
			} else {
				Box<D-1> box = obstacles[event.cell].box.project(dir/2);
				plane.remove(box, [](typename Plane::Index idx) {
					cout<<"remove "<<idx<<'\n';
				});
			}
		}
	}

	const ObstacleSet<D> obstacles;
	const Decomposition<D> decomposition;
	Point<D> endP;

	EventSet<D> curEvents;
	EventSet<D> nextEvents;

	Plane plane;
};

template<int D>
int pointCell(const Decomposition<D>& dec, Point<D> pt) {
	int i=0;
	while(!dec[i].box.contains(pt)) {
		++i;
		assert(i < (int)dec.size());
	}
	return i;
}

template<int D>
Box<D> unitBox(Point<D> pt) {
	Box<D> box;
	for(int i=0; i<D; ++i) box[i] = {pt[i], pt[i]+1};
	return box;
}

} // namespace

template<int D>
int linkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP) {
	IlluminateState<D> state(obstacles);
	state.endP = endP;
	const auto& decomposition = state.decomposition;
	int startCell = pointCell(decomposition, startP);
	Box<D> startBox = unitBox(startP);
	for(int d=0; d<D; ++d) {
		Event<D> e;
		e.type = EventType::ADD_RECT;
		e.position = startP[d];
		e.box = startBox.project(d);
		state.curEvents.events[2*d].push_back(e);
		e.position = -startP[d];
		state.curEvents.events[2*d+1].push_back(e);
	}
	for(int i=0; i<2*D; ++i) {
		state.curEvents.events[i].push_back(cellEvent(decomposition, i, startCell));
	}
	for(int i=0; i<2*D; ++i) {
		state.sweep(i);
	}
	return 0;
}

template
int linkDistance<2>(const ObstacleSet<2>& obstacles, Point<2> startP, Point<2> endP);
