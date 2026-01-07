#pragma once

#include "../core/LogEntry.h"
#include "AnalysisResult.h"
#include <concepts>

namespace loganalyzer {

class IAnalyzer {
public:
  virtual ~IAnalyzer() = default;

  // Process single log entry (streaming)
  virtual void process(const LogEntry &entry) = 0;

  // Finalize analysis and write results
  virtual void finalize(AnalysisResult &result) = 0;
};

// C++20 Concept
template <typename T>
concept Analyzer =
    requires(T a, const LogEntry &entry, AnalysisResult &result) {
      { a.process(entry) } -> std::same_as<void>;
      { a.finalize(result) } -> std::same_as<void>;
    };

} // namespace loganalyzer
