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

template<int A, int B>
int compare(const Obstacle<A>& a, const Obstacle<B>& b) {
	if (a.direction != b.direction) return a.direction - b.direction;
	int n = min(A,B);
	for(int i=0; i<n; ++i) {
		for(int j=0; j<2; ++j) {
			if (a.box[i][j] != b.box[i][j])
				return a.box[i][j] - b.box[i][j];
		}
	}
	return 0;
}

void addObstaclesForVolume(ObstacleSet<3>& result,
		const vector<vector<string>>& volume) {
	int n = volume.size();
	int h = volume[0].size();
	ObstacleSet<2> curPlane;
	ObstacleSet<3> activeRects, newRects;
	for(int i=0; i<n; ++i) {
		curPlane.clear();
		const auto& area = volume[i];
		for(int j=0; j+1<h; ++j) {
			addObstaclesOnLine(curPlane, area, j+1, UP);
			addObstaclesOnLine(curPlane, area, j, DOWN);
		}
		sort(curPlane.begin(), curPlane.end(),
				[](const Obstacle<2>& a, const Obstacle<2>& b) {
				return compare(a, b)<0;
				});
		sort(activeRects.begin(), activeRects.end(),
				[](const Obstacle<3>& a, const Obstacle<3>& b) {
				return compare(a, b)<0;
				});

		newRects.clear();
		size_t a=0, b=0;
		while(a < curPlane.size() && b < activeRects.size()) {
			const auto& curP = curPlane[a];
			const auto& actP = activeRects[b];
			int c = compare(curP, actP);
			if (c <= 0) {
				int start = c<0 ? i : actP.box[2].from;
				Box<3> box{{curP.box[0], curP.box[1], {start, i+1}}};
				newRects.push_back({box, curP.direction});
				a++;
				if (c==0) b++;
			} else {
				result.push_back(actP);
				b++;
			}
		}
		for(;a < curPlane.size(); ++a) {
			const auto& curP = curPlane[a];
			Box<3> box{{curP.box[0], curP.box[1], {i, i+1}}};
			newRects.push_back({box, curP.direction});
		}
		result.insert(result.end(), activeRects.begin()+b, activeRects.end());
		activeRects = move(newRects);
	}
	result.insert(result.end(), activeRects.begin(), activeRects.end());
}

vector<string> swapXY(const vector<string>& area) {
	int h = area.size();
	int w = area[0].size();
	vector<string> res(w, string(h, '#'));
	for(int i=0; i<h; ++i) {
		for(int j=0; j<w; ++j) {
			res[j][i] = area[i][j];
		}
	}
	return res;
}
vector<vector<string>> swapXY(const vector<vector<string>>& volume) {
	int n = volume.size();
	vector<vector<string>> res;
	res.reserve(n);
	for(int i=0; i<n; ++i) res.push_back(swapXY(volume[i]));
	return res;
}
vector<vector<string>> swapYZ(const vector<vector<string>>& volume) {
	int n = volume[0].size();
	int h = volume[0].size();
	int w = volume[0][0].size();
	vector<vector<string>> res;
	res.reserve(h);
	for(int i=0; i<h; ++i) res.emplace_back(n, string(w, '#'));
	for(int i=0; i<n; ++i) {
		for(int j=0; j<h; ++j) {
			for(int k=0; k<w; ++k) {
				res[j][i][k] = volume[i][j][k];
			}
		}
	}
	return res;
}

ObstacleSet<3> makeObstaclesForVolume(vector<vector<string>> volume) {
	volume = addBorderAroundVolume(volume);
	ObstacleSet<3> result;
	addObstaclesForVolume(result, volume);
	int n = result.size();
	auto xy = swapXY(volume);
	addObstaclesForVolume(result, xy);
	for(size_t i=n; i<result.size(); ++i) {
		swap(result[i].box[0], result[i].box[1]);
		result[i].direction += 2;
	}
	n = result.size();
	auto yz = swapYZ(volume);
	addObstaclesForVolume(result, yz);
	for(size_t i=n; i<result.size(); ++i) {
		swap(result[i].box[1], result[i].box[2]);
		result[i].direction += 4;
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

TEST(DecompositionTest3D, DecomposeTwoCells) {
	ObstacleSet<3> obs = makeObstaclesForVolume(
			{
			{"#.", ".."},
			{"##", ".."},
			});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<3> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box3({2,3}, {1,2}, {1,2}),
				box3({1,3}, {2,3}, {1,3})
				));
}

} // namespace
