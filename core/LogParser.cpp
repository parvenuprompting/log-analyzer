#include "LogParser.h"

namespace loganalyzer {

// Constants for robust parsing
namespace {
constexpr size_t MIN_LINE_LENGTH = 27;   // [YYYY-MM-DD HH:MM:SS] [L] m
constexpr size_t TIMESTAMP_LENGTH = 19;  // YYYY-MM-DD HH:MM:SS
constexpr size_t TIMESTAMP_MIN_END = 20; // Length of [ + TIMESTAMP + ]
} // namespace

ParseResult LogParser::parse(std::string_view sv, size_t lineNumber) {
  // Expected format: [YYYY-MM-DD HH:MM:SS] [LEVEL] message

  // Minimum length check
  if (sv.length() < MIN_LINE_LENGTH) {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Check opening bracket for timestamp
  if (sv[0] != '[') {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Find closing bracket for timestamp (more robust than fixed position)
  size_t tsEnd = sv.find(']', 1);
  if (tsEnd == std::string_view::npos || tsEnd < TIMESTAMP_MIN_END) {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Extract timestamp (zero-copy with string_view)
  std::string_view tsView = sv.substr(1, tsEnd - 1);
  if (tsView.length() != TIMESTAMP_LENGTH) {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Parse timestamp
  Timestamp ts;
  if (!Timestamp::parse(tsView, ts)) {
    return ParseError{ParseErrorCode::BadTimestamp, std::string(sv),
                      lineNumber};
  }

  // Check for space and opening bracket for level
  if (tsEnd + 2 >= sv.length() || sv[tsEnd + 1] != ' ' ||
      sv[tsEnd + 2] != '[') {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Find closing bracket for level
  size_t levelStart = tsEnd + 3;
  size_t levelEnd = sv.find(']', levelStart);
  if (levelEnd == std::string_view::npos) {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Extract and parse level (zero-copy)
  std::string_view levelView = sv.substr(levelStart, levelEnd - levelStart);
  LogLevel level;
  if (!parseLogLevel(levelView, level)) {
    return ParseError{ParseErrorCode::BadLevel, std::string(sv), lineNumber};
  }

  // Check for space after level bracket
  if (levelEnd + 1 >= sv.length()) {
    return ParseError{ParseErrorCode::MissingMessage, std::string(sv),
                      lineNumber};
  }

  if (sv[levelEnd + 1] != ' ') {
    return ParseError{ParseErrorCode::BadFormat, std::string(sv), lineNumber};
  }

  // Extract message (zero-copy)
  size_t messageStart = levelEnd + 2;
  std::string_view message;
  if (messageStart < sv.length()) {
    message = sv.substr(messageStart);
  }

  return LogEntry{ts, level, sv, message};
}

bool LogParser::parseLogLevel(std::string_view str, LogLevel &out) {
  if (str == "ERROR") {
    out = LogLevel::ERROR;
    return true;
  } else if (str == "WARNING") {
    out = LogLevel::WARNING;
    return true;
  } else if (str == "INFO") {
    out = LogLevel::INFO;
    return true;
  }
  return false;
}

} // namespace loganalyzer
