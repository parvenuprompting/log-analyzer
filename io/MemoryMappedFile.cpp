#include "MemoryMappedFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace loganalyzer {

MemoryMappedFile::MemoryMappedFile(const std::string &filepath) 
    : fd_(-1), size_(0), data_(MAP_FAILED) {
    
    fd_ = open(filepath.c_str(), O_RDONLY);
    if (fd_ == -1) {
        return;
    }

    struct stat sb;
    if (fstat(fd_, &sb) == -1) {
        close(fd_);
        fd_ = -1;
        return;
    }
    size_ = sb.st_size;

    // Handle empty files logic: mmap with length 0 is invalid/undefined usually.
    // We treat empty file as valid but no data.
    if (size_ == 0) {
        data_ = nullptr; // Sentinel for empty
        // keep fd open? or close? 
        // usually mmap requires size > 0.
        // let's say data_ is MAP_FAILED if we can't map, but for size 0 we might want "isOpen" to be true but data null?
        // Simpler: treat empty file as "not open" or just handle it. 
        // For log analysis, empty file is valid.
        // Let's set data_ to nullptr (not MAP_FAILED) and isOpen returns true?
        // But my isOpen checks != MAP_FAILED.
        // MAP_FAILED is usually (void*)-1.
        // Let's just handle usage code to check size > 0.
        // Actually, if size is 0, mmap fails with EINVAL on some systems.
        // Let's set fd_ to -1 if size is 0 to indicate "nothing to map", 
        // OR special case:
        if (size_ == 0) {
             // Empty file, valid but no mapping.
             close(fd_);
             fd_ = -1;
             data_ = nullptr; // Distinct from MAP_FAILED
             return;
        }
    } else {
        data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (data_ == MAP_FAILED) {
            close(fd_);
            fd_ = -1;
            size_ = 0;
        }
    }
}

MemoryMappedFile::~MemoryMappedFile() {
    if (data_ != MAP_FAILED && data_ != nullptr) {
        munmap(data_, size_);
    }
    if (fd_ != -1) {
        close(fd_);
    }
}

MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&& other) noexcept 
    : fd_(other.fd_), size_(other.size_), data_(other.data_) {
    other.fd_ = -1;
    other.size_ = 0;
    other.data_ = MAP_FAILED;
}

MemoryMappedFile& MemoryMappedFile::operator=(MemoryMappedFile&& other) noexcept {
    if (this != &other) {
        if (data_ != MAP_FAILED && data_ != nullptr) {
            munmap(data_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
        
        fd_ = other.fd_;
        size_ = other.size_;
        data_ = other.data_;
        
        other.fd_ = -1;
        other.size_ = 0;
        other.data_ = MAP_FAILED;
    }
    return *this;
}

bool MemoryMappedFile::isOpen() const {
    return data_ != MAP_FAILED; 
}

const char* MemoryMappedFile::data() const {
    return static_cast<const char*>(data_);
}

size_t MemoryMappedFile::size() const {
    return size_;
}

std::string_view MemoryMappedFile::getView() const {
    if (!isOpen() || data_ == nullptr) return {};
    return std::string_view(static_cast<const char*>(data_), size_);
}

} // namespace loganalyzer
