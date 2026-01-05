#pragma once

#include "../app/AppRequest.h"
#include "../app/AppResult.h"
#include "../app/Application.h"
#include "../core/Timestamp.h"
#include <optional>
#include <string>

namespace loganalyzer {

class GuiController {
public:
  GuiController();

  // Main render loop (called every frame)
  void render();

  // Get window should close status
  bool shouldClose() const { return shouldClose_; }

private:
  void renderFileInput();
  void renderFilters();
  void renderAnalyzeButton();
  void renderResults();
  void renderErrorDialog();

  // State
  Application app_;
  char inputPath_[512];
  char fromTimestamp_[32];
  char toTimestamp_[32];
  char keyword_[128];

  bool hasResults_;
  AppResult lastResult_;

  bool showError_;
  std::string errorMessage_;

  bool shouldClose_;

  // UI flags
  bool useTimeFilter_;
  bool useKeyword_;
};

} // namespace loganalyzer
