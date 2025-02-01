#pragma once
#include "Box.hpp"
#include "decomposition.hpp"

// Slow but simple link distance computation used for verification of the more
// complex algorithm.
template<int D>
int slowLinkDistance(const ObstacleSet<D>& obstacles, Point<D> startP, Point<D> endP);
