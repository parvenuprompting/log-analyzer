#include "AnalysisResult.h"
#include <algorithm>
#include <map>

namespace loganalyzer {

void AnalysisResult::merge(const AnalysisResult &other) {
  totalLines += other.totalLines;
  parsedLines += other.parsedLines;
  invalidLines += other.invalidLines;
  keywordHits += other.keywordHits;
  timeRangeMatched += other.timeRangeMatched;

  // Merge maps
  for (const auto &[code, count] : other.parseErrors) {
    parseErrors[code] += count;
  }

  for (const auto &[level, count] : other.levelCounts) {
    levelCounts[level] += count;
  }

  // Merge Heatmap
  for (size_t d = 0; d < 7; ++d) {
    for (size_t h = 0; h < 24; ++h) {
      heatmap[d][h] += other.heatmap[d][h];
    }
  }

  // Merge Timeline
  // NOTE: Simple concatenation. Sorting should happen at the end of pipeline.
  // Ideally, specialized merging that buckets by minute would be better,
  // but for now, append and let the GUI or finalizer sort it.
  timeline.insert(timeline.end(), other.timeline.begin(), other.timeline.end());

  // Merge topErrors
  // Strategy: Combine both vectors into a map to sum counts, then recreate
  // vector
  std::map<std::string, uint64_t> errorMap;

  // Add current
  for (const auto &pair : topErrors) {
    errorMap[pair.first] += pair.second;
  }

  // Add other
  for (const auto &pair : other.topErrors) {
    errorMap[pair.first] += pair.second;
  }

  // Convert back to vector
  std::vector<std::pair<std::string, uint64_t>> vec(errorMap.begin(),
                                                    errorMap.end());

  // Sort: descending by count, then alphabetically
  std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
    if (a.second != b.second) {
      return a.second > b.second; // Higher count first
    }
    return a.first < b.first; // Alphabetical tie-break
  });

  // Limit to top 10
  size_t limit = std::min(vec.size(), size_t(10));
  topErrors.assign(vec.begin(), vec.begin() + limit);
}

} // namespace loganalyzer
