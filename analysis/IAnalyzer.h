#pragma once

#include "../core/LogEntry.h"
#include "AnalysisResult.h"

namespace loganalyzer {

class IAnalyzer {
public:
  virtual ~IAnalyzer() = default;

  // Process single log entry (streaming)
  virtual void process(const LogEntry &entry) = 0;

  // Finalize analysis and write results
  virtual void finalize(AnalysisResult &result) = 0;
};

} // namespace loganalyzer
