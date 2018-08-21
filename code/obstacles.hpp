#pragma once

#include "decomposition.hpp"

ObstacleSet<2> makeObstaclesForPlane(const std::vector<std::string>& area);
ObstacleSet<3> makeObstaclesForVolume(std::vector<std::vector<std::string>> volume);
