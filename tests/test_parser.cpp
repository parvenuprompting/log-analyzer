#include "../core/LogParser.h"
#include "../core/Timestamp.h"
#include <cstring>
#include <iostream>

using namespace loganalyzer;

// Simple test harness
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

bool test_valid_log_entry() {
  std::string line = "[2026-01-05 10:30:15] [ERROR] Database connection failed";
  ParseResult result = LogParser::parse(line, 1);

  ASSERT_TRUE(std::holds_alternative<LogEntry>(result),
              "Should parse valid entry");

  const LogEntry &entry = std::get<LogEntry>(result);
  ASSERT_EQ(2026, entry.ts.year, "Year should match");
  ASSERT_EQ(1, entry.ts.month, "Month should match");
  ASSERT_EQ(5, entry.ts.day, "Day should match");
  ASSERT_EQ(10, entry.ts.hour, "Hour should match");
  ASSERT_EQ(30, entry.ts.minute, "Minute should match");
  ASSERT_EQ(15, entry.ts.second, "Second should match");
  ASSERT_TRUE(entry.level == LogLevel::ERROR, "Level should be ERROR");
  ASSERT_TRUE(entry.message == "Database connection failed",
              "Message should match");

  std::cout << "PASS: test_valid_log_entry\n";
  return true;
}

bool test_bad_format() {
  std::string line = "Not a valid log line";
  ParseResult result = LogParser::parse(line, 1);

  ASSERT_TRUE(std::holds_alternative<ParseError>(result),
              "Should return ParseError");

  const ParseError &error = std::get<ParseError>(result);
  ASSERT_TRUE(error.code == ParseErrorCode::BadFormat,
              "Should be BadFormat error");

  std::cout << "PASS: test_bad_format\n";
  return true;
}

bool test_bad_timestamp() {
  std::string line = "[2026-99-99 10:30:15] [ERROR] Test"; // Invalid month/day
  ParseResult result = LogParser::parse(line, 1);

  ASSERT_TRUE(std::holds_alternative<ParseError>(result),
              "Should return ParseError");

  const ParseError &error = std::get<ParseError>(result);
  ASSERT_TRUE(error.code == ParseErrorCode::BadTimestamp,
              "Should be BadTimestamp error");

  std::cout << "PASS: test_bad_timestamp\n";
  return true;
}

bool test_bad_level() {
  std::string line = "[2026-01-05 10:30:15] [UNKNOWN] Test";
  ParseResult result = LogParser::parse(line, 1);

  ASSERT_TRUE(std::holds_alternative<ParseError>(result),
              "Should return ParseError");

  const ParseError &error = std::get<ParseError>(result);
  ASSERT_TRUE(error.code == ParseErrorCode::BadLevel,
              "Should be BadLevel error");

  std::cout << "PASS: test_bad_level\n";
  return true;
}

bool test_missing_message() {
  std::string line = "[2026-01-05 10:30:15] [ERROR]";
  ParseResult result = LogParser::parse(line, 1);

  ASSERT_TRUE(std::holds_alternative<ParseError>(result),
              "Should return ParseError");

  const ParseError &error = std::get<ParseError>(result);
  ASSERT_TRUE(error.code == ParseErrorCode::MissingMessage,
              "Should be MissingMessage error");

  std::cout << "PASS: test_missing_message\n";
  return true;
}

bool test_timestamp_comparison() {
  Timestamp t1 = {2026, 1, 5, 10, 30, 15};
  Timestamp t2 = {2026, 1, 5, 10, 30, 20};
  Timestamp t3 = {2026, 1, 5, 10, 30, 15};

  ASSERT_TRUE(t1 < t2, "t1 should be less than t2");
  ASSERT_TRUE(t2 > t1, "t2 should be greater than t1");
  ASSERT_TRUE(t1 == t3, "t1 should equal t3");
  ASSERT_TRUE(t1 <= t3, "t1 should be less than or equal to t3");

  std::cout << "PASS: test_timestamp_comparison\n";
  return true;
}

int main() {
  std::cout << "Running parser tests...\n\n";

  bool allPassed = true;
  allPassed &= test_valid_log_entry();
  allPassed &= test_bad_format();
  allPassed &= test_bad_timestamp();
  allPassed &= test_bad_level();
  allPassed &= test_missing_message();
  allPassed &= test_timestamp_comparison();

  std::cout << "\n";
  if (allPassed) {
    std::cout << "All parser tests passed!\n";
    return 0;
  } else {
    std::cerr << "Some tests failed!\n";
    return 1;
  }
}
