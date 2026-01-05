#pragma once

#include "LogEntry.h"
#include "ParseError.h"
#include <variant>

namespace loganalyzer {

using ParseResult = std::variant<LogEntry, ParseError>;

} // namespace loganalyzer
