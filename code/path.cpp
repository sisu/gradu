#include "path.hpp"

#include "print.hpp"
#include "UnifiedTree.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <queue>

using namespace std;

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

template<class T, class C>
void removeEquals(vector<T>& v1, vector<T>& v2, C&& compare) {
	sort(v1.begin(), v1.end(), compare);
	sort(v2.begin(), v2.end(), compare);
	auto it1=v1.begin(), it2=v2.begin();
	auto k1=it1, k2=it2;
	while(it1 != v1.end() && it2 != v2.end()) {
		if (compare(*it1, *it2)) {
			if (it1 != k1) iter_swap(k1, it1);
			++k1,++it1;
		} else if (compare(*it2, *it1)) {
			if (it2 != k2) iter_swap(k2, it2);
			++k2,++it2;
		} else {
			++it1,++it2;
		}
	}
	for(; it1 != v1.end(); ++it1, ++k1) if (it1 != k1) iter_swap(it1, k1);
	for(; it2 != v2.end(); ++it2, ++k2) if (it2 != k2) iter_swap(it2, k2);
	v1.erase(k1, v1.end());
	v2.erase(k2, v2.end());
}

template<class T, class M>
void mergeAdjacentElements(vector<T>& vec, M&& tryMerge) {
	auto it = vec.begin(), keep=it;
	for(; it != vec.end(); ++it, ++keep) {
		auto n = next(it);
		if (n==vec.end()) {
			iter_swap(it++, keep++);
			break;
		}
		if (tryMerge(*it, *n)) {
			iter_swap(it, keep);
			++it;
		} else if (it != keep) {
			iter_swap(it, keep);
		}
	}
	vec.erase(keep, vec.end());
}

template<int D>
void mergeAdjacentEvents(vector<Event<D>>& events,
		int eventAxis, int mergeAxis) {
	int projAxis = mergeAxis - (mergeAxis > eventAxis);
	sort(events.begin(), events.end(), [projAxis](const Event<D>& a, const Event<D>& b) {
		if (a.position != b.position) return a.position < b.position;
		for(int i=0; i<D-1; ++i) if (i != projAxis) {
			for(int j=0; j<2; ++j) {
				if (a.box[i][j] != b.box[i][j]) return a.box[i][j] < b.box[i][j];
			}
		}
		for(int j=0; j<2; ++j) {
			if (a.box[projAxis][j] != b.box[projAxis][j]) return a.box[projAxis][j] < b.box[projAxis][j];
		}
		return false;
	});
	mergeAdjacentElements(events, [projAxis](Event<D>& a, Event<D>& b) {
		if (a.position != b.position) return false;
		for(int i=0; i<D-1; ++i) if (i != projAxis && a.box[i] != b.box[i]) return false;
		if (a.box[projAxis].to < b.box[projAxis].from) return false;
		a.box[projAxis].to = max(a.box[projAxis].to, b.box[projAxis].to);
		return true;
	});
}

template<int D>
struct EventSet {
	vector<Event<D>> events[2*D];
	vector<int> cells;

	bool empty() const {
		for(const auto& e: events) if (!e.empty()) return false;
		return true;
	}
	void clear() {
		for(auto& v: events) v.clear();
	}
	void genCellEvents(const Decomposition<D>& dec);

	void filterAddEvents() {
		for(int a=0; a<D; ++a) {
			removeEquals(events[2*a], events[2*a+1], [](const Event<D>& a, const Event<D>& b) {
				int p1=abs(a.position), p2=abs(b.position);
				if (p1!=p2) return p1<p2;
				return a.position < -b.position || a.box < b.box;
			});
		}
		for(int a=0; a<D; ++a) {
			for(int m=0; m<D; ++m) if (a!=m) {
				mergeAdjacentEvents(events[2*a], a, m);
				mergeAdjacentEvents(events[2*a+1], a, m);
			}
		}
	}
};

struct TreeItem {
	int start = -1;
};

ostream& operator<<(ostream& out, const TreeItem& item) {
	return out<<"("<<item.start<<")";
}

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
void EventSet<D>::genCellEvents(const Decomposition<D>& dec) {
	sortUnique(cells);
	for(int c: cells) {
		for(int i=0; i<2*D; ++i) {
			events[i].push_back(cellEvent(dec, i, c));
		}
	}
	cells.clear();
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
Event<D> addRectEvent(const Box<D>& box, int dir) {
	Event<D> event;
	event.type = EventType::ADD_RECT;
	event.position = box[dir>>1][dir&1];
	if (dir&1) event.position *= -1;
	event.box = box.project(dir>>1);
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
	using Index = typename Plane::Index;

	IlluminateState(ObstacleSet<D> obs):
		obstacles(obs), decomposition(decomposeFreeSpace(obstacles)),
	plane(buildSize(decomposition)),
	obstacleReachTime(obstacles.size(), -1)
	{}

	void newRound() {
		++curStep;
		swap(curEvents, nextEvents);
		nextEvents.clear();
		curEvents.filterAddEvents();
		curEvents.genCellEvents(decomposition);
	}

	void sweep(int dir) {
		cout<<"    sweep "<<dir<<'\n';
		const int axis = dir/2;
		priority_queue<Event<D>> events(curEvents.events[dir].begin(), curEvents.events[dir].end());
		while(!events.empty()) {
			Event<D> event = events.top();
			cout<<"event "<<event<<'\n';
			events.pop();
			int position = dir&1 ? -event.position : event.position;

			if (event.type == EventType::ADD_RECT) {
				plane.add(event.box, {position});
				cout<<"add box "<<event.box<<'\n';
			} else if (event.type == EventType::CELL) {
				const Cell<D>& cell = decomposition[event.cell];
				if (!plane.check(cell.box.project(axis))) {
					continue;
				}
				nextEvents.cells.push_back(event.cell);
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
				int& time = obstacleReachTime[event.cell];
				if (time<0) {
					time = curStep;
				}
				Box<D-1> box = obstacles[event.cell].box.project(dir/2);
				cout<<"remove box "<<box<<'\n';
				plane.remove(box, [&](Index idx, const TreeItem& item) {
					onRemove(axis, idx, item, position, time);
				});
			}
		}
	}

	void onRemove(int axis, Index index, const TreeItem& item, int position, int obsTime) {
		Range range = item.start<position ? Range{item.start, position} : Range{position, item.start};
		cout<<"Remove "<<axis<<' '<<index<<' '<<item<<' '<<position<<'\n';
		if (item.start == position) return;
		Box<D> box;
		for(int i=0; i<D; ++i) {
			box[i] = i<axis ? plane.rangeForIndex(i, index[i])
				: i==axis ? range
				: plane.rangeForIndex(i-1, index[i-1]);
		}
		cout<<"rm box "<<box<<'\n';
		if (box.contains(endP)) {
			endFound = true;
		}
		if (curStep > obsTime + D + 1) {
			return;
		}
		for(int i=0; i<2*D; ++i) {
			if (i/2 != axis) {
				nextEvents.events[i].push_back(addRectEvent(box, i));
			}
		}
	}

	const ObstacleSet<D> obstacles;
	const Decomposition<D> decomposition;
	Point<D> endP;
	bool endFound = false;

	EventSet<D> curEvents;
	EventSet<D> nextEvents;

	Plane plane;

	vector<int> obstacleReachTime;
	int curStep = 0;
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
	cout<<"decomposition: "<<decomposition<<' '<<startP<<'\n';
	int startCell = pointCell(decomposition, startP);
	Box<D> startBox = unitBox(startP);
	if (startBox.contains(endP)) return 0;
	state.curEvents.cells.push_back(startCell);
	for(int i=0; i<2*D; ++i) {
		auto& events = state.curEvents.events[i];
		events.push_back(addRectEvent(startBox, i));
	}
	state.curEvents.genCellEvents(decomposition);
	while(!state.curEvents.empty() && !state.endFound) {
		cout<<"\nround "<<state.curStep<<'\n';
		for(int i=0; i<2*D; ++i) {
			state.sweep(i);
		}
		state.newRound();
	}
	return state.endFound ? state.curStep : -1;
}

template
int linkDistance<2>(const ObstacleSet<2>& obstacles, Point<2> startP, Point<2> endP);
template
int linkDistance<3>(const ObstacleSet<3>& obstacles, Point<3> startP, Point<3> endP);
