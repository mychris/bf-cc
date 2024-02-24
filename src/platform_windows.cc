// SPDX-License-Identifier: MIT License
#include "platform.h"

#if defined(IS_WINDOWS)
#include <windows.h>

#include <cstring>
#include <memory>

std::string NativeErrorToString(int64_t native_error) {
  LPSTR messageBuffer = nullptr;
  size_t size =
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL,
                     (DWORD) native_error,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR) &messageBuffer,
                     0,
                     NULL);
  std::string message(messageBuffer, size);
  LocalFree(messageBuffer);
  return message;
}

std::variant<uint8_t *, Err> Allocate(size_t size) {
  uint8_t *mem = (uint8_t *) VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (mem == nullptr) {
    return Err::MemAllocate(GetLastError());
  }
  memset(mem, 0, size);
  DWORD old_protect = 0;
  if (!VirtualProtect(mem, size, PAGE_NOACCESS, &old_protect)) {
    return Err::MemProtect(GetLastError());
  }
  return mem;
}

void Deallocate(uint8_t *mem, size_t size) {
  VirtualFree(mem, size, MEM_DECOMMIT | MEM_RELEASE);
}

Err Protect(uint8_t *mem, size_t size, unsigned int protect) {
  DWORD old_protect = 0;
  if (!VirtualProtect(mem, size, protect, &old_protect)) {
    return Err::MemProtect(GetLastError());
  }
  return Err::Ok();
}

size_t Pagesize() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return (size_t) si.dwPageSize;
}

static void close_handle(HANDLE *handle) {
  CloseHandle(*handle);
}

std::variant<std::string, Err> ReadWholeFile(const std::string_view filename) {
  const std::unique_ptr<HANDLE, void (*)(HANDLE *)> fp{
      new HANDLE(
                 CreateFile(filename.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)),
      close_handle};
  if (INVALID_HANDLE_VALUE == *fp) {
    return Err::IO(GetLastError());
  }
  const DWORD fsize{GetFileSize(*fp, NULL)};
  std::string content(static_cast<std::string::size_type>(fsize + 1), '\0');
  DWORD bytes_read{0};
  if (!ReadFile(*fp, content.data(), fsize, &bytes_read, NULL)) {
    return Err::IO(GetLastError());
  }
  return content;
}

#endif
