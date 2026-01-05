#include "GuiController.h"
#include "../external/imgui/imgui.h"
#include <cstring>

namespace loganalyzer {

GuiController::GuiController()
    : hasResults_(false), showError_(false), shouldClose_(false),
      useTimeFilter_(false), useKeyword_(false), isAnalyzing_(false),
      analysisProgress_(0.0f), cancelRequested_(false),
      analysisComplete_(false) {
  std::memset(inputPath_, 0, sizeof(inputPath_));
  std::memset(fromTimestamp_, 0, sizeof(fromTimestamp_));
  std::memset(toTimestamp_, 0, sizeof(toTimestamp_));
  std::memset(keyword_, 0, sizeof(keyword_));

  // Default example
  std::strcpy(inputPath_, "tests/sample_log.txt");

  applyModernTheme();
}

void GuiController::applyModernTheme() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // Modern dark theme with vibrant accents
  colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] =
      ImVec4(0.10f, 0.10f, 0.18f, 1.00f); // Deep blue-black
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.14f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.24f, 0.27f, 0.38f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.13f, 0.24f, 1.00f); // Dark blue
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
  colors[ImGuiCol_CheckMark] =
      ImVec4(0.91f, 0.27f, 0.38f, 1.00f); // Coral accent
  colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.27f, 0.38f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.91f, 0.27f, 0.38f, 0.80f); // Coral button
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

  // Rounding for modern look
  style.WindowRounding = 8.0f;
  style.ChildRounding = 6.0f;
  style.FrameRounding = 6.0f;
  style.PopupRounding = 6.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 6.0f;
  style.TabRounding = 6.0f;

  // Padding and spacing
  style.WindowPadding = ImVec2(12, 12);
  style.FramePadding = ImVec2(8, 6);
  style.ItemSpacing = ImVec2(12, 8);
  style.ItemInnerSpacing = ImVec2(8, 6);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;
}

void GuiController::render() {
  // Check if analysis completed
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

  if (showError_) {
    renderErrorDialog();
  }
}

void GuiController::renderFileInput() {
  ImGui::SeparatorText("📁 Input File");
  ImGui::SetNextItemWidth(-1);
  ImGui::InputTextWithHint("##input", "Path to log file...", inputPath_,
                           sizeof(inputPath_));
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
    // Show cancel button during analysis
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button("❌ Cancel Analysis", ImVec2(-1, 50))) {
      cancelRequested_ = true;
    }
    ImGui::PopStyleColor();
  } else {
    // Show analyze button
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
  // Build request
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

  // Reset state
  isAnalyzing_ = true;
  analysisProgress_ = 0.0f;
  cancelRequested_ = false;
  analysisComplete_ = false;
  hasResults_ = false;

  // Start analysis in background thread
  analysisThread_ = std::thread([this]() {
    auto callback = [this](float progress) -> bool {
      analysisProgress_ = progress;
      return !cancelRequested_.load();
    };

    lastResult_ = app_.run(currentRequest_, callback);
    analysisComplete_ = true;
  });
}

void GuiController::checkAnalysisComplete() {
  if (analysisComplete_.load()) {
    // Join thread
    if (analysisThread_.joinable()) {
      analysisThread_.join();
    }

    isAnalyzing_ = false;

    if (lastResult_.wasCancelled) {
      // Analysis was cancelled
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

  ImGui::SeparatorText("📊 Analysis Results");

  // Counters in cards
  if (ImGui::BeginTable("stats_cards", 3, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::BeginChild("card1", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Total Lines");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    ImGui::PushFont(
        ImGui::GetIO().Fonts->Fonts[0]); // Would use larger font if loaded
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

  // [Rest of renderResults remains same as before...]
  // Level Counts
  if (!result.levelCounts.empty() &&
      ImGui::CollapsingHeader("📈 Level Counts",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();
    if (ImGui::BeginTable("level_counts", 2,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Level");
      ImGui::TableSetupColumn("Count");
      ImGui::TableHeadersRow();

      for (const auto &[level, count] : result.levelCounts) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        switch (level) {
        case LogLevel::ERROR:
          ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ERROR");
          break;
        case LogLevel::WARNING:
          ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "WARNING");
          break;
        case LogLevel::INFO:
          ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "INFO");
          break;
        }
        ImGui::TableNextColumn();
        ImGui::Text("%llu", count);
      }
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }

  // Top Errors
  if (!result.topErrors.empty() &&
      ImGui::CollapsingHeader("🔥 Top 10 ERROR Messages",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();
    if (ImGui::BeginTable("top_errors", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
      ImGui::TableSetupColumn("Message");
      ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableHeadersRow();

      int rank = 1;
      for (const auto &[message, count] : result.topErrors) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", rank++);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", message.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%llu", count);
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

} // namespace loganalyzer
