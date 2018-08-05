#pragma once

#include <ostream>
#include <vector>

template<class T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& v) {
	o<<'[';
	bool fst=true;
	for(const T& t: v) {
		if (!fst) o<<' ';
		fst = false;
		o<<t;
	}
	o<<']';
	return o;
}
