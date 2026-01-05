#pragma once

#include "AppRequest.h"
#include "AppResult.h"

namespace loganalyzer {

class Application {
public:
  // UI-agnostic entry point: runs analysis and returns result
  AppResult run(const AppRequest &request);
};

} // namespace loganalyzer
