#include "decomposition.hpp"
#include <cstring>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using namespace std;

using testing::ElementsAre;
using testing::IsEmpty;

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

vector<string> addBorderAroundArea(const vector<string>& area) {
	int h = area.size();
	int w = area[0].size();
	vector<string> res(h+2, string(w+2, '#'));
	for(int i=0; i<h; ++i) {
		memcpy(&res[i+1][1], &area[i][0], w);
	}
	return res;
}

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
				if (ex) {
					result.push_back({
							{{{i-count, i}, {keep, keep}}},
							dir});
				} else {
					result.push_back({
							{{{keep, keep}, {i-count, i}}},
							dir});
				}
			}
			count = 0;
		}
	}
	if (count) {
		if (ex) {
			result.push_back({
					{{{len-count, len}, {keep, keep}}},
					dir});
		} else {
			result.push_back({
					{{{keep, keep}, {len-count, len}}},
					dir});
		}
	}
}

ObstacleSet<2> makeObstaclesForPlane(const vector<string>& area0) {
	const vector<string> area = addBorderAroundArea(area0);
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

template<int D>
vector<Box<D>> getBoxes(const vector<Cell<D>>& cells) {
	vector<Box<D>> result;
	result.reserve(cells.size());
	for(const auto& c: cells) {
		result.push_back(c.box);
	}
	return result;
}

Box<2> box2(Range x, Range y) {
	return {{x, y}};
}

TEST(DecompositionTest2D, DecomposeEmpty) {
	ObstacleSet<2> obs;
	EXPECT_THAT(decomposeFreeSpace(obs), IsEmpty());
}

TEST(DecompositionTest2D, DecomposeSingleCell) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(box2({1,2}, {1,2})));
}

TEST(DecompositionTest2D, DecomposeTwoCells1) {
	ObstacleSet<2> obs = makeObstaclesForPlane({".#", ".."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1,2}, {1,2}), box2({1,3}, {2,3})));
}
TEST(DecompositionTest2D, DecomposeTwoCells2) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"..", "#."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1,3}, {1,2}), box2({2,3}, {2,3})));
}
TEST(DecompositionTest2D, DecomposeManyCells) {
	ObstacleSet<2> obs = makeObstaclesForPlane({
			"..#..",
			"#.##.",
			"...#.",
			"##..."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1, 3}, {1, 2}),
				box2({4, 6}, {1, 2}),
				box2({2, 3}, {2, 3}),
				box2({1, 4}, {3, 4}),
				box2({5, 6}, {2, 4}),
				box2({3, 6}, {4, 5})
			));
}

} // namespace
