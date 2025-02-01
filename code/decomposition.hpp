#pragma once
#include "Box.hpp"
#include "print.hpp"
#include <vector>

// Cell of `D`-dimensional free space decomposition.
template<int D>
struct Cell {
	Cell(Box<D> b): box(b) {}

	// Dimensions of the cell.
	Box<D> box;
	// Neighbor cells in each direction.
	std::vector<int> links[2*D];
	// Neighbor obstacles in each direction.
	std::vector<int> obstacles[2*D];
};
template<int D>
std::ostream& operator<<(std::ostream& o, const Cell<D>& c) {
	return o<<"Cell{"<<c.box<<"}";
}

// Free-space decomposition is represented as a vector of cells that have links
// to neighbor cells and obstacles represented by array indices.
template<int D>
using Decomposition = std::vector<Cell<D>>;

// Obstacle in free-space decomposition.
template<int D>
struct Obstacle {
	Box<D> box;
	int direction = 0;
};
template<int D>
std::ostream& operator<<(std::ostream& o, const Obstacle<D>& x) {
	return o<<"Obs{"<<x.box<<' '<<x.direction<<"}";
}

template<int D>
using ObstacleSet = std::vector<Obstacle<D>>;

// Returns decomposition of the free space (space not contained by any
// obstacles) into rectangular cells. Complexity O(n^(D-1)*log n).
template<int D>
Decomposition<D> decomposeFreeSpace(const ObstacleSet<D>& obstacles);
