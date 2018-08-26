#include "path.hpp"
#include "obstacles.hpp"
#include "slowPath.hpp"
#include <cstring>
#include <random>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

namespace {

using namespace std;

vector<string> genRandomGrid(int w, int h, mt19937& rng) {
	vector<string> res;
	for(int i=0; i<h; ++i) {
		string str(w+2, '#');
		for(int j=0; j<w; ++j) {
			str[j+1] = rng() < rng.max()/4 ? '#' : '.';
		}
		res.push_back(move(str));
	}
	return res;
}

Point<2> randomFreePoint(const vector<string>& grid, mt19937& rng) {
	Point<2> res;
	int w = grid[0].size(), h = grid.size();
	do {
		res[0] = rng()%w;
		res[1] = rng()%h;
	} while(grid[res[1]][res[0]] != '.');
	res[0]+=1;
	res[1]+=1;
	return res;
}

TEST(LinkDistance2D, StartEndPointSame) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {1,1}), 0);
}

TEST(LinkDistance2D, StartEndSameLine) {
	ObstacleSet<2> obs = makeObstaclesForPlane({".."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {2,1}), 1);
}

TEST(LinkDistance2D, OneTurn) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"..", ".."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {2,2}), 2);
}

TEST(LinkDistance2D, AroundObstacle) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"...", ".#.", "..."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {3,3}), 2);
}

TEST(LinkDistance2D, AroundObstacle2) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"#..", ".#.", "..."});
	EXPECT_EQ(linkDistance(obs, {2,1}, {1,2}), 4);
}

TEST(LinkDistance2D, Infeasible) {
	ObstacleSet<2> obs = makeObstaclesForPlane({"#..", "...", "..#"});
	EXPECT_EQ(linkDistance(obs, {2,1}, {3,3}), -1);
}

TEST(LinkDistance2D, ManyPaths) {
	ObstacleSet<2> obs = makeObstaclesForPlane(
		{".#...",
		 "...#.",
		 ".#...",
		 "...#."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {5,4}), 5);
}

TEST(LinkDistance2D, Spiral) {
	ObstacleSet<2> obs = makeObstaclesForPlane(
		{".#.....",
		 ".#.###.",
		 ".#.#.#.",
		 ".#...#.",
		 ".#####.",
		 "......."});
	EXPECT_EQ(linkDistance(obs, {1,1}, {5,3}), 7);
}

TEST(LinkDistance2D, RandomTest) {
	for(int i=0; i<10; ++i) {
		mt19937 rng(i);
		auto grid = genRandomGrid(32, 32, rng);
		cout<<"grid: "<<grid<<'\n';
		auto obs = makeObstaclesForPlane(grid);
		Point<2> start = randomFreePoint(grid, rng);
		Point<2> end = randomFreePoint(grid, rng);
		cout<<"RES: "<<slowLinkDistance(obs,start,end)<<'\n';
		EXPECT_EQ(linkDistance(obs, start, end),
				slowLinkDistance(obs, start, end));
	}
}

TEST(LinkDistance3D, Triv) {
	ObstacleSet<3> obs = makeObstaclesForVolume({
		{
			"..",
			"..",
		},{
			"..",
			"..",
		}});
	EXPECT_EQ(linkDistance(obs, {1,1,1}, {2,2,2}), 3);
}

TEST(LinkDistance3D, AroundObstacle) {
	ObstacleSet<3> obs = makeObstaclesForVolume({
		{
			"...",
			"###",
			"...",
		},{
			"#..",
			".#.",
			"#..",
		},{
			"...",
			".##",
			"...",
		}});
	EXPECT_EQ(linkDistance(obs, {1,1,1}, {1,2,2}), 5);
}

} // namespace
