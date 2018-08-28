#pragma once

#include "Range.hpp"
#include "util.hpp"

#include <vector>

template<class T>
class SegmentTree {
public:
  SegmentTree(int size): data(2*toPow2(size)) {}

  void clear() {
    for(auto& x: data) x.clear();
  }

  void add(Range range, const T& item) {
    for(int a=size()+range.from+1, b=size()+range.to-1; a<=b; a/=2, b/=2) {
      if (a&1) addRangeItem(a++, item);
      if (!(b&1)) addRangeItem(b--, item);
    }
    for(int a=size()+range.from; a>0; a/=2) {
      addPointItem(a, item);
    }
  }

  std::vector<T> find(Range range) const {
    std::vector<T> result;
    find(range, result);
    return result;
  }

  void find(Range range, std::vector<T>& result) const {
    for(int a=size()+range.from, b=size()+range.to-1; a<=b; a/=2, b/=2) {
      if (a&1) getPointItems(a++, result);
      if (!(b&1)) getPointItems(b--, result);
    }
    for(int a=size()+range.from; a>0; a/=2) {
      getRangeItems(a, result);
    }
  }

private:
  struct TreeNode {
    std::vector<T> rangeItems;
    std::vector<T> pointItems;

    void clear() {
      rangeItems.clear();
      pointItems.clear();
    }
  };

  void addRangeItem(int index, const T& item) {
    data[index].rangeItems.push_back(item);
  }
  void addPointItem(int index, const T& item) {
    data[index].pointItems.push_back(item);
  }
  void getRangeItems(int index, std::vector<T>& result) const {
    result.insert(result.end(), data[index].rangeItems.begin(), data[index].rangeItems.end());
  }
  void getPointItems(int index, std::vector<T>& result) const {
    result.insert(result.end(), data[index].pointItems.begin(), data[index].pointItems.end());
  }

  int size() const { return data.size()/2; }
  std::vector<TreeNode> data;
};
