#include "path.hpp"

#if 0
namespace {

struct EventSet {
};

void sweep();

} // namespace
#endif

template<int D>
int linkDistance(const ObstacleSet<D>& obstacles,
		Point<D> startP, Point<D> endP) {
	return 0;
}

template
int linkDistance<2>(const ObstacleSet<2>& obstacles, Point<2> startP, Point<2> endP);
