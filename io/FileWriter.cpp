#include "FileWriter.h"

namespace loganalyzer {

FileWriter::FileWriter(const std::string &filepath) {
  file_.open(filepath);
  if (!file_.is_open()) {
    errorMessage_ = "Failed to open file for writing: " + filepath;
  }
}

FileWriter::~FileWriter() {
  if (file_.is_open()) {
    file_.close();
  }
}

bool FileWriter::write(const std::string &content) {
  if (!file_.is_open()) {
    return false;
  }

  file_ << content;
  return file_.good();
}

bool FileWriter::isOpen() const { return file_.is_open(); }

std::string FileWriter::getError() const { return errorMessage_; }

} // namespace loganalyzer
