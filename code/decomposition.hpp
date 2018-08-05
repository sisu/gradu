#pragma once
#include "Box.hpp"
#include "print.hpp"
#include <vector>

template<int D>
struct Cell {
	Cell(Box<D> b): box(b) {}

	Box<D> box;
	std::vector<int> neighbors[2*D];
	std::vector<int> obstacles[2*D];
};
template<int D>
std::ostream& operator<<(std::ostream& o, const Cell<D>& c) {
	return o<<c.box;
}

template<int D>
using Decomposition = std::vector<Cell<D>>;

template<int D>
using Obstacle = Box<D>;

template<int D>
using ObstacleSet = std::vector<Obstacle<D>>;

template<int D>
Decomposition<D> decomposeFreeSpace(const ObstacleSet<D>& obstacles);
