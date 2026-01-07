#include "../external/IconsFontAwesome6.h"
#include "../external/imgui/imgui.h"
#include "GuiController.h"

namespace loganalyzer {

void GuiController::openLogForViewing(const std::string &path) {
  if (indexerThread_.joinable()) {
    // We don't want to block the UI, but we should ensure we don't leak
    // threads. In a real app, we might use a cancellation token. For now, we
    // join it (which might cause a small freeze if a thread was busy), but
    // usually it's fast enough.
    indexerThread_.join();
  }

  isIndexing_ = true;
  indexingProgress_ = 0.0f;
  indexerThread_ = std::thread(&GuiController::indexFileAsync, this, path);
}

void GuiController::indexFileAsync(const std::string &path) {
  try {
    auto file = std::make_unique<MemoryMappedFile>(path);
    if (!file->isOpen()) {
      isIndexing_ = false;
      return;
    }

    std::string_view data = file->getView();
    std::vector<size_t> localOffsets;

    if (!data.empty()) {
      // Memory Allocation Strategy (The "10GB Fix")
      // Average line length ~120 chars.
      size_t estimatedLines = data.size() / 120;
      localOffsets.reserve(estimatedLines + 100);

      localOffsets.push_back(0);
      size_t pos = 0;
      size_t totalSize = data.size();
      size_t lastUpdatePos = 0;

      while (true) {
        pos = data.find('\n', pos);
        if (pos == std::string_view::npos)
          break;

        pos++; // skip newline
        if (pos < totalSize) {
          localOffsets.push_back(pos);

          // Update progress every 1MB of processed data roughly
          if (pos - lastUpdatePos > 1024 * 1024) {
            indexingProgress_ = (float)pos / (float)totalSize;
            lastUpdatePos = pos;
          }
        } else {
          break;
        }
      }
    }

    // Success: Swap into controller state
    {
      std::lock_guard<std::mutex> lock(viewerMutex_);
      lineOffsets_ = std::move(localOffsets);
      logFile_ = std::move(file);
    }
  } catch (...) {
    // Error
  }
  isIndexing_ = false;
  indexingProgress_ = 1.0f;
}

void GuiController::renderLogViewer() {
  if (isIndexing_) {
    ImGui::Text(ICON_FA_SPINNER " Indexing Log File... %.1f%%",
                indexingProgress_ * 100.0f);
    ImGui::ProgressBar(indexingProgress_, ImVec2(-1, 0));
    ImGui::TextDisabled(
        "Please wait while the file is being indexed for efficient viewing.");
    return;
  }

  std::lock_guard<std::mutex> lock(viewerMutex_);

  if (!logFile_ || !logFile_->isOpen()) {
    ImGui::TextDisabled("No log file loaded for viewing.");
    return;
  }

  // Use child window for scrolling
  ImGui::BeginChild("LogView", ImVec2(0, -30), true,
                    ImGuiWindowFlags_HorizontalScrollbar);

  ImGuiListClipper clipper;
  clipper.Begin((int)lineOffsets_.size());

  std::string_view data = logFile_->getView();

  while (clipper.Step()) {
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
      size_t start = lineOffsets_[i];
      size_t end = (i + 1 < (int)lineOffsets_.size()) ? lineOffsets_[i + 1]
                                                      : data.size();

      // Adjust for newline char if present at end
      if (end > start && data[end - 1] == '\n')
        end--;
      if (end > start && data[end - 1] == '\r')
        end--;

      std::string_view line = data.substr(start, end - start);

      // Simple coloring based on content
      ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
      if (line.find("[ERROR]") != std::string::npos)
        color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
      else if (line.find("[WARNING]") != std::string::npos)
        color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
      else if (line.find("[INFO]") != std::string::npos)
        color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);

      ImGui::TextColored(color, "%.*s", (int)line.length(), line.data());
    }
  }

  ImGui::EndChild();

  ImGui::Text("Total Lines: %zu | View Mode: Read-Only (Memory Mapped)",
              lineOffsets_.size());
}

} // namespace loganalyzer
