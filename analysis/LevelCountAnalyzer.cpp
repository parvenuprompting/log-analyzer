#include "LevelCountAnalyzer.h"

namespace loganalyzer {

void LevelCountAnalyzer::process(const LogEntry &entry) {
  counts_[entry.level]++;
}

void LevelCountAnalyzer::finalize(AnalysisResult &result) {
  result.levelCounts = counts_;
}

} // namespace loganalyzer
