#pragma once

#include "AppRequest.h"
#include "AppResult.h"

namespace loganalyzer {

class Application {
public:
  // UI-agnostic entry point: runs analysis and returns result
  // Optional progress callback for cancellation and progress reporting
  AppResult run(const AppRequest &request,
                ProgressCallback progressCallback = nullptr);

  // Headless API: returns result without writing to file
  // Useful for HTTP/WASM/embedded frontends
  void runHeadless(const AppRequest &request, AppResult &result,
                   ProgressCallback progressCallback = nullptr);
};

} // namespace loganalyzer
