#pragma once

#include "LogLevel.h"
#include "Timestamp.h"
#include <string_view>

namespace loganalyzer {

struct LogEntry {
  Timestamp ts;
  LogLevel level;
  std::string_view rawLine;
  std::string_view message;
};

} // namespace loganalyzer
