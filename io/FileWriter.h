#pragma once

#include <fstream>
#include <string>

namespace loganalyzer {

class FileWriter {
public:
  explicit FileWriter(const std::string &filepath);
  ~FileWriter();

  bool write(const std::string &content);

  bool isOpen() const;
  std::string getError() const;

private:
  std::ofstream file_;
  std::string errorMessage_;
};

} // namespace loganalyzer
