#include "PatternLogParser.h"
#include <iostream>
#include <sstream>

namespace loganalyzer {

PatternLogParser::PatternLogParser(const std::string &pattern)
    : pattern_(pattern) {
  compilePattern(pattern);
}

void PatternLogParser::compilePattern(const std::string &pattern) {
  std::string rx;
  rx.reserve(pattern.size() * 2);

  groupMapping_.clear();

  for (size_t i = 0; i < pattern.size(); ++i) {
    if (pattern[i] == '%' && i + 1 < pattern.size()) {
      char token = pattern[i + 1];
      switch (token) {
      case 'D':
        rx += "(\\d{4}[-/]\\d{2}[-/]\\d{2})";
        groupMapping_.push_back(FieldType::Date);
        i++;
        break;
      case 'T':
        rx += "(\\d{2}:\\d{2}:\\d{2}(?:\\.\\d+)?)";
        groupMapping_.push_back(FieldType::Time);
        i++;
        break;
      case 'L':
        rx += "(\\w+)";
        groupMapping_.push_back(FieldType::Level);
        i++;
        break;
      case 'M':
        rx += "(.*)";
        groupMapping_.push_back(FieldType::Message);
        i++;
        break;
      default:
        rx += pattern[i];
        break;
      }
    } else {
      char c = pattern[i];
      if (c == '[' || c == ']' || c == '(' || c == ')' || c == '{' ||
          c == '}' || c == '.' || c == '*' || c == '+' || c == '?' ||
          c == '^' || c == '$' || c == '|' || c == '\\') {
        rx += '\\';
      }
      rx += c;
    }
  }

  regexString_ = rx;
  try {
    regex_ = std::regex(rx);
  } catch (const std::regex_error &e) {
    std::cerr << "Regex Error: " << e.what() << " in " << rx << std::endl;
    regex_ = std::regex("(.*)");
    groupMapping_ = {FieldType::Message};
  }
}

static LogLevel stringToLevel(std::string_view s) {
  if (s.find("ERROR") != std::string_view::npos ||
      s.find("ERR") != std::string_view::npos)
    return LogLevel::ERROR;
  if (s.find("WARN") != std::string_view::npos)
    return LogLevel::WARNING;
  return LogLevel::INFO;
}

ParseResult PatternLogParser::parse(std::string_view line,
                                    size_t lineNumber) const {
  std::string lineStr(line);
  std::smatch matches;

  if (std::regex_match(lineStr, matches, regex_)) {
    LogEntry entry;
    entry.rawLine = line;

    std::string_view dateSv, timeSv;

    for (size_t i = 0; i < groupMapping_.size(); ++i) {
      size_t matchIdx = i + 1;
      auto start = matches.position(matchIdx);
      auto len = matches.length(matchIdx);
      std::string_view val = line.substr(start, len);

      switch (groupMapping_[i]) {
      case FieldType::Date:
        dateSv = val;
        break;
      case FieldType::Time:
        timeSv = val;
        break;
      case FieldType::Level:
        entry.level = stringToLevel(val);
        break;
      case FieldType::Message:
        entry.message = val;
        break;
      }
    }

    if (!dateSv.empty() && !timeSv.empty()) {
      std::string combined = std::string(dateSv) + " " + std::string(timeSv);
      Timestamp::parse(combined, entry.ts);
    } else if (!timeSv.empty()) {
      Timestamp::parse(timeSv, entry.ts);
    }

    return entry;
  }

  return ParseError{ParseErrorCode::BadFormat, std::string(line), lineNumber};
}

} // namespace loganalyzer
