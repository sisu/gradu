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

vector<vector<string>> addBorderAroundVolume(const vector<vector<string>>& volume) {
	int n = volume.size();
	int h = volume[0].size();
	int w = volume[0][0].size();
	vector<vector<string>> res;
	res.reserve(n+2);
	res.emplace_back(vector<string>(h+2, string(w+2, '#')));
	for(int i=0; i<n; ++i) {
		res.push_back(addBorderAroundArea(volume[i]));
	}
	res.emplace_back(vector<string>(h+2, string(w+2, '#')));
	return res;
}

void addObstaclesOnLine(
		ObstacleSet<2>& result,
		const vector<string>& area,
		int y, int dir) {
	const int w = area[0].size();
	const auto& dv = dir2[dir];
	const int dx = dv[0], dy = dv[1];
	int yy = dy > 0 ? y+1 : y;
	int count = 0;
	for(int x=0; x<w; ++x) {
		if (area[y][x]==USED && area[y+dy][x+dx]!=USED) {
			count++;
		} else {
			if (count) {
				result.push_back({
						{{{x-count, x}, {yy, yy}}},
						dir});
			}
			count = 0;
		}
	}
	if (count) {
		result.push_back({
				{{{w-count, w}, {yy, yy}}},
				dir});
	}
}

ObstacleSet<2> makeObstaclesForPlane(const vector<string>& area0) {
	const vector<string> area = addBorderAroundArea(area0);
	ObstacleSet<2> result;
	int h = area.size();
	for(int i=0; i+1<h; ++i) {
		addObstaclesOnLine(result, area, i+1, UP);
		addObstaclesOnLine(result, area, i, DOWN);
	}
	return result;
}

ObstacleSet<3> makeObstaclesForVolume(vector<vector<string>> volume) {
	volume = addBorderAroundVolume(volume);
	ObstacleSet<3> result;
	int n = volume.size();
	int h = volume[0].size();
	int w = volume[0][0].size();
	ObstacleSet<2> curPlane;
	ObstacleSet<3> activeRects, newRects;
	for(int i=0; i+1<n; ++i) {
		curPlane.clear();
		const auto& area = volume[i];
		for(int j=0; j+1<h; ++j) {
			addObstaclesOnLine(curPlane, area, j+1, UP);
			addObstaclesOnLine(curPlane, area, j, DOWN);
		}
	}
	result.insert(result.end(), activeRects.begin(), activeRects.end());
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
Box<3> box3(Range x, Range y, Range z) {
	return {{x, y, z}};
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

TEST(DecompositionTest3D, DecomposeEmpty) {
	ObstacleSet<3> obs;
	EXPECT_THAT(decomposeFreeSpace(obs), IsEmpty());
}

TEST(DecompositionTest3D, DecomposeSingleCell) {
	ObstacleSet<3> obs = makeObstaclesForVolume({{"."}});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<3> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(box3({1,2}, {1,2}, {1,2})));
}

} // namespace
