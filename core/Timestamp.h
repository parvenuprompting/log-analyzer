#pragma once

#include <ctime>
#include <string>
#include <string_view>

namespace loganalyzer {

struct Timestamp {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  // Parse from YYYY-MM-DD HH:MM:SS (strict, with leading zeros)
  // Now accepts string_view for zero-copy parsing
  static bool parse(std::string_view str, Timestamp &out);

  // Comparison operators
  bool operator<(const Timestamp &other) const;
  bool operator>(const Timestamp &other) const;
  bool operator==(const Timestamp &other) const;
  bool operator<=(const Timestamp &other) const;
  bool operator>=(const Timestamp &other) const;

  // Convert to string for display
  std::string toString() const;

private:
  // Validate calendar logic (days per month, leap year)
  static bool isValidDate(int year, int month, int day);
};

} // namespace loganalyzer
