#pragma once

#include <array>
#include <ostream>
#include <vector>

namespace std {
template<class T>
ostream& operator<<(ostream& o, const vector<T>& v) {
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
template<class T, size_t N>
ostream& operator<<(ostream& out, const array<T, N>& arr) {
	out<<'[';
	for(size_t i=0; i<N; ++i) {
		if (i) out<<' ';
		out<<arr[i];
	}
	return out<<']';
}
} // namespace std
