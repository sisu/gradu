#include "Box.hpp"
#include "print.hpp"
#include "util.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

using std::make_pair;
using std::pair;
using std::vector;

template<int D>
inline vector<int> getOverlappingIndices(const vector<Box<D>>& boxes, int z) {
	vector<int> res;
	for(size_t i=0; i<boxes.size(); ++i) {
		if (boxes[i][D-1].contains(z)) {
			res.push_back(i);
		}
	}
	return res;
}

template<class T>
inline vector<T> vectorDifference(const vector<T>& a, const vector<T>& b) {
	vector<T> res;
	std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
	return res;
}

template<class T>
inline vector<T> vectorIntersection(const vector<T>& a, const vector<T>& b) {
	vector<T> res;
	std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
	return res;
}

template<int D>
inline vector<Box<D-1>> getProjected(const vector<Box<D>>& v, const vector<int>& idx) {
	vector<Box<D-1>> res;
	res.reserve(idx.size());
	for(int i: idx) res.push_back(v[i].project());
	return res;
}

template<int D>
void addOverlappingBoxes(vector<pair<int,int>>& result,
		const vector<Box<D>>& bs1,
		const vector<Box<D>>& bs2,
		const vector<int>& idx1,
		const vector<int>& idx2) {
	for(auto p : overlappingBoxes(getProjected(bs1, idx1), getProjected(bs2, idx2))) {
		result.emplace_back(idx1[p.first], idx2[p.second]);
	}
}

template<int D>
inline vector<pair<int,int>> overlappingBoxes(
		const vector<Box<D>>& bs1,
		const vector<Box<D>>& bs2) {
	vector<int> zs;
	for(const auto& box: bs1) {
		zs.push_back(box[D-1].from);
		zs.push_back(box[D-1].to);
	}
	sortUnique(zs);
	vector<int> old1, old2;
	vector<pair<int,int>> conns;
	for(int z: zs) {
		vector<int> idx1 = getOverlappingIndices(bs1, z);
		vector<int> idx2 = getOverlappingIndices(bs2, z);
		vector<int> keep1 = vectorIntersection(old1, idx1);
		vector<int> keep2 = vectorIntersection(old2, idx2);
		vector<int> added1 = vectorDifference(old1, idx1);
		vector<int> added2 = vectorDifference(old2, idx2);
		addOverlappingBoxes(conns, bs1, bs2, keep1, added2);
		addOverlappingBoxes(conns, bs1, bs2, added1, keep2);
		addOverlappingBoxes(conns, bs1, bs2, added1, added2);
		old1 = std::move(idx1);
		old2 = std::move(idx2);
	}
	return conns;
}

inline pair<int,int> makePair(int a, int b, bool swap) {
	return swap ? make_pair(b, a) : make_pair(a,b);
}

template<>
inline vector<pair<int,int>> overlappingBoxes(
		const vector<Box<2>>& bs1,
		const vector<Box<2>>& bs2) {
	constexpr int X_AXIS = 0;
	constexpr int Y_AXIS = 1;
	vector<pair<int,int>> conns;
	struct Event {
		int index = -1;
		int pos = -1;
		bool first = false;
		bool start = false;
		bool operator<(const Event& e) const {
			if (pos != e.pos) return pos < e.pos;
			return start < e.start;
		}
	};
	vector<Event> events;
	for(int i=0; i<(int)bs1.size(); ++i) {
		Range range = bs1[i][Y_AXIS];
		events.push_back({i, range.from, true, true});
		events.push_back({i, range.to, true, false});
	}
	for(int i=0; i<(int)bs2.size(); ++i) {
		Range range = bs2[i][Y_AXIS];
		events.push_back({i, range.from, false, true});
		events.push_back({i, range.to, false, false});
	}
	sort(events.begin(), events.end());

	struct Item {
		int end = -1;
		int index = -1;
	};
	std::map<int, Item> map1, map2;

	for(Event event: events) {
		auto& items = event.first ? map1 : map2;
		const auto& bs = event.first ? bs1 : bs2;
		Range range = bs[event.index][X_AXIS];
		if (!event.start) {
			items.erase(range.from);
			continue;
		}
		items[range.from] = {range.to, event.index};
		const auto& others = event.first ? map2 : map1;
		auto it = others.lower_bound(range.from);
		if (it != others.begin()) {
			auto x = prev(it);
			if (x->second.end > range.from) {
				conns.push_back(makePair(event.index, x->second.index, !event.first));
			}
		}
		for(; it != others.end() && it->first < range.to; ++it) {
			conns.push_back(makePair(event.index, it->second.index, !event.first));
		}
	}
	return conns;
}
