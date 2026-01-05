#include "TimeRangeFilter.h"

namespace loganalyzer {

TimeRangeFilter::TimeRangeFilter(std::optional<Timestamp> from,
                                 std::optional<Timestamp> to)
    : from_(from), to_(to) {}

bool TimeRangeFilter::accept(const Timestamp &ts) const {
  if (from_.has_value() && ts < from_.value()) {
    return false;
  }
  if (to_.has_value() && ts > to_.value()) {
    return false;
  }
  return true;
}

bool TimeRangeFilter::isActive() const {
  return from_.has_value() || to_.has_value();
}

} // namespace loganalyzer
