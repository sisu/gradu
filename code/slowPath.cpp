#include "slowPath.hpp"

#include "util.hpp"

#include <array>
#include <iostream>
#include <vector>

namespace {

using namespace std;

template<int D>
using Index = Point<D>;

template<int D>
struct Grid {
	Grid(Index<D> sz, int value) {
		int total = 1;
		for(int i=D-1; i>=0; --i) {
			stepSize[i] = total;
			size[i] = sz[i];
			total *= sz[i];
		}
		data.resize(total, value);
	}

	void setAll(Box<D> box, int value) {
		setRec(0, 0, box, value);
	}

	void setRec(int index, int axis, const Box<D>& box, int value) {
		if (axis == D) {
			assert(index>=0);
			assert(index<(int)data.size());
			data[index] = value;
		} else {
			Range range = box[axis];
			int step = stepSize[axis];
			for(int i=range.from; i<range.to; ++i) {
				setRec(index + step * i, axis+1, box, value);
			}
		}
	}

	int& get(Point<D> p) { return data[getIndex(p)]; }

	int& operator[](int i) { return data[i]; }
	int operator[](int i) const { return data[i]; }

	int getIndex(Point<D> p) {
		int x = 0;
		for(int i=0; i<D; ++i) x += stepSize[i] * p[i];
		return x;
	}

	vector<int> data;
	Index<D> size;
	Index<D> stepSize;
};

#if 0
template<int D>
void sortInDir(vector<Point<D>>& pts, int dir) {
	sort(pts.begin(), pts.end(), [dir](const Point<D>& a, const Point<D>& b) {
		return (a[dir>>1] < b[dir>>1]) ^ !(dir&1);
	});
}
#endif

template<int D>
void sweep(Grid<D>& grid, int dir, vector<int>& curP, vector<int>& nextP) {
#if 0
	sortInDir(curP, dir);
	int axis = dir/2;
	int start = dir&1 ? 0 : grid.size[axis]-1;
	int end = dir&1 ? grid.size[axis] : -1;
	int step = dir&1 ? 1 : -1;
	size_t idx = 0;
	vector<Point<D>> active;
	for(int i=start; i!=end; i+=step) {
		while(idx < curP.size() && curP[idx][axis]==i) {
			active.push_back(curP[idx]);
		}
	}
#endif
	size_t origSize = nextP.size();
	int axis = dir/2;
	int stepDir = dir&1 ? 1 : -1;
	int step = stepDir * grid.stepSize[axis];
	for(int pt : curP) {
		pt += step;
		while(pt>=0 && pt<(int)grid.data.size() && grid[pt] == -1) {
			grid[pt] = -2;
			nextP.push_back(pt);
			pt += step;
		}
	}
	for(size_t i=0; i<nextP.size(); ++i) grid[nextP[i]] = -1;
}

} // namespace

template<int D>
int slowLinkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP) {
	if (startP == endP) return 0;
	Index<D> size;
	for(const Obstacle<D>& obs: obstacles) {
		for(int i=0; i<D; ++i) {
			size[i] = max(size[i], obs.box[i].to+1);
		}
	}
	Grid<D> grid(size, -1);
	for(const Obstacle<D>& obs: obstacles) {
		Box<D> box = obs.box;
		int axis = obs.direction / 2;
		if (obs.direction & 1) box[axis].from--;
		else box[axis].to++;
		grid.setAll(box, -2);
	}

	vector<int> cur, next;
	cur.push_back(grid.getIndex(startP));
	int endI = grid.getIndex(endP);
	int dist = 0;
	while(!cur.empty() && grid[endI]<0) {
		dist++;
		for(int dir=0; dir<2*D; ++dir) {
			sweep(grid, dir, cur, next);
		}
		for(int p: next) grid[p] = dist;
		swap(cur, next);
		next.clear();
	}
	return grid[endI]<0 ? -1 : dist;
}

template
int slowLinkDistance<2>(const ObstacleSet<2>& obstacles, Point<2> startP, Point<2> endP);
template
int slowLinkDistance<3>(const ObstacleSet<3>& obstacles, Point<3> startP, Point<3> endP);
