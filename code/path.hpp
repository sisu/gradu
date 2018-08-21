#pragma once
#include "Box.hpp"
#include "decomposition.hpp"

template<int D>
int linkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP);
