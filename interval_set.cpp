#include "interval_set.h"

#include <stdexcept>

namespace novikov {

IntervalSet::IntervalSet(int rangeSize)
  : rangeSize_(rangeSize)
{
  if (rangeSize <= 0) {
    throw std::invalid_argument("range size must be positive");
  }
}

int IntervalSet::getRangeSize() const
{
  return rangeSize_;
}

}
