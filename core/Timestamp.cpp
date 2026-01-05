#include "Timestamp.h"
#include <cstring>
#include <iomanip>
#include <sstream>

namespace loganalyzer {

bool Timestamp::parse(const std::string &str, Timestamp &out) {
  // Expected format: YYYY-MM-DD HH:MM:SS (19 chars)
  if (str.length() != 19)
    return false;

  // Check format characters
  if (str[4] != '-' || str[7] != '-' || str[10] != ' ' || str[13] != ':' ||
      str[16] != ':') {
    return false;
  }

  // Parse components
  try {
    out.year = std::stoi(str.substr(0, 4));
    out.month = std::stoi(str.substr(5, 2));
    out.day = std::stoi(str.substr(8, 2));
    out.hour = std::stoi(str.substr(11, 2));
    out.minute = std::stoi(str.substr(14, 2));
    out.second = std::stoi(str.substr(17, 2));
  } catch (...) {
    return false;
  }

  // Validate ranges
  if (out.month < 1 || out.month > 12)
    return false;
  if (out.day < 1 || out.day > 31)
    return false;
  if (out.hour < 0 || out.hour > 23)
    return false;
  if (out.minute < 0 || out.minute > 59)
    return false;
  if (out.second < 0 || out.second > 59)
    return false;

  return true;
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
