#pragma once

#include "../core/Timestamp.h"
#include <optional>

namespace loganalyzer {

class TimeRangeFilter {
public:
  TimeRangeFilter(std::optional<Timestamp> from, std::optional<Timestamp> to);

  // Returns true if entry timestamp is within range (inclusive)
  bool accept(const Timestamp &ts) const;

  bool isActive() const;

private:
  std::optional<Timestamp> from_;
  std::optional<Timestamp> to_;
};

} // namespace loganalyzer
