#pragma once

#include <fstream>
#include <string>

namespace loganalyzer {

class FileReader {
public:
  explicit FileReader(const std::string &filepath);
  ~FileReader();

  // Streaming read: returns true if line read successfully
  bool nextLine(std::string &line, size_t &lineNumber);

  bool isOpen() const;
  std::string getError() const;

  // Close file explicitly (usually auto-closed in destructor)
  void close();

private:
  std::ifstream file_;
  size_t currentLine_;
  std::string errorMessage_;
};

} // namespace loganalyzer
