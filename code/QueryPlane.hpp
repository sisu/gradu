#pragma once

#include "Box.hpp"
#include <array>

template<int D>
using RectIndex = std::array<int, D>;

template<int D, class T>
struct QueryPlane {
	void add(Box<D> box, const T& value);
	bool check(Box<D> box) const;

	template<class Callback>
	void remove(Box<D> box, Callback& cb);
};
