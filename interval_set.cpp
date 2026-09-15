#include "interval_set.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>

namespace novikov {

namespace {

constexpr std::int8_t noLazy = -1;
constexpr std::size_t segmentTreeSizeMultiplier = 4;

}

int IntervalSet::SegmentTree::checkRangeSize(int rangeSize)
{
  if (rangeSize <= 0) {
    throw std::invalid_argument("segment tree range size must be positive");
  }
  return rangeSize;
}

IntervalSet::SegmentTree::SegmentTree(int rangeSize)
  : rangeSize_(checkRangeSize(rangeSize)),
    sum_(segmentTreeSizeMultiplier
             * static_cast<std::size_t>(rangeSize_),
         0),
    lazy_(segmentTreeSizeMultiplier
              * static_cast<std::size_t>(rangeSize_),
          noLazy)
{
}

void IntervalSet::SegmentTree::applyAssign(int node, int nodeLeft,
                                            int nodeRight, bool value)
{
  sum_[node] = value ? (nodeRight - nodeLeft) : 0;
  lazy_[node] = value ? 1 : 0;
}

void IntervalSet::SegmentTree::pushDown(int node, int nodeLeft,
                                         int nodeRight)
{
  if (lazy_[node] == noLazy) {
    return;
  }

  const int mid = nodeLeft + (nodeRight - nodeLeft) / 2;
  const bool value = (lazy_[node] != 0);

  applyAssign(2 * node, nodeLeft, mid, value);
  applyAssign(2 * node + 1, mid, nodeRight, value);
  lazy_[node] = noLazy;
}

void IntervalSet::SegmentTree::assignImpl(int node, int nodeLeft,
                                           int nodeRight, int queryLeft,
                                           int queryRight, bool value)
{
  if (queryRight <= nodeLeft || nodeRight <= queryLeft) {
    return;
  }

  if (queryLeft <= nodeLeft && nodeRight <= queryRight) {
    applyAssign(node, nodeLeft, nodeRight, value);
    return;
  }

  pushDown(node, nodeLeft, nodeRight);

  const int mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  assignImpl(2 * node, nodeLeft, mid, queryLeft, queryRight, value);
  assignImpl(2 * node + 1, mid, nodeRight, queryLeft, queryRight, value);
  sum_[node] = sum_[2 * node] + sum_[2 * node + 1];
}

void IntervalSet::SegmentTree::assign(int left, int right, bool value)
{
  assignImpl(1, 0, rangeSize_, left, right, value);
}

bool IntervalSet::SegmentTree::containsPointImpl(int node, int nodeLeft,
                                                  int nodeRight,
                                                  int index) const
{
  if (sum_[node] == 0) {
    return false;
  }
  if (sum_[node] == nodeRight - nodeLeft) {
    return true;
  }

  const int mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  if (index < mid) {
    return containsPointImpl(2 * node, nodeLeft, mid, index);
  }
  return containsPointImpl(2 * node + 1, mid, nodeRight, index);
}

bool IntervalSet::SegmentTree::containsPoint(int index) const
{
  return containsPointImpl(1, 0, rangeSize_, index);
}

std::int64_t IntervalSet::SegmentTree::totalLength() const
{
  return sum_[1];
}

void IntervalSet::SegmentTree::collectImpl(int node, int nodeLeft,
                                            int nodeRight,
                                            std::vector<Interval>& result,
                                            int& openStart) const
{
  if (sum_[node] == 0) {
    if (openStart != noLazy) {
      result.emplace_back(openStart, nodeLeft - 1);
      openStart = noLazy;
    }
    return;
  }

  if (sum_[node] == nodeRight - nodeLeft) {
    if (openStart == noLazy) {
      openStart = nodeLeft;
    }
    return;
  }

  const int mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  collectImpl(2 * node, nodeLeft, mid, result, openStart);
  collectImpl(2 * node + 1, mid, nodeRight, result, openStart);
}

std::vector<IntervalSet::Interval>
IntervalSet::SegmentTree::getIntervals() const
{
  std::vector<Interval> result;
  int openStart = noLazy;

  collectImpl(1, 0, rangeSize_, result, openStart);

  if (openStart != noLazy) {
    result.emplace_back(openStart, rangeSize_ - 1);
  }
  return result;
}

IntervalSet::IntervalSet(int rangeSize)
  : rangeSize_(rangeSize),
    tree_(rangeSize)
{
}

void IntervalSet::add(int left, int right)
{
  if (left < 0 || right >= rangeSize_ || left > right) {
    throw std::out_of_range("interval out of range");
  }
  tree_.assign(left, right + 1, true);
}

void IntervalSet::remove(int left, int right)
{
  if (left < 0 || right >= rangeSize_ || left > right) {
    throw std::out_of_range("interval out of range");
  }
  tree_.assign(left, right + 1, false);
}

bool IntervalSet::has(int point) const
{
  if (point < 0 || point >= rangeSize_) {
    return false;
  }
  return tree_.containsPoint(point);
}

std::int64_t IntervalSet::getLength() const
{
  return tree_.totalLength();
}

std::vector<IntervalSet::Interval> IntervalSet::getIntervals() const
{
  return tree_.getIntervals();
}

int IntervalSet::getRangeSize() const
{
  return rangeSize_;
}

IntervalSet IntervalSet::unite(const IntervalSet& left,
                                const IntervalSet& right)
{
  const int resultRangeSize = std::max(left.rangeSize_, right.rangeSize_);
  IntervalSet result(resultRangeSize);

  for (const auto& interval : left.getIntervals()) {
    result.add(interval.first, interval.second);
  }
  for (const auto& interval : right.getIntervals()) {
    result.add(interval.first, interval.second);
  }
  return result;
}

IntervalSet IntervalSet::intersect(const IntervalSet& left,
                                    const IntervalSet& right)
{
  const int resultRangeSize = std::min(left.rangeSize_, right.rangeSize_);
  IntervalSet result(resultRangeSize);

  const std::vector<Interval> leftIntervals = left.getIntervals();
  const std::vector<Interval> rightIntervals = right.getIntervals();

  std::size_t leftIndex = 0;
  std::size_t rightIndex = 0;

  while (leftIndex < leftIntervals.size()
         && rightIndex < rightIntervals.size()) {
    const int lower = std::max(leftIntervals[leftIndex].first,
                                rightIntervals[rightIndex].first);
    const int upper = std::min(leftIntervals[leftIndex].second,
                                rightIntervals[rightIndex].second);

    if (lower <= upper) {
      result.add(lower, upper);
    }

    if (leftIntervals[leftIndex].second
        < rightIntervals[rightIndex].second) {
      ++leftIndex;
    } else {
      ++rightIndex;
    }
  }
  return result;
}

bool IntervalSet::save(const std::string& filename) const
{
  std::ofstream out(filename);

  if (!out) {
    return false;
  }

  out << rangeSize_ << '\n';

  for (const auto& interval : getIntervals()) {
    out << interval.first << ' ' << interval.second << '\n';
  }
  return static_cast<bool>(out);
}

bool IntervalSet::load(const std::string& filename)
{
  std::ifstream in(filename);

  if (!in) {
    return false;
  }

  int loadedRangeSize = 0;

  if (!(in >> loadedRangeSize) || loadedRangeSize <= 0) {
    return false;
  }

  SegmentTree loadedTree(loadedRangeSize);

  int left = 0;
  int right = 0;

  while (in >> left >> right) {
    if (left < 0 || right >= loadedRangeSize || left > right) {
      return false;
    }
    loadedTree.assign(left, right + 1, true);
  }

  rangeSize_ = loadedRangeSize;
  tree_ = std::move(loadedTree);
  return true;
}

}
