#pragma once

#include <ctime>
#include <string>

namespace loganalyzer {

struct Timestamp {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  // Parse from YYYY-MM-DD HH:MM:SS (strict, with leading zeros)
  static bool parse(const std::string &str, Timestamp &out);

  // Comparison operators
  bool operator<(const Timestamp &other) const;
  bool operator>(const Timestamp &other) const;
  bool operator==(const Timestamp &other) const;
  bool operator<=(const Timestamp &other) const;
  bool operator>=(const Timestamp &other) const;

  // Convert to string for display
  std::string toString() const;
};

} // namespace loganalyzer
