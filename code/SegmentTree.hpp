#pragma once

template<class T>
struct SegTree {
	SegTree() = delete;
	SegTree(int size): size(size) {}

	SegTree<T> *children=0;
	T value;

	template<class F>
	void visitCanonicals(int a, int b, int from, int to, F f) {
		int xa=(*xs)[a], xb=(*xs)[b];
		if (xb <= from) return;
		if (xa >= to) return;
		if (xa>=from && xb<=to) {
			f.visitCanonical(value, xa, xb);
		} else {
			f.visitParent(value, max(xa, from), min(xb, to));
			int m = (a+b)/2;
			if (!children) {
				children = new SegTree[2];
				children[0].xs = xs;
				children[1].xs = xs;
			}
			children[0].visitCanonicals(a, m, from, to, f);
			children[1].visitCanonicals(m, b, from, to, f);
		}
	}
	template<class F>
	void visitCanonicals(int from, int to, F f) {
		visitCanonicals(0, xs->size()-1, from, to, f);
	}

private:
	int size;
};
