#include "TopErrorAnalyzer.h"
#include <algorithm>

namespace loganalyzer {

void TopErrorAnalyzer::process(const LogEntry &entry) {
  // Only track ERROR level messages
  if (entry.level == LogLevel::ERROR) {
    errorCounts_[entry.message]++;
  }
}

void TopErrorAnalyzer::finalize(AnalysisResult &result) {
  // Convert map to vector
  std::vector<std::pair<std::string, uint64_t>> vec(errorCounts_.begin(),
                                                    errorCounts_.end());

  // Sort: descending by count, then alphabetically by message (deterministic)
  std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
    if (a.second != b.second) {
      return a.second > b.second; // Higher count first
    }
    return a.first < b.first; // Alphabetical tie-break
  });

  // Take top 10
  size_t limit = std::min(vec.size(), size_t(10));
  result.topErrors.assign(vec.begin(), vec.begin() + limit);
}

} // namespace loganalyzer
