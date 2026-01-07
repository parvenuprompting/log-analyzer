#pragma once

#include "../core/LogLevel.h"
#include "../core/ParseError.h"
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

  void merge(const AnalysisResult &other);
};

} // namespace loganalyzer
