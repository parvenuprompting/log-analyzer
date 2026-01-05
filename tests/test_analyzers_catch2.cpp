#include "../analysis/KeywordHitAnalyzer.h"
#include "../analysis/LevelCountAnalyzer.h"
#include "../analysis/TimeRangeFilter.h"
#include "../analysis/TopErrorAnalyzer.h"
#include "../core/LogEntry.h"
#include "../external/catch2/catch_amalgamated.hpp"

using namespace loganalyzer;

TEST_CASE("LevelCountAnalyzer counts entries correctly", "[analyzer]") {
  LevelCountAnalyzer analyzer;
  AnalysisResult result;

  LogEntry e1 = {{2026, 1, 5, 10, 30, 15}, LogLevel::ERROR, "Test error"};
  LogEntry e2 = {{2026, 1, 5, 10, 30, 16}, LogLevel::ERROR, "Another error"};
  LogEntry e3 = {{2026, 1, 5, 10, 30, 17}, LogLevel::WARNING, "Test warning"};
  LogEntry e4 = {{2026, 1, 5, 10, 30, 18}, LogLevel::INFO, "Test info"};

  analyzer.process(e1);
  analyzer.process(e2);
  analyzer.process(e3);
  analyzer.process(e4);
  analyzer.finalize(result);

  CHECK(result.levelCounts[LogLevel::ERROR] == 2);
  CHECK(result.levelCounts[LogLevel::WARNING] == 1);
  CHECK(result.levelCounts[LogLevel::INFO] == 1);
}

TEST_CASE("KeywordHitAnalyzer counts keyword hits", "[analyzer]") {
  KeywordHitAnalyzer analyzer("database");
  AnalysisResult result;

  LogEntry e1 = {
      {2026, 1, 5, 10, 30, 15}, LogLevel::ERROR, "database connection failed"};
  LogEntry e2 = {{2026, 1, 5, 10, 30, 16}, LogLevel::ERROR, "network timeout"};
  LogEntry e3 = {
      {2026, 1, 5, 10, 30, 17}, LogLevel::WARNING, "The database is slow"};

  analyzer.process(e1);
  analyzer.process(e2);
  analyzer.process(e3);
  analyzer.finalize(result);

  CHECK(result.keywordHits == 2);
}

TEST_CASE("TimeRangeFilter accepts timestamps correctly", "[filter]") {
  Timestamp from = {2026, 1, 5, 10, 30, 15};
  Timestamp to = {2026, 1, 5, 10, 30, 20};
  TimeRangeFilter filter(from, to);

  SECTION("Before range is rejected") {
    Timestamp t = {2026, 1, 5, 10, 30, 10};
    CHECK_FALSE(filter.accept(t));
  }

  SECTION("At start (inclusive) is accepted") {
    Timestamp t = {2026, 1, 5, 10, 30, 15};
    CHECK(filter.accept(t));
  }

  SECTION("Within range is accepted") {
    Timestamp t = {2026, 1, 5, 10, 30, 17};
    CHECK(filter.accept(t));
  }

  SECTION("At end (inclusive) is accepted") {
    Timestamp t = {2026, 1, 5, 10, 30, 20};
    CHECK(filter.accept(t));
  }

  SECTION("After range is rejected") {
    Timestamp t = {2026, 1, 5, 10, 30, 25};
    CHECK_FALSE(filter.accept(t));
  }
}

TEST_CASE("TopErrorAnalyzer deterministic sorting", "[analyzer][determinism]") {
  TopErrorAnalyzer analyzer;
  AnalysisResult result;

  // Create entries with same count to test tie-breaking
  LogEntry e1 = {{2026, 1, 5, 10, 30, 15}, LogLevel::ERROR, "zebra error"};
  LogEntry e2 = {{2026, 1, 5, 10, 30, 16}, LogLevel::ERROR, "alpha error"};
  LogEntry e3 = {{2026, 1, 5, 10, 30, 17}, LogLevel::ERROR, "zebra error"};
  LogEntry e4 = {{2026, 1, 5, 10, 30, 18}, LogLevel::ERROR, "alpha error"};

  analyzer.process(e1);
  analyzer.process(e2);
  analyzer.process(e3);
  analyzer.process(e4);
  analyzer.finalize(result);

  REQUIRE(result.topErrors.size() == 2);

  // Both have count 2, should be sorted alphabetically
  CHECK(result.topErrors[0].first == "alpha error");
  CHECK(result.topErrors[0].second == 2);
  CHECK(result.topErrors[1].first == "zebra error");
  CHECK(result.topErrors[1].second == 2);
}

TEST_CASE("TopErrorAnalyzer bounded map strategy", "[analyzer][determinism]") {
  TopErrorAnalyzer analyzer;
  AnalysisResult result;

  SECTION("Limits to top 10") {
    // Add 15 different error messages
    for (int i = 0; i < 15; i++) {
      std::string msg = "error_" + std::to_string(i);
      // Create entries with varying counts (inverse: later = more frequent)
      for (int j = 0; j <= (14 - i); j++) {
        LogEntry e = {{2026, 1, 5, 10, 30, 15}, LogLevel::ERROR, msg};
        analyzer.process(e);
      }
    }

    analyzer.finalize(result);

    // Should only have top 10
    CHECK(result.topErrors.size() == 10);

    // First should have highest count (15)
    CHECK(result.topErrors[0].second == 15);

    // Last should have count 6
    CHECK(result.topErrors[9].second == 6);
  }
}
