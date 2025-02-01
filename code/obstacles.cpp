#include "obstacles.hpp"

#include <algorithm>

using namespace std;

namespace {

constexpr int LEFT = 0;
//constexpr int RIGHT = 1;
constexpr int UP = 2;
constexpr int DOWN = 3;
constexpr int ZMINUS = 4;
//constexpr int ZPLUS = 5;

template<int D>
Point<D> dirVec(int dir) {
	Point<D> pt;
	pt[dir>>1] = (dir&1)*2 - 1;
	return pt;
}

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
	const auto& dv = dirVec<2>(dir);
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

void addObstaclesForPlane(ObstacleSet<2>& result, const vector<string>& area) {
	int h = area.size();
	for(int i=0; i+1<h; ++i) {
		addObstaclesOnLine(result, area, i+1, UP);
		addObstaclesOnLine(result, area, i, DOWN);
	}
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
		activeRects = std::move(newRects);
	}
	result.insert(result.end(), activeRects.begin(), activeRects.end());
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

} // namespace

ObstacleSet<2> makeObstaclesForPlane(const vector<string>& area0) {
	const vector<string> area = addBorderAroundArea(area0);
	ObstacleSet<2> result;
	addObstaclesForPlane(result, area);
	int n = result.size();
	addObstaclesForPlane(result, swapXY(area));
	for(size_t i=n; i<result.size(); ++i) {
		swap(result[i].box[0], result[i].box[1]);
		result[i].direction += LEFT - UP;
	}
	return result;
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
		result[i].direction += LEFT - UP;
	}
	n = result.size();
	auto yz = swapYZ(volume);
	addObstaclesForVolume(result, yz);
	for(size_t i=n; i<result.size(); ++i) {
		swap(result[i].box[1], result[i].box[2]);
		result[i].direction += ZMINUS - UP;
	}
	return result;
}

