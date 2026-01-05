#include "KeywordHitAnalyzer.h"

namespace loganalyzer {

KeywordHitAnalyzer::KeywordHitAnalyzer(const std::string &keyword)
    : keyword_(keyword), hitCount_(0) {}

void KeywordHitAnalyzer::process(const LogEntry &entry) {
  // Case-sensitive substring match
  if (entry.message.find(keyword_) != std::string::npos) {
    hitCount_++;
  }
}

void KeywordHitAnalyzer::finalize(AnalysisResult &result) {
  result.keywordHits = hitCount_;
}

} // namespace loganalyzer
