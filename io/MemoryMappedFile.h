#pragma once

#include <string>
#include <string_view>
#include <cstddef>

namespace loganalyzer {

class MemoryMappedFile {
public:
  explicit MemoryMappedFile(const std::string &filepath);
  ~MemoryMappedFile();

  // Disable copy
  MemoryMappedFile(const MemoryMappedFile&) = delete;
  MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;

  // Enable move
  MemoryMappedFile(MemoryMappedFile&& other) noexcept;
  MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept;

  bool isOpen() const;
  const char* data() const;
  size_t size() const;
  std::string_view getView() const;

private:
  int fd_;
  size_t size_;
  void* data_;
};

} // namespace loganalyzer
