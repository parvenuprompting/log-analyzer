#pragma once

#include "ParseResult.h"
#include <string>

namespace loganalyzer {

class LogParser {
public:
  // Stateless parser: parse single line
  // Expected format: [YYYY-MM-DD HH:MM:SS] [LEVEL] message
  static ParseResult parse(const std::string &line, size_t lineNumber);

private:
  static bool parseLogLevel(const std::string &str, LogLevel &out);
};

} // namespace loganalyzer
