#include "GuiController.h"
#include "../external/imgui/imgui.h"
#include <cstring>

namespace loganalyzer {

GuiController::GuiController()
    : hasResults_(false), showError_(false), shouldClose_(false),
      useTimeFilter_(false), useKeyword_(false) {
  std::memset(inputPath_, 0, sizeof(inputPath_));
  std::memset(fromTimestamp_, 0, sizeof(fromTimestamp_));
  std::memset(toTimestamp_, 0, sizeof(toTimestamp_));
  std::memset(keyword_, 0, sizeof(keyword_));

  // Default example
  std::strcpy(inputPath_, "tests/sample_log.txt");
}

void GuiController::render() {
  ImGui::Begin("Log Analyzer", &shouldClose_,
               ImGuiWindowFlags_AlwaysAutoResize);

  renderFileInput();
  renderFilters();
  renderAnalyzeButton();

  if (hasResults_) {
    renderResults();
  }

  ImGui::End();

  if (showError_) {
    renderErrorDialog();
  }
}

void GuiController::renderFileInput() {
  ImGui::SeparatorText("Input File");
  ImGui::InputText("Path", inputPath_, sizeof(inputPath_));
  ImGui::Spacing();
}

void GuiController::renderFilters() {
  ImGui::SeparatorText("Filters (Optional)");

  ImGui::Checkbox("Time Range", &useTimeFilter_);
  if (useTimeFilter_) {
    ImGui::Indent();
    ImGui::InputText("From", fromTimestamp_, sizeof(fromTimestamp_));
    ImGui::SameLine();
    ImGui::TextDisabled("(YYYY-MM-DD HH:MM:SS)");
    ImGui::InputText("To  ", toTimestamp_, sizeof(toTimestamp_));
    ImGui::SameLine();
    ImGui::TextDisabled("(YYYY-MM-DD HH:MM:SS)");
    ImGui::Unindent();
  }

  ImGui::Checkbox("Keyword Search", &useKeyword_);
  if (useKeyword_) {
    ImGui::Indent();
    ImGui::InputText("Keyword", keyword_, sizeof(keyword_));
    ImGui::SameLine();
    ImGui::TextDisabled("(case-sensitive)");
    ImGui::Unindent();
  }

  ImGui::Spacing();
}

void GuiController::renderAnalyzeButton() {
  if (ImGui::Button("Analyze Log", ImVec2(200, 40))) {
    // Build request
    AppRequest request;
    request.inputPath = std::string(inputPath_);

    if (useTimeFilter_) {
      Timestamp from, to;
      if (Timestamp::parse(std::string(fromTimestamp_), from)) {
        request.fromTimestamp = from;
      }
      if (Timestamp::parse(std::string(toTimestamp_), to)) {
        request.toTimestamp = to;
      }
    }

    if (useKeyword_ && std::strlen(keyword_) > 0) {
      request.keyword = std::string(keyword_);
    }

    // Run analysis
    lastResult_ = app_.run(request);

    if (lastResult_.status == AppStatus::OK) {
      hasResults_ = true;
      showError_ = false;
    } else {
      hasResults_ = false;
      showError_ = true;
      errorMessage_ = lastResult_.message;
    }
  }

  ImGui::Spacing();
}

void GuiController::renderResults() {
  const auto &result = lastResult_.analysisResult;

  ImGui::SeparatorText("Analysis Results");

  // Counters
  if (ImGui::CollapsingHeader("Counters", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Indent();
    ImGui::Text("Total lines:   %llu", result.totalLines);
    ImGui::Text("Parsed lines:  %llu", result.parsedLines);
    ImGui::Text("Invalid lines: %llu", result.invalidLines);
    ImGui::Unindent();
  }

  // Parse Errors
  if (!result.parseErrors.empty() && ImGui::CollapsingHeader("Parse Errors")) {
    ImGui::Indent();
    if (ImGui::BeginTable("parse_errors", 2,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Error Type");
      ImGui::TableSetupColumn("Count");
      ImGui::TableHeadersRow();

      for (const auto &[code, count] : result.parseErrors) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        switch (code) {
        case ParseErrorCode::BadFormat:
          ImGui::Text("BadFormat");
          break;
        case ParseErrorCode::BadTimestamp:
          ImGui::Text("BadTimestamp");
          break;
        case ParseErrorCode::BadLevel:
          ImGui::Text("BadLevel");
          break;
        case ParseErrorCode::MissingMessage:
          ImGui::Text("MissingMessage");
          break;
        }
        ImGui::TableNextColumn();
        ImGui::Text("%llu", count);
      }
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }

  // Level Counts
  if (!result.levelCounts.empty() &&
      ImGui::CollapsingHeader("Level Counts", ImGuiTreeNodeFlags_DefaultOpen)) {
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

  // Keyword Hits
  if (result.keywordHits > 0) {
    if (ImGui::CollapsingHeader("Keyword Hits")) {
      ImGui::Indent();
      ImGui::Text("Matches: %llu", result.keywordHits);
      ImGui::Unindent();
    }
  }

  // Top Errors
  if (!result.topErrors.empty() &&
      ImGui::CollapsingHeader("Top 10 ERROR Messages",
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
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Analysis Failed");
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
