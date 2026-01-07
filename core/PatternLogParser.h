#pragma once

#include "ILogParser.h"
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace loganalyzer {

/**
 * @brief A configurable log parser that uses a pattern string to define the log
 * format.
 *
 * Supports tokens:
 * %D - Date (YYYY-MM-DD)
 * %T - Time (HH:MM:SS)
 * %L - Level (INFO, ERROR, etc.)
 * %M - Message (The rest of the log line)
 *
 * Example: "[%D %T] [%L] %M" registers as a regex for lines like "[2023-10-27
 * 10:00:00] [INFO] Hello"
 */
class PatternLogParser : public ILogParser {
public:
  explicit PatternLogParser(const std::string &pattern);

  ParseResult parse(std::string_view line, size_t lineNumber) const override;

  const std::string &getPattern() const { return pattern_; }
  const std::string &getRegexString() const { return regexString_; }

private:
  enum class FieldType { Date, Time, Level, Message };

  void compilePattern(const std::string &pattern);

  std::string pattern_;
  std::string regexString_;
  std::regex regex_;
  std::vector<FieldType> groupMapping_;
};

} // namespace loganalyzer
