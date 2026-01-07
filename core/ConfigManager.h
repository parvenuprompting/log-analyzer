#pragma once

#include <string>
#include <unordered_map>

namespace loganalyzer {

class ConfigManager {
public:
  static ConfigManager &instance();

  void load();
  void save();

  std::string getString(const std::string &key,
                        const std::string &defaultVal = "");
  void setString(const std::string &key, const std::string &value);

  int getInt(const std::string &key, int defaultVal = 0);
  void setInt(const std::string &key, int value);

  bool getBool(const std::string &key, bool defaultVal = false);
  void setBool(const std::string &key, bool value);

private:
  ConfigManager() = default;
  std::string getConfigPath();

  std::unordered_map<std::string, std::string> settings_;
};

} // namespace loganalyzer
