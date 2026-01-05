#pragma once

#include "../core/Timestamp.h"
#include <optional>
#include <string>

namespace loganalyzer {

struct AppRequest {
  std::string inputPath;
  std::optional<Timestamp> fromTimestamp;
  std::optional<Timestamp> toTimestamp;
  std::optional<std::string> keyword;
};

} // namespace loganalyzer
