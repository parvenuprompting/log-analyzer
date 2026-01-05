#include "Timestamp.h"
#include <charconv>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace loganalyzer {

bool Timestamp::parse(std::string_view str, Timestamp &out) {
  // Expected format: YYYY-MM-DD HH:MM:SS (19 chars)
  if (str.length() != 19)
    return false;

  // Check format characters
  if (str[4] != '-' || str[7] != '-' || str[10] != ' ' || str[13] != ':' ||
      str[16] != ':') {
    return false;
  }

  // Parse components using from_chars for zero-copy
  auto parse_int = [](std::string_view sv, int &out) -> bool {
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return ec == std::errc{} && ptr == sv.data() + sv.size();
  };

  if (!parse_int(str.substr(0, 4), out.year))
    return false;
  if (!parse_int(str.substr(5, 2), out.month))
    return false;
  if (!parse_int(str.substr(8, 2), out.day))
    return false;
  if (!parse_int(str.substr(11, 2), out.hour))
    return false;
  if (!parse_int(str.substr(14, 2), out.minute))
    return false;
  if (!parse_int(str.substr(17, 2), out.second))
    return false;

  // Validate ranges
  if (out.month < 1 || out.month > 12)
    return false;
  if (out.hour < 0 || out.hour > 23)
    return false;
  if (out.minute < 0 || out.minute > 59)
    return false;
  if (out.second < 0 || out.second > 59)
    return false;

  // Validate calendar logic (31 feb, 30 feb, etc.)
  if (!isValidDate(out.year, out.month, out.day))
    return false;

  return true;
}

bool Timestamp::isValidDate(int year, int month, int day) {
  if (day < 1)
    return false;

  // Days per month
  static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};

  int maxDays = daysInMonth[month];

  // Check for leap year in February
  if (month == 2) {
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap)
      maxDays = 29;
  }

  return day <= maxDays;
}

bool Timestamp::operator<(const Timestamp &other) const {
  if (year != other.year)
    return year < other.year;
  if (month != other.month)
    return month < other.month;
  if (day != other.day)
    return day < other.day;
  if (hour != other.hour)
    return hour < other.hour;
  if (minute != other.minute)
    return minute < other.minute;
  return second < other.second;
}

bool Timestamp::operator>(const Timestamp &other) const {
  return other < *this;
}

bool Timestamp::operator==(const Timestamp &other) const {
  return year == other.year && month == other.month && day == other.day &&
         hour == other.hour && minute == other.minute && second == other.second;
}

bool Timestamp::operator<=(const Timestamp &other) const {
  return !(other < *this);
}

bool Timestamp::operator>=(const Timestamp &other) const {
  return !(*this < other);
}

std::string Timestamp::toString() const {
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(4) << year << '-' << std::setw(2)
      << month << '-' << std::setw(2) << day << ' ' << std::setw(2) << hour
      << ':' << std::setw(2) << minute << ':' << std::setw(2) << second;
  return oss.str();
}

} // namespace loganalyzer
