#include "GuiController.h"
#include "../core/ConfigManager.h"
#include "../external/IconsFontAwesome6.h"
#include "../external/imgui/imgui.h"
#include "imgui_stdlib.h"
#include <algorithm>
#include <cstring>
#include <format>
#include <iostream>
#include <vector>

namespace loganalyzer {

GuiController::GuiController()
    : hasResults_(false), showError_(false), showAbout_(false),
      shouldClose_(false), isAnalyzing_(false), analysisProgress_(0.0f),
      cancelRequested_(false), analysisComplete_(false), useTimeFilter_(false),
      useKeyword_(false), showFilePicker_(false), showLogViewer_(false),
      isIndexing_(false), indexingProgress_(0.0f), useCustomParser_(false),
      customPattern_("[%D %T] [%L] %M") {

  // Init picker path to current directory
  currentPickerDir_ = std::filesystem::current_path();

  // Load Config
  ConfigManager::instance().load();
  inputPath_ =
      ConfigManager::instance().getString("inputPath", "tests/sample_log.txt");

  applyModernTheme();
}

GuiController::~GuiController() {
  if (analysisThread_.joinable())
    analysisThread_.join();
  if (indexerThread_.joinable())
    indexerThread_.join();
}

void GuiController::applyModernTheme() {
  // ... [Same theme code] ...
  ImGuiStyle &style = ImGui::GetStyle();

  // Rounding
  style.WindowRounding = 12.0f;
  style.ChildRounding = 12.0f;
  style.FrameRounding = 12.0f;
  style.GrabRounding = 12.0f;
  style.PopupRounding = 12.0f;
  style.ScrollbarRounding = 12.0f;
  style.TabRounding = 12.0f;

  // Spacing & Padding
  style.WindowPadding = ImVec2(20, 20);
  style.FramePadding = ImVec2(10, 8);
  style.ItemSpacing = ImVec2(12, 10);
  style.ItemInnerSpacing = ImVec2(8, 6);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 16.0f;
  style.GrabMinSize = 12.0f;

  // Borders
  style.WindowBorderSize = 1.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f; // Subtle border for inputs

  ImVec4 *colors = style.Colors;

  // Glassmorphism Palette
  ImVec4 glassBg = ImVec4(0.08f, 0.08f, 0.12f, 0.70f); // Semi-transparent dark
  ImVec4 glassBgHover = ImVec4(0.12f, 0.12f, 0.18f, 0.75f);
  ImVec4 glassBorder =
      ImVec4(1.00f, 1.00f, 1.00f, 0.15f); // Subtle white border
  ImVec4 accentColor =
      ImVec4(0.30f, 0.79f, 0.94f, 1.00f); // Soft Cyan (#4cc9f0)
  ImVec4 accentActive = ImVec4(0.45f, 0.04f, 0.72f, 1.00f); // Purple (#7209b7)
  ImVec4 textBase = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  ImVec4 textDim = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

  colors[ImGuiCol_Text] = textBase;
  colors[ImGuiCol_TextDisabled] = textDim;
  colors[ImGuiCol_WindowBg] = glassBg;
  colors[ImGuiCol_ChildBg] =
      ImVec4(0.00f, 0.00f, 0.00f, 0.20f); // Slightly darker for cards
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.95f);
  colors[ImGuiCol_Border] = glassBorder;
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Inputs (Frames)
  colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.79f, 0.94f, 0.20f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.79f, 0.94f, 0.30f);

  // Title Bar
  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.06f, 0.80f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.06f, 0.80f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

  // Menu
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);

  // Scrollbar
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
  colors[ImGuiCol_ScrollbarGrabHovered] = accentColor;
  colors[ImGuiCol_ScrollbarGrabActive] = accentActive;

  // Checkmark / Slider
  colors[ImGuiCol_CheckMark] = accentColor;
  colors[ImGuiCol_SliderGrab] = accentColor;
  colors[ImGuiCol_SliderGrabActive] = accentActive;

  // Buttons
  colors[ImGuiCol_Button] = ImVec4(0.30f, 0.79f, 0.94f, 0.20f); // Glass button
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.79f, 0.94f, 0.40f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.79f, 0.94f, 0.60f);

  // Headers (CollapsingHeader, etc)
  colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.15f);

  // Separator
  colors[ImGuiCol_Separator] = glassBorder;
  colors[ImGuiCol_SeparatorHovered] = accentColor;
  colors[ImGuiCol_SeparatorActive] = accentColor;

  // Resize Grip
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.79f, 0.94f, 0.40f);
  colors[ImGuiCol_TabActive] =
      ImVec4(0.30f, 0.79f, 0.94f, 0.60f); // Active tab is bright
  colors[ImGuiCol_TabUnfocused] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);

  // Plot
  colors[ImGuiCol_PlotLines] = accentColor;
  colors[ImGuiCol_PlotLinesHovered] = textBase;
  colors[ImGuiCol_PlotHistogram] = accentColor;
  colors[ImGuiCol_PlotHistogramHovered] = accentActive;

  // Table
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
  colors[ImGuiCol_TableBorderStrong] = glassBorder;
  colors[ImGuiCol_TableBorderLight] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
  colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);

  // Text Selection
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.79f, 0.94f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = accentColor;
  colors[ImGuiCol_NavHighlight] = accentColor;
  colors[ImGuiCol_NavWindowingHighlight] = textBase;
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

void GuiController::render() {
  if (isAnalyzing_) {
    checkAnalysisComplete();
  }

  ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);

  // Render Menu Bar
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu(ICON_FA_FILE " File")) {
      if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open Log...", "Ctrl+O")) {
        showFilePicker_ = true;
      }
      ImGui::Separator();
      if (ImGui::MenuItem(ICON_FA_XMARK " Exit", "Alt+F4")) {
        shouldClose_ = true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(ICON_FA_GEARS " Tools")) {
      if (ImGui::MenuItem(ICON_FA_TRASH " Clear Results", nullptr, false,
                          hasResults_)) {
        hasResults_ = false;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(ICON_FA_CIRCLE_INFO " Help")) {
      if (ImGui::MenuItem("About Log Analyzer")) {
        showAbout_ = true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Log Analyzer Pro", &shouldClose_);

  // Main scrollable area
  if (ImGui::BeginChild("MainRegion", ImVec2(0, -45), false,
                        ImGuiWindowFlags_None)) {
    if (ImGui::BeginTabBar("MainTabs")) {
      if (ImGui::BeginTabItem(ICON_FA_CHART_BAR " Dashboard")) {
        renderFileInput();
        renderFilters();
        renderAnalyzeButton();

        if (isAnalyzing_) {
          renderProgressBar();
        }

        if (hasResults_) {
          renderResults();
        }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(ICON_FA_FILE_LINES " Log Viewer")) {
        if (!logFile_ || !logFile_->isOpen()) {
          if (ImGui::Button(ICON_FA_FOLDER_OPEN
                            " Open Current File for Viewing")) {
            openLogForViewing(inputPath_);
          }
        }
        renderLogViewer();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::EndChild();
  }

  // Footer / Copyright
  ImGui::Separator();
  ImGui::TextDisabled(ICON_FA_COPYRIGHT
                      " 2026 Tiëndo Welles | Log Analyzer Pro v2.3-Zen");

  ImGui::SameLine(ImGui::GetWindowWidth() - 160);

  // Dynamic Toggle Button
  bool isFull =
      (ImGui::GetWindowWidth() >= ImGui::GetMainViewport()->Size.x - 20.0f);
  const char *btnLabel = isFull ? ICON_FA_COMPRESS : ICON_FA_EXPAND;

  if (ImGui::SmallButton(btnLabel)) {
    if (!isFull) {
      ImGui::SetWindowPos("Log Analyzer Pro", ImGui::GetMainViewport()->Pos);
      ImGui::SetWindowSize("Log Analyzer Pro", ImGui::GetMainViewport()->Size);
    } else {
      ImGui::SetWindowSize("Log Analyzer Pro", ImVec2(1000, 750));
      ImGui::SetWindowPos("Log Analyzer Pro", ImVec2(50, 50));
    }
  }

  // Dialogs (Must be inside the window that calls OpenPopup)
  if (showFilePicker_) {
    if (!ImGui::IsPopupOpen("File Picker")) {
      ImGui::OpenPopup("File Picker");
    }
    renderFilePicker();
  }

  if (showError_) {
    renderErrorDialog();
  }

  if (showAbout_) {
    if (!ImGui::IsPopupOpen("About")) {
      ImGui::OpenPopup("About");
    }
    renderAboutDialog();
  }

  ImGui::End();
}

void GuiController::renderFileInput() {
  ImGui::SeparatorText(ICON_FA_FOLDER " Input File");

  float buttonWidth = 100.0f;
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - 10);
  ImGui::InputTextWithHint("##input", "Path to log file...", &inputPath_);

  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Browse...",
                    ImVec2(buttonWidth, 0))) {
    showFilePicker_ = true;
    updateFileList();
    ImGui::OpenPopup("File Picker");
  }

  ImGui::Spacing();
}

void GuiController::renderFilters() {
  ImGui::SeparatorText(ICON_FA_FILTER " Filters (Optional)");

  ImGui::Checkbox("Time Range", &useTimeFilter_);
  if (useTimeFilter_) {
    ImGui::Indent();
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##from", "YYYY-MM-DD HH:MM:SS", &fromTimestamp_);
    ImGui::SameLine();
    ImGui::TextDisabled("From");

    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##to", "YYYY-MM-DD HH:MM:SS", &toTimestamp_);
    ImGui::SameLine();
    ImGui::TextDisabled("To");
    ImGui::Unindent();
  }

  ImGui::Checkbox("Keyword Search", &useKeyword_);
  if (useKeyword_) {
    ImGui::Indent();
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("##keyword", "Search term (case-sensitive)...",
                             &keyword_);
    ImGui::Unindent();
  }

  ImGui::Checkbox("Configurable Parser", &useCustomParser_);
  if (useCustomParser_) {
    ImGui::Indent();
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("##pattern", "Pattern (e.g. [%D %T] [%L] %M)",
                             &customPattern_);
    ImGui::TextDisabled("Tokens: %%D=Date, %%T=Time, %%L=Level, %%M=Message");
    ImGui::Unindent();
  }

  ImGui::Spacing();
}

void GuiController::renderAnalyzeButton() {
  if (isAnalyzing_) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button(ICON_FA_XMARK " Cancel Analysis", ImVec2(-1, 50))) {
      cancelRequested_ = true;
    }
    ImGui::PopStyleColor();
  } else {
    if (ImGui::Button(ICON_FA_ROCKET " Analyze Log", ImVec2(-1, 50))) {
      startAnalysis();
    }
  }
  ImGui::Spacing();
}

void GuiController::renderProgressBar() {
  float progress = analysisProgress_.load();
  ImGui::SeparatorText(ICON_FA_BOLT " Analysis in Progress");
  std::string overlay = std::format("{:.0f}%", progress * 100.0f);
  ImGui::ProgressBar(progress, ImVec2(-1, 30), overlay.c_str());
  ImGui::Spacing();
}

void GuiController::startAnalysis() {
  currentRequest_.inputPath = inputPath_;

  if (useTimeFilter_) {
    Timestamp from, to;
    if (Timestamp::parse(fromTimestamp_, from)) {
      currentRequest_.fromTimestamp = from;
    }
    if (Timestamp::parse(toTimestamp_, to)) {
      currentRequest_.toTimestamp = to;
    }
  }

  if (useKeyword_ && !keyword_.empty()) {
    currentRequest_.keyword = keyword_;
  }

  if (useCustomParser_ && !customPattern_.empty()) {
    currentRequest_.customPattern = customPattern_;
  } else {
    currentRequest_.customPattern = "";
  }

  // Save Config
  ConfigManager::instance().setString("inputPath", inputPath_);
  ConfigManager::instance().save();

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
  const auto &result = lastResult_.analysisResult;

  ImGui::SeparatorText(ICON_FA_SQUARE_POLL_VERTICAL " Analysis Results");

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
      ImGui::CollapsingHeader(ICON_FA_CHART_PIE " Level Counts",
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

      ImU32 colBase = colors[i];
      ImU32 colTop = colors[i] & 0x00FFFFFF |
                     0x99000000; // More transparent at top? Or lighter?
      // Actually standard ImGui colors are ABGR or RGBA? using IM_COL32 which
      // is usually R,G,B,A. Let's make top slightly transparent for glass
      // effect

      ImVec4 baseC = ImGui::ColorConvertU32ToFloat4(colors[i]);
      ImVec4 topC = baseC;
      topC.w = 0.6f; // Top is more transparent

      draw_list->AddRectFilledMultiColor(
          ImVec2(x + 5, y - barHeight - 20), ImVec2(x + barWidth, y - 20),
          ImGui::ColorConvertFloat4ToU32(topC),  // Upper Left
          ImGui::ColorConvertFloat4ToU32(topC),  // Upper Right
          ImGui::ColorConvertFloat4ToU32(baseC), // Bottom Right
          ImGui::ColorConvertFloat4ToU32(baseC)  // Bottom Left
      );

      draw_list->AddText(ImVec2(x + 5, y - 15), IM_COL32_WHITE, labels[i]);

      std::string valStr = std::format("{:.0f}", values[i]);
      draw_list->AddText(ImVec2(x + 5, y - barHeight - 35), IM_COL32_WHITE,
                         valStr.c_str());
    }
    ImGui::Dummy(ImVec2(width, height));
    ImGui::Unindent();
  }

  if (!result.topErrors.empty() &&
      ImGui::CollapsingHeader(ICON_FA_FIRE " Top 10 ERROR Messages",
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

  // Render Timeline
  renderTimeline();

  // Render Heatmap
  renderHeatmap();
}

void GuiController::renderTimeline() {
  std::lock_guard<std::mutex> lock(resultMutex_);
  if (!hasResults_)
    return;

  const auto &timeline = lastResult_.analysisResult.timeline;

  if (timeline.empty())
    return;

  if (ImGui::CollapsingHeader(ICON_FA_CHART_LINE " Event Timeline",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 150.0f;

    // Background
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
                             IM_COL32(20, 20, 30, 100), 8.0f);
    draw_list->AddRect(p, ImVec2(p.x + width, p.y + height),
                       IM_COL32(255, 255, 255, 30), 8.0f);

    // Find min/max time and max counts
    uint64_t minTime = timeline.front().timestamp;
    uint64_t maxTime = timeline.back().timestamp;

    // Safety: ensure span > 0
    if (maxTime <= minTime)
      maxTime = minTime + 1;

    uint32_t maxCount = 0;
    for (const auto &b : timeline) {
      maxCount = std::max(maxCount, b.errorCount + b.warningCount);
    }
    if (maxCount == 0)
      maxCount = 1;

    // Margin
    float marginX = 10.0f;
    float marginY = 10.0f;
    float plotW = width - 2 * marginX;
    float plotH = height - 2 * marginY;

    // Plot Bars
    // Since we might have thousands of minutes, we need to bin them visually if
    // width < buckets For simplicity, we just draw lines.

    for (const auto &b : timeline) {
      // Normalize Time 0..1
      // Note: timeKey is YYYYMMDDHHMM. This is monotonic but NOT linear (gaps
      // between hour/day). For true linearity, we'd need parse to Unix. But for
      // simple visualization, let's treat it as pseudo-linear or just
      // index-based if sorted? "timeline" vector is sorted by timeKey (if we
      // merged correctly? pipeline insert makes it semi-sorted but chunk
      // artifacts exist) Let's assume sorted for now.

      // Actually, we didn't sort timeline in merge. Let's do a sort here if
      // needed, or rely on visual density. Better: Just iterate.

      // X position based on value relative to min/max key?
      // 202601051030 vs 202601051031 is +1.
      // 202601051059 vs 202601051100 is +41.
      // This distorts x-axis.
      // Fallback: Use index in the vector if we assume it's somewhat
      // contiguous? Or better: Revert to 0..N indices for "Minutes Available".
      // Since we don't have unix conversion handy in this scope without
      // helpers.

      // REVISION: Draw indices.
      /*
         If we have gaps, they won't show. That's acceptable for "Event
         Timeline".
      */

      // Linear Index Approximation
    }

    // Improved Approach: Just loop through indices 0..N
    size_t count = timeline.size();
    for (size_t i = 0; i < count; ++i) {
      const auto &b = timeline[i];
      float x = p.x + marginX + (float)i / (float)(count - 1) * plotW;

      float errH = (float)b.errorCount / (float)maxCount * plotH;
      float warnH = (float)b.warningCount / (float)maxCount * plotH;

      // Draw Warning first (bottom)
      if (b.warningCount > 0) {
        draw_list->AddLine(ImVec2(x, p.y + marginY + plotH),
                           ImVec2(x, p.y + marginY + plotH - warnH),
                           IM_COL32(255, 204, 51, 200), 2.0f);
      }
      // Draw Error stacks on top? Or overlay?
      // Let's overlay red.
      if (b.errorCount > 0) {
        float baseY = p.y + marginY + plotH - warnH;
        draw_list->AddLine(ImVec2(x, baseY), ImVec2(x, baseY - errH),
                           IM_COL32(255, 76, 76, 200), 2.0f);
      }

      // Interaction: Click to jump?
      // Need log offsets in timeline bucket to support this. We currently don't
      // store them. Feature for Phase 2.

      // Tooltip
      if (ImGui::IsMouseHoveringRect(ImVec2(x - 2, p.y),
                                     ImVec2(x + 2, p.y + height))) {
        ImGui::BeginTooltip();
        ImGui::Text("Time: %llu", b.timestamp);
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Errors: %u",
                           b.errorCount);
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Warnings: %u",
                           b.warningCount);
        ImGui::EndTooltip();
      }
    }

    ImGui::Dummy(ImVec2(width, height));
    ImGui::Unindent();
  }
}

void GuiController::renderHeatmap() {
  std::lock_guard<std::mutex> lock(resultMutex_);
  if (!hasResults_)
    return;
  const auto &heatmap = lastResult_.analysisResult.heatmap;

  if (ImGui::CollapsingHeader(ICON_FA_TABLE " Activity Heatmap",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();

    ImDrawList *draw = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    // Grid: 7 Rows (Sun-Sat), 24 Cols
    float cellSize = 25.0f;
    // Adjust if too wide
    if (cellSize * 26 > availW)
      cellSize = availW / 26;

    const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    // Find max for normalization
    uint32_t maxVal = 0;
    for (const auto &row : heatmap)
      for (uint32_t val : row)
        maxVal = std::max(maxVal, val);
    if (maxVal == 0)
      maxVal = 1;

    for (int d = 0; d < 7; ++d) {
      // Label
      draw->AddText(ImVec2(p.x, p.y + d * (cellSize + 2) + 5), IM_COL32_WHITE,
                    days[d]);

      for (int h = 0; h < 24; ++h) {
        float val = (float)heatmap[d][h] / (float)maxVal;

        // Color Gradient: Transparent -> Blue -> Purple -> Red
        ImU32 col = IM_COL32(30, 30, 35, 255); // Empty
        if (heatmap[d][h] > 0) {
          if (val < 0.5f) {
            // Blue to Cyan
            int c = (int)(val * 2 * 255);
            col = IM_COL32(0, c, 255, 200);
          } else {
            // Cyan to Red
            int c = (int)((val - 0.5f) * 2 * 255);
            col = IM_COL32(c, 255 - c, 255 - c, 200);
          }
        }

        float x = p.x + 40 + h * (cellSize + 2);
        float y = p.y + d * (cellSize + 2);

        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + cellSize, y + cellSize),
                            col, 4.0f);

        // Hover
        if (ImGui::IsMouseHoveringRect(ImVec2(x, y),
                                       ImVec2(x + cellSize, y + cellSize))) {
          ImGui::BeginTooltip();
          ImGui::Text("%s %02d:00 - %02d:59", days[d], h, h);
          ImGui::Text("Logs: %u", heatmap[d][h]);
          ImGui::EndTooltip();

          // Highlight
          draw->AddRect(ImVec2(x, y), ImVec2(x + cellSize, y + cellSize),
                        IM_COL32_WHITE, 4.0f);
        }
      }
    }

    ImGui::Dummy(ImVec2(availW, 7 * (cellSize + 2) + 20));
    ImGui::Unindent();
  }
}

void GuiController::renderErrorDialog() {
  ImGui::OpenPopup("Error");
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("Error", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                       ICON_FA_CIRCLE_EXCLAMATION " Analysis Failed");
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

    if (ImGui::Button(ICON_FA_ARROW_UP " Up")) {
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
      if (ImGui::Selectable(
              (std::string(ICON_FA_FOLDER) + " " + dir.filename().string())
                  .c_str())) {
        currentPickerDir_ = dir;
        updateFileList();
      }
    }

    // Files second
    for (const auto &file : pickerFiles_) {
      if (ImGui::Selectable(
              (std::string(ICON_FA_FILE) + " " + file.filename().string())
                  .c_str())) {
        std::string pathStr = file.string();
        inputPath_ = pathStr;
        showFilePicker_ = false;
        ImGui::CloseCurrentPopup();
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
    std::sort(pickerDirs_.begin(), pickerDirs_.end());
    std::sort(pickerFiles_.begin(), pickerFiles_.end());
  } catch (const std::exception &e) {
    std::cerr << "Error listing directory: " << e.what() << std::endl;
  }
}

void GuiController::renderAboutDialog() {
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(400, 320));

  if (ImGui::BeginPopupModal("About", &showAbout_,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    // Header
    ImGui::TextDisabled(ICON_FA_CIRCLE_INFO " About Log Analyzer");
    ImGui::Separator();
    ImGui::Spacing();

    // App Info
    ImGui::Text("Log Analyzer Pro");
    ImGui::TextDisabled("Version 2.3-Zen");
    ImGui::TextDisabled("Build: 2026.1 (Release)");
    ImGui::Spacing();

    // Description
    ImGui::TextWrapped(
        "A high-performance, concurrent log analysis tool designed mainly for "
        "immense server logs, up to 10GB+ in size.");
    ImGui::Spacing();

    // Credits
    ImGui::Text("Built with C++, ImGui, and Love.");
    ImGui::TextDisabled("Creator: Tiëndo Welles");
    ImGui::TextDisabled("License: Proprietary");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Close Button
    float width = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Close", ImVec2(width, 0))) {
      showAbout_ = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

} // namespace loganalyzer
