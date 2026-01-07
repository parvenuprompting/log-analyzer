#pragma once

#include "../app/AppRequest.h"
#include "../app/AppResult.h"
#include "../app/Application.h"
#include "../core/Timestamp.h"
#include "../io/MemoryMappedFile.h"
#include <atomic>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

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
  void renderFilePicker();

  // File picker helpers
  void updateFileList();

  // Analysis thread management
  void startAnalysis();
  void checkAnalysisComplete();

  // Progress callback for analysis
  bool progressCallback(float progress);

  // State
  Application app_;
  std::string inputPath_;
  std::string fromTimestamp_;
  std::string toTimestamp_;
  std::string keyword_;

  bool hasResults_;
  AppResult lastResult_;
  mutable std::mutex resultMutex_; // Protects shared result data

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

  // File Picker State
  bool showFilePicker_;
  std::filesystem::path currentPickerDir_;
  std::vector<std::filesystem::path> pickerFiles_;
  std::vector<std::filesystem::path> pickerDirs_;
  std::string pickerSearch_;

  // Log Viewer
  void renderLogViewer();
  std::unique_ptr<MemoryMappedFile> logFile_;
  std::vector<size_t> lineOffsets_; // Cache of line start positions
  bool showLogViewer_;
  void openLogForViewing(const std::string &path);
};

} // namespace loganalyzer
