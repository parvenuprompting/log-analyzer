#pragma once

#include "ILogParser.h"
#include "ParseResult.h"
#include <string>
#include <string_view>

namespace loganalyzer {

class StandardLogParser : public ILogParser {
public:
  // Stateless parser: parse single line
  // Expected format: [YYYY-MM-DD HH:MM:SS] [LEVEL] message
  ParseResult parse(std::string_view line, size_t lineNumber) const override;

private:
  static bool parseLogLevel(std::string_view str, LogLevel &out);
};

} // namespace loganalyzer
