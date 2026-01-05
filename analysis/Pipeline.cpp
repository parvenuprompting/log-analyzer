#include "Pipeline.h"
#include "../core/LogParser.h"
#include "../io/FileReader.h"
#include "KeywordHitAnalyzer.h"
#include "LevelCountAnalyzer.h"
#include "TimeRangeFilter.h"
#include "TopErrorAnalyzer.h"
#include <memory>
#include <vector>

namespace loganalyzer {

AnalysisResult Pipeline::run(const std::string &inputPath,
                             const AnalysisContext &context) {
  AnalysisResult result;

  // Open file
  FileReader reader(inputPath);
  if (!reader.isOpen()) {
    return result; // Empty result on failure
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

  // Streaming processing
  std::string line;
  size_t lineNumber;

  while (reader.nextLine(line, lineNumber)) {
    result.totalLines++;

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

  // Finalize all analyzers
  for (auto &analyzer : analyzers) {
    analyzer->finalize(result);
  }

  return result;
}

} // namespace loganalyzer
