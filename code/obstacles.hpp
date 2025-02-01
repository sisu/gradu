#pragma once

#include "decomposition.hpp"

// Test helper to construct obstacles based on 2D grid like {"#.#", "..#"}.
ObstacleSet<2> makeObstaclesForPlane(const std::vector<std::string>& area);
// Test helper to construct obstacles based on 3D grid like {{"#.#", "..#"}, {"###", ".#."}}.
ObstacleSet<3> makeObstaclesForVolume(std::vector<std::vector<std::string>> volume);
