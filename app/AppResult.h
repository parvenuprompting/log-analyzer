#pragma once

#include "../analysis/AnalysisResult.h"
#include <functional>
#include <string>

namespace loganalyzer {

// Granular error taxonomy for better error handling
enum class AppStatus {
  OK,
  INVALID_ARGS,
  INPUT_IO_ERROR,
  OUTPUT_IO_ERROR,
  PIPELINE_ERROR, // Error during analysis pipeline execution
  PARSER_ERROR,   // Log parsing failed critically
  IO_ERROR        // Generic I/O error
};

// Progress callback: return false to cancel
// float parameter is progress from 0.0 to 1.0
using ProgressCallback = std::function<bool(float progress)>;

struct AppResult {
  AppStatus status;
  std::string message;
  AnalysisResult analysisResult;
  bool wasCancelled = false; // True if cancelled via progress callback
};

} // namespace loganalyzer
