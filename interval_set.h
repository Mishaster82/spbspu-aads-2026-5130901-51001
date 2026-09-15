#ifndef NOVIKOV_INTERVAL_SET_H
#define NOVIKOV_INTERVAL_SET_H

namespace novikov {

class IntervalSet
{
public:
  using Interval = std::pair<int, int>;

  explicit IntervalSet(int rangeSize);

  int getRangeSize() const;

private:
  int rangeSize_;
};

}

#endif
