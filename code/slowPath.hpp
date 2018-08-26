#pragma once
#include "Box.hpp"
#include "decomposition.hpp"

template<int D>
int slowLinkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP);
