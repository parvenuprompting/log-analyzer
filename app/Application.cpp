#include "Application.h"
#include "../analysis/AnalysisContext.h"
#include "../analysis/Pipeline.h"
#include "../io/FileReader.h"

namespace loganalyzer {

AppResult Application::run(const AppRequest &request) {
  AppResult result;
  result.status = AppStatus::OK;
  result.message = "Analysis completed successfully";

  // Validate input
  if (request.inputPath.empty()) {
    result.status = AppStatus::INVALID_ARGS;
    result.message = "Input path cannot be empty";
    return result;
  }

  // Check if file exists and is readable
  FileReader testReader(request.inputPath);
  if (!testReader.isOpen()) {
    result.status = AppStatus::INPUT_IO_ERROR;
    result.message = testReader.getError();
    return result;
  }

  // Build analysis context
  AnalysisContext context;
  context.fromTs = request.fromTimestamp;
  context.toTs = request.toTimestamp;
  context.keyword = request.keyword;

  // Run pipeline
  try {
    result.analysisResult = Pipeline::run(request.inputPath, context);
  } catch (const std::exception &e) {
    result.status = AppStatus::INTERNAL_ERROR;
    result.message = std::string("Internal error: ") + e.what();
    return result;
  } catch (...) {
    result.status = AppStatus::INTERNAL_ERROR;
    result.message = "Unknown internal error";
    return result;
  }

  return result;
}

} // namespace loganalyzer
