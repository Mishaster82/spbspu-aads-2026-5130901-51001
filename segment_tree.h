#ifndef NOVIKOV_SEGMENT_TREE_H
#define NOVIKOV_SEGMENT_TREE_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace novikov {
namespace detail {

template <class TIndex>
class SegmentTree
{
public:
  using Index = TIndex;
  using Interval = std::pair<Index, Index>;

  SegmentTree(const SegmentTree& other) = default;
  SegmentTree(SegmentTree&& other) noexcept = default;
  explicit SegmentTree(Index rangeSize);
  ~SegmentTree() = default;

  SegmentTree& operator=(const SegmentTree& other) = default;
  SegmentTree& operator=(SegmentTree&& other) noexcept = default;

  void assign(Index left, Index right, bool value);
  bool containsPoint(Index index) const;
  std::vector<Interval> getIntervals() const;

private:
  static constexpr std::int8_t noLazy = -1;
  static constexpr std::int8_t lazyOn = 1;
  static constexpr std::int8_t lazyOff = 0;
  static constexpr std::size_t segmentTreeSizeMultiplier = 4;

  static Index checkRangeSize(Index rangeSize);

  void assignImpl(std::int64_t node, Index nodeLeft, Index nodeRight,
                   Index queryLeft, Index queryRight, bool value);
  bool containsPointImpl(std::int64_t node, Index nodeLeft, Index nodeRight,
                          Index index) const;
  void pushDown(std::int64_t node, Index nodeLeft, Index nodeRight);
  void applyAssign(std::int64_t node, Index nodeLeft, Index nodeRight,
                    bool value);
  void collectImpl(std::int64_t node, Index nodeLeft, Index nodeRight,
                    std::vector<Interval>& result, Index& openStart) const;

  Index rangeSize_;
  std::vector<std::int64_t> sum_;
  std::vector<std::int8_t> lazy_;
};

template <class TIndex>
TIndex SegmentTree<TIndex>::checkRangeSize(Index rangeSize)
{
  if (rangeSize <= 0) {
    throw std::invalid_argument("segment tree range size must be positive");
  }
  return rangeSize;
}

template <class TIndex>
SegmentTree<TIndex>::SegmentTree(Index rangeSize):
  rangeSize_(checkRangeSize(rangeSize)),
  sum_(segmentTreeSizeMultiplier * static_cast<std::size_t>(rangeSize_), 0),
  lazy_(segmentTreeSizeMultiplier * static_cast<std::size_t>(rangeSize_),
        noLazy)
{
}

template <class TIndex>
void SegmentTree<TIndex>::applyAssign(std::int64_t node, Index nodeLeft,
                                       Index nodeRight, bool value)
{
  sum_[node] = value
                   ? static_cast<std::int64_t>(nodeRight - nodeLeft)
                   : 0;
  lazy_[node] = value ? lazyOn : lazyOff;
}

template <class TIndex>
void SegmentTree<TIndex>::pushDown(std::int64_t node, Index nodeLeft,
                                    Index nodeRight)
{
  if (lazy_[node] == noLazy) {
    return;
  }

  const Index mid = nodeLeft + (nodeRight - nodeLeft) / 2;
  const bool value = (lazy_[node] == lazyOn);

  applyAssign(2 * node, nodeLeft, mid, value);
  applyAssign(2 * node + 1, mid, nodeRight, value);
  lazy_[node] = noLazy;
}

template <class TIndex>
void SegmentTree<TIndex>::assignImpl(std::int64_t node, Index nodeLeft,
                                      Index nodeRight, Index queryLeft,
                                      Index queryRight, bool value)
{
  if (queryRight <= nodeLeft || nodeRight <= queryLeft) {
    return;
  }

  if (queryLeft <= nodeLeft && nodeRight <= queryRight) {
    applyAssign(node, nodeLeft, nodeRight, value);
    return;
  }

  pushDown(node, nodeLeft, nodeRight);

  const Index mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  assignImpl(2 * node, nodeLeft, mid, queryLeft, queryRight, value);
  assignImpl(2 * node + 1, mid, nodeRight, queryLeft, queryRight, value);
  sum_[node] = sum_[2 * node] + sum_[2 * node + 1];
}

template <class TIndex>
void SegmentTree<TIndex>::assign(Index left, Index right, bool value)
{
  assignImpl(1, 0, rangeSize_, left, right, value);
}

template <class TIndex>
bool SegmentTree<TIndex>::containsPointImpl(std::int64_t node, Index nodeLeft,
                                             Index nodeRight,
                                             Index index) const
{
  if (sum_[node] == 0) {
    return false;
  }
  if (sum_[node] == nodeRight - nodeLeft) {
    return true;
  }

  const Index mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  if (index < mid) {
    return containsPointImpl(2 * node, nodeLeft, mid, index);
  }
  return containsPointImpl(2 * node + 1, mid, nodeRight, index);
}

template <class TIndex>
bool SegmentTree<TIndex>::containsPoint(Index index) const
{
  return containsPointImpl(1, 0, rangeSize_, index);
}

template <class TIndex>
void SegmentTree<TIndex>::collectImpl(std::int64_t node, Index nodeLeft,
                                       Index nodeRight,
                                       std::vector<Interval>& result,
                                       Index& openStart) const
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

  const Index mid = nodeLeft + (nodeRight - nodeLeft) / 2;

  collectImpl(2 * node, nodeLeft, mid, result, openStart);
  collectImpl(2 * node + 1, mid, nodeRight, result, openStart);
}

template <class TIndex>
std::vector<std::pair<TIndex, TIndex>>
SegmentTree<TIndex>::getIntervals() const
{
  std::vector<Interval> result;
  Index openStart = noLazy;

  collectImpl(1, 0, rangeSize_, result, openStart);

  if (openStart != noLazy) {
    result.emplace_back(openStart, rangeSize_ - 1);
  }
  return result;
}

}
}

#endif
