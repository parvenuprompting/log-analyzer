#include "../external/imgui/imgui.h"
#include "GuiController.h"

namespace loganalyzer {

void GuiController::openLogForViewing(const std::string &path) {
  try {
    logFile_ = std::make_unique<MemoryMappedFile>(path);
    if (!logFile_->isOpen()) {
      logFile_.reset();
      return;
    }

    // Index lines for clipper
    lineOffsets_.clear();
    lineOffsets_.reserve(1000000); // Pre-allocate decent size

    std::string_view data = logFile_->getView();
    if (data.empty())
      return;

    lineOffsets_.push_back(0);
    size_t pos = 0;
    while (true) {
      pos = data.find('\n', pos);
      if (pos == std::string_view::npos)
        break;
      pos++; // skip newline
      if (pos < data.size()) {
        lineOffsets_.push_back(pos);
      } else {
        break;
      }
    }
  } catch (...) {
    logFile_.reset();
  }
}

void GuiController::renderLogViewer() {
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
