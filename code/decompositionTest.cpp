#include "decomposition.hpp"
#include "obstacles.hpp"
#include <cstring>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using namespace std;

using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::IsEmpty;

template<int D>
Point<D> dirVec(int dir) {
	Point<D> pt;
	pt[dir>>1] = (dir&1)*2 - 1;
	return pt;
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

template<int D>
Box<D> makeBoundaryBox(const Box<D>& b, int dir) {
	Box<D> res = b;
	Point<D> vdir = dirVec<D>(dir);
	for(int i=0; i<D; ++i) {
		if (vdir[i] < 0) {
			res[i] = {res[i].from-1, res[i].from+1};
		} else if (vdir[i] > 0) {
			res[i] = {res[i].to-1, res[i].to+1};
		}
	}
	return res;
}

template<int D>
vector<int> getLinksInDir(const Decomposition<D>& dec, size_t index, int dir) {
	Box<D> target = makeBoundaryBox(dec[index].box, dir);
	vector<int> res;
	for(size_t i=0; i<dec.size(); ++i) {
		if (i != index && target.intersects(dec[i].box)) {
			res.push_back(i);
		}
	}
	return res;
}
template<int D>
vector<int> getObstaclesInDir(const ObstacleSet<D>& obs, const Box<D>& box, int dir) {
	Box<D> target = makeBoundaryBox(box, dir);
	int targetDir = dir ^ 1;
	vector<int> res;
	for(size_t i=0; i<obs.size(); ++i) {
		if (obs[i].direction == targetDir && target.intersects(obs[i].box)) {
			res.push_back(i);
		}
	}
	return res;
}


template<int D>
void checkLinks(const Decomposition<D>& dec) {
	for(size_t i=0; i<dec.size(); ++i) {
		for(int j=0; j<2*D; ++j) {
			EXPECT_THAT(dec[i].links[j], ElementsAreArray(getLinksInDir(dec, i, j))) << i<<' '<<j<<' '<<dec[i].box;
		}
	}
}

template<int D>
void checkObstacles(const Decomposition<D>& dec, const ObstacleSet<D>& obstacles) {
	for(size_t i=0; i<dec.size(); ++i) {
		for(int j=0; j<2*D; ++j) {
			EXPECT_THAT(dec[i].obstacles[j], ElementsAreArray(getObstaclesInDir(obstacles, dec[i].box, j))) << i<<' '<<j<<' '<<dec[i].box;
		}
	}
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
	checkObstacles(result, obs);
}

TEST(DecompositionTest2D, DecomposeTwoCells1) {
	ObstacleSet<2> obs = makeObstaclesForPlane({".#", ".."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1,2}, {1,2}), box2({1,3}, {2,3})));
	checkLinks(result);
	checkObstacles(result, obs);
}
TEST(DecompositionTest2D, DecomposeTwoCells2) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"..", "#."});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1,3}, {1,2}), box2({2,3}, {2,3})));
	checkLinks(result);
	checkObstacles(result, obs);
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
	checkLinks(result);
	checkObstacles(result, obs);
}

TEST(DecompositionTest2D, ManyChangesOnSingleLevel) {
	ObstacleSet<2> obs = makeObstaclesForPlane({
			".#.#.",
			".....",
			"#.#.#"});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<2> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box2({1, 2}, {1, 2}),
				box2({3, 4}, {1, 2}),
				box2({5, 6}, {1, 2}),
				box2({1, 6}, {2, 3}),
				box2({2, 3}, {3, 4}),
				box2({4, 5}, {3, 4})
				));
	checkLinks(result);
	checkObstacles(result, obs);
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
	checkObstacles(result, obs);
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
	checkLinks(result);
	checkObstacles(result, obs);
}

TEST(DecompositionTest3D, DecomposeDeepCells) {
	ObstacleSet<3> obs = makeObstaclesForVolume(
			{
				{"...",
				 ".#.",
				 "..."},
				{"...",
				 "##.",
				 "..."},
				{"...",
				 "##.",
				 ".#."},
			});
	cout<<"obs: "<<obs<<'\n';
	Decomposition<3> result = decomposeFreeSpace(obs);
	EXPECT_THAT(getBoxes(result), ElementsAre(
				box3({1,4}, {1,2}, {1,4}),
				box3({1,2}, {2,3}, {1,2}),
				box3({3,4}, {2,3}, {1,3}),
				box3({1,4}, {3,4}, {1,3}),
				box3({1,2}, {3,4}, {3,4}),
				box3({3,4}, {2,4}, {3,4})
				));
	checkLinks(result);
	checkObstacles(result, obs);
}

} // namespace
