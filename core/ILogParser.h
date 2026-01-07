#pragma once

#include "ParseResult.h"
#include <string_view>

namespace loganalyzer {

class ILogParser {
public:
  virtual ~ILogParser() = default;

  // Thread-safe parse method
  virtual ParseResult parse(std::string_view line, size_t lineNumber) const = 0;
};

} // namespace loganalyzer
