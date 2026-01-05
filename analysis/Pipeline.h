#pragma once

#include "AnalysisContext.h"
#include "AnalysisResult.h"
#include <string>

namespace loganalyzer {

class Pipeline {
public:
  // Run analysis pipeline on input file with given context
  // Returns populated AnalysisResult
  static AnalysisResult run(const std::string &inputPath,
                            const AnalysisContext &context);
};

} // namespace loganalyzer
