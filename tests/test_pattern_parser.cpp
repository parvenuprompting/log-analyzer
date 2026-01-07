#include "../core/PatternLogParser.h"
#include "../external/catch2/catch_amalgamated.hpp"
#include <iostream>
#include <variant>

using namespace loganalyzer;

TEST_CASE("PatternLogParser - Basic Patterns", "[Parser][Pattern]") {
  SECTION("Standard format [%D %T] [%L] %M") {
    PatternLogParser parser("[%D %T] [%L] %M");

    std::string line = "[2023-10-27 10:00:00] [INFO] System started";
    auto result = parser.parse(line, 1);

    REQUIRE(std::holds_alternative<LogEntry>(result));
    auto entry = std::get<LogEntry>(result);
    CHECK(entry.level == LogLevel::INFO);
    CHECK(entry.message == "System started");
  }

  SECTION("Custom format %T - %L - %M") {
    PatternLogParser parser("%T - %L - %M");

    std::string line = "14:30:15.500 - ERROR - Database connection lost";
    auto result = parser.parse(line, 42);

    REQUIRE(std::holds_alternative<LogEntry>(result));
    auto entry = std::get<LogEntry>(result);
    CHECK(entry.level == LogLevel::ERROR);
    CHECK(entry.message == "Database connection lost");
  }
}

TEST_CASE("PatternLogParser - Special Characters", "[Parser][Pattern]") {
  SECTION("Brackets and Dots |%D| %L ... %M") {
    PatternLogParser parser("|%D| %L ... %M");

    std::string line = "|2023-11-01| WARNING ... Disk space low";
    auto result = parser.parse(line, 100);

    REQUIRE(std::holds_alternative<LogEntry>(result));
    auto entry = std::get<LogEntry>(result);
    CHECK(entry.level == LogLevel::WARNING);
    CHECK(entry.message == "Disk space low");
  }
}

TEST_CASE("PatternLogParser - Failure Cases", "[Parser][Pattern]") {
  SECTION("Mismatched pattern") {
    PatternLogParser parser("[%D] %M");
    std::string line = "2023-10-10 Error message without brackets";
    auto result = parser.parse(line, 1);

    CHECK(std::holds_alternative<ParseError>(result));
    auto error = std::get<ParseError>(result);
    CHECK(error.code == ParseErrorCode::BadFormat);
  }
}
