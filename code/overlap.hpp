#include "Box.hpp"
#include "util.hpp"
#include <algorithm>
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
	vector<Box<D-1>> sub1, sub2;
	vector<int> idx1, idx2;
	vector<pair<int,int>> conns;
	for(int z: zs) {
		sub1.clear(), sub2.clear(), idx1.clear(), idx2.clear();
		for(size_t i=0; i<bs1.size(); ++i) {
			if (bs1[i][D-1].contains(z)) {
				sub1.push_back(bs1[i].project());
				idx1.push_back(i);
			}
		}
		for(size_t i=0; i<bs2.size(); ++i) {
			if (bs2[i][D-1].contains(z)) {
				sub2.push_back(bs2[i].project());
				idx2.push_back(i);
			}
		}
		for(auto p : overlappingBoxes(sub1, sub2)) {
			conns.emplace_back(idx1[p.first], idx2[p.second]);
		}
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
