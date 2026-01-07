#pragma once

#include "../core/LogLevel.h"
#include "../core/ParseError.h"
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace loganalyzer {

struct AnalysisResult {
  uint64_t totalLines = 0;
  uint64_t parsedLines = 0;
  uint64_t invalidLines = 0;

  std::map<ParseErrorCode, uint64_t> parseErrors;
  std::map<LogLevel, uint64_t> levelCounts;

  uint64_t keywordHits = 0;
  uint64_t timeRangeMatched = 0;

  // Top 10 ERROR messages: (message, count), deterministically sorted
  std::vector<std::pair<std::string, uint64_t>> topErrors;

  // Timeline Data: Minute-by-minute error/warning counts
  // Storing simple counts per minute bucket (relative to start time or
  // absolute?) Let's store absolute timestamps (rounded to minute) -> count
  // Using vector of pairs for determinism and easy plotting
  struct TimelineBucket {
    uint64_t timestamp; // Unix timestamp for minute
    uint32_t errorCount;
    uint32_t warningCount;
  };
  std::vector<TimelineBucket> timeline;

  // Heatmap Data: 7 Days x 24 Hours
  // heatmap[day_of_week][hour_of_day] = count
  // day 0 = Sunday, 1 = Monday ... 6 = Saturday
  std::array<std::array<uint32_t, 24>, 7> heatmap = {};

  void merge(const AnalysisResult &other);
};

} // namespace loganalyzer
