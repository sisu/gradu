#pragma once
#include <ostream>

struct Range {
	Range(int from, int to): from(from), to(to) {}

	int from=-1,to=-1;
	bool operator<(const Range& r) const {
		return from<r.from;
	}
	bool operator<(int i) const {
		return from<i;
	}
	int size() const {
		return to-from;
	}
	int middle() const {
		return (from+to)>>1;
	}
	bool empty() const {
		return from==to;
	}
	bool unit() const {
		return from+1==to;
	}
	bool contains(int x) const {
		return x>=from && x<to;
	}
	bool intersects(Range r) const {
		return to>r.from && r.to>from;
	}
	bool contains(Range r) const {
		return r.from>=from && r.to<=to;
	}
	Range intersection(Range r) const {
		return {std::max(from, r.from), std::min(to, r.to)};
	}
	Range union_(Range r) const {
		return {std::min(from, r.from), std::max(to, r.to)};
	}

	int operator[](int i) const { return i==0 ? from : to; }
	int& operator[](int i) { return i==0 ? from : to; }

	bool operator==(const Range& r) const {
		return from==r.from && to==r.to;
	}
	bool operator!=(const Range& r) const {
		return !(*this == r);
	}
};
inline std::ostream& operator<<(std::ostream& o, Range r) {
	return o<<'('<<r.from<<".."<<r.to<<')';
}
