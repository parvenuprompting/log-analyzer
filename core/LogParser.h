#pragma once

#include "ParseResult.h"
#include <string>
#include <string_view>

namespace loganalyzer {

class LogParser {
public:
  // Stateless parser: parse single line
  // Expected format: [YYYY-MM-DD HH:MM:SS] [LEVEL] message
  static ParseResult parse(std::string_view line, size_t lineNumber);

private:
  static bool parseLogLevel(std::string_view str, LogLevel &out);
};

} // namespace loganalyzer
