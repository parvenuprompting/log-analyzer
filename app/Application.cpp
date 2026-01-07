#include "Application.h"
#include "../analysis/AnalysisContext.h"
#include "../analysis/Pipeline.h"
#include "../io/MemoryMappedFile.h"

namespace loganalyzer {

AppResult Application::run(const AppRequest &request,
                           ProgressCallback progressCallback) {
  AppResult result;
  runHeadless(request, result, progressCallback);
  return result;
}

void Application::runHeadless(const AppRequest &request, AppResult &result,
                              ProgressCallback progressCallback) {
  result.status = AppStatus::OK;
  result.message = "Analysis completed successfully";
  result.wasCancelled = false;

  // Validate input
  if (request.inputPath.empty()) {
    result.status = AppStatus::INVALID_ARGS;
    result.message = "Input path cannot be empty";
    return;
  }

  // Check if file exists and is readable
  MemoryMappedFile testReader(request.inputPath);
  if (!testReader.isOpen()) {
    result.status = AppStatus::INPUT_IO_ERROR;
    result.message = "Cannot open file (not found or permission denied)";
    return;
  }

  // Build analysis context
  AnalysisContext context;
  context.fromTs = request.fromTimestamp;
  context.toTs = request.toTimestamp;
  context.keyword = request.keyword;
  context.customPattern = request.customPattern;

  // Run pipeline with progress callback
  try {
    result.analysisResult = Pipeline::run(
        request.inputPath, context, progressCallback, &result.wasCancelled);

    if (result.wasCancelled) {
      result.status = AppStatus::OK; // Cancellation is not an error
      result.message = "Analysis cancelled by user";
    }
  } catch (const std::runtime_error &e) {
    // Pipeline-specific errors
    result.status = AppStatus::PIPELINE_ERROR;
    result.message = std::string("Pipeline error: ") + e.what();
    return;
  } catch (const std::exception &e) {
    // Generic errors
    result.status = AppStatus::IO_ERROR;
    result.message = std::string("I/O error: ") + e.what();
    return;
  } catch (...) {
    result.status = AppStatus::IO_ERROR;
    result.message = "Unknown error during analysis";
    return;
  }
}

} // namespace loganalyzer
