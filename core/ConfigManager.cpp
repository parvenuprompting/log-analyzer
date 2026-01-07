#include "ConfigManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace loganalyzer {

ConfigManager &ConfigManager::instance() {
  static ConfigManager instance;
  return instance;
}

std::string ConfigManager::getConfigPath() {
  // Save to current directory for simplicity/portability in this project
  return "log_analyzer_config.ini";
}

void ConfigManager::load() {
  std::ifstream file(getConfigPath());
  if (!file.is_open())
    return;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == ';' || line[0] == '#')
      continue;

    auto delimiterPos = line.find('=');
    if (delimiterPos != std::string::npos) {
      std::string key = line.substr(0, delimiterPos);
      std::string value = line.substr(delimiterPos + 1);
      settings_[key] = value;
    }
  }
}

void ConfigManager::save() {
  std::ofstream file(getConfigPath());
  if (!file.is_open())
    return;

  for (const auto &[key, value] : settings_) {
    file << key << "=" << value << "\n";
  }
}

std::string ConfigManager::getString(const std::string &key,
                                     const std::string &defaultVal) {
  if (settings_.find(key) != settings_.end()) {
    return settings_[key];
  }
  return defaultVal;
}

void ConfigManager::setString(const std::string &key,
                              const std::string &value) {
  settings_[key] = value;
}

int ConfigManager::getInt(const std::string &key, int defaultVal) {
  if (settings_.find(key) != settings_.end()) {
    try {
      return std::stoi(settings_[key]);
    } catch (...) {
      return defaultVal;
    }
  }
  return defaultVal;
}

void ConfigManager::setInt(const std::string &key, int value) {
  settings_[key] = std::to_string(value);
}

bool ConfigManager::getBool(const std::string &key, bool defaultVal) {
  if (settings_.find(key) != settings_.end()) {
    return settings_[key] == "1" || settings_[key] == "true";
  }
  return defaultVal;
}

void ConfigManager::setBool(const std::string &key, bool value) {
  settings_[key] = value ? "1" : "0";
}

} // namespace loganalyzer
