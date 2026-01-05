#pragma once

#include "IAnalyzer.h"
#include <map>
#include <string>

namespace loganalyzer {

class TopErrorAnalyzer : public IAnalyzer {
public:
  void process(const LogEntry &entry) override;
  void finalize(AnalysisResult &result) override;

private:
  std::map<std::string, uint64_t> errorCounts_;
};

} // namespace loganalyzer
