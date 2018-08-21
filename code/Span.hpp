#pragma once

#include <vector>

template<class T>
class Span {
public:
//	Span(std::vector<T>& v): Span(&*v.begin(), &*v.end()) {}
	template<class C>
	Span(C& v): Span(&*v.begin(), &*v.end()) {}
	Span(T* a, T* b): from(a), to(b) {}

	T& operator[](int i) const { return from[i]; }
	int size() const { return to-from; }
	T* begin() const { return from; }
	T* end() const { return to; }

private:
	T* from;
	T* to;
};
