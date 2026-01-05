#pragma once

#include "../analysis/AnalysisResult.h"
#include <string>

namespace loganalyzer {

enum class AppStatus {
  OK,
  INVALID_ARGS,
  INPUT_IO_ERROR,
  OUTPUT_IO_ERROR,
  INTERNAL_ERROR
};

struct AppResult {
  AppStatus status;
  std::string message;
  AnalysisResult analysisResult;
};

} // namespace loganalyzer
