#include "TextReportRenderer.h"
#include <iomanip>
#include <sstream>

namespace loganalyzer {

std::string TextReportRenderer::render(const AnalysisResult &result,
                                       const std::string &inputPath,
                                       const std::string &runTimestamp,
                                       const std::string &filtersDescription) {
  std::ostringstream oss;

  // Header
  oss << "=== Log Analysis Report ===\n";
  oss << "Input: " << inputPath << "\n";
  oss << "Run: " << runTimestamp << "\n";
  if (!filtersDescription.empty()) {
    oss << "Filters: " << filtersDescription << "\n";
  }
  oss << "\n";

  // Counters
  oss << "--- Counters ---\n";
  oss << "Total lines: " << result.totalLines << "\n";
  oss << "Parsed lines: " << result.parsedLines << "\n";
  oss << "Invalid lines: " << result.invalidLines << "\n";
  oss << "\n";

  // Parse Errors (deterministic order via map)
  if (!result.parseErrors.empty()) {
    oss << "--- Parse Errors ---\n";
    for (const auto &[code, count] : result.parseErrors) {
      std::string codeName;
      switch (code) {
      case ParseErrorCode::BadFormat:
        codeName = "BadFormat";
        break;
      case ParseErrorCode::BadTimestamp:
        codeName = "BadTimestamp";
        break;
      case ParseErrorCode::BadLevel:
        codeName = "BadLevel";
        break;
      case ParseErrorCode::MissingMessage:
        codeName = "MissingMessage";
        break;
      }
      oss << codeName << ": " << count << "\n";
    }
    oss << "\n";
  }

  // Level Counts (deterministic order via map)
  if (!result.levelCounts.empty()) {
    oss << "--- Level Counts ---\n";
    for (const auto &[level, count] : result.levelCounts) {
      std::string levelName;
      switch (level) {
      case LogLevel::ERROR:
        levelName = "ERROR";
        break;
      case LogLevel::WARNING:
        levelName = "WARNING";
        break;
      case LogLevel::INFO:
        levelName = "INFO";
        break;
      }
      oss << levelName << ": " << count << "\n";
    }
    oss << "\n";
  }

  // Time Range
  if (result.timeRangeMatched > 0) {
    oss << "--- Time Range ---\n";
    oss << "Matched: " << result.timeRangeMatched << "\n";
    oss << "\n";
  }

  // Keyword Hits
  if (result.keywordHits > 0) {
    oss << "--- Keyword Hits ---\n";
    oss << "Count: " << result.keywordHits << "\n";
    oss << "\n";
  }

  // Top Errors
  if (!result.topErrors.empty()) {
    oss << "--- Top 10 ERROR Messages ---\n";
    int rank = 1;
    for (const auto &[message, count] : result.topErrors) {
      oss << rank << ". " << message << " (" << count << ")\n";
      rank++;
    }
    oss << "\n";
  }

  return oss.str();
}

} // namespace loganalyzer
