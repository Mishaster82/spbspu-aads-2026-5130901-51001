#ifndef NOVIKOV_INTERVAL_SET_H
#define NOVIKOV_INTERVAL_SET_H

#include <cstdint>
#include <utility>
#include <vector>

namespace novikov {

class IntervalSet
{
public:
  using Interval = std::pair<int, int>;

  IntervalSet(const IntervalSet& other) = default;
  IntervalSet(IntervalSet&& other) noexcept = default;
  explicit IntervalSet(int rangeSize);
  ~IntervalSet() = default;

  IntervalSet& operator=(const IntervalSet& other) = default;
  IntervalSet& operator=(IntervalSet&& other) noexcept = default;

  void add(int left, int right);

  int getRangeSize() const;

private:
  class SegmentTree
  {
  public:
    SegmentTree(const SegmentTree& other) = default;
    SegmentTree(SegmentTree&& other) noexcept = default;
    explicit SegmentTree(int rangeSize);
    ~SegmentTree() = default;

    SegmentTree& operator=(const SegmentTree& other) = default;
    SegmentTree& operator=(SegmentTree&& other) noexcept = default;

    void assign(int left, int right, bool value);

  private:
    int rangeSize_;
    std::vector<std::int64_t> sum_;
    std::vector<std::int8_t> lazy_;

    static int checkRangeSize(int rangeSize);

    void assignImpl(int node, int nodeLeft, int nodeRight,
                     int queryLeft, int queryRight, bool value);
    void pushDown(int node, int nodeLeft, int nodeRight);
    void applyAssign(int node, int nodeLeft, int nodeRight, bool value);
  };

  int rangeSize_;
  SegmentTree tree_;
};

}

#endif
