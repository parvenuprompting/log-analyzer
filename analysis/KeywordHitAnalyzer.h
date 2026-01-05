#pragma once

#include "IAnalyzer.h"
#include <string>

namespace loganalyzer {

class KeywordHitAnalyzer : public IAnalyzer {
public:
  explicit KeywordHitAnalyzer(const std::string &keyword);

  void process(const LogEntry &entry) override;
  void finalize(AnalysisResult &result) override;

private:
  std::string keyword_;
  uint64_t hitCount_;
};

} // namespace loganalyzer
