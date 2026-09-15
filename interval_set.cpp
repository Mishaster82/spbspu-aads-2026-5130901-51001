#include "interval_set.h"

#include <cstddef>
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

int IntervalSet::getRangeSize() const
{
  return rangeSize_;
}

}
