#include "GuiController.h"
#include "../external/imgui/imgui.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

namespace loganalyzer {

GuiController::GuiController()
    : hasResults_(false), showError_(false), shouldClose_(false),
      isAnalyzing_(false), analysisProgress_(0.0f), cancelRequested_(false),
      analysisComplete_(false), useTimeFilter_(false), useKeyword_(false),
      showFilePicker_(false) {
  std::memset(inputPath_, 0, sizeof(inputPath_));
  std::memset(fromTimestamp_, 0, sizeof(fromTimestamp_));
  std::memset(toTimestamp_, 0, sizeof(toTimestamp_));
  std::memset(keyword_, 0, sizeof(keyword_));
  std::memset(pickerSearch_, 0, sizeof(pickerSearch_));

  // Default example
  std::strcpy(inputPath_, "tests/sample_log.txt");

  // Init picker path to current directory
  currentPickerDir_ = std::filesystem::current_path();

  applyModernTheme();
}

void GuiController::applyModernTheme() {
  // ... [Same theme code] ...
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.18f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.14f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.24f, 0.27f, 0.38f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.13f, 0.24f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.18f, 0.32f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.23f, 0.40f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.13f, 0.24f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.18f, 0.32f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.91f, 0.27f, 0.38f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.27f, 0.38f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.91f, 0.27f, 0.38f, 0.80f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.95f, 0.35f, 0.45f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.22f, 0.32f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.09f, 0.18f, 0.32f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.12f, 0.23f, 0.40f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.28f, 0.48f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.27f, 0.38f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.27f, 0.38f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.91f, 0.27f, 0.38f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.18f, 0.32f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.27f, 0.38f, 0.80f);
  colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.23f, 0.40f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.09f, 0.18f, 0.32f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.06f, 0.13f, 0.24f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.24f, 0.27f, 0.38f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.18f, 0.20f, 0.28f, 1.00f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.91f, 0.27f, 0.38f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  style.WindowRounding = 8.0f;
  style.ChildRounding = 6.0f;
  style.FrameRounding = 6.0f;
  style.PopupRounding = 6.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 6.0f;
  style.TabRounding = 6.0f;
  style.WindowPadding = ImVec2(12, 12);
  style.FramePadding = ImVec2(8, 6);
  style.ItemSpacing = ImVec2(12, 8);
  style.ItemInnerSpacing = ImVec2(8, 6);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;
}

void GuiController::render() {
  if (isAnalyzing_) {
    checkAnalysisComplete();
  }

  ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
  ImGui::Begin("Log Analyzer Pro", &shouldClose_, ImGuiWindowFlags_NoCollapse);

  renderFileInput();
  renderFilters();
  renderAnalyzeButton();

  if (isAnalyzing_) {
    renderProgressBar();
  }

  if (hasResults_) {
    renderResults();
  }

  ImGui::End();

  // Dialogs
  if (showFilePicker_) {
    renderFilePicker();
  }

  if (showError_) {
    renderErrorDialog();
  }
}

void GuiController::renderFileInput() {
  ImGui::SeparatorText("📁 Input File");

  float buttonWidth = 100.0f;
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - 10);
  ImGui::InputTextWithHint("##input", "Path to log file...", inputPath_,
                           sizeof(inputPath_));

  ImGui::SameLine();
  if (ImGui::Button("Browse...", ImVec2(buttonWidth, 0))) {
    showFilePicker_ = true;
    updateFileList();
    ImGui::OpenPopup("File Picker");
  }

  ImGui::Spacing();
}

void GuiController::renderFilters() {
  ImGui::SeparatorText("🔍 Filters (Optional)");

  ImGui::Checkbox("Time Range", &useTimeFilter_);
  if (useTimeFilter_) {
    ImGui::Indent();
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##from", "YYYY-MM-DD HH:MM:SS", fromTimestamp_,
                             sizeof(fromTimestamp_));
    ImGui::SameLine();
    ImGui::TextDisabled("From");

    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##to", "YYYY-MM-DD HH:MM:SS", toTimestamp_,
                             sizeof(toTimestamp_));
    ImGui::SameLine();
    ImGui::TextDisabled("To");
    ImGui::Unindent();
  }

  ImGui::Checkbox("Keyword Search", &useKeyword_);
  if (useKeyword_) {
    ImGui::Indent();
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("##keyword", "Search term (case-sensitive)...",
                             keyword_, sizeof(keyword_));
    ImGui::Unindent();
  }

  ImGui::Spacing();
}

void GuiController::renderAnalyzeButton() {
  if (isAnalyzing_) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button("❌ Cancel Analysis", ImVec2(-1, 50))) {
      cancelRequested_ = true;
    }
    ImGui::PopStyleColor();
  } else {
    if (ImGui::Button("🚀 Analyze Log", ImVec2(-1, 50))) {
      startAnalysis();
    }
  }
  ImGui::Spacing();
}

void GuiController::renderProgressBar() {
  float progress = analysisProgress_.load();
  ImGui::SeparatorText("⚡ Analysis in Progress");
  char overlay[64];
  snprintf(overlay, sizeof(overlay), "%.0f%%", progress * 100.0f);
  ImGui::ProgressBar(progress, ImVec2(-1, 30), overlay);
  ImGui::Spacing();
}

void GuiController::startAnalysis() {
  currentRequest_.inputPath = std::string(inputPath_);

  if (useTimeFilter_) {
    Timestamp from, to;
    if (Timestamp::parse(std::string(fromTimestamp_), from)) {
      currentRequest_.fromTimestamp = from;
    }
    if (Timestamp::parse(std::string(toTimestamp_), to)) {
      currentRequest_.toTimestamp = to;
    }
  }

  if (useKeyword_ && std::strlen(keyword_) > 0) {
    currentRequest_.keyword = std::string(keyword_);
  }

  isAnalyzing_ = true;
  analysisProgress_ = 0.0f;
  cancelRequested_ = false;
  analysisComplete_ = false;
  hasResults_ = false;

  analysisThread_ = std::thread([this]() {
    auto callback = [this](float progress) -> bool {
      analysisProgress_ = progress;
      return !cancelRequested_.load();
    };

    AppResult result = app_.run(currentRequest_, callback);

    {
      // Assuming resultMutex_ is a member of GuiController
      std::lock_guard<std::mutex> lock(resultMutex_);
      lastResult_ = result;
    }

    analysisComplete_ = true;
  });
}

void GuiController::checkAnalysisComplete() {
  if (analysisComplete_.load()) {
    if (analysisThread_.joinable()) {
      analysisThread_.join();
    }
    isAnalyzing_ = false;

    std::lock_guard<std::mutex> lock(resultMutex_);
    if (lastResult_.wasCancelled) {
      hasResults_ = false;
      showError_ = false;
    } else if (lastResult_.status == AppStatus::OK) {
      hasResults_ = true;
      showError_ = false;
    } else {
      hasResults_ = false;
      showError_ = true;
      errorMessage_ = lastResult_.message;
    }
  }
}

bool GuiController::progressCallback(float progress) {
  analysisProgress_ = progress;
  return !cancelRequested_.load();
}

void GuiController::renderResults() {
  // ... [Result rendering code - same as before] ...
  const auto &result = lastResult_.analysisResult;

  ImGui::SeparatorText("📊 Analysis Results");

  if (ImGui::BeginTable("stats_cards", 3, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::BeginChild("card1", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Total Lines");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%llu", result.totalLines);
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::TableNextColumn();
    ImGui::BeginChild("card2", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Parsed");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::Text("%llu", result.parsedLines);
    ImGui::EndChild();
    ImGui::TableNextColumn();
    ImGui::BeginChild("card3", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "Invalid");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::Text("%llu", result.invalidLines);
    ImGui::EndChild();
    ImGui::EndTable();
  }
  ImGui::Spacing();

  if (!result.levelCounts.empty() &&
      ImGui::CollapsingHeader("📈 Level Counts",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();
    struct LevelRow {
      LogLevel level;
      uint64_t count;
      const char *name;
      ImVec4 color;
    };
    std::vector<LevelRow> rows;
    for (const auto &[level, count] : result.levelCounts) {
      const char *name = "UNKNOWN";
      ImVec4 color = ImVec4(1, 1, 1, 1);
      switch (level) {
      case LogLevel::ERROR:
        name = "ERROR";
        color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        break;
      case LogLevel::WARNING:
        name = "WARNING";
        color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        break;
      case LogLevel::INFO:
        name = "INFO";
        color = ImVec4(0.3f, 0.8f, 1.0f, 1.0f);
        break;
      }
      rows.push_back({level, count, name, color});
    }

    if (ImGui::BeginTable("level_counts", 2,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Sortable)) {
      ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_DefaultSort);
      ImGui::TableSetupColumn("Count",
                              ImGuiTableColumnFlags_PreferSortDescending);
      ImGui::TableHeadersRow();
      if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs()) {
        if (sorts_specs->SpecsDirty) {
          std::sort(rows.begin(), rows.end(),
                    [sorts_specs](const LevelRow &a, const LevelRow &b) {
                      bool delta = false;
                      if (sorts_specs->Specs->ColumnIndex == 0)
                        delta = std::string(a.name) < std::string(b.name);
                      else
                        delta = a.count < b.count;
                      return sorts_specs->Specs->SortDirection ==
                                     ImGuiSortDirection_Ascending
                                 ? delta
                                 : !delta;
                    });
          sorts_specs->SpecsDirty = false;
        }
      }
      for (const auto &row : rows) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(row.color, "%s", row.name);
        ImGui::TableNextColumn();
        ImGui::Text("%llu", row.count);
      }
      ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Text("Distribution:");
    float values[3] = {0, 0, 0};
    auto itInfo = result.levelCounts.find(LogLevel::INFO);
    if (itInfo != result.levelCounts.end())
      values[0] = (float)itInfo->second;
    auto itWarn = result.levelCounts.find(LogLevel::WARNING);
    if (itWarn != result.levelCounts.end())
      values[1] = (float)itWarn->second;
    auto itErr = result.levelCounts.find(LogLevel::ERROR);
    if (itErr != result.levelCounts.end())
      values[2] = (float)itErr->second;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 100.0f;
    float barWidth = width / 3.0f - 10.0f;
    float maxVal = std::max({values[0], values[1], values[2]});
    if (maxVal < 1.0f)
      maxVal = 1.0f;

    const char *labels[] = {"INFO", "WARNING", "ERROR"};
    ImU32 colors[] = {IM_COL32(76, 204, 255, 255), IM_COL32(255, 204, 51, 255),
                      IM_COL32(255, 76, 76, 255)};
    for (int i = 0; i < 3; i++) {
      float barHeight = (values[i] / maxVal) * (height - 20);
      float x = p.x + i * (width / 3.0f);
      float y = p.y + height;
      draw_list->AddRectFilled(ImVec2(x + 5, y - barHeight - 20),
                               ImVec2(x + barWidth, y - 20), colors[i], 4.0f);
      draw_list->AddText(ImVec2(x + 5, y - 15), IM_COL32_WHITE, labels[i]);
      char valStr[32];
      snprintf(valStr, sizeof(valStr), "%.0f", values[i]);
      draw_list->AddText(ImVec2(x + 5, y - barHeight - 35), IM_COL32_WHITE,
                         valStr);
    }
    ImGui::Dummy(ImVec2(width, height));
    ImGui::Unindent();
  }

  if (!result.topErrors.empty() &&
      ImGui::CollapsingHeader("🔥 Top 10 ERROR Messages",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();
    struct ErrorRow {
      int rank;
      std::string message;
      uint64_t count;
    };
    std::vector<ErrorRow> errorRows;
    int rank = 1;
    for (const auto &[message, count] : result.topErrors)
      errorRows.push_back({rank++, message, count});
    if (ImGui::BeginTable("top_errors", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Sortable)) {
      ImGui::TableSetupColumn(
          "#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
          30.0f);
      ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_DefaultSort);
      ImGui::TableSetupColumn("Count",
                              ImGuiTableColumnFlags_PreferSortDescending |
                                  ImGuiTableColumnFlags_WidthFixed,
                              60.0f);
      ImGui::TableHeadersRow();
      if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs()) {
        if (sorts_specs->SpecsDirty) {
          std::sort(errorRows.begin(), errorRows.end(),
                    [sorts_specs](const ErrorRow &a, const ErrorRow &b) {
                      bool delta = false;
                      if (sorts_specs->Specs->ColumnIndex == 1)
                        delta = a.message < b.message;
                      else if (sorts_specs->Specs->ColumnIndex == 2)
                        delta = a.count < b.count;
                      return sorts_specs->Specs->SortDirection ==
                                     ImGuiSortDirection_Ascending
                                 ? delta
                                 : !delta;
                    });
          sorts_specs->SpecsDirty = false;
        }
      }
      for (const auto &row : errorRows) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.rank);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", row.message.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%llu", row.count);
      }
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }
}

void GuiController::renderErrorDialog() {
  ImGui::OpenPopup("Error");
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Error", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "❌ Analysis Failed");
    ImGui::Spacing();
    ImGui::TextWrapped("%s", errorMessage_.c_str());
    ImGui::Spacing();
    if (ImGui::Button("OK", ImVec2(120, 0))) {
      showError_ = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void GuiController::renderFilePicker() {
  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal("File Picker", &showFilePicker_)) {
    ImGui::Text("Current Directory: %s", currentPickerDir_.string().c_str());

    if (ImGui::Button("⬆️ Up")) {
      currentPickerDir_ = currentPickerDir_.parent_path();
      updateFileList();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      showFilePicker_ = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::Separator();

    ImGui::BeginChild("Files", ImVec2(0, -30), true);

    // Dirs first
    for (const auto &dir : pickerDirs_) {
      if (ImGui::Selectable(("📁 " + dir.filename().string()).c_str())) {
        currentPickerDir_ = dir;
        updateFileList();
      }
    }

    // Files second
    for (const auto &file : pickerFiles_) {
      if (ImGui::Selectable(("📄 " + file.filename().string()).c_str())) {
        std::string pathStr = file.string();
        if (pathStr.length() < sizeof(inputPath_)) {
          std::strcpy(inputPath_, pathStr.c_str());
          showFilePicker_ = false;
          ImGui::CloseCurrentPopup();
        }
      }
    }

    ImGui::EndChild();
    ImGui::EndPopup();
  }
}

void GuiController::updateFileList() {
  pickerDirs_.clear();
  pickerFiles_.clear();

  try {
    for (const auto &entry :
         std::filesystem::directory_iterator(currentPickerDir_)) {
      if (entry.is_directory()) {
        pickerDirs_.push_back(entry.path());
      } else if (entry.is_regular_file()) {
        pickerFiles_.push_back(entry.path());
      }
    }
    // Simple alpha sort
    std::sort(pickerDirs_.begin(), pickerDirs_.end());
    std::sort(pickerFiles_.begin(), pickerFiles_.end());
  } catch (const std::exception &e) {
    std::cerr << "Error listing directory: " << e.what() << std::endl;
  }
}

} // namespace loganalyzer
