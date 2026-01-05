#pragma once

#include "../core/Timestamp.h"
#include <optional>
#include <string>

namespace loganalyzer {

struct AnalysisContext {
  std::optional<Timestamp> fromTs;
  std::optional<Timestamp> toTs;
  std::optional<std::string> keyword;
};

} // namespace loganalyzer
