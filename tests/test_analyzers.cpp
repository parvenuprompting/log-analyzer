#include "../analysis/KeywordHitAnalyzer.h"
#include "../analysis/LevelCountAnalyzer.h"
#include "../analysis/TimeRangeFilter.h"
#include "../core/LogEntry.h"
#include "../core/Timestamp.h"
#include <iostream>

using namespace loganalyzer;

#define ASSERT_EQ(expected, actual, msg)                                       \
  if ((expected) != (actual)) {                                                \
    std::cerr << "FAIL: " << msg << " (expected: " << (expected)               \
              << ", got: " << (actual) << ")\n";                               \
    return false;                                                              \
  }

#define ASSERT_TRUE(condition, msg)                                            \
  if (!(condition)) {                                                          \
    std::cerr << "FAIL: " << msg << "\n";                                      \
    return false;                                                              \
  }

bool test_level_count_analyzer() {
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

  ASSERT_EQ(2u, result.levelCounts[LogLevel::ERROR],
            "Should count 2 ERROR entries");
  ASSERT_EQ(1u, result.levelCounts[LogLevel::WARNING],
            "Should count 1 WARNING entry");
  ASSERT_EQ(1u, result.levelCounts[LogLevel::INFO],
            "Should count 1 INFO entry");

  std::cout << "PASS: test_level_count_analyzer\n";
  return true;
}

bool test_keyword_hit_analyzer() {
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

  ASSERT_EQ(2u, result.keywordHits, "Should find 2 keyword hits");

  std::cout << "PASS: test_keyword_hit_analyzer\n";
  return true;
}

bool test_time_range_filter() {
  Timestamp from = {2026, 1, 5, 10, 30, 15};
  Timestamp to = {2026, 1, 5, 10, 30, 20};

  TimeRangeFilter filter(from, to);

  Timestamp t1 = {2026, 1, 5, 10, 30, 10}; // Before range
  Timestamp t2 = {2026, 1, 5, 10, 30, 15}; // At start (inclusive)
  Timestamp t3 = {2026, 1, 5, 10, 30, 17}; // Within range
  Timestamp t4 = {2026, 1, 5, 10, 30, 20}; // At end (inclusive)
  Timestamp t5 = {2026, 1, 5, 10, 30, 25}; // After range

  ASSERT_TRUE(!filter.accept(t1), "Should reject timestamp before range");
  ASSERT_TRUE(filter.accept(t2),
              "Should accept timestamp at start (inclusive)");
  ASSERT_TRUE(filter.accept(t3), "Should accept timestamp within range");
  ASSERT_TRUE(filter.accept(t4), "Should accept timestamp at end (inclusive)");
  ASSERT_TRUE(!filter.accept(t5), "Should reject timestamp after range");

  std::cout << "PASS: test_time_range_filter\n";
  return true;
}

int main() {
  std::cout << "Running analyzer tests...\n\n";

  bool allPassed = true;
  allPassed &= test_level_count_analyzer();
  allPassed &= test_keyword_hit_analyzer();
  allPassed &= test_time_range_filter();

  std::cout << "\n";
  if (allPassed) {
    std::cout << "All analyzer tests passed!\n";
    return 0;
  } else {
    std::cerr << "Some tests failed!\n";
    return 1;
  }
}
