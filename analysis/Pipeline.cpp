#include "Pipeline.h"
#include "../core/LogParser.h"
#include "../io/FileReader.h"
#include "KeywordHitAnalyzer.h"
#include "LevelCountAnalyzer.h"
#include "TimeRangeFilter.h"
#include "TopErrorAnalyzer.h"
#include <memory>
#include <sys/stat.h>
#include <vector>

namespace loganalyzer {

AnalysisResult Pipeline::run(const std::string &inputPath,
                             const AnalysisContext &context,
                             ProgressCallback progressCallback,
                             bool *wasCancelled) {
  AnalysisResult result;

  if (wasCancelled) {
    *wasCancelled = false;
  }

  // Open file
  FileReader reader(inputPath);
  if (!reader.isOpen()) {
    return result; // Empty result on failure
  }

  // Get file size for progress reporting
  struct stat st;
  uint64_t fileSize = 0;
  if (stat(inputPath.c_str(), &st) == 0) {
    fileSize = st.st_size;
  }

  // Create filter
  TimeRangeFilter filter(context.fromTs, context.toTs);

  // Create analyzers
  std::vector<std::unique_ptr<IAnalyzer>> analyzers;
  analyzers.push_back(std::make_unique<LevelCountAnalyzer>());
  analyzers.push_back(std::make_unique<TopErrorAnalyzer>());

  if (context.keyword.has_value()) {
    analyzers.push_back(
        std::make_unique<KeywordHitAnalyzer>(context.keyword.value()));
  }

  // Streaming processing with progress reporting
  std::string line;
  size_t lineNumber;
  uint64_t bytesProcessed = 0;
  uint64_t lastReportedProgress = 0;
  // Report every 1% or 1MB, whichever is larger, to avoid too frequent
  // callbacks on small files and ensure we don't divide by zero logic if
  // fileSize is small (though here we use subtraction) std::max ensures it's
  // never 0 if we pick a minimum like 1024.
  const uint64_t progressInterval =
      std::max<uint64_t>(fileSize / 100, 1024 * 1024);

  while (reader.nextLine(line, lineNumber)) {
    result.totalLines++;
    bytesProcessed += line.size() + 1; // +1 for newline

    // Progress callback (check periodically, not every line for performance)
    if (progressCallback && fileSize > 0) {
      if (bytesProcessed - lastReportedProgress >= progressInterval) {
        float progress =
            static_cast<float>(bytesProcessed) / static_cast<float>(fileSize);
        if (!progressCallback(progress)) {
          // Cancellation requested
          if (wasCancelled) {
            *wasCancelled = true;
          }
          return result;
        }
        lastReportedProgress = bytesProcessed;
      }
    }

    // Parse line
    ParseResult parseResult = LogParser::parse(line, lineNumber);

    if (std::holds_alternative<LogEntry>(parseResult)) {
      // Valid entry
      result.parsedLines++;
      const LogEntry &entry = std::get<LogEntry>(parseResult);

      // Apply time filter
      if (filter.accept(entry.ts)) {
        if (filter.isActive()) {
          result.timeRangeMatched++;
        }

        // Process with all analyzers
        for (auto &analyzer : analyzers) {
          analyzer->process(entry);
        }
      }
    } else {
      // Parse error
      result.invalidLines++;
      const ParseError &error = std::get<ParseError>(parseResult);
      result.parseErrors[error.code]++;
    }
  }

  // Final progress report (100%)
  if (progressCallback) {
    progressCallback(1.0f);
  }

  // Finalize all analyzers
  for (auto &analyzer : analyzers) {
    analyzer->finalize(result);
  }

  return result;
}

} // namespace loganalyzer
