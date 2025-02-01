#pragma once
#include "Box.hpp"
#include "decomposition.hpp"

// Computes the minimum-link path between `startP` and `endP` and returns the
// link distance.
template<int D>
int linkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP);
