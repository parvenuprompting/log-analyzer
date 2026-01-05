#pragma once

#include <string>

namespace loganalyzer {

enum class ParseErrorCode { BadFormat, BadTimestamp, BadLevel, MissingMessage };

struct ParseError {
  ParseErrorCode code;
  std::string rawLine;
  size_t lineNumber;
};

} // namespace loganalyzer
