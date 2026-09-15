#ifndef NOVIKOV_INTERVAL_SET_H
#define NOVIKOV_INTERVAL_SET_H

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>

#include "segment_tree.h"

namespace novikov {

template <class TIndex>
class IntervalSet
{
public:
  using Index = TIndex;
  using Interval = std::pair<Index, Index>;

  IntervalSet(const IntervalSet& other) = default;
  IntervalSet(IntervalSet&& other) noexcept = default;
  explicit IntervalSet(Index rangeSize);
  ~IntervalSet() = default;

  IntervalSet& operator=(const IntervalSet& other) = default;
  IntervalSet& operator=(IntervalSet&& other) noexcept = default;

  void add(Index left, Index right);
  void remove(Index left, Index right);
  bool has(Index point) const;
  std::int64_t getLength() const;
  std::vector<Interval> getIntervals() const;
  Index getRangeSize() const;

  static IntervalSet unite(const IntervalSet& left,
                            const IntervalSet& right);
  static IntervalSet intersect(const IntervalSet& left,
                                const IntervalSet& right);

  bool save(const std::string& filename) const;
  bool load(const std::string& filename);

private:
  Index rangeSize_;
  detail::SegmentTree<Index> tree_;
};

using IntIntervalSet = IntervalSet<int>;

template <class TIndex>
IntervalSet<TIndex>::IntervalSet(Index rangeSize):
  rangeSize_(rangeSize),
  tree_(rangeSize)
{
}

template <class TIndex>
void IntervalSet<TIndex>::add(Index left, Index right)
{
  if (left < 0 || right >= rangeSize_ || left > right) {
    throw std::out_of_range("interval out of range");
  }
  tree_.assign(left, right + 1, true);
}

template <class TIndex>
void IntervalSet<TIndex>::remove(Index left, Index right)
{
  if (left < 0 || right >= rangeSize_ || left > right) {
    throw std::out_of_range("interval out of range");
  }
  tree_.assign(left, right + 1, false);
}

template <class TIndex>
bool IntervalSet<TIndex>::has(Index point) const
{
  if (point < 0 || point >= rangeSize_) {
    return false;
  }
  return tree_.containsPoint(point);
}

template <class TIndex>
std::int64_t IntervalSet<TIndex>::getLength() const
{
  std::int64_t total = 0;

  for (const auto& interval : getIntervals()) {
    total += static_cast<std::int64_t>(interval.second)
           - static_cast<std::int64_t>(interval.first);
  }
  return total;
}

template <class TIndex>
std::vector<std::pair<TIndex, TIndex>>
IntervalSet<TIndex>::getIntervals() const
{
  return tree_.getIntervals();
}

template <class TIndex>
TIndex IntervalSet<TIndex>::getRangeSize() const
{
  return rangeSize_;
}

template <class TIndex>
IntervalSet<TIndex> IntervalSet<TIndex>::unite(const IntervalSet& left,
                                                const IntervalSet& right)
{
  const Index resultRangeSize = std::max(left.rangeSize_, right.rangeSize_);
  IntervalSet result(resultRangeSize);

  for (const auto& interval : left.getIntervals()) {
    result.add(interval.first, interval.second);
  }
  for (const auto& interval : right.getIntervals()) {
    result.add(interval.first, interval.second);
  }
  return result;
}

template <class TIndex>
IntervalSet<TIndex> IntervalSet<TIndex>::intersect(const IntervalSet& left,
                                                    const IntervalSet& right)
{
  const Index resultRangeSize = std::min(left.rangeSize_, right.rangeSize_);
  IntervalSet result(resultRangeSize);

  const std::vector<Interval> leftIntervals = left.getIntervals();
  const std::vector<Interval> rightIntervals = right.getIntervals();

  std::size_t leftIndex = 0;
  std::size_t rightIndex = 0;

  while (leftIndex < leftIntervals.size()
         && rightIndex < rightIntervals.size()) {
    const Index lower = std::max(leftIntervals[leftIndex].first,
                                  rightIntervals[rightIndex].first);
    const Index upper = std::min(leftIntervals[leftIndex].second,
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

template <class TIndex>
bool IntervalSet<TIndex>::save(const std::string& filename) const
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

template <class TIndex>
bool IntervalSet<TIndex>::load(const std::string& filename)
{
  std::ifstream in(filename);

  if (!in) {
    return false;
  }

  Index loadedRangeSize = 0;

  if (!(in >> loadedRangeSize) || loadedRangeSize <= 0) {
    return false;
  }

  detail::SegmentTree<Index> loadedTree(loadedRangeSize);

  Index left = 0;
  Index right = 0;

  while (in >> left) {
    if (!(in >> right)) {
      return false;
    }
    if (left < 0 || right >= loadedRangeSize || left > right) {
      return false;
    }
    loadedTree.assign(left, right + 1, true);
  }

  if (!in.eof()) {
    return false;
  }

  rangeSize_ = loadedRangeSize;
  tree_ = std::move(loadedTree);
  return true;
}

}

#endif
