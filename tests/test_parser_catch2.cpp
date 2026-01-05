#include "../core/LogParser.h"
#include "../core/Timestamp.h"
#include "../external/catch2/catch_amalgamated.hpp"

using namespace loganalyzer;

TEST_CASE("LogParser validates valid log entries", "[parser]") {
  std::string line = "[2026-01-05 10:30:15] [ERROR] Database connection failed";
  ParseResult result = LogParser::parse(line, 1);

  REQUIRE(std::holds_alternative<LogEntry>(result));

  const LogEntry &entry = std::get<LogEntry>(result);
  CHECK(entry.ts.year == 2026);
  CHECK(entry.ts.month == 1);
  CHECK(entry.ts.day == 5);
  CHECK(entry.ts.hour == 10);
  CHECK(entry.ts.minute == 30);
  CHECK(entry.ts.second == 15);
  CHECK(entry.level == LogLevel::ERROR);
  CHECK(entry.message == "Database connection failed");
}

TEST_CASE("LogParser detects bad format", "[parser]") {
  std::string line = "Not a valid log line";
  ParseResult result = LogParser::parse(line, 1);

  REQUIRE(std::holds_alternative<ParseError>(result));
  const ParseError &error = std::get<ParseError>(result);
  CHECK(error.code == ParseErrorCode::BadFormat);
}

TEST_CASE("LogParser detects bad timestamp", "[parser]") {
  std::string line = "[2026-99-99 10:30:15] [ERROR] Test";
  ParseResult result = LogParser::parse(line, 1);

  REQUIRE(std::holds_alternative<ParseError>(result));
  const ParseError &error = std::get<ParseError>(result);
  CHECK(error.code == ParseErrorCode::BadTimestamp);
}

TEST_CASE("LogParser detects bad level", "[parser]") {
  std::string line = "[2026-01-05 10:30:15] [UNKNOWN] Test";
  ParseResult result = LogParser::parse(line, 1);

  REQUIRE(std::holds_alternative<ParseError>(result));
  const ParseError &error = std::get<ParseError>(result);
  CHECK(error.code == ParseErrorCode::BadLevel);
}

TEST_CASE("LogParser detects missing message", "[parser]") {
  std::string line = "[2026-01-05 10:30:15] [ERROR]";
  ParseResult result = LogParser::parse(line, 1);

  REQUIRE(std::holds_alternative<ParseError>(result));
  const ParseError &error = std::get<ParseError>(result);
  CHECK(error.code == ParseErrorCode::MissingMessage);
}

TEST_CASE("Timestamp comparison operators work correctly", "[timestamp]") {
  Timestamp t1 = {2026, 1, 5, 10, 30, 15};
  Timestamp t2 = {2026, 1, 5, 10, 30, 20};
  Timestamp t3 = {2026, 1, 5, 10, 30, 15};

  CHECK(t1 < t2);
  CHECK(t2 > t1);
  CHECK(t1 == t3);
  CHECK(t1 <= t3);
  CHECK(t2 >= t1);
}

TEST_CASE("Timestamp validation rejects invalid dates", "[timestamp]") {
  Timestamp ts;

  SECTION("31 February is rejected") {
    CHECK_FALSE(Timestamp::parse("2026-02-31 10:30:15", ts));
  }

  SECTION("30 February is rejected") {
    CHECK_FALSE(Timestamp::parse("2026-02-30 10:30:15", ts));
  }

  SECTION("29 February in non-leap year is rejected") {
    CHECK_FALSE(Timestamp::parse("2023-02-29 10:30:15", ts));
  }

  SECTION("29 February in leap year is accepted") {
    CHECK(Timestamp::parse("2024-02-29 10:30:15", ts));
    CHECK(ts.year == 2024);
    CHECK(ts.month == 2);
    CHECK(ts.day == 29);
  }

  SECTION("31 April is rejected") {
    CHECK_FALSE(Timestamp::parse("2026-04-31 10:30:15", ts));
  }
}

TEST_CASE("LogParser handles edge cases", "[parser][edge]") {
  SECTION("Empty line is rejected") {
    ParseResult res = LogParser::parse("", 1);
    REQUIRE(std::holds_alternative<ParseError>(res));
    CHECK(std::get<ParseError>(res).code == ParseErrorCode::BadFormat);
  }

  SECTION("Line too short is rejected") {
    ParseResult res = LogParser::parse("[2026]", 2);
    REQUIRE(std::holds_alternative<ParseError>(res));
    CHECK(std::get<ParseError>(res).code == ParseErrorCode::BadFormat);
  }

  SECTION("Missing closing brackets") {
    // Missing timestamp bracket
    ParseResult res1 = LogParser::parse("[2026-01-01 10:00:00 [INFO] Msg", 3);
    REQUIRE(std::holds_alternative<ParseError>(res1));

    // Missing level bracket
    ParseResult res2 = LogParser::parse("[2026-01-01 10:00:00] [INFO Msg", 4);
    REQUIRE(std::holds_alternative<ParseError>(res2));
  }
}
