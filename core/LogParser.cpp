#include "LogParser.h"

namespace loganalyzer {

ParseResult LogParser::parse(const std::string &line, size_t lineNumber) {
  // Expected format: [YYYY-MM-DD HH:MM:SS] [LEVEL] message
  // Minimum length check: [19 chars] [3+ chars] 1+ chars = 27+

  if (line.length() < 27) {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Check opening bracket
  if (line[0] != '[') {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Check closing bracket for timestamp at position 20
  if (line.length() < 21 || line[20] != ']') {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Extract and parse timestamp
  std::string tsStr = line.substr(1, 19);
  Timestamp ts;
  if (!Timestamp::parse(tsStr, ts)) {
    return ParseError{ParseErrorCode::BadTimestamp, line, lineNumber};
  }

  // Check for space and opening bracket for level
  if (line[21] != ' ' || line[22] != '[') {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Find closing bracket for level
  size_t levelEnd = line.find(']', 23);
  if (levelEnd == std::string::npos) {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Extract and parse level
  std::string levelStr = line.substr(23, levelEnd - 23);
  LogLevel level;
  if (!parseLogLevel(levelStr, level)) {
    return ParseError{ParseErrorCode::BadLevel, line, lineNumber};
  }

  // Check for space after level bracket
  if (levelEnd + 1 >= line.length()) {
    // No space after level bracket = missing message
    return ParseError{ParseErrorCode::MissingMessage, line, lineNumber};
  }

  if (line[levelEnd + 1] != ' ') {
    return ParseError{ParseErrorCode::BadFormat, line, lineNumber};
  }

  // Extract message (everything after space)
  size_t messageStart = levelEnd + 2;
  std::string message;
  if (messageStart < line.length()) {
    message = line.substr(messageStart);
  }
  // Empty message is valid (message string remains empty)

  return LogEntry{ts, level, message};
}

bool LogParser::parseLogLevel(const std::string &str, LogLevel &out) {
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
