#pragma once
#include "Range.hpp"
#include <ostream>

template<int D>
struct Point {
	int data[D];
	int& operator[](int i) { return data[i]; }
	int operator[](int i) const { return data[i]; }
};

template<int D>
struct Box {
	Range ranges[D];

	Range& operator[](int i) { return ranges[i]; }
	Range operator[](int i) const { return ranges[i]; }

	Box<D-1> project(int rm=D-1) const {
		Box<D-1> res;
		for(int i=0; i<rm; ++i) for(int j=0; j<2; ++j) res[i][j]=(*this)[i][j];
		for(int i=rm+1; i<D; ++i) for(int j=0; j<2; ++j) res[i-1][j]=(*this)[i][j];
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
