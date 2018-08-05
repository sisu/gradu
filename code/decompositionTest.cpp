#include "decomposition.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock-more-matchers.h>

using namespace std;
using namespace testing;

constexpr int UP = 0;
constexpr int DOWN = 1;
constexpr int LEFT = 2;
constexpr int RIGHT = 3;

Point<2> dir2[4] = {
	{{0, -1}},
	{{0, 1}},
	{{-1, 0}},
	{{1, 0}},
};

constexpr char USED = '#';

void addObstaclesOnLine(
		ObstacleSet<2>& result,
		const vector<string>& area,
		int x, int y, int dir) {
	const int h = area.size();
	const int w = area[0].size();
	const auto& dv = dir2[dir];
	const int dx = dv[0], dy = dv[1];
	const int ex = abs(dy), ey = abs(dx);
	const int len = ex ? w : h;
	int keep = ex ? y : x;
	if (dx>0 || dy>0) keep++;
	int count = 0;
	for(int i=0; i<len; ++i, x+=ex, y+=ey) {
		if (area[y][x]==USED && area[y+dy][x+dx]!=USED) {
			count++;
		} else {
			if (count) {
				result.push_back({
						{{{i-count, i}, {keep, keep}}},
						dir});
			}
			count = 0;
		}
	}
	if (count) {
		result.push_back({
				{{{len-count, len}, {keep, keep}}},
				dir});
	}
}

ObstacleSet<2> makeObstaclesForPlane(const vector<string>& area) {
	ObstacleSet<2> result;
	if (area.empty()) return result;
	int h = area.size();
	int w = area[0].size();
	for(int i=0; i+1<h; ++i) {
		addObstaclesOnLine(result, area, 0, i+1, UP);
		addObstaclesOnLine(result, area, 0, i, DOWN);
	}
	for(int i=0; i+1<w; ++i) {
		addObstaclesOnLine(result, area, i+1, 0, LEFT);
		addObstaclesOnLine(result, area, i, 0, RIGHT);
	}
	return result;
}

TEST(DecompositionTest, Decompose2Empty) {
	ObstacleSet<2> obs;
	EXPECT_THAT(decomposeFreeSpace(obs), IsEmpty());
}

TEST(DecompositionTest, Decompose2SingleCell) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"###", "#.#", "###"});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(result, SizeIs(1));
	Box<2> expected = {{{1,2}, {1,2}}};
	EXPECT_EQ(result[0].box, expected);
}
