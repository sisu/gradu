#pragma once
#include "Range.hpp"
#include <initializer_list>
#include <ostream>
#include <cstring>
#include <cassert>

// Point in D-dimensional space.
template<int D>
struct Point {
	int data[D];

	Point() {
		memset(data,0,sizeof(data));
	}
	Point(std::initializer_list<int> x) {
		std::copy(x.begin(), x.end(), data);
	}

	int& operator[](int i) { return data[i]; }
	int operator[](int i) const { return data[i]; }
};
template<int D>
inline bool operator==(const Point<D>& a, const Point<D>& b) {
	for(int i=0; i<D; ++i) if (a[i] != b[i]) return false;
	return true;
}

// Axis-aligned D-dimensional rectangle.
//
// Represented as a cartesian product of D simple ranges.
template<int D>
struct Box {
	Range ranges[D];

	Range& operator[](int i) { return ranges[i]; }
	Range operator[](int i) const { return ranges[i]; }

	// Returns (D-1)-dimensional box with dimension `rm` removed.
	Box<D-1> project(int rm=D-1) const {
		Box<D-1> res;
		for(int i=0; i<rm; ++i) res[i]=(*this)[i];
		for(int i=rm+1; i<D; ++i) res[i-1]=(*this)[i];
		return res;
	}
	bool operator==(const Box& b) const {
		for(int i=0; i<D; ++i) {
			if (ranges[i] != b[i]) return false;
		}
		return true;
	}
	bool operator!=(const Box& b) const {
		return !(*this==b);
	}

	bool contains(const Point<D>& p) const {
		for(int i=0; i<D; ++i) {
			if (!ranges[i].contains(p[i])) return false;
		}
		return true;
	}
	bool intersects(const Box<D>& b) const {
		for(int i=0; i<D; ++i) {
			if (!ranges[i].intersects(b.ranges[i])) return false;
		}
		return true;
	}
};

template<int D>
inline bool operator<(const Box<D>& a, const Box<D>& b) {
	for(int i=0; i<D; ++i) {
		for(int j=0; j<2; ++j) {
			if (a[i][j] != b[i][j]) return a[i][j] < b[i][j];
		}
	}
	return false;
}

namespace std {
	template<int D>
	struct hash<Box<D>> {
		size_t operator()(const Box<D>& b) const {
			size_t res=101;
			for(int i=0; i<D; ++i)
				for(int j=0; j<2; ++j)
					res = 33331*res + 65537*b[i][j];
			return res;
		}
	};
};
template<int D>
inline std::ostream& operator<<(std::ostream& o, Box<D> b) {
	o<<'[';
	for(int i=0; i<D; ++i) o<<b[i];
	return o<<']';
}
template<int D>
inline std::ostream& operator<<(std::ostream& o, Point<D> b) {
	o<<'[';
	for(int i=0; i<D; ++i) o<<b[i]<<' ';
	return o<<']';
}
