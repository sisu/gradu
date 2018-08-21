#include "path.hpp"

namespace {

using namespace std;

enum class EventType { ADD_RECT, CELL, OBSTACLE };

template<int D>
struct Event {
	EventType type;
	int cell = -1;
	int position = -1;
	Box<D-1> box;
};

template<int D>
struct EventSet {
	vector<Event<D>> events[2*D];
};

void sweep() {
}

} // namespace

template<int D>
int linkDistance(const ObstacleSet<D>& obstacles,
		Point<D> startP, Point<D> endP) {
	Decomposition<D> decomposition = decomposeFreeSpace(obstacles);
	EventSet<D> currentEvents;
	for(int d=0; d<D; ++d) {
	}
	return 0;
}

template
int linkDistance<2>(const ObstacleSet<2>& obstacles, Point<2> startP, Point<2> endP);
