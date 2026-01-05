#include "app/AppRequest.h"
#include "app/AppResult.h"
#include "app/Application.h"
#include "core/Timestamp.h"
#include "io/FileWriter.h"
#include "report/TextReportRenderer.h"
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>

using namespace loganalyzer;

struct CliArgs {
  std::string inputPath;
  std::string reportPath;
  std::optional<Timestamp> from;
  std::optional<Timestamp> to;
  std::optional<std::string> keyword;
};

bool parseArgs(int argc, char *argv[], CliArgs &args) {
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc) {
        args.inputPath = argv[++i];
      } else {
        return false;
      }
    } else if (std::strcmp(argv[i], "--report") == 0) {
      if (i + 1 < argc) {
        args.reportPath = argv[++i];
      } else {
        return false;
      }
    } else if (std::strcmp(argv[i], "--from") == 0) {
      if (i + 1 < argc) {
        Timestamp ts;
        if (Timestamp::parse(argv[++i], ts)) {
          args.from = ts;
        } else {
          std::cerr << "Invalid --from timestamp format\n";
          return false;
        }
      } else {
        return false;
      }
    } else if (std::strcmp(argv[i], "--to") == 0) {
      if (i + 1 < argc) {
        Timestamp ts;
        if (Timestamp::parse(argv[++i], ts)) {
          args.to = ts;
        } else {
          std::cerr << "Invalid --to timestamp format\n";
          return false;
        }
      } else {
        return false;
      }
    } else if (std::strcmp(argv[i], "--keyword") == 0) {
      if (i + 1 < argc) {
        args.keyword = argv[++i];
      } else {
        return false;
      }
    }
  }

  return !args.inputPath.empty() && !args.reportPath.empty();
}

std::string getCurrentTimestamp() {
  std::time_t now = std::time(nullptr);
  std::tm *tm = std::localtime(&now);
  char buffer[64];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
  return std::string(buffer);
}

std::string buildFiltersDescription(const CliArgs &args) {
  std::ostringstream oss;
  bool first = true;

  if (args.from.has_value()) {
    oss << "from=" << args.from.value().toString();
    first = false;
  }

  if (args.to.has_value()) {
    if (!first)
      oss << ", ";
    oss << "to=" << args.to.value().toString();
    first = false;
  }

  if (args.keyword.has_value()) {
    if (!first)
      oss << ", ";
    oss << "keyword=\"" << args.keyword.value() << "\"";
  }

  return oss.str();
}

int main(int argc, char *argv[]) {
  // Parse CLI arguments
  CliArgs cliArgs;
  if (!parseArgs(argc, argv, cliArgs)) {
    std::cerr << "Usage: " << argv[0] << " --input <path> --report <path> "
              << "[--from <YYYY-MM-DD HH:MM:SS>] [--to <YYYY-MM-DD HH:MM:SS>] "
              << "[--keyword <text>]\n";
    return 2; // INVALID_ARGS
  }

  // Build AppRequest
  AppRequest request;
  request.inputPath = cliArgs.inputPath;
  request.fromTimestamp = cliArgs.from;
  request.toTimestamp = cliArgs.to;
  request.keyword = cliArgs.keyword;

  // Run application
  Application app;
  AppResult result = app.run(request);

  // Handle errors
  if (result.status != AppStatus::OK) {
    std::cerr << "Error: " << result.message << "\n";

    switch (result.status) {
    case AppStatus::INVALID_ARGS:
      return 2;
    case AppStatus::INPUT_IO_ERROR:
      return 3;
    case AppStatus::OUTPUT_IO_ERROR:
      return 4;
    case AppStatus::PIPELINE_ERROR:
    case AppStatus::PARSER_ERROR:
    case AppStatus::IO_ERROR:
      return 4;
    default:
      return 4;
    }
  }

  // Render report
  std::string reportText = TextReportRenderer::render(
      result.analysisResult, cliArgs.inputPath, getCurrentTimestamp(),
      buildFiltersDescription(cliArgs));

  // Write report
  FileWriter writer(cliArgs.reportPath);
  if (!writer.isOpen()) {
    std::cerr << "Error: " << writer.getError() << "\n";
    return 4; // OUTPUT_IO_ERROR
  }

  if (!writer.write(reportText)) {
    std::cerr << "Error: Failed to write report\n";
    return 4;
  }

  std::cout << "Analysis complete. Report written to: " << cliArgs.reportPath
            << "\n";
  return 0;
}
