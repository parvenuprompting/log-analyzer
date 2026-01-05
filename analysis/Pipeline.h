#pragma once

#include "AnalysisContext.h"
#include "AnalysisResult.h"
#include <functional>
#include <string>

namespace loganalyzer {

// Progress callback: return false to cancel
using ProgressCallback = std::function<bool(float progress)>;

class Pipeline {
public:
  // Run analysis pipeline on input file with given context
  // Optional progress callback for cancellation and progress reporting
  // wasCancelled is set to true if cancelled via callback
  static AnalysisResult run(const std::string &inputPath,
                            const AnalysisContext &context,
                            ProgressCallback progressCallback = nullptr,
                            bool *wasCancelled = nullptr);
};

} // namespace loganalyzer
