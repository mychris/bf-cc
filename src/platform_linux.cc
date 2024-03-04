// SPDX-License-Identifier: MIT License
#include "platform.h"

#if defined(IS_LINUX)
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <memory>

std::string NativeErrorToString(int64_t native_error) {
  return std::string(std::strerror(static_cast<int>(native_error)));
}

std::variant<uint8_t *, Err> Allocate(size_t size) {
  uint8_t *mem = (uint8_t *) mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    return Err::MemAllocate(errno);
  }
  return mem;
}

void Deallocate(uint8_t *mem, size_t size) {
  munmap(mem, size);
}

Err Protect(uint8_t *mem, size_t size, unsigned int protect) {
  if (0 == mprotect(mem, size, (int) protect)) {
    return Err::Ok();
  }
  return Err::MemProtect(errno);
}

size_t Pagesize() {
  return (size_t) sysconf(_SC_PAGESIZE);
}

static void close_file(int *fp) {
  close(*fp);
}

std::variant<std::string, Err> ReadWholeFile(const std::string_view filename) {
  const std::unique_ptr<int, void (*)(int *)> fp{new int(open(filename.data(), O_RDONLY)), close_file};
  if (0 > *fp) {
    return Err::IO(errno);
  }
  const off_t fsize{lseek(*fp, 0, SEEK_END)};
  if (0 > fsize) {
    return Err::IO(errno);
  }
  if (0 > lseek(*fp, 0, SEEK_SET)) {
    return Err::IO(errno);
  }
  std::string content(static_cast<std::string::size_type>(fsize + 1), '\0');
  off_t bytes_read{0};
  while (fsize > bytes_read) {
    const ssize_t r = read(*fp, content.data() + bytes_read, (size_t) (fsize - bytes_read));
    if (0 > r && errno != EAGAIN) {
      return Err::IO(errno);
    }
    bytes_read += r;
  }
  return content;
}

extern "C" void bf_write(uint8_t *c) {
  std::putchar((int) *c);
  std::fflush(stdout);
}

extern "C" void bf_read(uint8_t *c, uint32_t mode) {
  int input = std::getchar();
  if (EOF == input) {
    switch (mode) {
    case 1: {  // KEEP
      input = static_cast<int>(*c);
    } break;
    case 2: {  // ZERO
      input = 0;
    } break;
    case 3: {  // NEG_ONE
      input = -1;
    } break;
    default: {
      Error("internal read error");
    } break;
    }
  }
  *c = static_cast<uint8_t>(input);
}

#endif
