#pragma once

#include "IAnalyzer.h"
#include <map>

namespace loganalyzer {

class LevelCountAnalyzer : public IAnalyzer {
public:
  void process(const LogEntry &entry) override;
  void finalize(AnalysisResult &result) override;

private:
  std::map<LogLevel, uint64_t> counts_;
};

} // namespace loganalyzer
