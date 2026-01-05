#pragma once

#include "LogLevel.h"
#include "Timestamp.h"
#include <string>

namespace loganalyzer {

struct LogEntry {
  Timestamp ts;
  LogLevel level;
  std::string message;
};

} // namespace loganalyzer
