#pragma once
#include <ostream>

struct Range {
	Range(int x1, int x2): x1(x1), x2(x2) {}

	int x1=-1,x2=-1;
	bool operator<(const Range& r) const {
		return x1<r.x1;
	}
	bool operator<(int i) const {
		return x1<i;
	}
	int size() const {
		return x2-x1;
	}
	int middle() const {
		return (x1+x2)>>1;
	}
	bool empty() const {
		return x1==x2;
	}
	bool unit() const {
		return x1+1==x2;
	}
	bool contains(int x) const {
		return x>=x1 && x<x2;
	}
	bool intersects(Range r) const {
		return x2>r.x1 && r.x2>x1;
	}
	bool contains(Range r) const {
		return r.x1>=x1 && r.x2<=x2;
	}
	Range intersection(Range r) const {
		return {std::max(x1, r.x1), std::min(x2, r.x2)};
	}

	int operator[](int i) const { return i==0 ? x1 : x2; }
	int& operator[](int i) { return i==0 ? x1 : x2; }

	bool operator==(const Range& r) const {
		return x1==r.x1 && x2==r.x2;
	}
	bool operator!=(const Range& r) const {
		return !(*this == r);
	}
};
inline std::ostream& operator<<(std::ostream& o, Range r) {
	return o<<'('<<r.x1<<".."<<r.x2<<')';
}
