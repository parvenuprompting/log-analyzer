#pragma once

#include "../app/AppRequest.h"
#include "../app/AppResult.h"
#include "../app/Application.h"
#include "../core/Timestamp.h"
#include <atomic>
#include <optional>
#include <string>
#include <thread>

namespace loganalyzer {

class GuiController {
public:
  GuiController();

  // Main render loop (called every frame)
  void render();

  // Get window should close status
  bool shouldClose() const { return shouldClose_; }

private:
  void applyModernTheme();
  void renderFileInput();
  void renderFilters();
  void renderAnalyzeButton();
  void renderProgressBar();
  void renderResults();
  void renderErrorDialog();

  // Analysis thread management
  void startAnalysis();
  void checkAnalysisComplete();

  // Progress callback for analysis
  bool progressCallback(float progress);

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

  // Analysis state
  bool isAnalyzing_;
  std::atomic<float> analysisProgress_;
  std::atomic<bool> cancelRequested_;
  std::thread analysisThread_;
  std::atomic<bool> analysisComplete_;
  AppRequest currentRequest_;

  // UI flags
  bool useTimeFilter_;
  bool useKeyword_;
};

} // namespace loganalyzer
