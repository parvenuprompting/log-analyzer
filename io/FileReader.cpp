#include "FileReader.h"

namespace loganalyzer {

FileReader::FileReader(const std::string &filepath) : currentLine_(0) {
  file_.open(filepath);
  if (!file_.is_open()) {
    errorMessage_ = "Failed to open file: " + filepath;
  }
}

FileReader::~FileReader() {
  if (file_.is_open()) {
    file_.close();
  }
}

bool FileReader::nextLine(std::string &line, size_t &lineNumber) {
  if (!file_.is_open()) {
    return false;
  }

  if (std::getline(file_, line)) {
    ++currentLine_;
    lineNumber = currentLine_;
    return true;
  }

  return false;
}

bool FileReader::isOpen() const { return file_.is_open(); }

std::string FileReader::getError() const { return errorMessage_; }

} // namespace loganalyzer
