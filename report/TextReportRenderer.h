#pragma once

#include "../analysis/AnalysisResult.h"
#include <string>

namespace loganalyzer {

class TextReportRenderer {
public:
  // Renders AnalysisResult to plain text format (deterministic)
  static std::string render(const AnalysisResult &result,
                            const std::string &inputPath,
                            const std::string &runTimestamp,
                            const std::string &filtersDescription);
};

} // namespace loganalyzer
